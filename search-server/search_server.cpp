#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const std::string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)) {}

SearchServer::SearchServer(const std::string_view& stop_words_view) : SearchServer(SplitIntoWords(string(stop_words_view))) {}

void SearchServer::AddDocument(int document_id, const std::string_view& document, DocumentStatus status, const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }
    
    global_storage_.emplace_back(std::move(document));
    const auto words = SplitIntoWordsNoStop(global_storage_.back());

    const double inv_word_count = 1.0 / words.size();
    for (const string_view& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_to_word_freqs_[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status, global_storage_.back()});
    document_ids_.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(const string_view& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const string_view& raw_query, int document_id) const {
    return MatchDocument(execution::seq, raw_query, document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const execution::sequenced_policy&, const string_view& raw_query, int document_id) const {
    static const auto query = ParseQuery(raw_query, true);

    vector<string_view> matched_words;

    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            return {matched_words, documents_.at(document_id).status};
        }
    }

    return {matched_words, documents_.at(document_id).status};
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const execution::parallel_policy&, const string_view& raw_query, int document_id) const {
    static const auto query = ParseQuery(raw_query, false);

    bool flag = any_of(execution::par, query.minus_words.begin(), query.minus_words.end(), [this, document_id](const auto& entry) {
        return word_to_document_freqs_.count(entry) && word_to_document_freqs_.at(entry).count(document_id);
    });

    vector<string_view> matched_words;

    if (flag) {
        return {matched_words, documents_.at(document_id).status};
    }

    matched_words.resize(query.plus_words.size());

    auto it = copy_if(execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(), [this, document_id](const auto& entry) {
        return word_to_document_freqs_.count(entry) && word_to_document_freqs_.at(entry).count(document_id);
    });

    matched_words.resize(std::distance(matched_words.begin(), it));

    sort(execution::par, matched_words.begin(), matched_words.end());
    matched_words.erase(unique(execution::par, matched_words.begin(), matched_words.end()), matched_words.end());

    return {matched_words, documents_.at(document_id).status};
}


std::set<int>::const_iterator SearchServer::begin() const {
    return document_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const {
    return document_ids_.end();
}

const map<string_view, double>& SearchServer::GetWordFrequencies(const int document_id) const {
    if (document_to_word_freqs_.count(document_id)) {
        return document_to_word_freqs_.at(document_id);
    }
    static std::map<std::string_view, double> empty_map;
    return empty_map;
}

void SearchServer::RemoveDocument(const execution::sequenced_policy&, const int document_id) {
    RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const execution::parallel_policy&, const int document_id) {
    vector<string_view> words(document_to_word_freqs_[document_id].size());

    map<string_view, double>& word_to_freqs = document_to_word_freqs_.at(document_id);

    transform(execution::par, word_to_freqs.begin(), word_to_freqs.end(), words.begin(), [](const auto& entry) {
        return entry.first;
    });

    for_each(execution::par, words.begin(), words.end(), [this, document_id](const auto& entry_word) {
        if (word_to_document_freqs_.count(entry_word)) {
            word_to_document_freqs_.at(entry_word).erase(document_id);
        }
    });

    documents_.erase(document_id);

    for (auto it = document_ids_.begin(); it != document_ids_.end(); ++it) {
        if ((*it) == document_id) {
            document_ids_.erase(it);
            break;
        }
    }

    document_to_word_freqs_.erase(document_id);
}

void SearchServer::RemoveDocument(const int document_id) {
    for (auto& [word, freqs] : document_to_word_freqs_.at(document_id)) {
        word_to_document_freqs_.at(word).erase(document_id);
    }
    
    documents_.erase(document_id);

    for (auto it = document_ids_.begin(); it != document_ids_.end(); ++it) {
        if ((*it) == document_id) {
            document_ids_.erase(it);
            break;
        }
    }

    document_to_word_freqs_.erase(document_id);
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(const string_view& text) const {
    vector<string_view> words;
    for (const string_view& word : SplitIntoWords(text)) {
        if (!IsValidWord(string(word))) {
            throw invalid_argument("Word "s + string(word) + " is invalid"s);
        }
        if (!IsStopWord(string(word))) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
    if (text.empty()) {
        throw invalid_argument("Query word is empty"s);
    }
    string word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw invalid_argument("Query word "s + text + " is invalid");
    }

    return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const std::string_view& text, bool sort_flag) const {
    Query result;
    for (const string_view& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(string(word));
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            } else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }

    if (sort_flag) {
        sort(result.minus_words.begin(), result.minus_words.end());
        result.minus_words.erase(unique(result.minus_words.begin(), result.minus_words.end()), result.minus_words.end());

        sort(result.plus_words.begin(), result.plus_words.end());
        result.plus_words.erase(unique(result.plus_words.begin(), result.plus_words.end()), result.plus_words.end());
    }

    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
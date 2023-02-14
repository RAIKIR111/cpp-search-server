#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        if (document == "")
            return;
        const vector<string> words = SplitIntoWordsNoStop(document);
        for (string word : words) {
            word_and_ids_[word].insert(document_id);
            repetitions_[word][document_id] = FindTf(count(words.begin(), words.end(), word), words.size());
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        Query query = ParseQuery(raw_query);

        vector<Document> matched_documents = FindAllDocuments(query.plus_words, query.minus_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    void SetDocumentCount(const int& number) {
        document_count_ = number;
    }

private:
    struct Query {
        set<string> minus_words;
        set<string> plus_words;
    };

    int document_count_ = 0;

    map<string, set<int>> word_and_ids_;

    set<string> stop_words_;

    map<string, map<int, double>> repetitions_; // слово - id - количество повторений слова (TF)

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        Query obj;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            ParseQueryWord(word, obj);
        }
        return obj;
    }

    void ParseQueryWord(const string& word, Query& obj) const {
        string temp_str;
        if (word[0] != '-') {
            obj.plus_words.insert(word);
        }
        else
        {
            temp_str = word;
            temp_str.erase(0, 1);
            obj.minus_words.insert(temp_str);
        }
    }

    double FindIdf(const int& document_count, const int& id_count) const {
        return log(double(document_count) / double(id_count));
    }

    double FindTf(const int& repetition_count, const int& document_size) const {
        return (double(repetition_count) / double(document_size));
    }

    vector<Document> FindAllDocuments(const set<string>& plus_words, const set<string>& minus_words) const {
        map<int, int> id_and_plus_words;
        for (const string& plus_word : plus_words) {
            if (word_and_ids_.count(plus_word)) {
                for (int id : word_and_ids_.at(plus_word)) {
                    id_and_plus_words[id] += 1;
                }
            }
        }

        for (const string& minus_word : minus_words) {
            if (word_and_ids_.count(minus_word)) {
                for (int id : word_and_ids_.at(minus_word)) {
                    id_and_plus_words.erase(id);
                }
            }
        }

        //id_and_plus_words
        //word_and_ids_
        //doc_lengths_
        //repetitions_

        vector<Document> main_result;
        Document obj;
        double idf = 0.0;
        double tf = 0.0;
        double temp_result = 0.0;
        bool flag = false;
        for (int id = 0; id < document_count_; ++id) {
            flag = false;
            for (const string& plus_word : plus_words) {
                if (word_and_ids_.count(plus_word))
                    if (word_and_ids_.at(plus_word).count(id)) {
                        idf = FindIdf(document_count_, word_and_ids_.at(plus_word).size());
                        tf = repetitions_.at(plus_word).at(id);
                        temp_result += idf * tf;
                        flag = true;
                    }
            }
            if (flag == true) {
                obj.id = id;
                obj.relevance = temp_result;
                main_result.push_back(obj);
                temp_result = 0.0;
            }
        }

        return main_result;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    search_server.SetDocumentCount(document_count);

    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const Document item : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << item.id << ", " << "relevance = "s << item.relevance << " }"s << endl;
    }
}
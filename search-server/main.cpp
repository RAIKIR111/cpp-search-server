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
        for (int count = 0; count < words.size(); ++count) {
            word_and_ids_[words[count]].insert(document_id);
            repetitions_[{document_id, words[count]}] += 1;
        }

        /*for (auto& item : word_and_ids_) {
            cout << item.first << ": ";
            for (int id : item.second) {
                cout << id << ' ';
            }
            cout << endl;
        }*/

    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const set<string> query_words = ParseQuery(raw_query);
        const set<string> minus_words = SetMinusWords(query_words);
        const set<string> plus_words = SetPlusWords(query_words);

        vector<Document> matched_documents = FindAllDocuments(plus_words, minus_words);

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
    int document_count_ = 0;

    map<string, set<int>> word_and_ids_;

    set<string> stop_words_;

    map<pair<int, string>, int> repetitions_; // id, слово - количество повторений слова в документе

    set<string> SetMinusWords(const set<string>& words) const {
        set<string> minus_words;
        string str;
        for (string item : words) {
            if (item[0] == '-') {
                str = item;
                str.erase(0, 1);
                minus_words.insert(str);
            }
        }
        return minus_words;
    }

    set<string> SetPlusWords(const set<string>& words) const {
        set<string> plus_words;
        for (string item : words) {
            if (item[0] != '-') {
                plus_words.insert(item);
            }
        }
        return plus_words;
    }

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

    set<string> ParseQuery(const string& text) const {
        set<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            query_words.insert(word);
        }
        return query_words;
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


        /*for (auto& item : id_and_plus_words) {
            cout << item.first << ' ' << item.second << endl;
        }*/


        //id_and_plus_words
        //word_and_ids_
        //doc_lengths_
        //repetitions_

        vector<Document> main_result;
        Document obj;
        double var1 = 0.0;
        double var2 = 0.0;
        double temp_result = 0.0;
        int doc_length = 0;


        /*for (auto& item : word_and_ids_) {
            cout << item.first << ": ";
            for (int id : item.second) {
                cout << id << ' ';
            }
            cout << endl;
        }*/

        bool flag = false;
        for (int id = 0; id < document_count_; ++id) {
            flag = false;
            for (const auto& item : word_and_ids_) {
                if (item.second.count(id))
                    doc_length += 1;
            }
            for (const string& plus_word : plus_words) {
                if (word_and_ids_.count(plus_word))
                    if (word_and_ids_.at(plus_word).count(id)) {
                        var1 = log(double(document_count_) / double(word_and_ids_.at(plus_word).size()));
                        var2 = double(repetitions_.at({ id, plus_word })) / double(doc_length);
                        temp_result += var1 * var2;
                        flag = true;
                    }
            }
            if (flag == true) {
                obj.id = id;
                obj.relevance = temp_result;
                main_result.push_back(obj);
                temp_result = 0.0;
            }
            doc_length = 0;
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
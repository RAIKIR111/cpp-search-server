#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    std::vector<int> need_remove;

    std::set<std::set<std::string>> words_in_documents;

    for (const int document_id : search_server) {
        const std::map<std::string, double> word_to_freq = search_server.GetWordFrequencies(document_id);

        std::set<std::string> temp_words_set;
        for (const auto [word, freq] : word_to_freq) {
            temp_words_set.insert(word);
        }

        if (words_in_documents.count(temp_words_set)) {
            std::cout << "Found duplicate document id "s << document_id << std::endl;
            need_remove.push_back(document_id);
        }
        else {
            words_in_documents.insert(temp_words_set);
        }
    }

    for (auto id : need_remove) {
        search_server.RemoveDocument(id);
    }
}
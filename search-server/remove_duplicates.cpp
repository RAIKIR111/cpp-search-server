#include "remove_duplicates.h"

bool AreSameWords(std::map<std::string, double> first, std::map<std::string, double> second) {
    if (first.size() != second.size()) return false;

    for (auto first_it = first.begin(); first_it != first.end(); ++first_it) {
        if (!second.count((*first_it).first)) return false;
    }

    return true;
}

void RemoveDuplicates(SearchServer& search_server) {
    std::vector<int> need_remove;

    for (const int document_id : search_server) {
        if (document_id == (*search_server.begin())) continue;

        for (int id = document_id - 1; id >= (*search_server.begin()); --id) {
            if (AreSameWords(search_server.GetWordFrequencies(document_id), search_server.GetWordFrequencies(id))) {
                if (count(need_remove.begin(), need_remove.end(), document_id)) {
                    continue;
                }
                std::cout << "Found duplicate document id "s << document_id << std::endl;
                need_remove.push_back(document_id);
                if (id == (*search_server.begin())) {
                    break;
                }
            }
        }
    }

    for (auto id : need_remove) {
        search_server.RemoveDocument(id);
    }
}
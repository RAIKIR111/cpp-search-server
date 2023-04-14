#pragma once

#include <vector>
#include <queue>
#include <iostream>

#include "search_server.h"
#include "document.h"

template <typename Iterator>
std::ostream& std::operator<<(ostream& out, pair<Iterator, Iterator> documents_on_page) {
    if (documents_on_page.first == documents_on_page.second) {
        out << "{ document_id = "s << documents_on_page.first->id << ", relevance = "s << documents_on_page.first->relevance << ", rating = "s << documents_on_page.first->rating << " }"s;
        return out;
    }

    bool flag = false;
    while (documents_on_page.first != documents_on_page.second) {
        if (flag == false) flag = true;
        else ++documents_on_page.first;

        out << "{ document_id = "s << documents_on_page.first->id << ", relevance = "s << documents_on_page.first->relevance << ", rating = "s << documents_on_page.first->rating << " }"s;
    }

    return out;
}

class RequestQueue {
public:
    explicit RequestQueue(SearchServer& search_server);
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        // напишите реализацию
        request_count_++;
        if (request_count_ < min_in_day_) {
            requests_.pop_front();
            request_count_--;
        }
        requests_.push_back({search_server_->FindTopDocuments(raw_query, document_predicate)});

        return search_server_->FindTopDocuments(raw_query, document_predicate);
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        // напишите реализацию
        request_count_++;
        if (request_count_ < min_in_day_) {
            requests_.pop_front();
            request_count_--;
        }
        requests_.push_back({search_server_->FindTopDocuments(raw_query, status)});

        return search_server_->FindTopDocuments(raw_query, status);
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query) {
        // напишите реализацию
        request_count_++;
        if (request_count_ > min_in_day_) {
            requests_.pop_front();
            request_count_--;
        }
        requests_.push_back({search_server_->FindTopDocuments(raw_query)});

        return search_server_->FindTopDocuments(raw_query);
    }
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        // определите, что должно быть в структуре
        std::vector<Document> top_docs;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    // возможно, здесь вам понадобится что-то ещё
    int request_count_ = 0;
    SearchServer* search_server_;
};
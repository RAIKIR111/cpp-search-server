#include "request_queue.h"

using namespace std;

RequestQueue::RequestQueue(SearchServer& search_server) : search_server_(&search_server) {
    // напишите реализацию
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    // напишите реализацию
    request_count_++;
    if (request_count_ < min_in_day_) {
        requests_.pop_front();
        request_count_--;
    }
    requests_.push_back({search_server_->FindTopDocuments(raw_query, status)});

    return search_server_->FindTopDocuments(raw_query, status);
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    // напишите реализацию
    request_count_++;
    if (request_count_ > min_in_day_) {
        requests_.pop_front();
        request_count_--;
    }
    requests_.push_back({search_server_->FindTopDocuments(raw_query)});

    return search_server_->FindTopDocuments(raw_query);
}

int RequestQueue::GetNoResultRequests() const {
    // напишите реализацию
    int dst_count = 0;
    auto requests_copy = requests_;
    while (!requests_copy.empty()) {
        if (requests_copy.back().top_docs.size() == 0) {
            dst_count += 1;
        }
        requests_copy.pop_back();
    }
    return dst_count;
}
#include "request_queue.h"

using namespace std;

RequestQueue::RequestQueue(SearchServer& search_server) : search_server_(&search_server) {
    // напишите реализацию
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
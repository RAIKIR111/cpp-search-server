#pragma once

#include <map>

template <typename Iterator>
class Paginator {
public:
    explicit Paginator(const Iterator begin_it, const Iterator end_it, size_t page_size) {
        auto documents_number = distance(begin_it, end_it);
        if (documents_number == 0) return;

        auto begin_it_copy = begin_it;
        auto end_it_copy = end_it - 1;

        if (page_size == 1) {
            while (begin_it_copy != end_it_copy) {
                page_to_documents_.push_back(make_pair(begin_it_copy, begin_it_copy));
                ++begin_it_copy;
            }
            page_to_documents_.push_back(make_pair(begin_it_copy, begin_it_copy));
        }
        else {
            while (documents_number > page_size) {
                auto documents_range = begin_it_copy + page_size - 1;
                page_to_documents_.push_back(make_pair(begin_it_copy, documents_range));
                begin_it_copy += page_size;
                documents_number -= page_size;
            }
            page_to_documents_.push_back(make_pair(begin_it_copy, end_it_copy));
        }
    }

    auto begin() const {
        return page_to_documents_.begin();
    }

    auto end() const {
        return page_to_documents_.end();
    }

    auto size() const {
        return page_to_documents_.size();
    }

private:
    std::vector<std::pair<Iterator, Iterator>> page_to_documents_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
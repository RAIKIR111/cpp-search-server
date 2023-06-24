#pragma once

#include <mutex>

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
        Access(const Key& key, typename ConcurrentMap::Bucket& bucket) : guard(bucket.mtx), ref_to_value(bucket.data[key]) {}

        void operator+=(const Value& value) {
            ref_to_value += value;
        }

        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count) {}

    Access operator[](const Key& key) {
        uint64_t bucket_index = static_cast<uint64_t>(key) % buckets_.size();
        auto& bucket = buckets_[bucket_index];
        return {key, bucket};
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        std::map<Key, Value> dst;
        for (Bucket& item : buckets_) {
            std::lock_guard<std::mutex> guard(item.mtx);
            dst.merge(item.data);
        }
        return dst;
    }

    void Erase(const Key& key) {
        uint64_t bucket_index = static_cast<uint64_t>(key) % buckets_.size();
        auto& bucket = buckets_[bucket_index];
        bucket.data.erase(key);
    }



private:
    struct Bucket {
        std::mutex mtx;
        std::map<Key, Value> data;
    };

    std::vector<Bucket> buckets_;
};
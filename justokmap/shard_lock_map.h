#pragma once
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <shared_mutex>
#include "sparsehash/dense_hash_map"
#include "../utils/logs.h"

#define NUM_SHARD 16
#define EMB_LEN 16

using google::dense_hash_map;
using std::vector;

typedef double *embedding_t;

class google_concurrent_hash_map
{
private:
    dense_hash_map<int64_t, embedding_t> map;
    size_t _true_size;
    std::shared_mutex a_mutex;

public:
    google_concurrent_hash_map()
    {
        map.set_empty_key(0);
        _true_size = 0;
    }
    void set(const int64_t &key, embedding_t value)
    {
        std::lock_guard<std::shared_mutex> guard(a_mutex);
        // LOGINFO << "_true_size:" << _true_size << std::endl;
        // auto iter = map.find(key);
        embedding_t cur_value = map[key];
        // LOGINFO << key << "----" << value << std::endl;
        if (value != nullptr && cur_value == nullptr)
        {
            // LOGINFO << value << "且没找到" << std::endl;
            _true_size += 1;
        }
        else if (value == nullptr && cur_value != nullptr)
        {
            // LOGINFO << value << "找到了" << std::endl;
            _true_size -= 1;
        }
        map[key] = value;
    }
    embedding_t get(const int64_t &key)
    {
        std::shared_lock<std::shared_mutex> lock(a_mutex);
        embedding_t value = map[key];
        return value;
    }
    void erase(const int64_t &key)
    {
        std::lock_guard<std::shared_mutex> guard(a_mutex);
        _true_size -= 1;
        map.erase(key);
    }
    size_t true_size()
    {
        std::lock_guard<std::shared_mutex> guard(a_mutex);
        return _true_size;
    }
};

class shard_lock_map
{
private:
    const size_t a_mask;
    std::vector<google_concurrent_hash_map> a_shards;
    google_concurrent_hash_map &get_shard(const int64_t &key)
    {
        std::hash<int64_t> hash_func;
        auto h = hash_func(key);
        return a_shards[h & a_mask];
    }

public:
    shard_lock_map(size_t num_shard = 16) : a_mask(num_shard - 1), a_shards(std::vector<google_concurrent_hash_map>(num_shard)) {}
    void set(const int64_t &key, embedding_t value)
    {
        get_shard(key).set(key, value);
    }
    embedding_t get(const int64_t &key)
    {
        return get_shard(key).get(key);
    }
    void erase(const int64_t &key)
    {
        get_shard(key).erase(key);
    }
    size_t true_size() // 不要求很强的一致性
    {
        size_t sum_size = 0;
        for (google_concurrent_hash_map &m : a_shards)
        {
            sum_size += m.true_size();
        }
        return sum_size;
    }
};

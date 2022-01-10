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
    std::shared_mutex a_mutex;

public:
    google_concurrent_hash_map()
    {
        map.set_empty_key(0);
    }
    void set(const int64_t &key, embedding_t value)
    {
        std::lock_guard<std::shared_mutex> guard(a_mutex);
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
        map.erase(key);
    }
};

class shard_lock_map
{
private:
    const size_t a_mask;
    size_t _true_size;
    std::vector<google_concurrent_hash_map> a_shards;
    google_concurrent_hash_map &get_shard(const int64_t &key)
    {
        std::hash<int64_t> hash_func;
        auto h = hash_func(key);
        return a_shards[h & a_mask];
    }

public:
    shard_lock_map(size_t num_shard = 16) : a_mask(num_shard - 1), _true_size(0), a_shards(std::vector<google_concurrent_hash_map>(num_shard)) {}
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
    size_t true_size()
    {
        return _true_size;
    }
    void increase()
    {
        ++_true_size;
    }
    void decrease()
    {
        --_true_size;
    }
};

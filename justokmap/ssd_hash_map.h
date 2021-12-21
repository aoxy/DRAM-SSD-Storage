#pragma once
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <string>
#include <shared_mutex>
#include "sparsehash/dense_hash_map"

#define NUM_SHARD 16
#define EMB_LEN 16

using google::dense_hash_map;
using std::vector;

class ssd_google_concurrent_hash_map
{
private:
    dense_hash_map<int64_t, size_t> map;
    std::shared_mutex a_mutex;

public:
    ssd_google_concurrent_hash_map()
    {
        map.set_empty_key(0);
    }
    void set(const int64_t &key, size_t value)
    {
        std::lock_guard<std::shared_mutex> guard(a_mutex);
        map[key] = value;
    }
    size_t get(int64_t key)
    {
        std::shared_lock<std::shared_mutex> lock(a_mutex);
        size_t value = map[key];
        return value;
    }
    void erase(int64_t key)
    {
        std::lock_guard<std::shared_mutex> guard(a_mutex);
        map.erase(key);
    }
};

class ssd_hash_map
{
private:
    const size_t a_mask;
    std::vector<ssd_google_concurrent_hash_map> a_shards;
    ssd_google_concurrent_hash_map &get_shard(const int64_t &key)
    {
        return a_shards[shard_idx(key)];
    }

public:
    ssd_hash_map(size_t num_shard = 16) : a_mask(num_shard - 1), a_shards(std::vector<ssd_google_concurrent_hash_map>(num_shard)) {}
    void set(const int64_t &key, size_t value)
    {
        get_shard(key).set(key, value);
    }
    size_t get(int64_t key)
    {
        return get_shard(key).get(key);
    }
    void erase(int64_t key)
    {
        get_shard(key).erase(key);
    }
    std::string filepath(int64_t key)
    {
        return std::string("storage/emb") + std::to_string(shard_idx(key)) + std::string(".hdss");
    }
    size_t shard_idx(const int64_t &key) const
    {
        std::hash<int64_t> hash_func;
        auto h = hash_func(key);
        return h & a_mask;
    }
};

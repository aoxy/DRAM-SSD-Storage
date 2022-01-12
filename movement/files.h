#pragma once
#include <vector>
#include <fstream>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"

class FilePool
{
private:
    // std::vector<std::ofstream> ofs;
    // std::vector<std::ifstream> ifs;
    std::vector<std::fstream> fs;

    size_t num_shard;

    size_t shard_idx(const int64_t &key) const
    {
        std::hash<int64_t> hash_func;
        auto h = hash_func(key);
        return h & (num_shard - 1);
    }

public:
    FilePool(ssd_hash_map &smap)
    {
        num_shard = smap.get_num_shard();
        for (size_t i = 0; i < num_shard; ++i)
        {
            // ofs.push_back(std::ofstream(smap.savepath(i), std::ios::app | std::ios::binary));
            // ifs.push_back(std::ifstream(smap.filepath(i), std::ios::in | std::ios::binary));
            fs.push_back(std::fstream(smap.filepath(i), std::ios::app | std::ios::in | std::ios::out | std::ios::binary));
        }
    }
    ~FilePool()
    {
        for (size_t i = 0; i < num_shard; ++i)
        {
            // ofs[i].close();
            // ifs[i].close();
            // fs[i].close();
            close();
        }
    }

    // std::ofstream &write_s(const int64_t &key)
    // {
    //     return std::ref(ofs[shard_idx(key)]);
    // }

    // std::ifstream &read_s(const int64_t &key)
    // {
    //     return std::ref(ifs[shard_idx(key)]);
    // }

    std::fstream &rw_s(const int64_t &key)
    {
        return std::ref(fs[shard_idx(key)]);
    }

    void close()
    {
        for (size_t i = 0; i < num_shard; ++i)
        {
            fs[i].close();
        }
    }
};
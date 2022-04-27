
#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/files.h"
#include "../ranking/cache.h"
#include "../ranking/arc.h"
#include "../ranking/arf.h"

class DataLoader
{
private:
    std::vector<int64_t> ids;
    size_t offset;

public:
    DataLoader(std::string filepath, size_t feature_id = 0)
    {
        std::ifstream fp(filepath);
        if (!fp)
        {
            LOGINFO << "open " << filepath << " fail." << std::endl
                    << std::flush;
            exit(1);
        }
        std::string line;
        std::getline(fp, line); //跳过列名，第一行不做处理

        while (std::getline(fp, line))
        {
            std::string number;
            std::istringstream readstr(line);
            for (size_t j = 0; j <= feature_id; ++j)
            {
                std::getline(readstr, number, ',');
            }
            ids.push_back(std::atoi(number.c_str()));
        }
        offset = 0;
    }
    void init() { offset = 0; }
    void sample(int64_t *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; offset < ids.size() && i < batch_size; ++offset, ++i)
        {
            batch_ids[i] = ids[offset];
        }
    }
    size_t size() { return ids.size(); }
};

void init_ssd_map(ssd_hash_map &smap);

void compaction(shard_lock_map &dmap, ssd_hash_map &smap, FilePool &old_fp);

class Config
{
public:
    std::string feature;
    size_t feature_id;
    BatchCache<int64_t> *cache;
    size_t dsize;

    Config(int argc, char *argv[])
    {
        std::string cache_policy;
        int max_emb_num_perc;
        if (argc < 4)
        {
            std::cout << "missing parameters" << std::endl;
            std::cout << "usage: ./main <ad/user> <1~100> <lru/lfu>" << std::endl;
            exit(0);
        }
        feature = std::string(argv[1]);
        if (feature == "user")
        {
            dsize = 1141729;
            feature_id = 0;
        }
        else if (feature == "ad")
        {
            dsize = 846811;
            feature_id = 1;
        }
        else
        {
            std::cout << "invalid: feature must be `ad` or `user`" << std::endl;
            exit(0);
        }
        max_emb_num_perc = std::stoi(argv[2]);
        if (max_emb_num_perc < 1)
        {
            std::cout << "invalid: cache size must be `1` to `100`" << std::endl;
        }
        else if (max_emb_num_perc > 100)
        {
            max_emb_num_perc = 100;
        }
        cache_policy = std::string(argv[3]);
        if (cache_policy == "lru")
        {
            cache = new LRUCache<int64_t>(dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "lfu")
        {
            cache = new LFUCache<int64_t>(dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "fifo")
        {
            cache = new FIFOCache<int64_t>(dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "arc")
        {
            cache = new ARCCache<int64_t>(dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "arf")
        {
            cache = new ARFCache<int64_t>(dsize * max_emb_num_perc / 100);
        }
        else
        {
            std::cout << "invalid: feature must be `lru` or `lfu`" << std::endl;
            exit(0);
        }
    }
};
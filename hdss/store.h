
#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/files.h"
#include "../ranking/cache.h"
#include "../ranking/single_cache.h"

class DataLoader
{
private:
    std::vector<int64_t> ids;
    size_t offset;

public:
    DataLoader(std::string filepath, size_t feature_id = 0, size_t count = 0)
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
            if (count > 0 && ids.size() >= count)
            {
                break;
            }
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
    int64_t at(int i) { return ids[i]; }
    size_t count_dup_size()
    {
        std::unordered_set<int64_t> set;
        for (const int64_t &id : ids)
        {
            set.insert(id);
        }
        return set.size();
    }
};

void init_ssd_map(ssd_hash_map &smap);

void compaction(shard_lock_map &dmap, ssd_hash_map &smap, FilePool &old_fp);

class Config
{
public:
    std::string feature;
    size_t feature_id;
    ReplacementCache<int64_t> *cache;
    size_t dsize;

    Config(int argc, char *argv[])
    {
        std::string cache_policy;
        int max_emb_num_perc;
        if (argc < 4)
        {
            std::cout << "missing parameters" << std::endl;
            std::cout << "usage: ./main <ad/user> <1~100> <lru/lfu/fifo/arc/arf>" << std::endl;
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
            cache = new ReplacementLRUCache<int64_t>(-1, dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "lfu")
        {
            cache = new ReplacementLFUCache<int64_t>(-1, dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "fifo")
        {
            cache = new ReplacementFIFOCache<int64_t>(-1, dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "arc")
        {
            cache = new ReplacementARCCache<int64_t>(-1, dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "arf")
        {
            cache = new ReplacementARFCache<int64_t>(-1, dsize * max_emb_num_perc / 100);
        }
        else
        {
            std::cout << "invalid: feature must be `lru` or `lfu`" << std::endl;
            exit(0);
        }
    }
};

class SingleConfig
{
public:
    std::string feature;
    std::string filepath;
    size_t feature_id;
    SingleCache<int64_t, bool> *cache;
    size_t dsize;
    bool verbose;
    DataLoader *dl;

    SingleConfig(int argc, char *argv[])
    {
        std::string cache_policy;
        double max_emb_num_perc;
        if (argc < 4)
        {
            std::cout << "missing parameters" << std::endl;
            std::cout << "usage: ./main <ad/user> <1~100> <lru/lfu/fifo/arc/arf>" << std::endl;
            exit(0);
        }
        if (argc > 4 && std::string(argv[4]) == "verbose")
        {
            verbose = true;
        }
        else
        {
            verbose = false;
        }
        std::cout << std::string(32, '=') << std::endl;
        feature = std::string(argv[1]);
        std::cout << "feature = " << feature << std::endl;
        if (feature == "user")
        {
            filepath = "dataset/taobao/shuffled_sample.csv";
            feature_id = 0;
        }
        else if (feature == "ad")
        {
            filepath = "dataset/taobao/shuffled_sample.csv";
            feature_id = 1;
        }
        else if (feature == "cate_id")
        {
            filepath = "dataset/taobao/shuffled_ad_feature.csv";
            feature_id = 1;
        }
        else if (feature == "campaign_id")
        {
            filepath = "dataset/taobao/shuffled_ad_feature.csv";
            feature_id = 2;
        }
        else if (feature == "customer")
        {
            filepath = "dataset/taobao/shuffled_ad_feature.csv";
            feature_id = 3;
        }
        else if (feature == "brand")
        {
            filepath = "dataset/taobao/shuffled_ad_feature.csv";
            feature_id = 4;
        }
        else if (feature == "price")
        {
            filepath = "dataset/taobao/shuffled_ad_feature.csv";
            feature_id = 5;
        }
        else if (feature == "random1")
        {
            filepath = "dataset/taobao/random_data.csv";
            feature_id = 0;
        }
        else if (feature == "random2")
        {
            filepath = "dataset/taobao/random_data.csv";
            feature_id = 1;
        }
        else if (feature == "random3")
        {
            filepath = "dataset/taobao/random_data.csv";
            feature_id = 2;
        }
        else
        {
            std::cout << "invalid: feature" << std::endl;
            exit(0);
        }
        dl = new DataLoader(filepath, feature_id);
        dsize = dl->count_dup_size();
        max_emb_num_perc = std::stod(argv[2]);
        std::cout << "cache size = " << max_emb_num_perc << std::endl;
        if (max_emb_num_perc <= 0)
        {
            std::cout << "invalid: cache size must be (0, 100]" << std::endl;
            max_emb_num_perc = 1;
        }
        else if (max_emb_num_perc > 100)
        {
            max_emb_num_perc = 100;
        }
        cache_policy = std::string(argv[3]);
        if (cache_policy != "alfu")
        {
            std::cout << "cache policy = " << cache_policy << std::endl;
        }

        if (cache_policy == "lru")
        {
            cache = new SingleLRUCache<int64_t, bool>(std::make_pair(-1, false), dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "lfu")
        {
            cache = new SingleLFUCache<int64_t, bool>(std::make_pair(-1, false), dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "fifo")
        {
            cache = new SingleFIFOCache<int64_t, bool>(std::make_pair(-1, false), dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "arc")
        {
            cache = new SingleARCCache<int64_t, bool>(std::make_pair(-1, false), dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "arf")
        {
            cache = new SingleARFCache<int64_t, bool>(std::make_pair(-1, false), dsize * max_emb_num_perc / 100);
        }
        else if (cache_policy == "alfu")
        {
            if (argc < 5)
            {
                std::cout << "missing parameters: max_aging_times" << std::endl;
                exit(0);
            }
            size_t max_aging_times = std::stod(argv[4]);
            std::cout << "cache policy = " << cache_policy << max_aging_times << std::endl;
            cache = new SingleAgingLFUCache<int64_t, bool>(std::make_pair(-1, false), dsize * max_emb_num_perc / 100, dl->size() / (max_aging_times + 1));
        }
        else if (cache_policy == "dalfu")
        {
            cache = new SingleDynamicAgingLFUCache<int64_t, bool>(std::make_pair(-1, false), dsize * max_emb_num_perc / 100);
        }
        else
        {
            std::cout << "invalid: cache policy must be `lru`, `lfu`, `fifo`, `arc`, `arf`" << std::endl;
            exit(0);
        }
    }
};
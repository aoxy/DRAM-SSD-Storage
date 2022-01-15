#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include "cache.h"
#include "../utils/dataloder.h"

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        std::cout << "usage: ./cache_test <ad/user> <1~100> <lru/lfu>" << std::endl;
        std::cout << "example: ./cache_test ad 70 lfu" << std::endl;
        return 0;
    }
    std::string feature_name(argv[1]);
    int max_emb_num_perc = atoi(argv[2]);
    size_t dsize;
    std::string data_file;
    if (feature_name == "ad")
    {
        data_file = "../dataset/taobao/shuffled_adgroupid.csv";
        dsize = 846811;
    }
    else if (feature_name == "user")
    {
        data_file = "../dataset/taobao/shuffled_userid.csv";
        dsize = 1141729;
    }
    else
    {
        std::cout << "invalid: feature must be `ad` or `user`" << std::endl;
        return 0;
    }
    const size_t max_emb_num = size_t(max_emb_num_perc * dsize / 100);
    std::string cache_policy(argv[3]);
    BatchCache *cache;
    if (cache_policy == "lru")
    {
        cache = new LRUCache(max_emb_num);
    }
    else if (cache_policy == "lfu")
    {
        cache = new LFUCache(max_emb_num);
    }
    else
    {
        std::cout << "invalid: feature must be `lru` or `lfu`" << std::endl;
        return 0;
    }
    auto begin = std::chrono::high_resolution_clock::now();
    dataloder dl(data_file);
    const size_t batch_size = 512;
    const size_t num_worker = 1;
    const size_t k_size = 8 * batch_size;
    const size_t epoch = 3;
    int64_t *batch_ids = new int64_t[batch_size];
    std::set<int64_t> s;

    size_t access_count = 0;
    size_t hit_count = 0;
    int64_t *evic_ids = new int64_t[k_size];

    {
        dl.init();
        size_t remain_size = dl.size();
        size_t total_size = remain_size;

        while (remain_size > batch_size)
        {
            dl.sample(batch_ids, batch_size);
            cache->add_to_rank(batch_ids, batch_size);
            if (s.size() > cache->max_emb_num())
            {
                size_t true_size = cache->get_evic_ids(evic_ids, k_size);
                for (size_t i = 0; i < true_size; ++i)
                {
                    s.erase(evic_ids[i]);
                }
            }
            for (size_t i = 0; i < batch_size; ++i)
            {
                int64_t id = batch_ids[i];
                if (s.find(id) == s.end())
                    s.insert(id);
            }

            remain_size -= batch_size;
            if (int(remain_size / batch_size) % 8000 == 0)
            {
                std::cout << "\r(0/0)epoch training... " << std::fixed << std::setprecision(2) << double((total_size - remain_size) * 100.0) / total_size << " %" << std::flush;
            }
        }
        dl.sample(batch_ids, remain_size);

        cache->add_to_rank(batch_ids, remain_size);
        if (s.size() > cache->max_emb_num())
        {
            size_t true_size = cache->get_evic_ids(evic_ids, k_size);
            for (size_t i = 0; i < true_size; ++i)
            {
                s.erase(evic_ids[i]);
            }
        }
        for (size_t i = 0; i < remain_size; ++i)
        {
            int64_t id = batch_ids[i];
            if (s.find(id) == s.end())
            {
                s.insert(id);
            }
        }
        remain_size -= remain_size;
    }

    for (size_t e = 0; e < epoch; ++e)
    {
        dl.init();
        size_t remain_size = dl.size();
        size_t total_size = remain_size;

        while (remain_size > batch_size)
        {
            dl.sample(batch_ids, batch_size);
            cache->add_to_rank(batch_ids, batch_size);
            if (s.size() > cache->max_emb_num())
            {
                size_t true_size = cache->get_evic_ids(evic_ids, k_size);
                for (size_t i = 0; i < true_size; ++i)
                {
                    s.erase(evic_ids[i]);
                }
            }
            for (size_t i = 0; i < batch_size; ++i)
            {
                int64_t id = batch_ids[i];
                access_count++;
                if (s.find(id) != s.end())
                {
                    hit_count++;
                }
                else
                {
                    s.insert(id);
                }
            }
            remain_size -= batch_size;
            if (int(remain_size / batch_size) % 8000 == 0)
            {
                std::cout << "\r(" << e + 1 << "/" << epoch << ")epoch training... " << std::fixed << std::setprecision(2) << double((total_size - remain_size) * 100.0) / total_size << " %" << std::flush;
            }
        }
        dl.sample(batch_ids, remain_size);

        cache->add_to_rank(batch_ids, remain_size);
        if (s.size() > cache->max_emb_num())
        {
            size_t true_size = cache->get_evic_ids(evic_ids, k_size);
            for (size_t i = 0; i < true_size; ++i)
            {
                s.erase(evic_ids[i]);
            }
        }
        for (size_t i = 0; i < remain_size; ++i)
        {
            int64_t id = batch_ids[i];
            access_count++;
            if (s.find(id) != s.end())
            {
                hit_count++;
            }
            else
            {
                s.insert(id);
            }
        }
        remain_size -= remain_size;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_zerotrain(end - begin);
    std::cout << std::endl
              << "cost time = " << duration_zerotrain.count() << " s" << std::endl
              << std::flush;
    std::cout << "total hit rate = " << double(hit_count * 100.0) / access_count << " %" << std::endl
              << std::flush;
}
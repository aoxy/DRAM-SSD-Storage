#include <iostream>
#include <fstream>
#include <iomanip>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include "cache.h"
#include "../hdss/store.h"

int main(int argc, char *argv[])
{
    Config conf(argc, argv);
    auto begin = std::chrono::high_resolution_clock::now();
    DataLoader dl("dataset/taobao/shuffled_sample.csv", conf.feature_id);
    BatchCache<int64_t> *cache = conf.cache;
    const size_t batch_size = 512;
    const size_t num_worker = 1;
    const size_t k_size = 8 * batch_size;
    const size_t epoch = 1;
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
            if (s.size() > cache->max_emb_num())
            {
                size_t true_size = cache->get_evic_ids(evic_ids, k_size);
                for (size_t i = 0; i < true_size; ++i)
                {
                    s.erase(evic_ids[i]);
                }
            }
            cache->add_to_rank(batch_ids, batch_size);
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

        if (s.size() > cache->max_emb_num())
        {
            size_t true_size = cache->get_evic_ids(evic_ids, k_size);
            for (size_t i = 0; i < true_size; ++i)
            {
                s.erase(evic_ids[i]);
            }
        }
        cache->add_to_rank(batch_ids, remain_size);
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
            if (s.size() > cache->max_emb_num())
            {
                size_t true_size = cache->get_evic_ids(evic_ids, k_size);
                for (size_t i = 0; i < true_size; ++i)
                {
                    s.erase(evic_ids[i]);
                }
            }
            cache->add_to_rank(batch_ids, batch_size);
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

        if (s.size() > cache->max_emb_num())
        {
            size_t true_size = cache->get_evic_ids(evic_ids, k_size);
            for (size_t i = 0; i < true_size; ++i)
            {
                s.erase(evic_ids[i]);
            }
        }
        cache->add_to_rank(batch_ids, remain_size);
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
#include <fstream>
#include "eviction.h"
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../utils/logs.h"
#include "../movement/files.h"

void eviction(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *evic_ids, size_t k_size, size_t num_workers, FilePool &fp)
{
    size_t work_size = size_t((k_size + num_workers - 1) / num_workers); //上取整
    std::vector<std::thread> workers;
    for (size_t w = 1; w < num_workers; ++w)
    {
        workers.emplace_back(eviction_aux, std::ref(dmap), std::ref(smap), evic_ids, (w - 1) * work_size, w * work_size, std::ref(fp));
    }
    // auto start = std::chrono::high_resolution_clock::now();
    eviction_aux(std::ref(dmap), std::ref(smap), evic_ids, (num_workers - 1) * work_size, k_size, std::ref(fp)); //自己也要干活(也许会少点)
    for (auto &worker : workers)
    {
        worker.join();
    }
}

void eviction_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *evic_ids, size_t begin, size_t end, FilePool &fp)
{
    int64_t key;
    embedding_t value;
    int64_t offset;
    size_t file_idx;
    for (size_t i = begin; i < end; ++i)
    {
        key = evic_ids[i];

        value = dmap.get(key);
        dmap.set(key, nullptr);
        if (value != nullptr)
        { // 在内存中，存到SSD里去
            // for (int ii = 0; ii < EMB_LEN; ++ii)
            // {
            //     LOGINFO << value[ii] << " ";
            // }
            // std::cout << std::endl;
            dmap.decrease();
            assert(value != nullptr && "get embedding failed in eviction.");
            offset = smap.get(key) - 1; //offset - 1 是因为存的时候+1了
            // LOGINFO << key << "被淘汰了，放到SSD中 offset = " << offset << std::endl
            //           << std::flush;

            if (offset < 0)
            {
                //SSD里没存
                LOGINFO << key << "的embedding不在SSD中，致命错误！" << std::endl
                        << std::flush;
                fp.rw_s(key).seekp(0, std::ios::end);
                offset = size_t(fp.rw_s(key).tellp()) + 1; // 存的时候+1了
                smap.set(key, offset);
                fp.rw_s(key).write((char *)&key, sizeof(int64_t));
                fp.rw_s(key).write((char *)value, EMB_LEN * sizeof(double));
            }
            else
            {
                //SSD增量更新
                fp.rw_s(key).seekp(0, std::ios::end);
                offset = size_t(fp.rw_s(key).tellp()) + 1; //存的时候+1了
                smap.set(key, offset);
                fp.rw_s(key).write((char *)&key, sizeof(int64_t));
                fp.rw_s(key).write((char *)value, EMB_LEN * sizeof(double));
            }
            delete[] value;
        }
        // else
        // {
        //     LOGINFO << key << "已经不在DRAM中" << std::endl
        //             << std::flush;
        // }
    }
}
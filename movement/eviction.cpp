#include <fstream>
#include "eviction.h"
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../utils/logs.h"

void eviction(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *evic_ids, size_t k_size, size_t num_workers)
{
    size_t work_size = size_t((k_size + num_workers - 1) / num_workers); //上取整
    std::vector<std::thread> workers;
    for (size_t w = 1; w < num_workers; ++w)
    {
        workers.emplace_back(eviction_aux, std::ref(dmap), std::ref(smap), evic_ids, (w - 1) * work_size, w * work_size);
    }
    // auto start = std::chrono::high_resolution_clock::now();
    eviction_aux(std::ref(dmap), std::ref(smap), evic_ids, (num_workers - 1) * work_size, k_size); //自己也要干活(也许会少点)
    for (auto &worker : workers)
    {
        worker.join();
    }
}

void eviction_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *evic_ids, size_t begin, size_t end)
{
    for (size_t i = begin; i < end; ++i)
    {
        int64_t key = evic_ids[i];

        embedding_t value = dmap.get(key);
        dmap.set(key, nullptr);
        if (value != nullptr)
        { // 在内存中，存到SSD里去
            // for (int ii = 0; ii < EMB_LEN; ++ii)
            // {
            //     LOGINFO << value[ii] << " ";
            // }
            // std::cout << std::endl;
            assert(value != nullptr && "get embedding failed in eviction.");
            int64_t offset = smap.get(key) - 1; //offset - 1 是因为存的时候+1了
            // LOGINFO << key << "被淘汰了，放到SSD中 offset = " << offset << std::endl
            //           << std::flush;
            std::string filepath = smap.filepath(key);

            if (offset < 0)
            {
                //SSD里没存
                LOGINFO << key << "的embedding不在SSD中，致命错误！" << std::endl
                        << std::flush;
                std::ofstream ofs(filepath, std::ios::app | std::ios::binary);
                ofs.seekp(0, std::ios::end);
                offset = size_t(ofs.tellp()) + 1; // 存的时候+1了
                smap.set(key, offset);
                ofs.write((char *)value, EMB_LEN * sizeof(double));
                ofs.close();
            }
            else
            {
                //SSD更新
                std::fstream fs(filepath, std::ios::out | std::ios::in | std::ios::binary);
                fs.seekp(offset, std::ios::beg);
                fs.write((char *)value, EMB_LEN * sizeof(double));
                fs.close();
            }
            delete[] value; // FIXME: 加上这个就段错误
        }
        // else
        // {
        //     LOGINFO << key << "已经不在DRAM中" << std::endl
        //             << std::flush;
        // }
    }
}
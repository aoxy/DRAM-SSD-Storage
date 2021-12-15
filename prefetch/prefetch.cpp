#include <thread>
#include <vector>
#include <fstream>
#include <iostream>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "prefetch.h"
#include "../utils/logs.h"

embedding_t *prefetch(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, size_t num_workers)
{
    embedding_t *ret = new embedding_t[batch_size];

    size_t work_size = int(batch_size + num_workers - 1 / num_workers); //上取整
    std::vector<std::thread> workers;
    for (size_t w = 1; w < num_workers; ++w)
    { //TODO:分任务的方式还可以优化，使得两个线程不操作同一个文件
        workers.emplace_back(fetch_aux, std::ref(dmap), std::ref(smap), batch_ids, (w - 1) * work_size, w * work_size, ret);
    }
    // auto start = std::chrono::high_resolution_clock::now();
    fetch_aux(std::ref(dmap), std::ref(smap), batch_ids, (num_workers - 1) * work_size, batch_size, ret); //自己也要干活(也许会少点)
    // LOGINFO << std::endl << std::flush;
    for (auto &worker : workers)
    {
        worker.join();
    }
    // LOGINFO << std::endl << std::flush;
    return ret;
}

void fetch_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t begin, size_t end, embedding_t *ret)
{
    for (size_t i = begin; i < end; ++i)
    {
        int64_t key = batch_ids[i];
        embedding_t value = dmap.get(key);
        if (value == nullptr)
        { // 在SSD中，读取并分配地址

            value = new double[EMB_LEN];
            assert(value != nullptr && "allocate failed in prefetch.");
            int64_t offset = smap.get(key) - 1; //offset - 1 是因为存的时候+1了
            if (offset < 0)
            {
                LOGINFO << "not exist key = " << key << std::endl;
                continue;
            }
            // assert(offset >= 0 && "offset < 0.");

            std::string filepath = smap.filepath(key);
            // TODO: LOGINFO << key << " 不在内存中，在文件" << filepath << " offset = " << offset << std::endl
            // TODO:         << std::flush;
            std::ifstream ifs(filepath, std::ios::in | std::ios::binary);
            ifs.seekg(offset, std::ios::beg);
            ifs.read((char *)value, EMB_LEN * sizeof(double));
            ifs.close();
            dmap.set(key, value);
        }
        ret[i] = value;
    }
}
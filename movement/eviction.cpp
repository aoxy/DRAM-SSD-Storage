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

        if (value != nullptr)
        {
            dmap.set(key, nullptr);
            dmap.decrease();
            //SSD增量更新
            fp.rw_s(key).seekp(0, std::ios::end);
            offset = size_t(fp.rw_s(key).tellp()) + 1; //存的时候+1了
            smap.set(key, offset);
            fp.rw_s(key).write((char *)value, EMB_LEN * sizeof(double));
            delete[] value;
        }
    }
}
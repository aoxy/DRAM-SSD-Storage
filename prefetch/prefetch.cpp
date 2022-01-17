#include <thread>
#include <vector>
#include <fstream>
#include <iostream>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "prefetch.h"
#include "../utils/logs.h"
#include "../ranking/cache_manager.h"
#include "../utils/xhqueue.h"
#include "../ranking/cache.h"
#include "../movement/files.h"

void prefetch(shard_lock_map &dmap, ssd_hash_map &smap, embedding_t *ret, int64_t *batch_ids, size_t batch_size, size_t num_workers, CacheRecord &cr, BatchCache *cache, size_t k_size, FilePool &fp)
{
    cache_manager_once(std::ref(dmap), std::ref(smap), cache, k_size, 1, std::ref(fp)); // TODO: 不能淘汰本次要用的，先淘汰
    cache->add_to_rank(batch_ids, batch_size);

    size_t work_size = int((batch_size + num_workers - 1) / num_workers); //上取整
    std::vector<std::thread> workers;
    // TODO:分任务的方式还可以优化，使得两个线程不操作同一个文件
    for (size_t w = 1; w < num_workers; ++w)
    {
        workers.emplace_back(fetch_aux, std::ref(dmap), std::ref(smap), batch_ids, (w - 1) * work_size, w * work_size, ret, std::ref(cr), std::ref(fp));
    }

    fetch_aux(std::ref(dmap), std::ref(smap), batch_ids, (num_workers - 1) * work_size, batch_size, ret, std::ref(cr), std::ref(fp)); //自己也要干活(也许会少点)

    for (auto &worker : workers)
    {
        worker.join();
    }
}

void fetch_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t begin, size_t end, embedding_t *ret, CacheRecord &cr, FilePool &fp)
{
    int64_t key;
    embedding_t value;
    int64_t offset;
    for (size_t i = begin; i < end; ++i)
    {
        key = batch_ids[i];
        value = dmap.get(key);
        if (value == nullptr)
        { // 在SSD中，读取并分配地址
            value = new double[EMB_LEN];
            assert(value != nullptr && "allocate failed in prefetch.");
            offset = smap.get(key) - 1; //offset - 1 是因为存的时候+1了
            assert(offset >= 0 && "key not exist.");
            fp.rw_s(key).seekg(offset, std::ios::beg);
            fp.rw_s(key).read((char *)value, EMB_LEN * sizeof(double));
            dmap.set(key, value);
            dmap.increase();
        }
        else
        {
            cr.hit();
        }
        cr.access();
        ret[i] = value;
    }
}
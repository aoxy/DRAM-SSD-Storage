
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/eviction.h"
#include "cache_manager.h"
#include "../ranking/strategy.h"
#include "../utils/logs.h"
#include "../ranking/cache.h"

void cache_inspect(shard_lock_map &dmap, bool &running)
{
    while (running)
    {
        size_t emb_num = dmap.true_size();
        LOGINFO << emb_num << " embedding in DRAM" << std::endl
                << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void cache_manager_once(shard_lock_map &dmap, ssd_hash_map &smap, BatchCache<int64_t> *cache, size_t k_size, size_t num_workers, FilePool &fp)
{
    int64_t *evic_ids = new int64_t[k_size];
    if (evic_ids == nullptr)
    {
        LOGINFO << " malloc evic_ids failed." << std::endl;
        exit(1);
    }

    if (dmap.true_size() > cache->max_emb_num())
    {
        size_t true_size = cache->get_evic_ids(evic_ids, k_size);
        eviction(std::ref(dmap), std::ref(smap), evic_ids, true_size, num_workers, std::ref(fp));
    }

    delete[] evic_ids;
}
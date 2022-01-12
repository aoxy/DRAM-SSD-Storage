
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/eviction.h"
#include "cache_manager.h"
#include "../ranking/strategy.h"
#include "../utils/logs.h"
#include "../ranking/lru.h"

void cache_manager(shard_lock_map &dmap, ssd_hash_map &smap, BatchLRUCache &cache, size_t k_size, size_t num_workers, bool uniform, bool &running)
{
    int64_t *evic_ids = new int64_t[k_size];
    if (evic_ids == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
    }
    while (running)
    {
        // LOGINFO << std::endl;
        size_t emb_num = dmap.true_size();
        LOGINFO << "当前DRAM中存了 " << emb_num << " 个embedding" << std::endl
                << std::flush;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    delete[] evic_ids;
}

void cache_manager_once(shard_lock_map &dmap, ssd_hash_map &smap, BatchLRUCache &cache, size_t k_size, size_t num_workers, bool uniform, size_t max_emb_num, FilePool &fp)
{
    int64_t *evic_ids = new int64_t[k_size];
    if (evic_ids == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
    }

    // LOGINFO << std::endl;
    size_t emb_num = dmap.true_size();
    // LOGINFO << "当前DRAM中存了 " << emb_num << " 个embedding" << std::endl
    //         << std::flush;
    if (emb_num > max_emb_num)
    {
        // LOGINFO << que << std::endl
        //         << std::flush;
        size_t true_size = cache.get_evic_ids(evic_ids, k_size);
        eviction(std::ref(dmap), std::ref(smap), evic_ids, true_size, num_workers, std::ref(fp));
    }

    delete[] evic_ids;
}

#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/eviction.h"
#include "cache_manager.h"
#include "../ranking/strategy.h"
#include "../utils/logs.h"
#include "../ranking/lru.h"

void cache_manager_once(shard_lock_map &dmap, ssd_hash_map &smap, BatchLRUCache &cache, size_t k_size, size_t num_workers, size_t max_emb_num, FilePool &fp)
{
    int64_t *evic_ids = new int64_t[k_size];
    if (evic_ids == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
    }

    size_t emb_num = dmap.true_size();
    if (emb_num > max_emb_num)
    {
        size_t true_size = cache.get_evic_ids(evic_ids, k_size);
        eviction(std::ref(dmap), std::ref(smap), evic_ids, true_size, num_workers, std::ref(fp));
    }

    delete[] evic_ids;
}
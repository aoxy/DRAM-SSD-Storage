
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/eviction.h"
#include "cache_manager.h"
#include "../ranking/strategy.h"
#include "../utils/logs.h"

void cache_manager(shard_lock_map &dmap, ssd_hash_map &smap, xhqueue<int64_t> &que, size_t k_size, size_t num_workers, bool uniform, bool &running)
{
    int64_t *evic_ids = new int64_t[k_size];
    while (running)
    {
        // LOGINFO << std::endl;
        size_t emb_num = dmap.true_size();
        LOGINFO << "当前DRAM中存了 " << emb_num << " 个embedding" << std::endl
                << std::flush;
        if (emb_num > MAX_EMB_NUM)
        {
            // TODO: LOGINFO << que << std::endl
            // TODO:         << std::flush;
            size_t true_size = ranking(std::ref(que), evic_ids, k_size, uniform);
            eviction(std::ref(dmap), std::ref(smap), evic_ids, true_size, num_workers);
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    delete[] evic_ids;
}

void cache_manager_once(shard_lock_map &dmap, ssd_hash_map &smap, xhqueue<int64_t> &que, size_t k_size, size_t num_workers, bool uniform)
{
    int64_t *evic_ids = new int64_t[k_size];

    // LOGINFO << std::endl;
    size_t emb_num = dmap.true_size();
    // LOGINFO << "当前DRAM中存了 " << emb_num << " 个embedding" << std::endl
    //         << std::flush;
    if (emb_num > MAX_EMB_NUM)
    {
        // TODO: LOGINFO << que << std::endl
        // TODO:         << std::flush;
        size_t true_size = ranking(std::ref(que), evic_ids, k_size, uniform);
        eviction(std::ref(dmap), std::ref(smap), evic_ids, true_size, num_workers);
    }

    delete[] evic_ids;
}
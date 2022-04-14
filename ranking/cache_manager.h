
#pragma once
#include <queue>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../utils/xhqueue.h"
#include "../ranking/cache.h"
#include "../movement/files.h"

void cache_manager_once(shard_lock_map &dmap, ssd_hash_map &smap, BatchCache<int64_t> *cache, size_t k_size, size_t num_workers, FilePool &fp);

void cache_inspect(shard_lock_map &dmap, bool &running);
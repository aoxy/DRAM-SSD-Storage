#pragma once
#include <thread>
#include <vector>
#include <fstream>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../utils/xhqueue.h"
#include "../ranking/cache.h"
#include "../movement/files.h"

embedding_t *prefetch(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, size_t num_workers, CacheRecord &cr, BatchCache *cache, size_t k_size, FilePool &fp);

void fetch_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t begin, size_t end, embedding_t *ret, CacheRecord &cr, FilePool &fp);
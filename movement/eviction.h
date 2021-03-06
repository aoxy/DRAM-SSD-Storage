#pragma once
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/files.h"

void eviction(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *evic_ids, size_t k_size, size_t num_workers, FilePool &fp);

void eviction_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *evic_ids, size_t begin, size_t end, FilePool &fp);
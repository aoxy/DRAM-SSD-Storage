#include <thread>
#include <vector>
#include <fstream>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../utils/xhqueue.h"
#include "../ranking/lru.h"

embedding_t *prefetch(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, size_t num_workers, size_t &access_count, size_t &hit_count, BatchLRUCache &cache, size_t k_size);

void fetch_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t begin, size_t end, embedding_t *ret, size_t &access_count, size_t &hit_count, size_t k_size);
#include <thread>
#include <vector>
#include <fstream>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"

embedding_t *prefetch(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, size_t num_workers);

void fetch_aux(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t begin, size_t end, embedding_t *ret);
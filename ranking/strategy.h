
#pragma once
#include <queue>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/eviction.h"
#include "../utils/xhqueue.h"

#define QUEUE_SIZE 2048

// 记录最近8*batch_size次访问中各个id的访问次数
void add_to_rank(xhqueue<int64_t> &que, int64_t *batch_ids, size_t batch_size);

// 获取要淘汰的ids
size_t ranking(xhqueue<int64_t> &que, int64_t *evic_ids, size_t k_size, bool uniform);
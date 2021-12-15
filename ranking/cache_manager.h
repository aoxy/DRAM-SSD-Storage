
#pragma once
#include <queue>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../utils/xhqueue.h"

// #define MAX_EMB_NUM size_t(2000 * 1024 * 1024 / (EMB_LEN * sizeof(double))) // 16384000 = 2000 MB 内存
#define MAX_EMB_NUM size_t(20 * 1024 * 1024 / (EMB_LEN * sizeof(double))) // 163840 = 20 MB 内存

// 一个异步线程
void cache_manager(shard_lock_map &dmap, ssd_hash_map &smap, xhqueue<int64_t> &que, size_t k_size, size_t num_workers, bool uniform, bool &running);
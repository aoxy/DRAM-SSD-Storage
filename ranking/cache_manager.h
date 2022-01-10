
#pragma once
#include <queue>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../utils/xhqueue.h"
#include "../ranking/lru.h"

// #define MAX_EMB_NUM size_t(2000 * 1024 * 1024 / (EMB_LEN * sizeof(double))) // 16384000 = 2000 MB 内存
// #define MAX_EMB_NUM size_t(50 * 1024 * 1024 / (EMB_LEN * sizeof(double))) // 409600 = 50 MB 内存
// #define MAX_EMB_NUM  8468
// #define MAX_EMB_NUM_PERC  0.4

// 一个异步线程
void cache_manager(shard_lock_map &dmap, ssd_hash_map &smap, BatchLRUCache &cache, size_t k_size, size_t num_workers, bool uniform, bool &running);
void cache_manager_once(shard_lock_map &dmap, ssd_hash_map &smap, BatchLRUCache &cache, size_t k_size, size_t num_workers, bool uniform, size_t max_emb_num);
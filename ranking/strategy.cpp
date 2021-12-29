
#include <queue>
#include <algorithm>
#include "sparsehash/dense_hash_map"
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/eviction.h"
#include "../ranking/strategy.h"
#include "../utils/xhqueue.h"

using google::dense_hash_map;

class counter
{
public:
    int64_t key;
    size_t count;
    counter(int64_t k, size_t c) : key(k), count(c) {}
};

void add_to_rank(xhqueue<int64_t> &que, int64_t *batch_ids, size_t batch_size)
{
    for (size_t i = 0; i < batch_size; ++i)
    {
        int64_t key = batch_ids[i];
        que.push(key);
    }
}

// 记录最近16*batch_size次访问中各个id的访问次数
size_t ranking(xhqueue<int64_t> &que, int64_t *evic_ids, size_t k_size, bool uniform)
{
    dense_hash_map<int64_t, size_t> map;
    map.set_empty_key(-1);
    for (size_t i = que.begin; i != que.end; i = que.next(i)) // TODO: std::queue 没有迭代器
    {
        int64_t key = que.elements[i];
        if (key != 0)
        {
            map[key] = map[key] + 1;
        }
    }
    std::vector<counter> v;
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        v.emplace_back(counter(it->first, it->second));
    }
    if (uniform)
    {
        std::sort(v.begin(), v.end(), [](const counter &c1, const counter &c2)
                  { return c1.count > c2.count; }); // 大的在前
    }
    else
    {
        std::sort(v.begin(), v.end(), [](const counter &c1, const counter &c2)
                  { return c1.count < c2.count; });
    }
    size_t true_size = std::min(v.size(), k_size);
    for (size_t i = 0; i < true_size; ++i)
    {
        evic_ids[i] = v[i].key;
        for (size_t j = que.begin; j != que.end; j = que.next(j)) // 去除淘汰的
        {
            if (que.elements[j] == evic_ids[i])
            {
                que.elements[j] = 0;
            }
        }
    }
    return true_size;
}
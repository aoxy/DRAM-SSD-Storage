#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <iomanip>
#include "store.h"
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../prefetch/prefetch.h"
#include "../process/get_embeddings.h"
#include "../utils/logs.h"
#include "../ranking/cache.h"
#include "../movement/files.h"
#include "../ranking/cache_manager.h"

void epoch_zero(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, BatchCache *cache, size_t k_size, DataLoader &dl, FilePool &fp, size_t num_worker);
double train(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, BatchCache *cache, size_t k_size, DataLoader &dl, FilePool &fp, size_t num_worker, size_t epoch);

int main(int argc, char *argv[])
{
    Config conf(argc, argv);
    auto ts1 = std::chrono::high_resolution_clock::now();
    DataLoader dl("dataset/taobao/shuffled_sample.csv", conf.feature_id);
    auto ts2 = std::chrono::high_resolution_clock::now();
    const size_t epoch = 3;
    const size_t batch_size = 512;
    const size_t num_worker = 1;
    const size_t k_size = 8 * batch_size;
    double hit_rate;
    LOGINFO << "dsize = " << conf.dsize << std::endl
            << std::flush;
    LOGINFO << "data size = " << dl.size() << std::endl
            << std::flush;
    LOGINFO << "max emb num = " << conf.cache->max_emb_num() << std::endl
            << std::flush;
    LOGINFO << "batch_size = " << batch_size << std::endl
            << std::flush;
    LOGINFO << "k_size = " << k_size << std::endl
            << std::flush;
    shard_lock_map dmap;
    ssd_hash_map smap(conf.feature);
    FilePool fp(smap);
    BatchCache *cache = conf.cache;

    int64_t *batch_ids = new int64_t[batch_size];
    if (batch_ids == nullptr)
    {
        LOGINFO << " malloc batch_ids failed." << std::endl;
        exit(1);
    }

    init_ssd_map(std::ref(smap)); // 先加载存在SSD上的offset map
    bool running = true;
    // std::thread th(cache_inspect, std::ref(dmap), std::ref(running));

    // 先训练一轮不记录数据，用于使得“训练开始前已经有一部分数据在内存中”
    auto ts3 = std::chrono::high_resolution_clock::now();
    epoch_zero(std::ref(dmap), std::ref(smap), batch_ids, batch_size, cache, k_size, std::ref(dl), std::ref(fp), 1);
    auto ts4 = std::chrono::high_resolution_clock::now();
    // 正式训练
    hit_rate = train(std::ref(dmap), std::ref(smap), batch_ids, batch_size, cache, k_size, std::ref(dl), std::ref(fp), 1, epoch);
    auto ts5 = std::chrono::high_resolution_clock::now(); //训练时间

    delete[] batch_ids;

    compaction(std::ref(dmap), std::ref(smap), std::ref(fp));
    assert(dmap.true_size() == 0 && "dmap final size is nonzero");
    auto ts6 = std::chrono::high_resolution_clock::now(); //持久化时间

    running = false;
    // th.join();

    auto ts7 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_train(ts5 - ts4);
    std::chrono::duration<double, std::ratio<1, 1>> duration_save(ts6 - ts5);
    std::chrono::duration<double, std::ratio<1, 1>> duration_total(ts7 - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_readid(ts2 - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_zerotrain(ts4 - ts3);
    LOGINFO << "read id time = " << duration_readid.count() << " s" << std::flush;
    LOGINFO << "zero-th epoch time = " << duration_zerotrain.count() << " s" << std::flush;
    LOGINFO << "train time = " << duration_train.count() << " s" << std::flush;
    LOGINFO << "compaction time = " << duration_save.count() << " s" << std::flush;
    LOGINFO << "total time = " << duration_total.count() << " s" << std::flush;
    LOGINFO << "total hit rate = " << hit_rate << " %" << std::endl
            << std::flush;
    return 0;
}

double train(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, BatchCache *cache, size_t k_size, DataLoader &dl, FilePool &fp, size_t num_worker, size_t epoch)
{
    CacheRecord cr;
    for (size_t e = 0; e < epoch; ++e)
    {
        dl.init();
        size_t remain_size = dl.size();
        size_t total_size = remain_size;

        while (remain_size > batch_size)
        {
            dl.sample(batch_ids, batch_size);
            embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, batch_size, num_worker, std::ref(cr), cache, k_size, std::ref(fp));
            get_embs(ret, batch_size, num_worker);
            remain_size -= batch_size;
            if (int(remain_size / batch_size) % 8000 == 0)
            {
                std::cout << "\r(" << e + 1 << "/" << epoch << ")epoch training... " << std::fixed << std::setprecision(2) << double((total_size - remain_size) * 100.0) / total_size << " %" << std::flush;
            }
        }
        dl.sample(batch_ids, remain_size);
        embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, remain_size, num_worker, std::ref(cr), cache, k_size, std::ref(fp));
        get_embs(ret, remain_size, num_worker);
        remain_size -= remain_size;
    }
    return cr.hit_rate();
}

void epoch_zero(shard_lock_map &dmap, ssd_hash_map &smap, int64_t *batch_ids, size_t batch_size, BatchCache *cache, size_t k_size, DataLoader &dl, FilePool &fp, size_t num_worker)
{
    CacheRecord temp_cr;
    dl.init();
    size_t remain_size = dl.size();
    size_t total_size = remain_size;

    while (remain_size > batch_size)
    {
        dl.sample(batch_ids, batch_size);
        embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, batch_size, num_worker, std::ref(temp_cr), cache, k_size, std::ref(fp));
        update_embs(ret, batch_size); // 一种梯度更新的方式
        // output(ret, 0, batch_size); // 加100次1.0
        remain_size -= batch_size;
        if (int(remain_size / batch_size) % 8000 == 0)
        {
            std::cout << "\repoch zero training... " << std::fixed << std::setprecision(2) << double((total_size - remain_size) * 100.0) / total_size << " %" << std::flush;
        }
    }
    dl.sample(batch_ids, remain_size);
    embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, remain_size, num_worker, std::ref(temp_cr), cache, k_size, std::ref(fp));
    update_embs(ret, remain_size); // 一种梯度更新的方式
    // output(ret, 0, remain_size);// 加100次1.0
    remain_size -= remain_size;
}

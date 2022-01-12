#include <iostream>
#include <thread>
#include <queue>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstring>
#include "../ranking/cache_manager.h"
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../prefetch/prefetch.h"
#include "../process/get_embeddings.h"
#include "../ranking/strategy.h"
#include "../utils/logs.h"
#include "../utils/xhqueue.h"
#include "../movement/eviction.h"
#include "../ranking/lru.h"
#include "../movement/files.h"

class dataloder
{
private:
    std::vector<int64_t> ids;
    size_t offset;

public:
    dataloder(std::string filepath, size_t feature_id = 0)
    {
        std::ifstream fp(filepath);
        if (!fp)
        {
            LOGINFO << "open" << filepath << "fail." << std::endl
                    << std::flush;
            exit(1);
        }
        std::string line;
        std::getline(fp, line); //跳过列名，第一行不做处理

        while (std::getline(fp, line))
        {
            std::string number;
            std::istringstream readstr(line);
            for (size_t j = 0; j <= feature_id; ++j)
            {
                std::getline(readstr, number, ',');
            }
            ids.push_back(std::atoi(number.c_str()));
        }
        offset = 0;
    }
    void init() { offset = 0; }
    void sample(int64_t *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; offset < ids.size() && i < batch_size; ++offset, ++i)
        {
            batch_ids[i] = ids[offset];
        }
    }
    size_t size() { return ids.size(); }
};

void init_ssd_map(ssd_hash_map &smap)
{
    auto feature_name = smap.feature_name();
    const std::string filepath = std::string("storage/") + feature_name + std::string("/offset.txt");
    std::ifstream fp(filepath);

    if (!fp)
    {
        LOGINFO << "open " << filepath << " fail." << std::endl
                << std::flush;
        exit(1);
    }
    int64_t key;
    size_t value;
    std::string line;
    while (std::getline(fp, line))
    {
        std::string number;
        std::istringstream readstr(line);
        std::getline(readstr, number, ' ');
        key = std::atoi(number.c_str());
        std::getline(readstr, number, ' ');
        value = std::atoi(number.c_str());
        smap.set(key, value);
    }
}

void update_ssd_map(ssd_hash_map &smap)
{
    auto feature_name = smap.feature_name();
    const std::string ids_file = std::string("storage/") + feature_name + std::string("/all_ids.txt");
    std::ifstream ifs(ids_file);
    std::vector<int64_t> ids;
    int64_t key;
    while (ifs >> key)
    {
        ids.push_back(key);
    }
    ifs.close();
    const std::string offset_map_file = std::string("storage/") + feature_name + std::string("/offset_saved.txt");
    std::ofstream ofs_off(offset_map_file, std::ios::binary | std::ios::trunc);
    for (int64_t &key : ids)
    {
        ofs_off << key << ' ' << smap.get(key) << std::endl;
    }
    ofs_off.close();
}

void save_ssd(shard_lock_map &dmap, ssd_hash_map &smap, dataloder &dl, FilePool &fp)
{
    dl.init();
    size_t remain_size = dl.size();
    int64_t *batch_ids = new int64_t[remain_size];
    if (batch_ids == nullptr)
    {
        LOGINFO << " malloc failed. " << std::endl;
        exit(1);
    }
    dl.sample(batch_ids, remain_size);
    eviction(std::ref(dmap), std::ref(smap), batch_ids, remain_size, 1, std::ref(fp));
    update_ssd_map(std::ref(smap));
    delete[] batch_ids;
}

void compaction(shard_lock_map &dmap, ssd_hash_map &smap, FilePool &old_fp)
{
    auto feature_name = smap.feature_name();
    size_t offset;
    size_t file_idx;
    embedding_t value;
    const std::string ids_file = std::string("storage/") + feature_name + std::string("/all_ids.txt");
    std::ifstream ifs(ids_file);
    std::vector<int64_t> ids;
    int64_t key;
    while (ifs >> key)
    {
        ids.push_back(key);
    }
    ifs.close();
    std::string offset_map_file = std::string("storage/") + feature_name + std::string("/offset.txt");
    std::remove(offset_map_file.c_str());
    std::ofstream ofs_off(offset_map_file, std::ios::app | std::ios::binary);
    for (int64_t &key : ids)
    {
        value = dmap.get(key);
        if (value == nullptr) // 不在内存里，先读进来
        {
            value = new double[EMB_LEN];
            assert(value != nullptr && "allocate failed in compaction.");
            offset = smap.get(key) - 1; //offset - 1 是因为存的时候+1了
            assert(offset >= 0 && "offset < 0.");
            old_fp.rw_s(key).seekg(offset, std::ios::beg);
            old_fp.rw_s(key).read((char *)value, EMB_LEN * sizeof(double));
            dmap.set(key, value);
            dmap.increase();
        }
    }
    old_fp.close();
    for (size_t i = 0; i < NUM_SHARD; ++i)
    {
        std::remove(smap.filepath(i).c_str());
    }
    FilePool new_fp(smap);
    for (int64_t &key : ids)
    {
        value = dmap.get(key);
        assert(value != nullptr && "unknown error failed.");
        dmap.set(key, nullptr);
        dmap.decrease();
        offset = size_t(new_fp.rw_s(key).tellp()) + 1; //存的时候+1了
        new_fp.rw_s(key).write((char *)value, EMB_LEN * sizeof(double));
        ofs_off << key << ' ' << offset << std::endl;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "usage: ./main <feature name> <cache percent>" << std::endl;
        exit(0);
    }
    std::string data_file;
    size_t dsize;
    if (std::strcmp(argv[1], "ad") == 0)
    {
        data_file = "dataset/taobao/shuffled_adgroupid.csv";
        dsize = 846811;
        // userid size:  1141729
        // adgroupid size:  846811
    }
    else if (std::strcmp(argv[1], "user") == 0)
    {
        data_file = "dataset/taobao/shuffled_userid.csv";
        dsize = 1141729;
    }
    else
    {
        LOGINFO << " invalid dataset " << std::endl;
        exit(0);
    }
    auto ts1 = std::chrono::high_resolution_clock::now();
    // dataloder dl("dataset/taobao/raw_sample.csv", 2); // 0: user,1: time_stamp,2: adgroup_id,3: pid,4: nonclk,5: clk
    dataloder dl(data_file); // 0: user,1: time_stamp,2: adgroup_id,3: pid,4: nonclk,5: clk

    auto ts2 = std::chrono::high_resolution_clock::now();
    LOGINFO << "dsize = " << dsize << std::endl
            << std::flush;
    LOGINFO << "data size = " << dl.size() << std::endl
            << std::flush;
    shard_lock_map dmap;
    std::string feature_name(argv[1]);
    ssd_hash_map smap(feature_name);
    FilePool fp(smap);
    int max_emb_num_perc = atoi(argv[2]);
    const size_t max_emb_num = size_t(max_emb_num_perc * dsize / 100);
    LOGINFO << "max emb num = " << max_emb_num << std::endl
            << std::flush;
    BatchLRUCache cache(max_emb_num);
    const size_t epoch = 3;
    const size_t batch_size = 512;
    const size_t num_worker = 1;
    const size_t k_size = 8 * batch_size;
    LOGINFO << "batch_size = " << batch_size << std::endl
            << std::flush;
    LOGINFO << "k_size = " << k_size << std::endl
            << std::flush;
    int64_t *batch_ids = new int64_t[batch_size];
    if (batch_ids == nullptr)
    {
        LOGINFO << " malloc failed. " << std::endl;
        exit(1);
    }

    init_ssd_map(std::ref(smap)); // 先加载存在SSD上的offset map
    bool running = true;
    // std::thread th(cache_manager, std::ref(dmap), std::ref(smap), std::ref(cache), k_size, num_worker, true, std::ref(running));

    size_t access_count = 0;
    size_t hit_count = 0;
    auto fetch_b = std::chrono::high_resolution_clock::now();
    auto fetch_e = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_fetch;

    auto update_b = std::chrono::high_resolution_clock::now();
    auto update_e = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_update;

    auto ts3 = std::chrono::high_resolution_clock::now();
    // 先训练一轮不记录数据，用于使得“训练开始前已经有一部分数据在内存中”
    {
        dl.init();
        size_t remain_size = dl.size();
        size_t total_size = remain_size;

        while (remain_size > batch_size)
        {
            dl.sample(batch_ids, batch_size);
            embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, batch_size, num_worker, std::ref(access_count), std::ref(hit_count), std::ref(cache), k_size, max_emb_num, std::ref(fp));
            update_embs(ret, batch_size);
            remain_size -= batch_size;
            if (int(remain_size / batch_size) % 8000 == 0)
            {
                std::cout << "\r(0/0)epoch training... " << std::fixed << std::setprecision(2) << double((total_size - remain_size) * 100.0) / total_size << " %" << std::flush;
            }
        }
        dl.sample(batch_ids, remain_size);
        embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, remain_size, num_worker, std::ref(access_count), std::ref(hit_count), std::ref(cache), k_size, max_emb_num, std::ref(fp));
        update_embs(ret, remain_size);
        remain_size -= remain_size;

        access_count = 0;
        hit_count = 0;
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t e = 0; e < epoch; ++e)
    {
        dl.init();
        size_t remain_size = dl.size();
        size_t total_size = remain_size;

        while (remain_size > batch_size)
        {
            dl.sample(batch_ids, batch_size);
            fetch_b = std::chrono::high_resolution_clock::now();
            embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, batch_size, num_worker, std::ref(access_count), std::ref(hit_count), std::ref(cache), k_size, max_emb_num, std::ref(fp));
            fetch_e = std::chrono::high_resolution_clock::now();
            get_embs(ret, batch_size, num_worker);
            update_e = std::chrono::high_resolution_clock::now();
            duration_fetch += fetch_e - fetch_b;
            duration_update = update_e - fetch_e;
            remain_size -= batch_size;
            if (int(remain_size / batch_size) % 8000 == 0)
            {
                std::cout << "\r(" << e + 1 << "/" << epoch << ")epoch training... " << std::fixed << std::setprecision(2) << double((total_size - remain_size) * 100.0) / total_size << " %" << std::flush;
            }
        }
        dl.sample(batch_ids, remain_size);
        embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, remain_size, num_worker, std::ref(access_count), std::ref(hit_count), std::ref(cache), k_size, max_emb_num, std::ref(fp));
        get_embs(ret, remain_size, num_worker);
        remain_size -= remain_size;
    }
    auto point1 = std::chrono::high_resolution_clock::now(); //训练时间
    delete[] batch_ids;
    // save_ssd(std::ref(dmap), std::ref(smap), std::ref(dl), std::ref(fp));
    compaction(std::ref(dmap), std::ref(smap), std::ref(fp));

    assert(dmap.true_size() == 0 && "dmap final size is nonzero");
    auto point2 = std::chrono::high_resolution_clock::now(); //持久化时间
    running = false;
    // th.join();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_train(point1 - start);
    std::chrono::duration<double, std::ratio<1, 1>> duration_save(point2 - point1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_total(end - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_readid(ts2 - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_zerotrain(start - ts3);
    LOGINFO << "read id time = " << duration_readid.count() << " s" << std::flush;
    LOGINFO << "zero-th epoch time = " << duration_zerotrain.count() << " s" << std::flush;
    LOGINFO << "train time = " << duration_train.count() << " s" << std::flush;
    LOGINFO << "\tfetch time = " << duration_fetch.count() << " s" << std::flush;
    LOGINFO << "\tupdate time = " << duration_update.count() << " s" << std::flush;
    LOGINFO << "compaction time = " << duration_save.count() << " s" << std::flush;
    LOGINFO << "total time = " << duration_total.count() << " s" << std::flush;

    LOGINFO << "total hit rate = " << double(hit_count * 100.0) / access_count << " %" << std::endl
            << std::flush;
    return 0;
}
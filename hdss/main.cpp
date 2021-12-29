#include <iostream>
#include <thread>
#include <queue>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdio>
#include "../ranking/cache_manager.h"
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../prefetch/prefetch.h"
#include "../process/get_embeddings.h"
#include "../ranking/strategy.h"
#include "../utils/logs.h"
#include "../utils/xhqueue.h"
#include "../movement/eviction.h"

class dataloder
{
private:
    std::vector<int64_t> ids;
    size_t offset;

public:
    dataloder(std::string filepath, size_t feature_id)
    {
        // std::cout << filepath << std::endl;
        std::ifstream fp(filepath); //定义声明一个ifstream对象，指定文件路径
        if (!fp)
        {
            LOGINFO << "open" << filepath << "fail." << std::endl
                    << std::flush;
            exit(1);
        }
        std::string line;
        std::getline(fp, line); //跳过列名，第一行不做处理
        // std::cout << line;
        while (std::getline(fp, line))
        { //循环读取每行数据
            std::string number;
            std::istringstream readstr(line); //string数据流化
            //将一行数据按'，'分割
            for (size_t j = 0; j <= feature_id; ++j)
            {                                       //可根据数据的实际情况取循环获取
                std::getline(readstr, number, ','); //循环读取数据
            }
            ids.push_back(std::atoi(number.c_str())); //插入到vector中
        }
        offset = 0;
    }
    void init() { offset = 0; }
    void sample(int64_t *batch_ids, size_t batch_size)
    {
        // int64_t *batch_ids = new int64_t[batch_size];
        for (size_t i = 0; offset < ids.size() && i < batch_size; ++offset, ++i)
        {
            batch_ids[i] = ids[offset];
        }
        // return batch_ids;
    }
    size_t size() { return ids.size(); }
};

void init_ssd_map(ssd_hash_map &smap)
{
    const std::string filepath = "storage/offset.txt";
    std::ifstream fp(filepath); //定义声明一个ifstream对象，指定文件路径
    if (!fp)
    {
        LOGINFO << "open" << filepath << "fail." << std::endl
                << std::flush;
        exit(1);
    }
    int64_t key;
    size_t value;
    std::string line;
    while (std::getline(fp, line))
    { //循环读取每行数据
        std::string number;
        std::istringstream readstr(line); //string数据流化
        //将一行数据按'，'分割
        std::getline(readstr, number, ' ');
        key = std::atoi(number.c_str());
        std::getline(readstr, number, ' ');
        value = std::atoi(number.c_str());
        // std::cout << key << " " << value << std::endl;
        smap.set(key, value);
    }
}

void save_ssd(shard_lock_map &dmap, ssd_hash_map &smap, dataloder &dl)
{
    dl.init();
    size_t remain_size = dl.size();
    int64_t *batch_ids = new int64_t[remain_size];
    if (batch_ids == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
    }
    dl.sample(batch_ids, remain_size);
    eviction(std::ref(dmap), std::ref(smap), batch_ids, remain_size, 1);
}

int main()
{
    auto ts1 = std::chrono::high_resolution_clock::now();
    dataloder dl("dataset/taobao/raw_sample.csv", 0);
    auto ts2 = std::chrono::high_resolution_clock::now();
    LOGINFO << "data size = " << dl.size() << std::endl
            << std::flush;
    shard_lock_map dmap;
    ssd_hash_map smap;
    xhqueue<int64_t> que(QUEUE_SIZE);
    const size_t epoch = 3;
    const size_t batch_size = 128;
    const size_t num_worker = 1;
    const size_t k_size = 6 * batch_size;
    int64_t *batch_ids = new int64_t[batch_size];
    if (batch_ids == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
        ;
        exit(1);
    }
    // LOGINFO << std::endl << std::flush;
    init_ssd_map(std::ref(smap)); // 先加载存在SSD上的offset map
    // return 0;
    bool running = true;
    // std::thread th(cache_manager, std::ref(dmap), std::ref(smap), std::ref(que), k_size, num_worker, true, std::ref(running)); // TODO:
    auto start = std::chrono::high_resolution_clock::now();
    size_t access_count = 0;
    size_t hit_count = 0;

    for (size_t e = 0; e < epoch; ++e)
    {
        dl.init();
        size_t remain_size = dl.size();
        size_t total_size = remain_size;

        while (remain_size > batch_size)
        {
            dl.sample(batch_ids, batch_size);
            // TODO: LOGINFO << e + 1 << " batch: ";
            // TODO: for (size_t i = 0; i < batch_size; ++i)
            // TODO: {
            // TODO:     std::cout << batch_ids[i] << " " << std::flush;
            // TODO: }
            // TODO: std::cout << std::endl
            // TODO:           << std::flush;
            add_to_rank(std::ref(que), batch_ids, batch_size);
            // LOGINFO << std::endl << std::flush;
            embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, batch_size, num_worker, std::ref(access_count), std::ref(hit_count), std::ref(que), k_size);
            get_embs(ret, batch_size, num_worker);
            remain_size -= batch_size;
            if (int(remain_size / batch_size) % 1000 == 0)
            {
                std::cout << "\r(" << e + 1 << "/" << epoch << ")epoch training... " << std::fixed << std::setprecision(2) << double((total_size - remain_size) * 100.0) / total_size << " %" << std::flush;
            }
        }
        dl.sample(batch_ids, remain_size);
        add_to_rank(std::ref(que), batch_ids, remain_size);
        embedding_t *ret = prefetch(std::ref(dmap), std::ref(smap), batch_ids, remain_size, num_worker, std::ref(access_count), std::ref(hit_count), std::ref(que), k_size);
        get_embs(ret, remain_size, num_worker);
        remain_size -= remain_size;

        double curr = double((e + 1) * 100.0) / epoch;
        std::cout << "\rtraining... " << std::fixed << std::setprecision(2) << curr << " %" << std::flush;
        // TODO: printf("\r迭代中[%.2lf%%]:", curr);
    }
    auto point1 = std::chrono::high_resolution_clock::now(); //训练时间
    delete[] batch_ids;
    save_ssd(std::ref(dmap), std::ref(smap), std::ref(dl));
    auto point2 = std::chrono::high_resolution_clock::now(); //持久化时间
    running = false;
    // th.join();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_train(point1 - start);
    std::chrono::duration<double, std::ratio<1, 1>> duration_save(point2 - point1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_total(end - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_readid(ts2 - ts1);
    LOGINFO << "read id time = " << duration_readid.count() << " s" << std::flush;
    LOGINFO << "train time = " << duration_train.count() << " s" << std::flush;
    LOGINFO << "save time = " << duration_save.count() << " s" << std::flush;
    LOGINFO << "total time = " << duration_total.count() << " s" << std::flush;

    LOGINFO << "total hit rate = " << double(hit_count * 100.0) / access_count << " %" << std::endl
            << std::flush;
    return 0;
}
#include <thread>
#include <vector>
#include "../justokmap/shard_lock_map.h"
#include "get_embeddings.h"
#include "../utils/logs.h"

void get_embs(double *batch_emb_ptrs[EMB_LEN], size_t batch_size, size_t num_workers)
{
    size_t work_size = size_t(batch_size + num_workers - 1 / num_workers); //上取整
    std::vector<std::thread> workers;
    for (size_t w = 1; w < num_workers; ++w)
    {
        workers.emplace_back(output, batch_emb_ptrs, (w - 1) * work_size, w * work_size);
    }
    // auto start = std::chrono::high_resolution_clock::now();
    output(batch_emb_ptrs, (num_workers - 1) * work_size, batch_size); //自己也要干活(也许会少点)
    for (auto &worker : workers)
    {
        worker.join();
    }
    // auto end = std::chrono::high_resolution_clock::now();
}

// 输出和更新，模拟系统中的output
void output(double *batch_emb_ptrs[EMB_LEN], size_t begin, size_t end)
{
    for (size_t i = begin; i < end; ++i)
    {
        double *vec = batch_emb_ptrs[i];
        // LOGINFO << "emb[" << i << "]:\t";
        for (size_t c = 0; c < 100; ++c) //加100次1.0模拟计算
        {
            for (size_t j = 0; j < EMB_LEN; ++j)
            {
                // std::cout << vec[j] << ", ";
                vec[j] += 1.0;
            }
        }
        // std::cout << std::endl;
    }
}
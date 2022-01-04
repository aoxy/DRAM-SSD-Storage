#include <thread>
#include <vector>
#include <cstring>
#include "../justokmap/shard_lock_map.h"
#include "get_embeddings.h"
#include "../utils/logs.h"

void get_embs(double *batch_emb_ptrs[EMB_LEN], size_t batch_size, size_t num_workers)
{
    update_embs(batch_emb_ptrs, batch_size);
    /*
    size_t work_size = size_t((batch_size + num_workers - 1) / num_workers); //上取整
    std::vector<std::thread> workers;
    for (size_t w = 1; w < num_workers; ++w)
    {
        workers.emplace_back(output, batch_emb_ptrs, (w - 1) * work_size, w * work_size);
    }
    output(batch_emb_ptrs, (num_workers - 1) * work_size, batch_size); //自己也要干活(也许会少点)
    for (auto &worker : workers)
    {
        worker.join();
    }
    */
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

void update_embs(double *batch_emb_ptrs[EMB_LEN], size_t batch_size)
{
    const size_t av_len = EMB_LEN >> 1;
    const size_t conti_len = av_len * batch_size;
    double *conti_space_a = new double[av_len * batch_size];
    double *conti_space_v = new double[av_len * batch_size];
    if (conti_space_a == nullptr || conti_space_v == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
    }
    const size_t av_emb_size = av_len * sizeof(double);
    double g = 1.01;
    // 拷贝到连续内存空间
    for (size_t i = 0; i < batch_size; ++i)
    {
        std::memcpy(conti_space_a + i * av_len, batch_emb_ptrs[i], av_emb_size);
        std::memcpy(conti_space_v + i * av_len, batch_emb_ptrs[i] + av_len, av_emb_size);
    }

    //实际的计算
    for (size_t i = 0; i < conti_len; ++i)
    {
        conti_space_a[i] += g * g;
        conti_space_v[i] -= 0.1 * g * (1.0 / sqrt(conti_space_a[i]));
    }

    // 从连续内存空间拷贝回去更新
    for (size_t i = 0; i < batch_size; ++i)
    {
        std::memcpy(batch_emb_ptrs[i], conti_space_a + i * av_len, av_emb_size);
        std::memcpy(batch_emb_ptrs[i] + av_len, conti_space_v + i * av_len, av_emb_size);
    }

    delete[] conti_space_a;
    delete[] conti_space_v;
}
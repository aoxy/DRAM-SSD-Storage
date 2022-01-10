#include <cstring>
#include <iostream>
#include <chrono>
#include <cmath>

#include "../utils/logs.h"

#define EMB_LEN 32

typedef double *embedding_t;

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

void print_embs(double *batch_emb_ptrs[EMB_LEN], size_t batch_size)
{
    for (size_t b = 0; b < batch_size; ++b)
    {
        LOGINFO << "emb" << b << ": " << std::endl;
        double *emb = batch_emb_ptrs[b];
        for (size_t i = 0; i < EMB_LEN; ++i)
        {
            std::cout << emb[i] << ", ";
        }
        std::cout << std::endl;
    }
}

int main()
{
    const size_t batch_size = 128;
    const size_t epoch = 3;
    embedding_t *batch_emb_ptrs = new embedding_t[batch_size];

    for (size_t b = 0; b < batch_size; ++b)
    {
        double *emb = new double[EMB_LEN];
        if (emb == nullptr)
        {
            LOGINFO << "malloc failed." << std::endl;
            exit(1);
        }
        for (size_t i = 0; i < EMB_LEN; ++i)
        {
            emb[i] = 1.1;
        }
        batch_emb_ptrs[b] = emb;
    }

    print_embs(batch_emb_ptrs, batch_size);
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t e = 0; e < epoch; ++e)
    {
        for (size_t i = 0; i < 26557961; i += batch_size)
        {
            update_embs(batch_emb_ptrs, batch_size);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    print_embs(batch_emb_ptrs, batch_size);
    std::chrono::duration<double, std::ratio<1, 1>> duration_cal(end - start);
    LOGINFO << "update time = " << duration_cal.count() << " s" << std::flush;
    // (EMB_LEN 16)update time = 17.7331 s
    // (EMB_LEN 32)update time = 32.1123 s
    return 0;
}
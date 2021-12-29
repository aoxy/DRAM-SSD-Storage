#pragma once
#include <thread>
#include <vector>

void get_embs(double *batch_emb_ptrs[EMB_LEN], size_t batch_size, size_t num_workers);
// 输出和更新，模拟系统中的output
void output(double *batch_emb_ptrs[EMB_LEN], size_t begin, size_t end);
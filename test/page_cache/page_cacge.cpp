#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>

#include "get_mem.h"
#include "emb_file.h"
#include "../utils/logs.h"

template <typename T>
void *get_value_ptr(int total_dims, T value)
{
    T *value_ptr = (T *)malloc(total_dims * sizeof(T));
    assert(value_ptr != nullptr);
    for (int i = 0; i < total_dims; ++i)
    {
        value_ptr[i] = value;
    }
    return (void *)value_ptr;
}

template <typename T>
bool check_equal(void *value_ptr, int total_dims, T value)
{
    for (int i = 0; i < total_dims; ++i)
    {
        if (*((T *)value_ptr + i) != value)
        {
            return false;
        }
    }
    return true;
}

int main()
{
    const int total_dims = 128;
    int val_len = total_dims * sizeof(float);
    size_t curr_offset = 0;

    LOGINFO << "val_len = " << val_len;
    int ids_num = (12 << 20) / val_len; // 12 MiB
    LOGINFO << "ids_num = " << ids_num;
    int log_mod = ids_num / 100;
    LOGINFO << "log_mod = " << log_mod;
    std::vector<int> ids(ids_num);
    for (int i = 0; i < ids_num; i++)
    {
        ids[i] = i + 1000;
    }
    std::map<int, int> offset_map;
    EmbFile file("pc_test");
    for (int i = 0; i < ids_num; i++)
    {
        int key = ids[i];
        void *value_ptr = get_value_ptr(total_dims, (float)key);
        offset_map[key] = curr_offset;
        curr_offset += val_len;
        file.Write((char *)value_ptr, val_len);
        if (i % int(ids_num / 100) == 0)
        {
            LOGINFO << i << "\tWrite read_proc_memory = " << read_proc_memory();
        }
    }
    // 关闭文件
    file.fs.close();
    void *read_ptr = get_value_ptr(total_dims, 0.0);
    srand((unsigned)time(NULL));
    for (int i = 0; i < 100000; i++) // 100w次随机访问
    {
        int key = ids[rand() % ids_num];
        int offset = offset_map[key];
        file.Read((char *)read_ptr, val_len, offset);
        if (!check_equal(read_ptr, total_dims, float(key)))
        {
            LOGINFO << key << " -> R/W Error!";
        }
        if (i % int(100000 / 100) == 0)
        {
            LOGINFO << i << "\tRead read_proc_memory = " << read_proc_memory();
        }
    }

    srand((unsigned)time(NULL));
    LOGINFO << "read_proc_memory = " << read_proc_memory();
}
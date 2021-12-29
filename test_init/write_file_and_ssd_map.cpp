#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"

inline bool exists(const std::string &filepath)
{
    std::ifstream f(filepath);
    return f.good();
}

int main()
{
    std::ifstream ifs("storage/all_userid.txt");
    std::vector<int64_t> ids;
    int64_t key;
    auto ts1 = std::chrono::high_resolution_clock::now();
    while (ifs >> key)
    {
        ids.push_back(key);
    }
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::cout << "data size: " << ids.size() << std::endl
              << std::flush;
    ssd_hash_map smap;
    embedding_t value = new double[EMB_LEN];
    if (value == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
    }
    for (size_t i = 0; i < EMB_LEN; ++i)
    {
        value[i] = 1.1;
    }
    std::string offset_map_file = "storage/offset.txt";
    std::remove(offset_map_file.c_str());
    std::ofstream ofs_off(offset_map_file, std::ios::app | std::ios::binary);

    std::vector<bool> has_old_storage_file(true, NUM_SHARD);

    auto ts3 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ids.size(); ++i)
    {
        if (i % 4096 == 0 || i == ids.size() - 1)
        {
            double curr = double((i + 1) * 100.0) / ids.size();
            std::cout << "\rwriting embedding... " << std::fixed << std::setprecision(2) << curr << " %" << std::flush;
        }
        int64_t key = ids[i];
        std::string filepath = smap.filepath(key);
        size_t file_idx = smap.shard_idx(key);
        if (has_old_storage_file[file_idx] && exists(filepath))
        {
            std::remove(filepath.c_str());
            has_old_storage_file[file_idx] = false;
        }
        std::ofstream ofs(filepath, std::ios::app | std::ios::binary);
        ofs.seekp(0, std::ios::end);
        size_t offset = size_t(ofs.tellp()) + 1;
        smap.set(key, offset); //存的时候+1了
        ofs.write((char *)value, EMB_LEN * sizeof(double));
        ofs.close();
        ofs_off << key << ' ' << offset << std::endl;
    }
    auto ts4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_id(ts2 - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_emb(ts4 - ts3);
    std::cout << std::endl;
    std::cout << "read id time: " << duration_id.count() << " s" << std::endl;
    std::cout << "wirte embedding time: " << duration_emb.count() << " s" << std::endl;
}
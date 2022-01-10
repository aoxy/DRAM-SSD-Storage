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
    std::ifstream ifs_u("storage/user/all_userid.txt");
    std::ifstream ifs_a("storage/ad/all_adgroupid.txt");

    std::vector<int64_t> ids_u;
    std::vector<int64_t> ids_a;
    int64_t key;
    auto ts1 = std::chrono::high_resolution_clock::now();
    while (ifs_u >> key)
    {
        ids_u.push_back(key);
    }
    while (ifs_a >> key)
    {
        ids_a.push_back(key);
    }
    auto ts2 = std::chrono::high_resolution_clock::now();
    std::cout << "user data size: " << ids_u.size() << std::endl
              << std::flush;
    std::cout << "ad data size: " << ids_a.size() << std::endl
              << std::flush;
    ssd_hash_map smap_u("user");
    ssd_hash_map smap_a("ad");
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
    std::string offset_map_file_u = "storage/user/offset.txt";
    std::remove(offset_map_file_u.c_str());
    std::ofstream ofs_off_u(offset_map_file_u, std::ios::app | std::ios::binary);
    std::vector<bool> has_old_storage_file_u(true, NUM_SHARD);

    auto ts3 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ids_u.size(); ++i)
    {
        if (i % 4096 == 0 || i == ids_u.size() - 1)
        {
            double curr = double((i + 1) * 100.0) / ids_u.size();
            std::cout << "\rwriting user embedding... " << std::fixed << std::setprecision(2) << curr << " %" << std::flush;
        }
        int64_t key = ids_u[i];
        std::string filepath = smap_u.filepath(key);
        size_t file_idx = smap_u.shard_idx(key);
        if (has_old_storage_file_u[file_idx] && exists(filepath))
        {
            std::remove(filepath.c_str());
            has_old_storage_file_u[file_idx] = false;
        }
        std::ofstream ofs(filepath, std::ios::app | std::ios::binary);
        ofs.seekp(0, std::ios::end);
        size_t offset = size_t(ofs.tellp()) + 1;
        smap_u.set(key, offset); //存的时候+1了
        ofs.write((char *)value, EMB_LEN * sizeof(double));
        ofs.close();
        ofs_off_u << key << ' ' << offset << std::endl;
    }
    auto ts4 = std::chrono::high_resolution_clock::now();
    std::cout << std::endl
              << std::flush;

    std::string offset_map_file_a = "storage/ad/offset.txt";
    std::remove(offset_map_file_a.c_str());
    std::ofstream ofs_off_a(offset_map_file_a, std::ios::app | std::ios::binary);
    std::vector<bool> has_old_storage_file_a(true, NUM_SHARD);

    auto ts5 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ids_a.size(); ++i)
    {
        if (i % 4096 == 0 || i == ids_a.size() - 1)
        {
            double curr = double((i + 1) * 100.0) / ids_a.size();
            std::cout << "\rwriting adgroup embedding... " << std::fixed << std::setprecision(2) << curr << " %" << std::flush;
        }
        int64_t key = ids_a[i];
        std::string filepath = smap_a.filepath(key);
        size_t file_idx = smap_a.shard_idx(key);
        if (has_old_storage_file_a[file_idx] && exists(filepath))
        {
            std::remove(filepath.c_str());
            has_old_storage_file_a[file_idx] = false;
        }
        std::ofstream ofs(filepath, std::ios::app | std::ios::binary);
        ofs.seekp(0, std::ios::end);
        size_t offset = size_t(ofs.tellp()) + 1;
        smap_a.set(key, offset); //存的时候+1了
        ofs.write((char *)value, EMB_LEN * sizeof(double));
        ofs.close();
        ofs_off_a << key << ' ' << offset << std::endl;
    }
    auto ts6 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_id(ts2 - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_emb_user(ts4 - ts3);
    std::chrono::duration<double, std::ratio<1, 1>> duration_emb_ad(ts6 - ts5);
    std::cout << std::endl;
    std::cout << "read id time: " << duration_id.count() << " s" << std::endl;
    std::cout << "wirte user embedding time: " << duration_emb_user.count() << " s" << std::endl;
    std::cout << "wirte adgroup embedding time: " << duration_emb_ad.count() << " s" << std::endl;
}
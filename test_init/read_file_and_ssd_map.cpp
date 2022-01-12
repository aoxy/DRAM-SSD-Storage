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
#include "../utils/logs.h"

void init_ssd_map(ssd_hash_map &smap)
{
    auto feature_name = smap.feature_name();
    const std::string filepath = std::string("storage/") + feature_name + std::string("/offset.txt");
    std::ifstream fp(filepath);
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

int main()
{
    std::ifstream ifs_u("storage/user/all_ids.txt");
    std::ifstream ifs_a("storage/ad/all_ids.txt");
    std::vector<int64_t> ids_u;
    std::vector<int64_t> ids_a;
    int64_t key;
    const size_t one_emb_size = sizeof(int64_t) + EMB_LEN * sizeof(double);
    auto ts1 = std::chrono::high_resolution_clock::now();
    while (ifs_u >> key)
    {
        ids_u.push_back(key);
    }
    while (ifs_a >> key)
    {
        ids_a.push_back(key);
    }
    auto ts6 = std::chrono::high_resolution_clock::now();
    std::cout << "user data size: " << ids_u.size() << std::endl
              << std::flush;
    std::cout << "ad data size: " << ids_a.size() << std::endl
              << std::flush;
    ssd_hash_map smap_u("user");
    ssd_hash_map smap_a("ad");
    init_ssd_map(std::ref(smap_u));
    init_ssd_map(std::ref(smap_a));
    embedding_t value = new double[EMB_LEN];
    if (value == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
        exit(1);
    }

    std::string emb_out_file_u = "storage/user/embeddings.txt";
    std::ofstream ofs_off_u(emb_out_file_u, std::ios::binary);
    std::vector<std::ifstream> ofs_list_u(NUM_SHARD);
    for (size_t i = 0; i < NUM_SHARD; ++i)
    {
        ofs_list_u[i] = std::ifstream(smap_u.filepath(i), std::ios::binary);
    }
    auto ts2 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ids_u.size(); ++i)
    {
        if (i % 4096 == 0 || i == ids_u.size() - 1)
        {
            double curr = double((i + 1) * 100.0) / ids_u.size();
            std::cout << "\rreading user embedding... " << std::fixed << std::setprecision(2) << curr << " %" << std::flush;
        }
        int64_t key = ids_u[i];
        size_t file_idx = smap_u.shard_idx(key);

        size_t offset = smap_u.get(key) - 1;
        ofs_list_u[file_idx].seekg(offset, std::ios::beg);
        ofs_list_u[file_idx].read((char *)value, EMB_LEN * sizeof(double));
        ofs_off_u << key << ": \t";
        for (size_t j = 0; j < EMB_LEN; ++j)
        {
            ofs_off_u << value[j] << " ";
        }
        ofs_off_u << std::endl;
    }
    auto ts3 = std::chrono::high_resolution_clock::now();
    for (auto &ofs : ofs_list_u)
    {
        ofs.close();
    }
    std::cout << std::endl
              << std::flush;

    std::string emb_out_file_a = "storage/ad/embeddings.txt";
    std::ofstream ofs_off_a(emb_out_file_a, std::ios::binary);
    std::vector<std::ifstream> ofs_list_a(NUM_SHARD);
    for (size_t i = 0; i < NUM_SHARD; ++i)
    {
        ofs_list_a[i] = std::ifstream(smap_a.filepath(i), std::ios::binary);
    }
    auto ts4 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ids_a.size(); ++i)
    {
        if (i % 4096 == 0 || i == ids_a.size() - 1)
        {
            double curr = double((i + 1) * 100.0) / ids_a.size();
            std::cout << "\rreading adgroup embedding... " << std::fixed << std::setprecision(2) << curr << " %" << std::flush;
        }
        int64_t key = ids_a[i];
        size_t file_idx = smap_a.shard_idx(key);

        size_t offset = smap_a.get(key) - 1;
        ofs_list_a[file_idx].seekg(offset, std::ios::beg);
        ofs_list_a[file_idx].read((char *)value, EMB_LEN * sizeof(double));
        ofs_off_a << key << ": \t";
        for (size_t j = 0; j < EMB_LEN; ++j)
        {
            ofs_off_a << value[j] << " ";
        }
        ofs_off_a << std::endl;
    }
    auto ts5 = std::chrono::high_resolution_clock::now();
    for (auto &ofs : ofs_list_a)
    {
        ofs.close();
    }

    std::chrono::duration<double, std::ratio<1, 1>> duration_readid(ts6 - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_read_user_emb(ts3 - ts2);
    std::chrono::duration<double, std::ratio<1, 1>> duration_read_ad_emb(ts5 - ts4);
    std::cout << std::endl;
    std::cout << "read id time: " << duration_readid.count() << " s" << std::endl;
    std::cout << "read user embedding time: " << duration_read_user_emb.count() << " s" << std::endl;
    std::cout << "read adgroup embedding time: " << duration_read_ad_emb.count() << " s" << std::endl;
}
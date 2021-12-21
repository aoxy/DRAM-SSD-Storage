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
    const std::string filepath = "storage/offset.txt";
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
    std::ifstream ifs("storage/all_userid.txt");
    std::vector<int64_t> ids;
    int64_t key;
    auto ts1 = std::chrono::high_resolution_clock::now();
    while (ifs >> key)
    {
        ids.push_back(key);
    }
    std::cout << "data size: " << ids.size() << std::endl
              << std::flush;
    ssd_hash_map smap;
    init_ssd_map(std::ref(smap));
    embedding_t value = new double[EMB_LEN];
    std::string emb_out_file = "storage/embeddings.txt";
    std::ofstream ofs_off(emb_out_file, std::ios::binary);
    auto ts2 = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < ids.size(); ++i)
    {
        if (i % 4096 == 0 || i == ids.size() - 1)
        {
            double curr = double((i + 1) * 100.0) / ids.size();
            std::cout << "\rreading embedding... " << std::fixed << std::setprecision(2) << curr << " %" << std::flush;
        }
        int64_t key = ids[i];
        std::string filepath = smap.filepath(key);
        std::ifstream ifs(filepath, std::ios::binary);

        size_t offset = smap.get(key) - 1;
        ifs.seekg(offset, std::ios::beg);
        ifs.read((char *)value, EMB_LEN * sizeof(double));
        ofs_off << key << ": \t";
        for (size_t j = 0; j < EMB_LEN; ++j)
        {
            ofs_off << value[j] << " ";
        }
        ofs_off << std::endl;
        ifs.close();
    }
    auto ts3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_readid(ts2 - ts1);
    std::chrono::duration<double, std::ratio<1, 1>> duration_reademb(ts3 - ts2);
    std::cout << std::endl;
    std::cout << "read id time: " << duration_readid.count() << " s" << std::endl;
    std::cout << "read embedding time: " << duration_reademb.count() << " s" << std::endl;
}
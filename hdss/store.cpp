#include <string>
#include <fstream>
#include "store.h"
#include "../justokmap/shard_lock_map.h"
#include "../justokmap/ssd_hash_map.h"
#include "../movement/files.h"
#include "../utils/logs.h"

void init_ssd_map(ssd_hash_map &smap)
{
    std::string feature_name = smap.feature_name();
    const std::string filepath = std::string("storage/") + feature_name + std::string("/offset.txt");
    std::ifstream fp(filepath);

    if (!fp)
    {
        LOGINFO << "open " << filepath << " fail." << std::endl
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

void compaction(shard_lock_map &dmap, ssd_hash_map &smap, FilePool &old_fp)
{
    auto feature_name = smap.feature_name();
    size_t offset;
    size_t file_idx;
    embedding_t value;
    const std::string ids_file = std::string("storage/") + feature_name + std::string("/all_ids.txt");
    std::ifstream ifs(ids_file);
    std::vector<int64_t> ids;
    int64_t key;
    while (ifs >> key)
    {
        ids.push_back(key);
    }
    ifs.close();
    std::string offset_map_file = std::string("storage/") + feature_name + std::string("/offset.txt");
    std::remove(offset_map_file.c_str());
    std::ofstream ofs_off(offset_map_file, std::ios::app | std::ios::binary);
    for (int64_t &key : ids)
    {
        value = dmap.get(key);
        if (value == nullptr) // 不在内存里，先读进来
        {
            value = new double[EMB_LEN];
            assert(value != nullptr && "allocate failed in compaction.");
            offset = smap.get(key) - 1; //offset - 1 是因为存的时候+1了
            old_fp.rw_s(key).seekg(offset, std::ios::beg);
            old_fp.rw_s(key).read((char *)value, EMB_LEN * sizeof(double));
            dmap.set(key, value);
            dmap.increase();
        }
    }
    old_fp.close();
    for (size_t i = 0; i < NUM_SHARD; ++i)
    {
        std::remove(smap.filepath(i).c_str());
    }
    FilePool new_fp(smap);
    for (int64_t &key : ids)
    {
        value = dmap.get(key);
        assert(value != nullptr && "unknown error failed.");
        dmap.set(key, nullptr);
        dmap.decrease();
        offset = size_t(new_fp.rw_s(key).tellp()) + 1; //存的时候+1了
        new_fp.rw_s(key).write((char *)value, EMB_LEN * sizeof(double));
        ofs_off << key << ' ' << offset << std::endl;
    }
}
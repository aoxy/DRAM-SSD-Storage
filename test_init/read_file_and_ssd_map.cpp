#include <iostream>
#include <cstdio>
#include <thread>
#include <queue>
#include <fstream>
#include <string>
#include <sstream>
#include "../justokmap/ssd_hash_map.h"
#include "../justokmap/shard_lock_map.h"

class dataloder
{
public:
    std::vector<int64_t> ids;
    size_t offset;
    dataloder(std::string filepath, size_t feature_id)
    {
        // std::cout << filepath << std::endl;
        std::ifstream fp(filepath); //定义声明一个ifstream对象，指定文件路径
        if (!fp)
        {
            std::cout << "open fail." << std::endl;
            exit(1);
        }
        std::string line;
        std::getline(fp, line); //跳过列名，第一行不做处理
        // std::cout << line;
        while (std::getline(fp, line))
        { //循环读取每行数据
            std::string number;
            std::istringstream readstr(line); //string数据流化
            //将一行数据按'，'分割
            for (size_t j = 0; j <= feature_id; ++j)
            {                                       //可根据数据的实际情况取循环获取
                std::getline(readstr, number, ','); //循环读取数据
            }
            ids.push_back(std::atoi(number.c_str())); //插入到vector中
        }
        offset = 0;
    }
    void init() { offset = 0; }
    void sample(int64_t *batch_ids, size_t batch_size)
    {
        // int64_t *batch_ids = new int64_t[batch_size];
        for (size_t i = 0; offset < ids.size() && i < batch_size; ++offset, ++i)
        {
            batch_ids[i] = ids[offset];
        }
        // return batch_ids;
    }
    size_t size() { return ids.size(); }
};

void init_ssd_map(ssd_hash_map &smap)
{
    const std::string filepath = "storage/offset.txt";
    std::ifstream fp(filepath); //定义声明一个ifstream对象，指定文件路径
    if (!fp)
    {
        std::cout << "open fail." << std::endl;
        exit(1);
    }
    int64_t key;
    size_t value;
    std::string line;
    while (std::getline(fp, line))
    { //循环读取每行数据
        std::string number;
        std::istringstream readstr(line); //string数据流化
        //将一行数据按'，'分割
        std::getline(readstr, number, ' ');
        key = std::atoi(number.c_str());
        std::getline(readstr, number, ' ');
        value = std::atoi(number.c_str());
        // std::cout << key << " " << value << std::endl;
        smap.set(key, value);
    }
}

int main()
{
    dataloder dl("../dataset/taobao/user_profile.csv", 0);
    std::cout << "data size: " << dl.size() << std::endl
              << std::flush;
    ssd_hash_map smap;
    init_ssd_map(std::ref(smap));
    embedding_t value = new double[EMB_LEN];
    std::string emb_out_file = "storage/embeddings.txt";
    std::ofstream ofs_off(emb_out_file, std::ios::binary);

    for (size_t i = 0; i < dl.ids.size(); ++i)
    {
        if (i % 4096 == 0)
        {
            double curr = double(i * 100.0) / dl.ids.size();
            printf("\r读取中[%.2lf%%]:", curr);
        }
        int64_t key = dl.ids[i];
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
}
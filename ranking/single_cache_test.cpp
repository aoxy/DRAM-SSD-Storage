#include <iostream>
#include <fstream>
#include <iomanip>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include "cache.h"
#include "../hdss/store.h"

int main(int argc, char *argv[])
{
    SingleConfig conf(argc, argv);
    auto begin = std::chrono::high_resolution_clock::now();
    DataLoader dl = *conf.dl;
    SingleCache<int64_t, bool> *cache = conf.cache;
    CacheRecord cr;

    // 一次访问
    {
        cr.clear();
        for (size_t i = 0; i < dl.size(); i++)
        {
            cr.access();
            if (cache->get(dl.at(i)))
            {
                cr.hit();
            }
            cache->set(dl.at(i), true);
            if (conf.verbose && i % 8000 == 0)
            {
                std::cout << "\r[1]cache testing... " << std::fixed << std::setprecision(2) << double(i * 100.0) / dl.size() << " %" << std::flush;
            }
        }
        if (conf.verbose)
        {
            std::cout << std::endl;
        }
        std::cout << "[1]total hit rate = " << cr.hit_rate() << " %" << std::endl
                  << std::flush;
    }

    // 找出最高频id
    {
        cr.clear();
        for (size_t i = 0; i < dl.size(); i++)
        {
            cr.access();
            if (cache->get(dl.at(i)))
            {
                cr.hit();
            }
            if (conf.verbose && i % 8000 == 0)
            {
                std::cout << "\r[2]cache testing... " << std::fixed << std::setprecision(2) << double(i * 100.0) / dl.size() << " %" << std::flush;
            }
        }
        if (conf.verbose)
        {
            std::cout << std::endl;
        }
        std::cout << "[2]total hit rate = " << cr.hit_rate() << " %" << std::endl
                  << std::flush;
    }

    // 预热后动态调整cache
    {
        cr.clear();
        for (size_t i = 0; i < dl.size(); i++)
        {
            cr.access();
            if (cache->get(dl.at(i)))
            {
                cr.hit();
            }
            else
            {
                cache->set(dl.at(i), true);
            }
            if (conf.verbose && i % 8000 == 0)
            {
                std::cout << "\r[3]cache testing... " << std::fixed << std::setprecision(2) << double(i * 100.0) / dl.size() << " %" << std::flush;
            }
        }
        if (conf.verbose)
        {
            std::cout << std::endl;
        }
        std::cout << "[3]total hit rate = " << cr.hit_rate() << " %" << std::endl
                  << std::flush;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::ratio<1, 1>> duration_zerotrain(end - begin);
    std::cout << "cost time = " << duration_zerotrain.count() << " s" << std::endl
              << std::flush;
}
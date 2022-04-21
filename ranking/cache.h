#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include "utils/logs.h"

class CacheRecord
{
private:
    size_t access_count;
    size_t hit_count;

public:
    CacheRecord()
    {
        access_count = 0;
        hit_count = 0;
    }
    void access() { ++access_count; }
    void hit() { ++hit_count; }
    double hit_rate()
    {
        return double(hit_count * 100.0) / access_count;
    }
    void clear()
    {
        access_count = 0;
        hit_count = 0;
    }
};

template <class K>
class BatchCache
{
protected:
    size_t capacity;

public:
    BatchCache(size_t cap = 16) : capacity(cap) {}
    virtual size_t get_evic_ids(K *evic_ids, size_t k_size) = 0;
    virtual void add_to_rank(const K *batch_ids, size_t batch_size) = 0;
    virtual size_t size() = 0;
    size_t max_emb_num() const
    {
        return capacity;
    }
};

template <class K>
class LRUCache : public BatchCache<K>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;
    LRUCache(size_t cap) : BatchCache<K>(cap)
    {
        mp.clear();
        ls.clear();
    }

    size_t size()
    {
        return mp.size();
    }

    size_t get_evic_ids(K *evic_ids, size_t k_size)
    {
        size_t true_size = 0;
        for (size_t i = 0; i < k_size; ++i)
        {
            if (ls.size() > 0)
            {
                evic_ids[true_size] = ls.back();
                ls.pop_back();
                mp.erase(evic_ids[true_size]);
                true_size++;
            }
        }
        return true_size;
    }

    void add_to_rank(const K *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            K id = batch_ids[i];
            typename std::unordered_map<K, list_iterator>::iterator it = mp.find(id);
            if (it != mp.end())
            {
                ls.erase(it->second);
            }
            ls.push_front(id);
            mp[id] = ls.begin();
        }
    }

private:
    std::unordered_map<K, list_iterator> mp;
    std::list<K> ls;
};

template <class K>
class LFUCache : public BatchCache<K>
{
private:
    class LFUNode
    {
    public:
        K key;
        size_t freq;
        LFUNode(K key, size_t freq) : key(key), freq(freq) {}
    };
    size_t min_freq;
    size_t max_freq;
    std::unordered_map<K, typename std::list<LFUNode>::iterator> key_table;
    std::unordered_map<K, typename std::list<LFUNode>> freq_table;

public:
    LFUCache(size_t cap) : BatchCache<K>(cap)
    {
        min_freq = 0;
        max_freq = 0;
        key_table.clear();
        freq_table.clear();
    }

    size_t size()
    {
        return key_table.size();
    }

    size_t get_evic_ids(K *evic_ids, size_t k_size)
    {
        size_t true_size = 0;
        for (size_t i = 0; i < k_size; ++i)
        {
            auto rm_it = freq_table[min_freq].back();
            key_table.erase(rm_it.key);
            evic_ids[i] = rm_it.key;
            ++true_size;
            freq_table[min_freq].pop_back();
            if (freq_table[min_freq].size() == 0)
            {
                freq_table.erase(min_freq);
                ++min_freq;
                while (min_freq <= max_freq)
                {
                    auto it = freq_table.find(min_freq);
                    if (it == freq_table.end() || it->second.size() == 0)
                    {
                        ++min_freq;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        return true_size;
    }

    void add_to_rank(const K *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            K id = batch_ids[i];
            auto it = key_table.find(id);
            if (it == key_table.end())
            {
                freq_table[1].push_front(LFUNode(id, 1));
                key_table[id] = freq_table[1].begin();
                min_freq = 1;
            }
            else
            {
                typename std::list<LFUNode>::iterator node = it->second;
                size_t freq = node->freq;
                freq_table[freq].erase(node);
                if (freq_table[freq].size() == 0)
                {
                    freq_table.erase(freq);
                    if (min_freq == freq)
                        min_freq += 1;
                }
                max_freq = std::max(max_freq, freq + 1);
                freq_table[freq + 1].push_front(LFUNode(id, freq + 1));
                key_table[id] = freq_table[freq + 1].begin();
            }
        }
    }
};

template <class K>
class FIFOCache : public BatchCache<K>
{
public:
    using fifo_iterator = typename std::list<K>::const_iterator;

    FIFOCache(size_t cap) : BatchCache<K>(cap)
    {
        fifo_queue.clear();
        key_lookup.clear();
    }

    size_t size()
    {
        LOGINFO << "key_lookup.size() = " << key_lookup.size() << ", fifo_queue.size() = " << fifo_queue.size();
        return key_lookup.size();
    }

    size_t get_evic_ids(K *evic_ids, size_t k_size)
    {
        size_t true_size = 0;
        for (size_t i = 0; i < k_size; ++i)
        {
            auto rm_it = fifo_queue.back();
            evic_ids[true_size++] = rm_it;
            key_lookup.erase(rm_it);
            fifo_queue.pop_back();
        }
        return true_size;
    }
    void add_to_rank(const K *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            K id = batch_ids[i];
            auto it = key_lookup.find(id);
            if (it == key_lookup.end())
            {
                fifo_queue.emplace_front(id);
                key_lookup[id] = fifo_queue.begin();
            }
            else
            {
                fifo_queue.erase(it->second);
                fifo_queue.emplace_front(id);
                key_lookup[id] = fifo_queue.begin();
            }
        }
    }

private:
    std::list<K> fifo_queue;
    std::unordered_map<K, fifo_iterator> key_lookup;
};
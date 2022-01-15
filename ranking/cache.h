#pragma once
#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <list>

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

class BatchCache
{
protected:
    size_t capacity;

public:
    BatchCache(size_t cap = 16) : capacity(cap) {}
    virtual size_t get_evic_ids(int64_t *evic_ids, size_t k_size) = 0;
    virtual void add_to_rank(int64_t *batch_ids, size_t batch_size) = 0;
    size_t max_emb_num() const
    {
        return capacity;
    }
};

class LRUCache : public BatchCache
{
private:
    class LRUNode
    {
    public:
        int64_t id;
        LRUNode *pre, *next;
        LRUNode(int64_t id) : id(id), pre(nullptr), next(nullptr) {}
    };
    LRUNode *head, *tail;
    std::map<int64_t, LRUNode *> mp;

public:
    LRUCache(size_t cap) : BatchCache(cap)
    {
        mp.clear();
        head = new LRUNode(0);
        tail = new LRUNode(0);
        head->next = tail;
        tail->pre = head;
    }

    size_t get_evic_ids(int64_t *evic_ids, size_t k_size)
    {
        size_t true_size = 0;
        LRUNode *evic_node = tail->pre;
        LRUNode *rm_node = evic_node;
        for (size_t i = 0; i < k_size && evic_node != head; ++i)
        {
            evic_ids[i] = evic_node->id;
            rm_node = evic_node;
            evic_node = evic_node->pre;
            mp.erase(rm_node->id);
            delete rm_node;
            true_size++;
        }
        evic_node->next = tail;
        tail->pre = evic_node;
        return true_size;
    }
    void add_to_rank(int64_t *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            int64_t id = batch_ids[i];
            std::map<int64_t, LRUNode *>::iterator it = mp.find(id);
            if (it != mp.end())
            {
                LRUNode *node = it->second;
                node->pre->next = node->next;
                node->next->pre = node->pre;
                head->next->pre = node;
                node->next = head->next;
                head->next = node;
                node->pre = head;
            }
            else
            {
                LRUNode *newNode = new LRUNode(id);
                if (mp.size() >= capacity)
                {
                    LRUNode *rm_node = tail->pre;
                    std::map<int64_t, LRUNode *>::iterator iter = mp.find(rm_node->id);
                    tail->pre = rm_node->pre;
                    rm_node->pre->next = tail;
                    delete rm_node;
                    mp.erase(iter);
                }
                head->next->pre = newNode;
                newNode->next = head->next;
                head->next = newNode;
                newNode->pre = head;
                mp[id] = newNode;
            }
        }
    }
};

class LFUCache : public BatchCache
{
private:
    class LFUNode
    {
    public:
        int64_t key;
        size_t freq;
        LFUNode(int64_t key, size_t freq) : key(key), freq(freq) {}
    };
    size_t min_freq;
    std::unordered_map<int64_t, std::list<LFUNode>::iterator> key_table;
    std::unordered_map<int64_t, std::list<LFUNode>> freq_table;

public:
    LFUCache(size_t cap) : BatchCache(cap)
    {
        min_freq = 0;
        key_table.clear();
        freq_table.clear();
    }
    size_t max_emb_num()
    {
        return capacity;
    }

    size_t get_evic_ids(int64_t *evic_ids, size_t k_size)
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
                auto it = freq_table.find(min_freq);
                while (it != freq_table.end() && it->second.size() == 0)
                {
                    ++min_freq;
                    it = freq_table.find(min_freq);
                }
                if (it == freq_table.end())
                {
                    break;
                }
            }
        }
        return true_size;
    }

    void add_to_rank(int64_t *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            int64_t id = batch_ids[i];
            auto it = key_table.find(id);
            if (it == key_table.end())
            {
                if (key_table.size() == capacity)
                {
                    auto it2 = freq_table[min_freq].back();
                    key_table.erase(it2.key);
                    freq_table[min_freq].pop_back();
                    if (freq_table[min_freq].size() == 0)
                    {
                        freq_table.erase(min_freq);
                    }
                }
                freq_table[1].push_front(LFUNode(id, 1));
                key_table[id] = freq_table[1].begin();
                min_freq = 1;
            }
            else
            {
                std::list<LFUNode>::iterator node = it->second;
                size_t freq = node->freq;
                freq_table[freq].erase(node);
                if (freq_table[freq].size() == 0)
                {
                    freq_table.erase(freq);
                    if (min_freq == freq)
                        min_freq += 1;
                }
                freq_table[freq + 1].push_front(LFUNode(id, freq + 1));
                key_table[id] = freq_table[freq + 1].begin();
            }
        }
    }
};

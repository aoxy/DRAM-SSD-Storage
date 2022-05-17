#pragma once
#include <unordered_map>
#include <list>
#include "utils/logs.h"

class CacheRecord
{
private:
    size_t access_count;
    size_t hit_count;

public:
    CacheRecord() : access_count(0), hit_count(0) {}
    void access() { ++access_count; }
    void hit() { ++hit_count; }
    double hit_rate()
    {
        if (access_count == 0)
            return 0.0;
        return hit_count * 100.0 / access_count;
    }
    void clear()
    {
        access_count = 0;
        hit_count = 0;
    }
};

template <class K>
class ReplacementCache
{
protected:
    size_t cap;
    K invalid_id;

public:
    ReplacementCache(K invalid_id, size_t cap = 16) : invalid_id(invalid_id), cap(cap) {}

    virtual K ranking(const K &id) = 0;

    virtual size_t size() = 0;
    size_t capacity() const
    {
        return cap;
    }
};

template <class K>
class ReplacementLRUCache : public ReplacementCache<K>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;
    ReplacementLRUCache(K invalid_id, size_t cap = 16) : ReplacementCache<K>(invalid_id, cap)
    {
        mp.clear();
        ls.clear();
    }
    size_t size()
    {
        return mp.size();
    }

    K ranking(const K &id)
    {
        K evic_id = ReplacementCache<K>::invalid_id;
        auto it = mp.find(id);
        if (it != mp.end())
        {
            ls.erase(it->second);
        }
        else if (size() >= ReplacementCache<K>::capacity() && ls.size() > 0)
        {
            evic_id = ls.back();
            ls.pop_back();
            mp.erase(evic_id);
        }
        ls.emplace_front(id);
        mp[id] = ls.begin();
        return evic_id;
    }

private:
    std::unordered_map<K, list_iterator> mp;
    std::list<K> ls;
};

template <class K>
class ReplacementLFUCache : public ReplacementCache<K>
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
    std::unordered_map<size_t, typename std::list<LFUNode> > freq_table;

public:
    ReplacementLFUCache(K invalid_id, size_t cap = 16) : ReplacementCache<K>(invalid_id, cap)
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

    K ranking(const K &id)
    {
        K evic_id = ReplacementCache<K>::invalid_id;
        auto it = key_table.find(id);
        if (it != key_table.end())
        {
            typename std::list<LFUNode>::iterator node = it->second;
            size_t freq = node->freq;
            auto &freq_ls = freq_table[freq];
            if (freq_table[freq].size() <= 1)
            {
                freq_table.erase(freq);
                if (min_freq == freq)
                    min_freq += 1;
            }
            else
                freq_table[freq].erase(node);
            max_freq = std::max(max_freq, ++freq);
            auto &freq_add_ls = freq_table[freq];
            freq_add_ls.emplace_front(LFUNode(id, freq));
            key_table[id] = freq_add_ls.begin();
        }
        else
        {
            if (size() >= ReplacementCache<K>::capacity() && key_table.size() > 0)
            {
                auto &freq_ls = freq_table[min_freq];
                auto rm_it = freq_ls.back();
                freq_ls.pop_back();
                evic_id = rm_it.key;
                key_table.erase(evic_id);
                if (freq_ls.size() == 0)
                {
                    freq_table.erase(min_freq);
                    ++min_freq;
                    while (min_freq <= max_freq)
                    {
                        auto it = freq_table.find(min_freq);
                        if (it == freq_table.end() || it->second.size() == 0)
                            ++min_freq;
                        else
                            break;
                    }
                }
            }
            freq_table[1].emplace_front(LFUNode(id, 1));
            key_table[id] = freq_table[1].begin();
            min_freq = 1;
        }
        return evic_id;
    }
};

template <class K>
class ReplacementFIFOCache : public ReplacementCache<K>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;

    ReplacementFIFOCache(K invalid_id, size_t cap = 16) : ReplacementCache<K>(invalid_id, cap)
    {
        fifo_queue.clear();
        key_lookup.clear();
    }

    size_t size()
    {
        return key_lookup.size();
    }

    K ranking(const K &id)
    {
        K evic_id = ReplacementCache<K>::invalid_id;
        if (key_lookup.find(id) == key_lookup.end())
        {
            if (size() >= ReplacementCache<K>::capacity() && key_lookup.size() > 0)
            {
                evic_id = fifo_queue.back();
                fifo_queue.pop_back();
                key_lookup.erase(evic_id);
            }
            fifo_queue.emplace_front(id);
            key_lookup[id] = fifo_queue.begin();
        }
        return evic_id;
    }

private:
    std::list<K> fifo_queue;
    std::unordered_map<K, list_iterator> key_lookup;
};

template <class K>
struct ReplacementLRUPart
{
public:
    using list_iterator = typename std::list<K>::const_iterator;

    void clear()
    {
        ls.clear();
        mp.clear();
    }
    size_t size() { return mp.size(); }
    bool hit(const K &x)
    {
        return mp.find(x) != mp.end();
    }
    void remove(const K &x)
    {
        if (hit(x))
        {
            ls.erase(mp[x]);
            mp.erase(x);
        }
    }
    void add(const K &x)
    {
        ls.emplace_front(x);
        mp[x] = ls.begin();
    }
    void readd(const K &x)
    {
        auto it = mp.find(x);
        ls.erase(it->second);
        mp.erase(x);
        ls.emplace_front(x);
        mp[x] = ls.begin();
    }
    K evic(K invalid_id)
    {
        K evic_id = invalid_id;
        if (ls.size() > 0)
        {
            evic_id = ls.back();
            ls.pop_back();
            mp.erase(evic_id);
        }
        return evic_id;
    }

private:
    std::list<K> ls;
    std::unordered_map<K, list_iterator> mp;
};

template <class K>
struct ReplacementLFUPart
{
    void clear()
    {
        min_freq = 0;
        max_freq = 0;
        key_table.clear();
        freq_table.clear();
    }
    size_t size() { return key_table.size(); }
    bool hit(const K &x)
    {
        return key_table.find(x) != key_table.end();
    }
    void remove(const K &x)
    {
        if (hit(x))
        {
            auto it = key_table[x];
            size_t freq = it->freq;
            auto &freq_ls = freq_table[freq];
            freq_ls.erase(it);
            key_table.erase(x);
            if (freq_ls.size() == 0)
            {
                freq_table.erase(freq);
                if (freq == min_freq)
                {
                    ++min_freq;
                    while (min_freq <= max_freq)
                    {
                        auto it = freq_table.find(min_freq);
                        if (it == freq_table.end() || it->second.size() == 0)
                            ++min_freq;
                        else
                            break;
                    }
                }
                else if (freq == max_freq)
                {
                    --max_freq;
                    while (min_freq <= max_freq)
                    {
                        auto it = freq_table.find(min_freq);
                        if (it == freq_table.end() || it->second.size() == 0)
                            --max_freq;
                        else
                            break;
                    }
                }
            }
        }
    }
    void add(const K &x)
    {
        freq_table[1].emplace_front(LFUNode(x, 1));
        key_table[x] = freq_table[1].begin();
        min_freq = 1;
    }
    void readd(const K &x)
    {
        auto it = key_table.find(x);
        typename std::list<LFUNode>::iterator node = it->second;
        size_t freq = node->freq;
        auto &freq_ls = freq_table[freq];
        if (freq_table[freq].size() <= 1)
        {
            freq_table.erase(freq);
            if (min_freq == freq)
                min_freq += 1;
        }
        else
            freq_table[freq].erase(node);
        max_freq = std::max(max_freq, ++freq);
        auto &freq_add_ls = freq_table[freq];
        freq_add_ls.emplace_front(LFUNode(x, freq));
        key_table[x] = freq_add_ls.begin();
    }
    K evic(K invalid_id)
    {
        if (key_table.size() > 0)
        {
            auto &freq_ls = freq_table[min_freq];
            auto rm_it = freq_ls.back();
            freq_ls.pop_back();
            key_table.erase(rm_it.key);
            K evic_id = rm_it.key;

            if (freq_ls.size() == 0)
            {
                freq_table.erase(min_freq);
                ++min_freq;
                while (min_freq <= max_freq)
                {
                    auto it = freq_table.find(min_freq);
                    if (it == freq_table.end() || it->second.size() == 0)
                        ++min_freq;
                    else
                        break;
                }
            }
            return evic_id;
        }
        return invalid_id;
    }

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
    std::unordered_map<size_t, typename std::list<LFUNode> > freq_table;
};

template <class K, class BaseCache1, class BaseCache2>
class ReplacementCompositeCache : public ReplacementCache<K>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;
    ReplacementCompositeCache(K invalid_id, size_t cap = 16) : ReplacementCache<K>(invalid_id, cap)
    {
        t1.clear();
        b1.clear();
        t2.clear();
        b2.clear();
        p = 0.0;
    }

    size_t size()
    {
        return t1.size() + t2.size();
    }

    K ranking(const K &id)
    {
        K evic_id = ReplacementCache<K>::invalid_id;
        if (t1.hit(id))
        {
            t1.remove(id);
            t2.add(id);
        }
        else if (t2.hit(id))
        {
            t2.readd(id);
        }
        else if (b1.hit(id))
        {
            p = (float)std::min((float)ReplacementCache<K>::cap, (float)(p + std::max((b2.size() * 1.0) / b1.size(), 1.0)));
            evic_id = replace(id, p);
            b1.remove(id);
            t2.add(id);
        }
        else if (b2.hit(id))
        {
            p = (float)std::max((float)0.0, (float)(p - std::max((b1.size() * 1.0) / b2.size(), 1.0)));
            evic_id = replace(id, p);
            b2.remove(id);
            t2.add(id);
        }
        else
        {
            if ((t1.size() + b1.size()) == ReplacementCache<K>::cap)
            {
                if (t1.size() < ReplacementCache<K>::cap)
                {
                    b1.evic(ReplacementCache<K>::invalid_id);
                    evic_id = replace(id, p);
                }
                else
                {
                    evic_id = t1.evic(ReplacementCache<K>::invalid_id);
                }
            }

            else if ((t1.size() + b1.size() < ReplacementCache<K>::cap) && ((t1.size() + t2.size() + b1.size() + b2.size()) >= ReplacementCache<K>::cap))
            {
                if ((t1.size() + t2.size() + b1.size() + b2.size()) == (2 * ReplacementCache<K>::cap))
                {
                    b2.evic(ReplacementCache<K>::invalid_id);
                }
                evic_id = replace(id, p);
            }
            t1.add(id);
        }
        return evic_id;
    }

private:
    K replace(const K &x, float p)
    {
        K evic_id = ReplacementCache<K>::invalid_id;
        if ((t1.size() > 0) && ((t1.size() > p) || (b2.hit(x) && t1.size() == p)))
        {
            if (t1.size() > 0)
            {
                evic_id = t1.evic(ReplacementCache<K>::invalid_id);
                b1.add(evic_id);
            }
        }
        else
        {
            if (t2.size() > 0)
            {
                evic_id = t2.evic(ReplacementCache<K>::invalid_id);
                b2.add(evic_id);
            }
        }
        return evic_id;
    }

    BaseCache1 t1, b1;
    BaseCache2 t2, b2;
    float p;
};

template <class K>
class ReplacementARCCache : public ReplacementCompositeCache<K, ReplacementLRUPart<K>, ReplacementLRUPart<K> >
{
public:
    ReplacementARCCache(K invalid_id, size_t cap = 16) : ReplacementCompositeCache<K, ReplacementLRUPart<K>, ReplacementLRUPart<K> >(invalid_id, cap) {}
};

template <class K>
class ReplacementARFCache : public ReplacementCompositeCache<K, ReplacementLRUPart<K>, ReplacementLFUPart<K> >
{
public:
    ReplacementARFCache(K invalid_id, size_t cap = 16) : ReplacementCompositeCache<K, ReplacementLRUPart<K>, ReplacementLFUPart<K> >(invalid_id, cap) {}
};

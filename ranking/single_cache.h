#pragma once
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <queue>
#include "utils/logs.h"

template <class K, class V>
class SingleCache
{
protected:
    size_t cap;
    std::unordered_map<K, V> cache;
    std::pair<K, V> invalid_kv;

public:
    SingleCache(std::pair<K, V> invalid_kv, size_t cap = 16) : invalid_kv(invalid_kv), cap(cap) {}

    void clear()
    {
        cache.clear();
    }

    V get(const K &id)
    {
        auto it = SingleCache<K, V>::cache.find(id);
        if (it != SingleCache<K, V>::cache.end())
        {
            return it->second;
        }
        return SingleCache<K, V>::invalid_kv.second;
    }

    virtual void set(const K &id, V value) = 0;

    size_t size()
    {
        return SingleCache<K, V>::cache.size();
    }
    size_t capacity() const
    {
        return cap;
    }
};

template <class K, class V>
class SingleLRUCache : public SingleCache<K, V>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;
    SingleLRUCache(std::pair<K, V> invalid_kv, size_t cap = 16) : SingleCache<K, V>(invalid_kv, cap)
    {
        mp.clear();
        ls.clear();
    }

    void set(const K &id, V value)
    {
        auto it = mp.find(id);
        if (it != mp.end())
        {
            ls.erase(it->second);
        }
        else if (SingleCache<K, V>::size() >= SingleCache<K, V>::capacity() && ls.size() > 0)
        {
            K evic_id = ls.back();
            ls.pop_back();
            mp.erase(evic_id);
            SingleCache<K, V>::cache.erase(evic_id);
        }
        ls.emplace_front(id);
        mp[id] = ls.begin();
        SingleCache<K, V>::cache[id] = value;
    }

private:
    std::unordered_map<K, list_iterator> mp;
    std::list<K> ls;
};

template <class K, class V>
class SingleLFUCache : public SingleCache<K, V>
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
    std::unordered_map<size_t, typename std::list<LFUNode>> freq_table;

public:
    SingleLFUCache(std::pair<K, V> invalid_kv, size_t cap = 16) : SingleCache<K, V>(invalid_kv, cap)
    {
        min_freq = 0;
        max_freq = 0;
        key_table.clear();
        freq_table.clear();
    }

    void set(const K &id, V value)
    {
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
            if (SingleCache<K, V>::size() >= SingleCache<K, V>::capacity() && key_table.size() > 0)
            {
                auto &freq_ls = freq_table[min_freq];
                auto rm_it = freq_ls.back();
                freq_ls.pop_back();
                K evic_id = rm_it.key;
                key_table.erase(evic_id);
                SingleCache<K, V>::cache.erase(evic_id);
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
        SingleCache<K, V>::cache[id] = value;
    }
};

template <class K, class V>
class SingleFIFOCache : public SingleCache<K, V>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;

    SingleFIFOCache(std::pair<K, V> invalid_kv, size_t cap = 16) : SingleCache<K, V>(invalid_kv, cap)
    {
        fifo_queue.clear();
        key_lookup.clear();
    }

    void set(const K &id, V value)
    {
        if (key_lookup.find(id) == key_lookup.end())
        {
            if (SingleCache<K, V>::size() >= SingleCache<K, V>::capacity() && key_lookup.size() > 0)
            {
                K evic_id = fifo_queue.back();
                fifo_queue.pop_back();
                key_lookup.erase(evic_id);
                SingleCache<K, V>::cache.erase(evic_id);
            }
            fifo_queue.emplace_front(id);
            key_lookup[id] = fifo_queue.begin();
        }
        SingleCache<K, V>::cache[id] = value;
    }

private:
    std::list<K> fifo_queue;
    std::unordered_map<K, list_iterator> key_lookup;
};

template <class K, class V>
struct LRUPart
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
    K evic()
    {
        K evic_id = 0;
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

template <class K, class V>
struct LFUPart
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
    K evic()
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
        return 0;
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
    std::unordered_map<size_t, typename std::list<LFUNode>> freq_table;
};

template <class K, class V, class BaseCache1, class BaseCache2>
class CompositeCache : public SingleCache<K, V>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;
    CompositeCache(std::pair<K, V> invalid_kv, size_t cap = 16) : SingleCache<K, V>(invalid_kv, cap)
    {
        t1.clear();
        b1.clear();
        t2.clear();
        b2.clear();
        p = 0.0;
    }

    void set(const K &id, V value)
    {
        if (t1.size() + t2.size() != SingleCache<K, V>::cache.size())
        {
            LOGINFO << "size = " << t1.size() + t2.size() << std::endl;
            LOGINFO << "mpsize = " << SingleCache<K, V>::cache.size() << std::endl;
        }
        SingleCache<K, V>::cache[id] = value;
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
            p = (float)std::min((float)SingleCache<K, V>::cap, (float)(p + std::max((b2.size() * 1.0) / b1.size(), 1.0)));
            replace(id, p);
            b1.remove(id);
            t2.add(id);
        }
        else if (b2.hit(id))
        {
            p = (float)std::max((float)0.0, (float)(p - std::max((b1.size() * 1.0) / b2.size(), 1.0)));
            replace(id, p);
            b2.remove(id);
            t2.add(id);
        }
        else
        {
            if ((t1.size() + b1.size()) == SingleCache<K, V>::cap)
            {
                if (t1.size() < SingleCache<K, V>::cap)
                {
                    b1.evic();
                    replace(id, p);
                }
                else
                {
                    K id = t1.evic();
                    SingleCache<K, V>::cache.erase(id);
                }
            }

            else if ((t1.size() + b1.size() < SingleCache<K, V>::cap) && ((t1.size() + t2.size() + b1.size() + b2.size()) >= SingleCache<K, V>::cap))
            {
                if ((t1.size() + t2.size() + b1.size() + b2.size()) == (2 * SingleCache<K, V>::cap))
                {
                    b2.evic();
                }
                replace(id, p);
            }
            t1.add(id);
        }
    }

private:
    void replace(const K &x, float p)
    {
        if ((t1.size() > 0) && ((t1.size() > p) || (b2.hit(x) && t1.size() == p)))
        {
            if (t1.size() > 0)
            {
                K id = t1.evic();
                SingleCache<K, V>::cache.erase(id);
                b1.add(id);
            }
        }
        else
        {
            if (t2.size() > 0)
            {
                K id = t2.evic();
                SingleCache<K, V>::cache.erase(id);
                b2.add(id);
            }
        }
    }

    BaseCache1 t1, b1;
    BaseCache2 t2, b2;
    float p;
};

template <class K, class V>
class SingleARCCache : public CompositeCache<K, V, LRUPart<K, V>, LRUPart<K, V>>
{
public:
    SingleARCCache(std::pair<K, V> invalid_kv, size_t cap = 16) : CompositeCache<K, V, LRUPart<K, V>, LRUPart<K, V>>(invalid_kv, cap) {}
};

template <class K, class V>
class SingleARFCache : public CompositeCache<K, V, LRUPart<K, V>, LFUPart<K, V>>
{
public:
    SingleARFCache(std::pair<K, V> invalid_kv, size_t cap = 16) : CompositeCache<K, V, LRUPart<K, V>, LFUPart<K, V>>(invalid_kv, cap) {}
};

template <class K, class V>
class SingleAgingLFUCache : public SingleCache<K, V>
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
    size_t count;
    size_t max_count;
    size_t temp_aging_count;
    std::unordered_map<K, typename std::list<LFUNode>::iterator> key_table;
    std::unordered_map<size_t, typename std::list<LFUNode>> freq_table;

public:
    SingleAgingLFUCache(std::pair<K, V> invalid_kv, size_t cap = 16, size_t max_c = 0xffffffff) : SingleCache<K, V>(invalid_kv, cap)
    {
        min_freq = 0;
        max_freq = 0;
        count = 0;
        max_count = max_c;
        temp_aging_count = 0;
        key_table.clear();
        freq_table.clear();
    }

    inline size_t age_freq(size_t freq)
    {
        return freq >= 2 ? freq >> 1 : freq;
    }

    void aging()
    {
        if (count < max_count)
        {
            count++;
            return;
        }
        count = 0;
        temp_aging_count++;
        // LOGINFO << "aging temp_aging_count = " << temp_aging_count << std::endl;

        std::unordered_map<K, size_t> key_freq_table;
        min_freq = key_table.begin()->second->freq;
        max_freq = min_freq;
        for (auto it = key_table.begin(); it != key_table.end(); it++)
        {
            size_t freq = age_freq(it->second->freq);
            key_freq_table[it->second->key] = freq;
            min_freq = std::min(min_freq, freq);
            max_freq = std::max(max_freq, freq);
        }
        freq_table.clear();
        key_table.clear();
        for (auto it = key_freq_table.begin(); it != key_freq_table.end(); it++)
        {
            freq_table[it->second].emplace_front(LFUNode(it->first, it->second));
        }

        for (auto it1 = freq_table.begin(); it1 != freq_table.end(); it1++)
        {
            for (auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
            {
                key_table[it2->key] = it2;
            }
        }
    }

    void check()
    {
        LOGINFO << "check-->";
        std::unordered_set<size_t> freq_set;
        for (auto it = key_table.begin(); it != key_table.end(); it++)
        {
            size_t freq = it->second->freq;
            if (freq_set.count(freq))
                freq_set.insert(freq);
        }
        for (auto iter = freq_set.begin(); iter != freq_set.end(); ++iter)
        {
            auto &freq_ls = freq_table[*iter];
            for (auto it = freq_ls.begin(); it != freq_ls.end(); ++it)
            {
                assert(it->freq == *iter);
            }
        }
    }

    void set(const K &id, V value)
    {
        aging();
        // check();
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
            if (SingleCache<K, V>::size() >= SingleCache<K, V>::capacity() && key_table.size() > 0)
            {
                auto &freq_ls = freq_table[min_freq];
                auto rm_it = freq_ls.back();
                freq_ls.pop_back();
                K evic_id = rm_it.key;
                key_table.erase(evic_id);
                SingleCache<K, V>::cache.erase(evic_id);
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
        SingleCache<K, V>::cache[id] = value;
    }
};

template <class K, class V>
class SingleDynamicAgingLFUCache : public SingleCache<K, V>
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
    size_t mid_freq;
    size_t count;
    size_t temp_aging_count;
    size_t recent_len;
    std::unordered_map<K, typename std::list<LFUNode>::iterator> key_table;
    std::unordered_map<size_t, typename std::list<LFUNode>> freq_table;
    std::deque<size_t> freq_recent;

public:
    SingleDynamicAgingLFUCache(std::pair<K, V> invalid_kv, size_t cap = 16) : SingleCache<K, V>(invalid_kv, cap)
    {
        min_freq = 0;
        max_freq = 0;
        mid_freq = 0xffffffff;
        count = 0;
        temp_aging_count = 0;
        recent_len = 1 << 6;
        key_table.clear();
        freq_table.clear();
        freq_recent.clear();
    }

    inline size_t age_freq(size_t freq)
    {
        return freq >= 2 ? freq >> 1 : freq;
    }

    void aging()
    {
        if (count < 100000)
        {
            count++;
            return;
        }
        count = 0;
        if (!need_age(freq_recent))
            return;
        temp_aging_count++;
        LOGINFO << "aging temp_aging_count = " << temp_aging_count << std::endl;

        std::unordered_map<K, size_t> key_freq_table;
        std::vector<size_t> freq_arr;
        min_freq = key_table.begin()->second->freq;
        max_freq = min_freq;
        for (auto it = key_table.begin(); it != key_table.end(); it++)
        {

            size_t freq = age_freq(it->second->freq);
            freq_arr.push_back(freq);
            key_freq_table[it->second->key] = freq;
            min_freq = std::min(min_freq, freq);
            max_freq = std::max(max_freq, freq);
        }
        std::sort(freq_arr.begin(), freq_arr.end());
        mid_freq = freq_arr[freq_arr.size() >> 1];
        freq_table.clear();
        key_table.clear();
        for (auto it = key_freq_table.begin(); it != key_freq_table.end(); it++)
        {
            freq_table[it->second].emplace_front(LFUNode(it->first, it->second));
        }

        for (auto it1 = freq_table.begin(); it1 != freq_table.end(); it1++)
        {
            for (auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
            {
                key_table[it2->key] = it2;
            }
        }
    }

    bool need_age(std::deque<size_t> freq_recent)
    {
        size_t sum = 0;
        std::vector<size_t> freq_arr;
        LOGINFO << "freq_recent = " << array_str(freq_recent);
        for (auto it = freq_recent.begin(); it != freq_recent.end(); it++)
        {
            sum += *it;
            freq_arr.push_back(*it);
        }
        std::sort(freq_arr.begin(), freq_arr.end());
        size_t mid_freq = freq_arr[freq_arr.size() >> 1];
        // LOGINFO << "mid_freq = " << freq_arr[freq_arr.size() >> 1];
        // LOGINFO << "sum = " << sum / freq_recent.size();
        return freq_recent.size() * mid_freq <= sum;
    }

    void check()
    {
        LOGINFO << "check-->";
        std::unordered_set<size_t> freq_set;
        for (auto it = key_table.begin(); it != key_table.end(); it++)
        {
            size_t freq = it->second->freq;
            if (freq_set.count(freq))
                freq_set.insert(freq);
        }
        for (auto iter = freq_set.begin(); iter != freq_set.end(); ++iter)
        {
            auto &freq_ls = freq_table[*iter];
            for (auto it = freq_ls.begin(); it != freq_ls.end(); ++it)
            {
                assert(it->freq == *iter);
            }
        }
    }

    void set(const K &id, V value)
    {
        aging();
        // check();
        if (freq_recent.size() > recent_len)
        {
            freq_recent.pop_front();
        }

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
            freq_recent.push_back(freq);
        }
        else
        {
            if (SingleCache<K, V>::size() >= SingleCache<K, V>::capacity() && key_table.size() > 0)
            {
                auto &freq_ls = freq_table[min_freq];
                auto rm_it = freq_ls.back();
                freq_ls.pop_back();
                K evic_id = rm_it.key;
                key_table.erase(evic_id);
                SingleCache<K, V>::cache.erase(evic_id);
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
        SingleCache<K, V>::cache[id] = value;
    }
};
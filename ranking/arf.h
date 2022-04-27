// Adaptive Recentl Frequent cache /// 文档
#include "cache.h"
template <class K>
class ARFCache : public BatchCache<K>
{
public:
    using list_iterator = typename std::list<K>::const_iterator;
    ARFCache(size_t cap) : BatchCache<K>(cap)
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

    size_t get_evic_ids(K *evic_ids, size_t k_size)
    {
        size_t true_size = 0;
        for (size_t i = 0; i < k_size; ++i)
        {
            K rm_it;
            if (t1.size() > 0)
            {
                evic_ids[true_size++] = t1.evic();
            }
            else if (t2.size() > 0)
            {
                evic_ids[true_size++] = t2.evic();
            }
        }
        return true_size;
    }

    void add_to_rank(const K *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            K id = batch_ids[i];
            get(id);
        }
    }

    void get(K id)
    {
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
            p = (float)std::min((float)BatchCache<K>::capacity, (float)(p + std::max((b2.size() * 1.0) / b1.size(), 1.0)));
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
            if ((t1.size() + b1.size()) == BatchCache<K>::capacity)
            {
                if (t1.size() < BatchCache<K>::capacity)
                {
                    b1.evic();
                    replace(id, p);
                }
                else
                {
                    t1.evic();
                }
            }

            else if ((t1.size() + b1.size() < BatchCache<K>::capacity) && ((t1.size() + t2.size() + b1.size() + b2.size()) >= BatchCache<K>::capacity))
            {
                if ((t1.size() + t2.size() + b1.size() + b2.size()) == (2 * BatchCache<K>::capacity))
                {
                    b2.evic();
                }
                replace(id, p);
            }

            t1.add(id);
        }
    }
    void replace(const K &x, float p)
    {
        if ((t1.size() > 0) && ((t1.size() > p) || (b2.hit(x) && t1.size() == p)))
        {
            K id = t1.evic();
            b1.add(id);
        }
        else
        {
            K id = t2.evic();
            b2.add(id);
        }
    }

private:
    struct LRUPart
    {
        std::list<K> ls;
        std::unordered_map<K, list_iterator> mp;
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
            ls.erase(mp[x]);
            mp.erase(x);
        }
        void add(const K &x)
        {
            ls.emplace_front(x);
            mp[x] = ls.begin();
        }
        K evic()
        {
            if (ls.size() > 0)
            {
                K evic_id = ls.back();
                ls.pop_back();
                mp.erase(evic_id);
                return evic_id;
            }
            return 0;
        }
    };
    struct LFUPart
    {
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
    };

    LRUPart t1, b1;
    LFUPart t2, b2;
    float p;
};
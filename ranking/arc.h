#include "cache.h"
template <class K>
class ARCCache : public BatchCache<K>
{
public:
    using fifo_iterator = typename std::list<K>::const_iterator;
    ARCCache(size_t cap) : BatchCache<K>(cap)
    {
        t1_l.clear();
        t2_l.clear();
        b1_l.clear();
        b2_l.clear();
        t1_m.clear();
        t2_m.clear();
        b1_m.clear();
        b2_m.clear();
    }

    size_t size()
    {
        return t1_m.size() + t2_m.size();
    }

    size_t get_evic_ids(K *evic_ids, size_t k_size)
    {
        size_t true_size = 0;
        for (size_t i = 0; i < k_size; ++i)
        {
            K rm_it;
            if (t1_m.size() > 0)
            {
                rm_it = t1_l.back();
                t1_l.pop_back();
                t1_m.erase(rm_it);
            }
            else if (t2_m.size() > 0)
            {
                rm_it = t2_l.back();
                t2_l.pop_back();
                t2_m.erase(rm_it);
            }
            evic_ids[true_size++] = rm_it;
        }
        return true_size;
    }

    void add_to_rank(const K *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            K id = batch_ids[i];
            arc_lookup(id);
        }
    }
    int check(std::unordered_map<K, fifo_iterator> &m, K x)
    {
        if (m.find(x) != m.end())
        {
            return 1;
        }
        return 0;
    }

    void queue_insert(std::unordered_map<K, fifo_iterator> &m, std::list<K> &l, K i)
    {
        if (m.size() == BatchCache<K>::capacity)
        {
            auto rm_it = l.back();
            l.pop_back();
            m.erase(rm_it);
        }
        l.emplace_front(i);
        m[i] = l.begin();
    }

    void queue_delete(std::unordered_map<K, fifo_iterator> &m, std::list<K> &l)
    {
        if (m.size() > 0)
        {
            auto rm_it = l.back();
            l.pop_back();
            m.erase(rm_it);
        }
    }
    void movefrom(std::unordered_map<K, fifo_iterator> &m1, std::list<K> &l1, std::unordered_map<K, fifo_iterator> &m2, std::list<K> &l2, int x)
    {
        auto rm_it = m1.find(x);
        if (rm_it != m1.end())
        {
            l1.erase(rm_it->second);
            m1.erase(x);
        }

        if (m2.size() == BatchCache<K>::capacity)
        {
            m2.erase(l2.back());
            l2.pop_back();
        }
        l2.emplace_front(x);
        m2[x] = l2.begin();
    }

    void Replace(const K i, const float p)
    {
        if ((t1_m.size() >= 1) && ((t1_m.size() > p) || (check(b2_m, i)) && (p == t1_m.size())))
        {
            if (t1_m.size() > 0)
            {
                movefrom(t1_m, t1_l, b1_m, b1_l, t1_l.back());
            }
        }
        else
        {
            if (t2_m.size() > 0)
            {
                movefrom(t2_m, t2_l, b2_m, b2_l, t1_l.back());
            }
        }
    }

    void arc_lookup(K i)
    {
        // Case 1: Page found in MRU
        if (check(t1_m, i))
        {
            movefrom(t1_m, t1_l, t2_m, t2_l, i);
        }
        // Case 1: Part B:Page found in MFU
        else if (check(t2_m, i))
        {
            movefrom(t2_m, t2_l, t2_m, t2_l, i);
        }
        // Case 2: Page found in MRUG
        else if (check(b1_m, i))
        {
            p = (float)std::min((float)BatchCache<K>::capacity, (float)(p + std::max((b2_m.size() * 1.0) / b1_m.size(), 1.0)));
            Replace(i, p);
            movefrom(b1_m, b1_l, t2_m, t2_l, i);
        }
        // Case 3: Page found in MFUG
        else if (check(b2_m, i))
        {
            p = (float)std::max((float)0.0, (float)(p - std::max((b1_m.size() * 1.0) / b2_m.size(), 1.0)));
            Replace(i, p);
            movefrom(b2_m, b2_l, t2_m, t2_l, i);
        }

        // Case 4:  Page not found in any of the queues.
        else
        {
            // Case 4: Part A: When L1 has c pages
            if ((t1_m.size() + b1_m.size()) == BatchCache<K>::capacity)
            {
                if (t1_m.size() < BatchCache<K>::capacity)
                {
                    queue_delete(b1_m, b1_l);
                    Replace(i, p);
                }
                else
                {
                    queue_delete(t1_m, t1_l);
                }
            }
            // Case 4: Part B: L1 has less than c pages
            else if ((t1_m.size() + b1_m.size() < BatchCache<K>::capacity) && ((t1_m.size() + t2_m.size() + b1_m.size() + b2_m.size()) >= BatchCache<K>::capacity))
            {
                if ((t1_m.size() + t2_m.size() + b1_m.size() + b2_m.size()) == (2 * BatchCache<K>::capacity))
                {
                    queue_delete(b2_m, b2_l);
                }
                Replace(i, p);
            }

            // Move the page to the most recently used position
            queue_insert(t1_m, t1_l, i);
        }
    }

private:
    std::list<K> t1_l, t2_l, b1_l, b2_l;
    std::unordered_map<K, fifo_iterator> t1_m, t2_m, b1_m, b2_m;
    float p;
};
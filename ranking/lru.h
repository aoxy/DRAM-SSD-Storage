/**
 * System design: LRU cache (least recently used).
 *
 * cpselvis(cpselvis@gmail.com)
 * Oct 8th, 2016
 */
#pragma once
#include <iostream>
#include <map>

using namespace std;

/**
 * Definition of cachelist node, it's double linked list node.
 */
struct CacheNode
{
    int64_t id;
    CacheNode *pre, *next;
    CacheNode(int id) : id(id), pre(nullptr), next(nullptr) {}
};

class BatchLRUCache
{
private:
    size_t capacity; // Maximum of cachelist size.
    CacheNode *head, *tail;
    map<int64_t, CacheNode *> mp; // Use hashmap to store
public:
    BatchLRUCache(size_t cap)
    {
        capacity = cap;
        head = new CacheNode(0);
        tail = new CacheNode(0);
        head->next = tail;
        tail->pre = head;
    }

    size_t get_evic_ids(int64_t *evic_ids, size_t k_size)
    {
        size_t true_size = 0;
        CacheNode *evicNode = tail->pre;
        CacheNode *rmNode = evicNode;
        for (size_t i = 0; i < k_size && evicNode != head; ++i)
        {
            evic_ids[i] = evicNode->id;
            rmNode = evicNode;
            evicNode = evicNode->pre;
            mp.erase(rmNode->id);
            delete rmNode;
            true_size++;
        }
        evicNode->next = tail;
        tail->pre = evicNode;
        return true_size;
    }
    void add_to_rank(int64_t *batch_ids, size_t batch_size)
    {
        for (size_t i = 0; i < batch_size; ++i)
        {
            int64_t id = batch_ids[i];
            map<int64_t, CacheNode *>::iterator it = mp.find(id);
            if (it != mp.end())
            {
                CacheNode *node = it->second;
                node->pre->next = node->next;
                node->next->pre = node->pre;
                head->next->pre = node;
                node->next = head->next;
                head->next = node;
                node->pre = head;
            }
            else
            {
                CacheNode *newNode = new CacheNode(id);
                if (mp.size() >= capacity)
                {
                    CacheNode *rmNode = tail->pre;
                    assert(rmNode->id != 0 && "rmNode->id!=0");
                    map<int64_t, CacheNode *>::iterator iter = mp.find(rmNode->id);
                    assert(iter != mp.end() && "iter!=mp.end()");
                    tail->pre = rmNode->pre;
                    rmNode->pre->next = tail;
                    delete rmNode;
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

    int get(int64_t id)
    {
        map<int64_t, CacheNode *>::iterator it = mp.find(id);
        if (it != mp.end())
        {
            CacheNode *node = it->second;
            remove(node);
            setHead(node);
            return node->id;
        }
        else
        {
            return -1;
        }
    }

    void set(int64_t id)
    {
        map<int64_t, CacheNode *>::iterator it = mp.find(id);
        if (it != mp.end())
        {
            CacheNode *node = it->second;
            node->id = id;
            remove(node);
            setHead(node);
        }
        else
        {
            CacheNode *newNode = new CacheNode(id);
            if (mp.size() >= capacity)
            {
                map<int64_t, CacheNode *>::iterator iter = mp.find(tail->id);
                remove(tail);
                mp.erase(iter);
            }
            setHead(newNode);
            mp[id] = newNode;
        }
    }

    void print()
    {
        CacheNode *p = head;
        while (p != nullptr)
        {
            cout << p->id << " ";
            p = p->next;
        }
        cout << endl;
    }

    void remove(CacheNode *node)
    {
        if (node->pre != NULL)
        {
            node->pre->next = node->next;
        }
        else
        {
            head = node->next;
        }
        if (node->next != NULL)
        {
            node->next->pre = node->pre;
        }
        else
        {
            tail = node->pre;
        }
    }

    void setHead(CacheNode *node)
    {
        node->next = head;
        node->pre = NULL;

        if (head != NULL)
        {
            head->pre = node;
        }
        head = node;
        if (tail == NULL)
        {
            tail = head;
        }
    }
};

// int main(int argc, char **argv)
// {
//     BatchLRUCache lruCache(20);
//     int64_t a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//     int64_t b[] = {10, 11, 12, 13, 14, 15, 16, 17, 18};
//     int64_t c[] = {19, 20, 21, 22, 23, 24, 25, 26, 27};
//     lruCache.add_to_rank(a, 9);
//     lruCache.print();
//     lruCache.add_to_rank(b, 9);
//     lruCache.print();
//     lruCache.add_to_rank(c, 9);
//     lruCache.print();
//     int64_t *ret = new int64_t[6];
//     size_t esize = lruCache.get_evic_ids(ret, 6);
//     lruCache.print();
//     cout << esize << "===" << endl;
//     for (size_t i = 0; i < esize; ++i)
//     {
//         cout << i + 1 << ": " << ret[i] << endl;
//     }
//     int64_t d[] = {19, 21, 22, 23, 14, 25, 26, 17, 22};
//     lruCache.add_to_rank(d, 9);
//     lruCache.print();

//     int64_t e[] = {194, 231, 222, 23, 14, 215, 26, 17, 22};
//     lruCache.add_to_rank(e, 9);
//     lruCache.print();
// }
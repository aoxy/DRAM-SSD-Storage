#include <iostream>
#include "shard_lock_map.h"
#include "ssd_hash_map.h"

using std::cout;
using std::endl;

int main()
{

    // =================================== Test 1 ==========================
    shard_lock_map map(NUM_SHARD);

    embedding_t vec1 = new double[EMB_LEN];
    if (vec1 == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
            exit(1);
    }
    for (int i = 0; i < EMB_LEN; ++i)
    {
        vec1[i] = 1.1;
    }

    embedding_t vec2 = new double[EMB_LEN];
    if (vec2 == nullptr)
    {
        LOGINFO << "malloc failed." << std::endl;
            exit(1);
    }
    for (int i = 0; i < EMB_LEN; ++i)
    {
        vec2[i] = 2.2;
    }

    map.set(1, vec1);
    embedding_t ret1 = map.get(1);
    for (int i = 0; i < EMB_LEN; ++i)
    {
        cout << ret1[i] << ", ";
    }
    cout << endl;

    map.set(2, vec2);
    embedding_t ret2 = map.get(2);
    for (int i = 0; i < EMB_LEN; ++i)
    {
        cout << ret2[i] << ", ";
    }
    cout << endl;

    embedding_t ret3 = map.get(3);
    for (int i = 0; ret3 != nullptr && i < EMB_LEN; ++i)
    {
        cout << ret3 << ", ";
    }
    cout << endl;

    // map.erase(2);
    // embedding_t ret2_erase = map.get(2);
    // for (int i = 0; i < EMB_LEN; ++i)
    // {
    //     cout << ret2_erase[i] << ", ";
    // }
    // cout << endl;

    delete[] vec1;
    delete[] vec2;

    // =================================== Test 2 ==========================
    ssd_hash_map s_map(NUM_SHARD);
    cout << s_map.get(123) << endl;
    s_map.set(123, 987);
    cout << s_map.get(123) << endl;

    return 0;
}
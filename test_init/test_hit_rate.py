# -*- coding:utf-8 -*-
import collections
import csv
import random
import os
import time
import numpy as np

from lfu_cache import lfu_hit_rate


def get_ori_visit_list():
    ori_user_filepath = "test_init/temp/ori_userid_visit_list.npy"
    ori_ad_filepath = "test_init/temp/ori_adgroupid_visit_list.npy"
    if os.path.exists(ori_user_filepath) and os.path.exists(ori_ad_filepath):
        return np.load(ori_user_filepath), np.load(ori_ad_filepath)
    userid_visit_list = []
    adgroupid_visit_list = []
    with open("dataset/taobao/raw_sample.csv", encoding="utf-8") as csvfile:
        reader = csv.reader(csvfile)
        cnt = 0
        next(reader)
        for row in reader:
            if cnt % 1000000 == 0:
                i = int(cnt / 2655796.2)
                print("\rReading dataset: {:.2f} %".format(cnt / 265579.62), end="")
            cnt += 1
            userid_visit_list.append(int(row[0]))
            adgroupid_visit_list.append(int(row[2]))
        print()
    np.save(ori_user_filepath, userid_visit_list)
    np.save(ori_ad_filepath, adgroupid_visit_list)
    return userid_visit_list, adgroupid_visit_list


def gen_visit_list(epoch):
    user_filepath = "test_init/temp/userid_visit_list{}.npy".format(epoch)
    ad_filepath = "test_init/temp/adgroupid_visit_list{}.npy".format(epoch)
    if os.path.exists(user_filepath) and os.path.exists(ad_filepath):
        return np.load(user_filepath), np.load(ad_filepath)
    ori_user_list, ori_ad_list = get_ori_visit_list()

    random.shuffle(ori_user_list)
    random.shuffle(ori_ad_list)
    np.save(user_filepath, ori_user_list)
    np.save(ad_filepath, ori_ad_list)
    return ori_user_list, ori_ad_list


def lru_hit_rate(visit_list, capacity_percent, visit_dup_size):
    visit_count = 0
    hit_count = 0
    miss_count = 0
    append_count = 0  # 淘汰时需要写文件，这是追加写入文件的次数
    capacity = int(capacity_percent * visit_dup_size / 100)
    cache = collections.OrderedDict()  # 有序字典
    # 预先加载一些数据进内存
    for key in visit_list:
        if key in cache.keys():
            cache.pop(key)
            cache[key] = "v"
        elif capacity == len(cache):
            cache.popitem(last=False)
            cache[key] = "v"
        else:
            cache[key] = "v"

    # 正式开始迭代
    for key in visit_list:
        visit_count += 1
        if key in cache.keys():
            hit_count += 1
            cache.pop(key)
            cache[key] = "v"
        elif capacity == len(cache):
            miss_count += 1
            append_count += 1
            cache.popitem(last=False)
            cache[key] = "v"
        else:
            miss_count += 1
            cache[key] = "v"
    print(
        "LRUCache[{}]({} %) {} visit, {} miss, {} hit({} %), {} append({})".format(
            capacity,
            capacity_percent,
            visit_count,
            miss_count,
            hit_count,
            "%.6g" % (hit_count * 100 / visit_count),
            append_count,
            append_count / visit_dup_size,
        )
    )
    return hit_count * 100 / visit_count


cap_perc = [1, 5, 10, 20, 30, 50, 70, 100]

user_data = [
    (cap_perc, [7.993177638900818, 28.910928064093476, 43.6831050395774, 61.14668592216097, 72.43348613999396, 86.21781242919967, 93.66259329923709, 100.0]),
]


ad_data = [
    (cap_perc, [20.10309451090767, 48.92210286776157, 65.21746530164721, 81.05090221346435, 88.65144805356104, 95.25646942549542, 97.68756343907576, 100.0]),
]


def plot_avg():
    import matplotlib.pyplot as plt

    res1 = np.zeros_like(np.array(user_data[0][1]))
    res2 = np.zeros_like(np.array(ad_data[0][1]))
    for i in user_data:
        res1 += np.array(i[1])
    for i in ad_data:
        res2 += np.array(i[1])
    avg_data = [
        (cap_perc, res1 / len(user_data)),
        (cap_perc, res2 / len(ad_data)),
    ]

    fig2 = plt.figure()
    ax1 = fig2.add_subplot(1, 2, 1)
    ax1.plot(avg_data[0][0], avg_data[0][1])
    ax1.set_title("user")
    ax1.set_xlabel("LRU Cache Size(%)")
    ax1.set_ylabel("Hit Rate (%)")
    ax1.grid(True)

    ax2 = fig2.add_subplot(1, 2, 2)
    ax2.plot(avg_data[1][0], avg_data[1][1])
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LRU Cache Size(%)")
    ax2.set_ylabel("Hit Rate (%)")
    ax2.grid(True)
    plt.show()


def run(algo="lru"):
    userid_visit_dup_size = None
    adgroupid_visit_dup_size = None
    result_list_user = []
    result_list_ad = []
    for i in range(1):
        print("\n\nepoch: ", i + 1)
        userid_visit_list, adgroupid_visit_list = gen_visit_list(i + 1)
        if not userid_visit_dup_size or not adgroupid_visit_dup_size:
            userid_visit_dup_size = len(set(userid_visit_list))
            adgroupid_visit_dup_size = len(set(adgroupid_visit_list))
            print("user dup size =", userid_visit_dup_size)
            print("ad dup size =", adgroupid_visit_dup_size)
        if algo == "lru":
            lru_res_user_list = []
            lru_res_ad_list = []
            print("user" + "=" * 20)
            for per in cap_perc:
                lru_res_user_list.append(lru_hit_rate(userid_visit_list, per, userid_visit_dup_size))
            print("adgroup" + "=" * 20)
            for per in cap_perc:
                lru_res_ad_list.append(lru_hit_rate(adgroupid_visit_list, per, adgroupid_visit_dup_size))
            result_list_user.append(lru_res_user_list)
            result_list_ad.append(lru_res_ad_list)
        elif algo == "lfu":
            lfu_res_user_list = []
            lfu_res_ad_list = []
            print("user" + "=" * 20)
            for per in cap_perc:
                lfu_res_user_list.append(lfu_hit_rate(userid_visit_list, per, userid_visit_dup_size))
            print("adgroup" + "=" * 20)
            for per in cap_perc:
                lfu_res_ad_list.append(lfu_hit_rate(adgroupid_visit_list, per, adgroupid_visit_dup_size))
            result_list_user.append(lfu_res_user_list)
            result_list_ad.append(lfu_res_ad_list)
        else:
            pass
    print("cache: ", algo)
    print("cap_perc =", cap_perc)
    print("result_list_user =", result_list_user)
    print("result_list_ad =", result_list_ad)


def gen_file():
    T1 = time.time()
    run("null")
    T2 = time.time()
    print("gen file time: %s s" % (T2 - T1))
    # gen file time: 297.9827558994293 s


def test_lru():
    T1 = time.time()
    run("lru")
    T2 = time.time()
    print("run lru time: %s s" % (T2 - T1))


def test_lfu():
    T1 = time.time()
    run("lfu")
    T2 = time.time()
    print("run lfu time: %s s" % (T2 - T1))


if __name__ == "__main__":
    # plot_avg()
    # gen_file()
    test_lfu()
    # test_lru()

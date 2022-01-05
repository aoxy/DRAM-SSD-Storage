# -*- coding:utf-8 -*-
import collections
import csv
import random
import os
import numpy as np


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
                print("\r{:.2f} %".format(cnt / 265579.62), end="")
            cnt += 1
            userid_visit_list.append(row[0])
            adgroupid_visit_list.append(row[2])
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


def hit_rate(visit_list, capacity_percent, visit_dup_size):
    visit_count = 0
    hit_count = 0
    miss_count = 0
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
            break
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
            cache.popitem(last=False)
            cache[key] = "v"
        else:
            miss_count += 1
            cache[key] = "v"
    print(
        "LRUCache[{}]({} %) {}次访问，{}次命中（{} %）".format(
            capacity,
            capacity_percent,
            visit_count,
            hit_count,
            "%.6g" % (hit_count * 100 / visit_count),
        )
    )
    return visit_count, hit_count, miss_count


cap_perc = [1, 5, 10, 20, 30, 50, 70, 100]

user_data = [
    (cap_perc, [7.99985, 28.9187, 43.6927, 61.157, 72.4288, 86.2242, 93.6659, 100]),
    (cap_perc, [7.99983, 28.9223, 43.6926, 61.1669, 72.4383, 86.2248, 93.6606, 100]),
    (cap_perc, [7.99867, 28.9174, 43.6895, 61.1708, 72.4364, 86.2309, 93.6696, 100]),
    (cap_perc, [8.00056, 28.9269, 43.6925, 61.1595, 72.4363, 86.2292, 93.6587, 100]),
    (cap_perc, [7.99964, 28.9264, 43.6869, 61.1704, 72.4436, 86.2316, 93.6613, 100]),
    (cap_perc, [7.99677, 28.9128, 43.6888, 61.1556, 72.4278, 86.2234, 93.6606, 100]),
]


ad_data = [
    (cap_perc, [20.09, 48.9124, 65.2088, 81.0437, 88.6488, 95.2525, 97.6852, 100]),
    (cap_perc, [20.0971, 48.9118, 65.2206, 81.0412, 88.6403, 95.2569, 97.6879, 100]),
    (cap_perc, [20.1036, 48.9074, 65.2075, 81.0423, 88.6498, 95.2578, 97.687, 100]),
    (cap_perc, [20.1025, 48.8989, 65.2182, 81.0491, 88.6483, 95.2511, 97.6853, 100]),
    (cap_perc, [20.0995, 48.9255, 65.2224, 81.0434, 88.6495, 95.2574, 97.6852, 100]),
    (cap_perc, [20.0979, 48.9093, 65.2215, 81.0467, 88.6445, 95.2498, 97.6853, 100]),
]


def plot_all():
    import matplotlib.pyplot as plt

    fig1 = plt.figure()
    for i in range(len(user_data)):
        ax = fig1.add_subplot(6, 2, 2 * i + 1)
        ax.plot(user_data[i][0], user_data[i][1])
    for i in range(len(ad_data)):
        ax = fig1.add_subplot(6, 2, 2 * i + 2)
        ax.plot(ad_data[i][0], ad_data[i][1])
    plt.show()


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


def run():
    userid_visit_dup_size = None
    adgroupid_visit_dup_size = None
    for i in range(5):
        print("\n\nepoch: ", i + 1)
        userid_visit_list, adgroupid_visit_list = gen_visit_list(i + 1)
        if not userid_visit_dup_size or not adgroupid_visit_dup_size:
            userid_visit_dup_size = len(set(userid_visit_list))
            adgroupid_visit_dup_size = len(set(adgroupid_visit_list))
            print(
                "user, ad dup size =", userid_visit_dup_size, adgroupid_visit_dup_size
            )
        print("user")
        hit_rate(userid_visit_list, 1, userid_visit_dup_size)
        hit_rate(userid_visit_list, 5, userid_visit_dup_size)
        hit_rate(userid_visit_list, 10, userid_visit_dup_size)
        hit_rate(userid_visit_list, 20, userid_visit_dup_size)
        hit_rate(userid_visit_list, 30, userid_visit_dup_size)
        hit_rate(userid_visit_list, 50, userid_visit_dup_size)
        hit_rate(userid_visit_list, 70, userid_visit_dup_size)
        hit_rate(userid_visit_list, 100, userid_visit_dup_size)
        print("adgroup")
        hit_rate(adgroupid_visit_list, 1, adgroupid_visit_dup_size)
        hit_rate(adgroupid_visit_list, 5, adgroupid_visit_dup_size)
        hit_rate(adgroupid_visit_list, 10, adgroupid_visit_dup_size)
        hit_rate(adgroupid_visit_list, 20, adgroupid_visit_dup_size)
        hit_rate(adgroupid_visit_list, 30, adgroupid_visit_dup_size)
        hit_rate(adgroupid_visit_list, 50, adgroupid_visit_dup_size)
        hit_rate(adgroupid_visit_list, 70, adgroupid_visit_dup_size)
        hit_rate(adgroupid_visit_list, 100, adgroupid_visit_dup_size)


if __name__ == "__main__":
    plot_avg()
    run()
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


if __name__ == "__main__":
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


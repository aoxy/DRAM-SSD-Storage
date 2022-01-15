# -*- coding:utf-8 -*-
import csv
import time
import random
import os

T1 = time.time()


with open("dataset/taobao/raw_sample.csv", encoding="utf-8") as csvfile:
    all_userid_set = set()
    all_userid_dict = dict()
    all_userid_list = []
    all_adgroupid_set = set()
    all_adgroupid_dict = dict()
    all_adgroupid_list = []
    visit_user_ad_list = []
    reader = csv.reader(csvfile)
    cnt = 0
    next(reader)
    for row in reader:
        if cnt % 1000000 == 0:
            i = int(cnt / 2655796.2)
            print("\rreading raw_sample.csv: {:.2f} %".format(cnt / 265579.62), end="")
        cnt += 1
        userid = row[0]
        adgroupid = row[2]
        visit_user_ad_list.append([userid, adgroupid])
        all_userid_dict[userid] = all_userid_dict.get(userid, 0) + 1
        all_adgroupid_dict[adgroupid] = all_adgroupid_dict.get(adgroupid, 0) + 1
        if userid not in all_userid_set:
            all_userid_list.append(userid)
            all_userid_set.add(userid)
        if adgroupid not in all_adgroupid_set:
            all_adgroupid_list.append(adgroupid)
            all_adgroupid_set.add(adgroupid)
    print("\rreading raw_sample.csv: {:.2f} %".format(100))

    print("userid size: ", len(all_userid_list))
    print("adgroupid size: ", len(all_adgroupid_list))

    with open("storage/user/all_ids.txt", mode="w", encoding="utf-8") as file_obj:
        for v in all_userid_list:
            file_obj.write(f"{v} ")

    with open("storage/ad/all_ids.txt", mode="w", encoding="utf-8") as file_obj:
        for v in all_adgroupid_list:
            file_obj.write(f"{v} ")

    with open("storage/user/id_count.txt", mode="w", encoding="utf-8") as file_obj:
        for k, v in all_userid_dict.items():
            file_obj.write(f"{k}\t:{v}\n")

    with open("storage/ad/id_count.txt", mode="w", encoding="utf-8") as file_obj:
        for k, v in all_adgroupid_dict.items():
            file_obj.write(f"{k}\t:{v}\n")

    shuffled_filepath = "dataset/taobao/shuffled_sample.csv"
    if not os.path.exists(shuffled_filepath):
        random.shuffle(visit_user_ad_list)
        with open(shuffled_filepath, mode="w", encoding="utf-8") as file_obj:
            file_obj.write(f"user,adgroupid\n")
            for user_ad in visit_user_ad_list:
                file_obj.write(f"{user_ad[0]},{user_ad[1]}\n")

T2 = time.time()
print("total time: %s s" % (T2 - T1))

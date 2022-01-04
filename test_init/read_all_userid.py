import csv
import platform
import time

T1 = time.time()


with open("dataset/taobao/raw_sample.csv", encoding="utf-8") as csvfile:
    # visit_userid_list = []
    # visit_adgroupid_list = []
    all_userid_set = set()
    all_userid_dict = dict()
    all_userid_list = []
    all_adgroupid_set = set()
    all_adgroupid_dict = dict()
    all_adgroupid_list = []
    reader = csv.reader(csvfile)
    cnt = 0
    next(reader)
    for row in reader:
        if cnt % 1000000 == 0:
            i = int(cnt / 2655796.2)
            print("\r{:.2f} %".format(cnt / 265579.62), end="")
        cnt += 1
        userid = row[0]
        adgroupid = row[2]
        # visit_userid_list.append(userid)
        # visit_adgroupid_list.append(adgroupid)
        all_userid_dict[userid] = all_userid_dict.get(userid, 0) + 1
        all_adgroupid_dict[adgroupid] = all_adgroupid_dict.get(adgroupid, 0) + 1
        if userid not in all_userid_set:
            all_userid_list.append(userid)
            all_userid_set.add(userid)
        if adgroupid not in all_adgroupid_set:
            all_adgroupid_list.append(adgroupid)
            all_adgroupid_set.add(adgroupid)

    file_path = "storage/all_userid.txt"
    with open(file_path, mode="w", encoding="utf-8") as file_obj:
        for v in all_userid_list:
            file_obj.write(v + " ")
    file_path = "storage/all_adgroupid.txt"
    with open(file_path, mode="w", encoding="utf-8") as file_obj:
        for v in all_adgroupid_list:
            file_obj.write(v + " ")
    # with open("../storage/visit_userid.txt", mode="w", encoding="utf-8") as file_obj:
    #     file_obj.write(",".join(visit_userid_list))
    # with open("../storage/visit_adgroupid.txt", mode="w", encoding="utf-8") as file_obj:
    #     file_obj.write(",".join(visit_adgroupid_list))
    file_path = "storage/userid_count.txt"
    with open(file_path, mode="w", encoding="utf-8") as file_obj:
        for k, v in all_userid_dict.items():
            file_obj.write(k + "\t:" + str(v) + "\n")
    file_path = "storage/adgroupid_count.txt"
    with open(file_path, mode="w", encoding="utf-8") as file_obj:
        for k, v in all_adgroupid_dict.items():
            file_obj.write(k + "\t:" + str(v) + "\n")
    print("\r{:.2f} %".format(100), end="")
    print()
    print("userid size: ", len(all_userid_list))
    print("adgroupid size: ", len(all_adgroupid_list))

T2 = time.time()
print("total time: %s s" % (T2 - T1))

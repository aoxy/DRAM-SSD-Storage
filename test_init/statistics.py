# -*- coding:utf-8 -*-
import numpy as np
from test_hit_rate import get_ori_visit_list


def gen_result_file():
    ori_user_list, ori_ad_list = get_ori_visit_list()
    ori_user_len = len(ori_user_list)
    ori_ad_len = len(ori_ad_list)
    all_userid_dict = dict()
    all_adgroupid_dict = dict()
    for userid in ori_user_list:
        all_userid_dict[userid] = all_userid_dict.get(userid, 0) + 1
    for adgroupid in ori_ad_list:
        all_adgroupid_dict[adgroupid] = all_adgroupid_dict.get(adgroupid, 0) + 1

    user_key_count_list = []
    for key in all_userid_dict.keys():
        user_key_count_list.append((key, all_userid_dict[key]))
    adgroup_key_count_list = []
    for key in all_adgroupid_dict.keys():
        adgroup_key_count_list.append((key, all_adgroupid_dict[key]))

    user_key_count_list.sort(key=lambda kc: kc[1], reverse=True)
    adgroup_key_count_list.sort(key=lambda kc: kc[1], reverse=True)

    user_count = len(user_key_count_list)
    ad_count = len(adgroup_key_count_list)
    print("ori_user_len =", ori_user_len)
    print("ori_ad_len =", ori_ad_len)
    print("user_count =", user_count)
    print("ad_count =", ad_count)
    # ori_user_len = 26557961
    # ori_ad_len = 26557961
    # user_count = 1141729
    # ad_count = 846811
    ux = np.zeros(user_count + 1)
    uy = np.zeros(user_count + 1)

    ax = np.zeros(ad_count + 1)
    ay = np.zeros(ad_count + 1)

    accu_count = 0
    for i in range(user_count):
        accu_count += user_key_count_list[i][1]
        ux[i + 1] = (i + 1) * 100 / user_count
        uy[i + 1] = accu_count * 100 / ori_user_len

    accu_count = 0
    for i in range(ad_count):
        accu_count += adgroup_key_count_list[i][1]
        ax[i + 1] = (i + 1) * 100 / ad_count
        ay[i + 1] = accu_count * 100 / ori_user_len

    np.save("test_init/temp/ux.npy", ux)
    np.save("test_init/temp/uy.npy", uy)
    np.save("test_init/temp/ax.npy", ax)
    np.save("test_init/temp/ay.npy", ay)


def plot():
    import matplotlib.pyplot as plt

    ux = np.load("test_init/temp/ux.npy")
    uy = np.load("test_init/temp/uy.npy")
    ax = np.load("test_init/temp/ax.npy")
    ay = np.load("test_init/temp/ay.npy")
    fig = plt.figure()
    ax1 = fig.add_subplot(1, 2, 1)
    ax1.plot(ux, uy)
    ax1.set_title("user")
    ax1.set_xlabel("Top ID(%)")
    ax1.set_ylabel("Count (%)")
    ax1.grid(True)

    ax2 = fig.add_subplot(1, 2, 2)
    ax2.plot(ax, ay)
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("Top ID(%)")
    ax2.set_ylabel("Count (%)")
    ax2.grid(True)

    plt.show()


plot()

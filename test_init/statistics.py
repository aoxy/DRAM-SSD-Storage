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


def plot_ratio_lru():
    import matplotlib.pyplot as plt

    per_points = [1, 5, 10, 20, 30, 50, 70, 100]
    user_count = [22.9816, 46.01803, 59.0954, 74.1374, 82.7866, 92.1246, 96.6018, 100.0]
    ad_count = [36.6051, 64.0675, 76.9989, 88.3513, 93.3913, 97.5309, 99.0434, 100.0]

    # ============================================================ LRU (
    user_hit_rate = [7.993177638900818, 28.910928064093476, 43.6831050395774, 61.14668592216097, 72.43348613999396, 86.21781242919967, 93.66259329923709, 100.0]
    ad_hit_rate = [20.10309451090767, 48.92210286776157, 65.21746530164721, 81.05090221346435, 88.65144805356104, 95.25646942549542, 97.68756343907576, 100.0]
    # ============================================================ LRU (

    user_hit_div_count = np.array(user_hit_rate) / np.array(user_count)
    ad_hit_div_count = np.array(ad_hit_rate) / np.array(ad_count)
    user_count_sub_hit = np.array(user_count) - np.array(user_hit_rate)
    ad_count_sub_hit = np.array(ad_count) - np.array(ad_hit_rate)
    # print(user_hit_div_count)
    # print(ad_hit_div_count)

    fig = plt.figure()
    ax1 = fig.add_subplot(2, 2, 1)
    ax1.plot(per_points, user_hit_div_count)
    ax1.set_title("user")
    ax1.set_xlabel("LRU Cache Size (%)")
    ax1.set_ylabel("Hit Rate Ratio")
    ax1.grid(True)

    ax2 = fig.add_subplot(2, 2, 2)
    ax2.plot(per_points, ad_hit_div_count)
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LRU Cache Size (%)")
    ax2.set_ylabel("Hit Rate Ratio")
    ax2.grid(True)

    ax3 = fig.add_subplot(2, 2, 3)
    ax3.plot(per_points, user_count_sub_hit)
    ax3.set_title("user")
    ax3.set_xlabel("LRU Cache Size (%)")
    ax3.set_ylabel("Hit Rate Diff (%)")
    ax3.grid(True)

    ax4 = fig.add_subplot(2, 2, 4)
    ax4.plot(per_points, ad_count_sub_hit)
    ax4.set_title("adgroup_id")
    ax4.set_xlabel("LRU Cache Size (%)")
    ax4.set_ylabel("Hit Rate Diff (%)")
    ax4.grid(True)

    plt.show()


def plot_ratio_lfu():
    import matplotlib.pyplot as plt

    per_points = [1, 5, 10, 20, 30, 50, 70, 100]
    user_count = [22.9816, 46.01803, 59.0954, 74.1374, 82.7866, 92.1246, 96.6018, 100.0]
    ad_count = [36.6051, 64.0675, 76.9989, 88.3513, 93.3913, 97.5309, 99.0434, 100.0]

    # ============================================================ LFU (
    u_result_lfu = [17.694547408967125, 40.61199954318782, 53.713427020997585, 69.7419391496207, 79.5804165839388, 90.5226647482463, 95.94674079083104, 100.0]
    a_result_lfu = [31.43179177046009, 59.408672224497955, 73.53103274758179, 86.40466035777371, 92.32975754426329, 97.10975929213843, 98.54443644977113, 100.0]
    user_hit_rate = u_result_lfu
    ad_hit_rate = a_result_lfu
    # ============================================================ LFU )

    user_hit_div_count = np.array(user_hit_rate) / np.array(user_count)
    ad_hit_div_count = np.array(ad_hit_rate) / np.array(ad_count)
    user_count_sub_hit = np.array(user_count) - np.array(user_hit_rate)
    ad_count_sub_hit = np.array(ad_count) - np.array(ad_hit_rate)
    # print(user_hit_div_count)
    # print(ad_hit_div_count)

    fig = plt.figure()
    ax1 = fig.add_subplot(2, 2, 1)
    ax1.plot(per_points, user_hit_div_count)
    ax1.set_title("user")
    ax1.set_xlabel("LFU Cache Size (%)")
    ax1.set_ylabel("Hit Rate Ratio")
    ax1.grid(True)

    ax2 = fig.add_subplot(2, 2, 2)
    ax2.plot(per_points, ad_hit_div_count)
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LFU Cache Size (%)")
    ax2.set_ylabel("Hit Rate Ratio")
    ax2.grid(True)

    ax3 = fig.add_subplot(2, 2, 3)
    ax3.plot(per_points, user_count_sub_hit)
    ax3.set_title("user")
    ax3.set_xlabel("LFU Cache Size (%)")
    ax3.set_ylabel("Hit Rate Diff (%)")
    ax3.grid(True)

    ax4 = fig.add_subplot(2, 2, 4)
    ax4.plot(per_points, ad_count_sub_hit)
    ax4.set_title("adgroup_id")
    ax4.set_xlabel("LFU Cache Size (%)")
    ax4.set_ylabel("Hit Rate Diff (%)")
    ax4.grid(True)

    plt.show()


def plot():
    import matplotlib.pyplot as plt

    ux = np.load("test_init/temp/ux.npy")
    uy = np.load("test_init/temp/uy.npy")
    ax = np.load("test_init/temp/ax.npy")
    ay = np.load("test_init/temp/ay.npy")
    #############################################################################
    # per_points = [1, 5, 10, 20, 30, 50, 70, 100]
    # user_hit_rate = [7.99922, 28.921, 43.6905, 61.16, 72.4352, 86.227, 93.6628, 100.0]
    # ad_hit_rate = [20.0984, 48.910, 65.2165, 81.0444, 88.64687, 95.25425, 97.686, 100.0]
    # user_count = [0] * len(per_points)
    # ad_count = [0] * len(per_points)

    # min_delta = [float("inf")] * len(per_points)
    # for xi in range(len(ux)):
    #     for i in range(len(per_points)):
    #         if abs(ux[xi] - per_points[i]) < min_delta[i]:
    #             min_delta[i] = abs(ux[xi] - per_points[i])
    #             user_count[i] = uy[xi]

    # min_delta = [float("inf")] * len(per_points)
    # for xi in range(len(ax)):
    #     for i in range(len(per_points)):
    #         if abs(ax[xi] - per_points[i]) < min_delta[i]:
    #             min_delta[i] = abs(ax[xi] - per_points[i])
    #             ad_count[i] = ay[xi]
    # # print(user_count)
    # # print(ad_count)
    # user_count = [22.9816, 46.01803, 59.0954, 74.1374, 82.7866, 92.1246, 96.6018, 100.0]
    # ad_count = [36.6051, 64.0675, 76.9989, 88.3513, 93.3913, 97.5309, 99.0434, 100.0]
    #############################################################################
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


def plot_append_lru():
    import matplotlib.pyplot as plt

    per_points = [1, 5, 10, 20, 30, 50, 70, 100]
    user_append = [21.401870321240853, 16.53615525225338, 13.099972935784235, 9.037738377495886, 6.41229573742981, 3.205899123171961, 1.474155425674569, 0.0]
    ad_append = [25.05752641380426, 16.019215621903825, 10.908611248554873, 5.942877454355222, 3.5591696376168946, 1.4876814306852415, 0.725233847930648, 0.0]

    fig = plt.figure()
    ax1 = fig.add_subplot(1, 2, 1)
    ax1.plot(per_points, np.log2(np.array(user_append) + 1))
    ax1.set_title("user")
    ax1.set_xlabel("LRU Cache Size (%)")
    ax1.set_ylabel("log2(Append Ratio + 1)")
    ax1.grid(True)

    ax2 = fig.add_subplot(1, 2, 2)
    ax2.plot(per_points, np.log2(np.array(ad_append) + 1))
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LRU Cache Size (%)")
    ax2.set_ylabel("log2(Append Ratio + 1)")
    ax2.grid(True)

    plt.show()


def plot_append_lfu():
    import matplotlib.pyplot as plt

    per_points = [1, 5, 10, 20, 30, 50, 70, 100]
    user_append = [19.145217472797835, 13.814348238504934, 10.766801929354514, 7.0383812621033535, 4.749835556423634, 2.204539781331647, 0.9428358218106048, 0.0]
    ad_append = [21.504583667429923, 12.730383757414582, 8.301283285172252, 4.263814475721265, 2.4055662951945593, 0.9064466569281693, 0.4564985575293661, 0.0]

    fig = plt.figure()
    ax1 = fig.add_subplot(1, 2, 1)
    ax1.plot(per_points, np.log2(np.array(user_append) + 1))
    ax1.set_title("user")
    ax1.set_xlabel("LFU Cache Size (%)")
    ax1.set_ylabel("log2(Append Ratio + 1)")
    ax1.grid(True)

    ax2 = fig.add_subplot(1, 2, 2)
    ax2.plot(per_points, np.log2(np.array(ad_append) + 1))
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LFU Cache Size (%)")
    ax2.set_ylabel("log2(Append Ratio + 1)")
    ax2.grid(True)

    plt.show()

# gen_result_file()
# plot()
plot_ratio_lfu()
plot_ratio_lru()
plot_append_lru()
plot_append_lfu()

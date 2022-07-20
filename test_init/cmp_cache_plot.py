import re
import numpy as np
from statistics import plot_dataset_dist_aux


def store(result_map, feature, cache_policy, cache_size, res_tuple):
    if feature not in result_map:
        result_map[feature] = dict()
    if cache_policy not in result_map[feature]:
        result_map[feature][cache_policy] = dict()
    result_map[feature][cache_policy][cache_size] = res_tuple


def get_result_map(all_log_file):
    result_map = dict()
    with open(all_log_file) as f:
        content = f.readlines()
        line_num = len(content)
        idx = 0
        while idx < line_num:
            if "====" in content[idx]:
                feature = re.findall("(?<=feature = ).[a-z0-9_]*", content[idx + 1])[0]
                cache_size = re.findall("(?<=cache size = ).[0-9.]*", content[idx + 2])[0]
                cache_policy = re.findall("(?<=cache policy = ).[a-z0-9]*", content[idx + 3])[0]
                hit_rate1 = re.findall("(?<=\[1\]total hit rate = ).[0-9.]*", content[idx + 4])[0]
                hit_rate2 = re.findall("(?<=\[2\]total hit rate = ).[0-9.]*", content[idx + 5])[0]
                hit_rate3 = re.findall("(?<=\[3\]total hit rate = ).[0-9.]*", content[idx + 6])[0]
                cost_time = re.findall("(?<=cost time = ).[0-9.]*", content[idx + 7])[0]
                res_tuple = (hit_rate1, hit_rate2, hit_rate3, cost_time)
                store(result_map, feature, cache_policy, cache_size, res_tuple)
                idx += 8
    return result_map


def plot_res(result_map, res_i, ylabel):
    import matplotlib.pyplot as plt

    feature_list = list(set(result_map.keys()))
    cache_policy_list = list(set(result_map[feature_list[0]].keys()))
    cache_size_list = list(set(result_map[feature_list[0]][cache_policy_list[0]].keys()))
    cache_size_list_float = [float(x) for x in cache_size_list]
    # print(feature_list)
    # print(cache_policy_list)
    # print(cache_size_list)
    filepath = {
        "user": ["dataset/taobao/shuffled_sample.csv", 0],
        "ad": ["dataset/taobao/shuffled_sample.csv", 1],
        "cate_id": ["dataset/taobao/shuffled_ad_feature.csv", 1],
        "campaign_id": ["dataset/taobao/shuffled_ad_feature.csv", 2],
        "customer": ["dataset/taobao/shuffled_ad_feature.csv", 3],
        "brand": ["dataset/taobao/shuffled_ad_feature.csv", 4],
        "price": ["dataset/taobao/shuffled_ad_feature.csv", 5],
        "random1": ["dataset/taobao/random_data.csv", 0],
        "random2": ["dataset/taobao/random_data.csv", 1],
        "random3": ["dataset/taobao/random_data.csv", 2],
    }
    # ["cate_id", "campaign_id", "customer", "brand", "price", "random1", "random2"]
    # res_list = {"ad": dict(), "user": dict()}
    # plot_f = ["ad", "user"]  # (13)
    # plot_f = ["cate_id", "campaign_id"]  # (14)
    # plot_f = ["customer", "brand"]  # (15)
    # plot_f = ["price", "random1"] # (16)
    # plot_f = ["random2", "random3"]  # (17)
    # plot_f = ["ad", "user", "cate_id", "campaign_id", "customer", "brand", "random2", "random3"]  # (18)
    # plot_f = ["ad", "user"]  # (19)
    # plot_f = ["user", "random3"]  # (20)
    plot_f = ["random3"]  # (21)
    res_list = {}
    for it in plot_f:
        res_list[it] = dict()
    for f in plot_f:
        if f not in res_list:
            res_list[f] = dict()
        for c in cache_policy_list:
            if c not in res_list[f]:
                res_list[f][c] = [cache_size_list_float, []]
            for s in cache_size_list:
                res_list[f][c][1].append(float(result_map[f][c][s][res_i]))

    color_list = ["royalblue", "chocolate", "olivedrab", "deepskyblue", "silver", "black", "tomato"]
    linestyle_list = ["solid", "dotted"]
    color_linestyle_list = []
    for c in color_list:
        for l in linestyle_list:
            color_linestyle_list.append([c, l])
    color_map = {}
    i = 0
    for c in cache_policy_list:
        color_map[c] = color_linestyle_list[i % len(color_linestyle_list)]
        i += 1
    color_map["dist"] = color_linestyle_list[i % len(color_linestyle_list)]

    fig2 = plt.figure()
    for i in range(len(plot_f)):
        # ax = fig2.add_subplot(2, len(plot_f) // 2, i + 1)
        ax = fig2.add_subplot(1, 1, i + 1)  # 1行2列
        for c in cache_policy_list:
            list1, list2 = (list(t) for t in zip(*sorted(zip(res_list[plot_f[i]][c][0], res_list[plot_f[i]][c][1]))))
            if res_i == 3:
                begin_idx = int(len(list1) * 0.3)
                list1 = list1[begin_idx:]
                list2 = list2[begin_idx:]
            ax.plot(list1, list2, color=color_map[c][0], label=c.upper(), linestyle=color_map[c][1])
        if res_i < 3:
            if plot_f[i] in dist_cache:
                ux, uy = dist_cache[plot_f[i]]
            else:
                ux, uy, _, _ = plot_dataset_dist_aux(filepath[plot_f[i]][0], filepath[plot_f[i]][1])
                dist_cache[plot_f[i]] = (ux, uy)
            ax.plot(ux, uy, color=color_map["dist"][0], label="dist", linestyle=color_map[c][1])
        ax.set_title(plot_f[i])
        ax.set_xlabel("Cache Size(%)")
        ax.set_ylabel(ylabel, labelpad=-4)
        ax.grid(True)
        ax.legend()
    plt.show()


all_log_file = "logs/cache/temp/single/all_age.log"
result_map = get_result_map(all_log_file)

dist_cache = {}

# plot_res(result_map, 0, "Hit Rate[1] (%)")
# plot_res(result_map, 1, "Hit Rate[2] (%)")
plot_res(result_map, 2, "Hit Rate[3] (%)")
plot_res(result_map, 3, "Time (s)")

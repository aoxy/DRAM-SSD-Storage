import re
import numpy as np


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
                feature = re.findall("(?<=feature = ).[a-z]*", content[idx + 1])[0]
                cache_size = re.findall("(?<=cache size = ).[0-9.]*", content[idx + 2])[0]
                cache_policy = re.findall("(?<=cache policy = ).[a-z]*", content[idx + 3])[0]
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
    res_list = {"ad": dict(), "user": dict()}
    for f in feature_list:
        if f not in res_list:
            res_list[f] = dict()
        for c in cache_policy_list:
            if c not in res_list[f]:
                res_list[f][c] = [cache_size_list_float, []]
            for s in cache_size_list:
                res_list[f][c][1].append(float(result_map[f][c][s][res_i]))

    fig2 = plt.figure()
    ax1 = fig2.add_subplot(1, 2, 1)
    color_list = ["tomato", "chocolate", "olivedrab", "deepskyblue", "silver", "royalblue"]
    color_map = {}
    i = 0
    for c in cache_policy_list:
        color_map[c] = color_list[i % len(color_list)]
        i += 1
    for c in cache_policy_list:
        list1, list2 = (list(t) for t in zip(*sorted(zip(res_list["user"][c][0], res_list["user"][c][1]))))
        ax1.plot(list1, list2, color=color_map[c], label=c.upper())
    ax1.set_title("user")
    ax1.set_xlabel("Cache Size(%)")
    ax1.set_ylabel(ylabel)
    ax1.grid(True)
    ax1.legend()

    ax2 = fig2.add_subplot(1, 2, 2)
    for c in cache_policy_list:
        list1, list2 = (list(t) for t in zip(*sorted(zip(res_list["ad"][c][0], res_list["ad"][c][1]))))
        ax2.plot(list1, list2, color=color_map[c], label=c.upper())
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("Cache Size(%)")
    ax2.set_ylabel(ylabel)
    ax2.grid(True)
    ax2.legend()
    plt.show()


all_log_file = "logs/cache/temp/single/all.log"
result_map = get_result_map(all_log_file)


plot_res(result_map, 0, "Hit Rate[1] (%)")
plot_res(result_map, 1, "Hit Rate[2] (%)")
plot_res(result_map, 2, "Hit Rate[3] (%)")
plot_res(result_map, 3, "Time (s)")

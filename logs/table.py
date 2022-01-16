import os
import re


def read_compaction(versions):
    res_list = []
    dir_name = "logs/compaction/" + versions[0]
    logs = os.listdir(dir_name)
    for log in logs:
        feature = log[5]  # "a" or "u"
        cache_size = re.findall(r"\d+\.?\d*", log)[0]
        cache_policy = log[-8:-5]
        filepath = dir_name + os.sep + log
        with open(filepath) as f:
            lines = f.readlines()
            duration_readid = float(re.findall(r"\d+\.?\d*", lines[-6])[1])
            duration_zerotrain = float(re.findall(r"\d+\.?\d*", lines[-5])[1])
            duration_train = float(re.findall(r"\d+\.?\d*", lines[-4])[1])
            duration_save = float(re.findall(r"\d+\.?\d*", lines[-3])[1])
            duration_total = float(re.findall(r"\d+\.?\d*", lines[-2])[1])
            hit_rate = float(re.findall(r"\d+\.?\d*", lines[-1])[1])
        res_list.append([feature, cache_policy, cache_size, hit_rate, duration_zerotrain, duration_train, duration_save, duration_total])
    for v in versions[1:]:
        dir_name = "logs/compaction/" + v
        for i, log in enumerate(logs):
            feature = log[5]  # "a" or "u"
            cache_size = re.findall(r"\d+\.?\d*", log)[0]
            cache_policy = log[-8:-5]
            filepath = dir_name + os.sep + log
            with open(filepath) as f:
                lines = f.readlines()
                duration_readid = float(re.findall(r"\d+\.?\d*", lines[-6])[1])
                duration_zerotrain = float(re.findall(r"\d+\.?\d*", lines[-5])[1])
                duration_train = float(re.findall(r"\d+\.?\d*", lines[-4])[1])
                duration_save = float(re.findall(r"\d+\.?\d*", lines[-3])[1])
                duration_total = float(re.findall(r"\d+\.?\d*", lines[-2])[1])
                hit_rate = float(re.findall(r"\d+\.?\d*", lines[-1])[1])
            cur_res = [feature, cache_policy, cache_size, hit_rate, duration_zerotrain, duration_train, duration_save, duration_total]
            for j in range(3, len(cur_res)):
                res_list[i][j] = res_list[i][j] + cur_res[j]
    if len(versions) > 1:
        for j in range(3, len(cur_res)):
            for i in range(len(logs)):
                res_list[i][j] = res_list[i][j] / len(versions)

    return res_list


def print_table(feature, cache_policy, res_list):
    print("===" * 20, feature, cache_policy)
    fmt_str = "| {} %      | {:.2f} % | {:.2f} s    | {:.2f} s   | {:.2f} s     | {:.2f} s   |"
    fmt_res = []
    for res in res_list:
        if res[0] == feature and res[1] == cache_policy:
            fmt_res.append([res[2], fmt_str.format(*res[2:])])
    fmt_res.sort(key=lambda x: int(x[0]), reverse=True)
    for i in fmt_res:
        print(i[1])


if __name__ == "__main__":
    res_list = read_compaction(["v2", "v3"])
    # print(res_list)
    print_table("u", "lru", res_list)
    print_table("u", "lfu", res_list)
    print_table("a", "lru", res_list)
    print_table("a", "lfu", res_list)

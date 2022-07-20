from math import prod

# feature_list = ["ad", "user"]
# feature_list = ["cate_id", "campaign_id", "customer", "brand", "price", "random1", "random2"]
feature_list = ["random3"]
cache_size_list = [5, 7, 10, 15, 20, 30, 40, 50, 60, 80, 90, 100]
cache_policy_list = ["lru", "fifo", "arf"]  # ["lru", "lfu", "fifo", "arc", "arf"]
max_aging_times_list = [0]  # [0, 1, 2, 5, 10, 50, 100, 500]
# print("# total epoch =", prod([len(feature_list), len(cache_size_list), len(cache_policy_list)]))
total_epoch = 0
for feature in feature_list:
    for cache_size in cache_size_list:
        for cache_policy in cache_policy_list:
            for max_aging_times in max_aging_times_list:
                print(f'make run_single_cache var="{feature} {cache_size} {cache_policy} {max_aging_times}"')
                total_epoch += 1
print("# total epoch =", total_epoch)

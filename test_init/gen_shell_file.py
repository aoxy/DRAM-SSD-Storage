from math import prod

# feature_list = ["ad", "user"]
feature_list = ["cate_id", "campaign_id", "customer", "brand", "price", "random1", "random2"]
cache_size_list = [0.1, 0.3, 0.6, 1, 2, 3, 4, 5, 7, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 95, 100]
cache_policy_list = ["lru", "lfu", "fifo", "arc", "arf"]
print("# total epoch =", prod([len(feature_list), len(cache_size_list), len(cache_policy_list)]))
for feature in feature_list:
    for cache_size in cache_size_list:
        for cache_policy in cache_policy_list:
            print(f'make run_single_cache var="{feature} {cache_size} {cache_policy}"')

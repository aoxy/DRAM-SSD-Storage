from math import prod

# feature_list = ["ad", "user"]
# feature_list = ["cate_id", "campaign_id", "customer", "brand", "price", "random1", "random2"]
feature_list = ["user"]
cache_size_list = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
cache_policy_list = ["lru", "lfu", "fifo", "arc", "arf"]
print(
    "# total epoch =",
    prod([len(feature_list), len(cache_size_list), len(cache_policy_list)]),
)
for feature in feature_list:
    for cache_size in cache_size_list:
        for cache_policy in cache_policy_list:
            print(f'make run8 var="{feature} {cache_size} {cache_policy}"')

import random

filepath = "dataset/taobao/random_data.csv"
line_num = 10000000
with open(filepath, "w") as f:
    f.write("random1,random2\n")
    for _ in range(line_num):
        random1 = random.randint(1, line_num / 10)
        random2 = random.randint(1, line_num / 10)
        if random2 % 10 > 7:
            random2 -= 7
        f.write(f"{random1},{random2}\n")

import random
import numpy as np

filepath = "dataset/taobao/random_data.csv"
line_num = 10000000
dsize = line_num // 20
line_size = [line_num // 3, line_num // 3, line_num - 2 * (line_num // 3)]
print(line_size)
scale = 2000
gauss1 = np.abs(np.random.normal(loc=dsize * 0.33, scale=scale, size=line_size[0]))
gauss2 = np.abs(np.random.normal(loc=dsize * 0.66, scale=scale, size=line_size[1]))
gauss3 = np.abs(np.random.normal(loc=dsize, scale=scale, size=line_size[2]))
gauss = np.concatenate((gauss1, gauss2, gauss3))
# gauss = np.abs(np.random.normal(loc=dsize, scale=scale, size=line_num))
# print(gauss1)
# print(gauss2)
# print(gauss3)
# print(gauss)
# exit(0)
with open(filepath, "w") as f:
    f.write("random1,random2,random3\n")
    for i in range(line_num):
        random1 = random.randint(1, line_num / 10)
        random2 = random.randint(1, line_num / 10)
        if random2 % 10 > 7:
            random2 -= 7
        random3 = int(gauss[i])
        f.write(f"{random1},{random2},{random3}\n")

# DRAM-SSD 混合 embedding 存储 POC

## 结果记录

### logs说明

- inplace 目录中的是每次读写embedding文件都打开关闭一次，且原地读写
- filepool 目录中的是打开embedding文件，最后训练结束才关闭，且追加写
- compaction 目录中的是打开embedding文件，最后训练结束才关闭，且追加写，最后compaction
### 最新结果

数据说明：

- dsize：不同的id数目
- data size：每次迭代访问的id数量
- Cache Size：cache大小占dsize的比例
- Hit Rate：3次迭代总的命中率
- Epoch Zero：刚开始时内存中一个embedding都没有，所以先训练一轮，不计入命中率，这样可以先在内存中加载热数据
- Compaction：compaction使用的时间，这一步会读如所有dsize个不同id和他们的embedding到内存，然后重写emb.hdss和offset文件
- Total Time：程序执行的总时间

PS：磁盘使用量可以参考[LRU淘汰append写文件](#lru淘汰append写文件时写入embedding的数量与id的数量的比值只遍历一轮的比较)

#### User 数据

```
dsize = 1141729,
data size = 26557961
batch_size = 512
k_size = 4096
```

##### LRU

| Cache Size | Hit Rate | Epoch Zero | Train Time | Compaction | Total Time |
| ---------- | -------- | ---------- | ---------- | ---------- | ---------- |
| 100 %      | 100.00 % | 46.88 s    | 150.13 s   | 4.83 s     | 216.72 s   |
| 70 %       | 93.61 %  | 57.51 s    | 196.19 s   | 5.29 s     | 273.27 s   |
| 50 %       | 86.15 %  | 73.80 s    | 248.67 s   | 7.16 s     | 343.84 s   |
| 40 %       | 80.31 %  | 88.09 s    | 293.10 s   | 7.50 s     | 402.93 s   |
| 30 %       | 72.26 %  | 108.03 s   | 336.01 s   | 7.96 s     | 466.26 s   |
| 20 %       | 60.93 %  | 126.28 s   | 402.17 s   | 8.66 s     | 552.20 s   |
| 10 %       | 43.29 %  | 156.01 s   | 477.06 s   | 10.39 s    | 657.76 s   |

##### LFU

| Cache Size | Hit Rate | Epoch Zero | Train Time | Compaction | Total Time |
| ---------- | -------- | ---------- | ---------- | ---------- | ---------- |
| 100 %      | 100.00 % | 30.24 s    | 105.02 s   | 5.39 s     | 154.96 s   |
| 70 %       | 95.91 %  | 41.26 s    | 140.29 s   | 6.57 s     | 202.43 s   |
| 50 %       | 90.49 %  | 56.83 s    | 182.57 s   | 7.23 s     | 260.89 s   |
| 40 %       | 86.16 %  | 64.68 s    | 211.60 s   | 7.70 s     | 298.25 s   |
| 30 %       | 79.74 %  | 76.02 s    | 242.32 s   | 8.21 s     | 340.92 s   |
| 20 %       | 69.99 %  | 98.92 s    | 307.53 s   | 9.43 s     | 430.18 s   |
| 10 %       | 54.06 %  | 125.62 s   | 396.65 s   | 9.87 s     | 546.51 s   |
#### Adgroup 数据

```
dsize = 846811
data size = 26557961
batch_size = 512
k_size = 4096
```

##### LRU

| Cache Size | Hit Rate | Epoch Zero | Train Time | Compaction | Total Time |
| ---------- | -------- | ---------- | ---------- | ---------- | ---------- |
| 100 %      | 100.00 % | 32.36 s    | 111.13 s   | 3.53 s     | 162.58 s   |
| 70 %       | 97.67 %  | 34.02 s    | 136.52 s   | 3.84 s     | 190.11 s   |
| 50 %       | 95.21 %  | 41.39 s    | 151.69 s   | 4.43 s     | 213.15 s   |
| 30 %       | 88.51 %  | 54.95 s    | 190.06 s   | 5.51 s     | 266.02 s   |
| 20 %       | 80.79 %  | 73.34 s    | 238.15 s   | 5.86 s     | 332.89 s   |
| 10 %       | 64.66 %  | 98.06 s    | 320.21 s   | 6.96 s     | 440.73 s   |
| 5 %        | 47.87 %  | 130.71 s   | 435.97 s   | 8.06 s     | 590.48 s   |

##### LFU

| Cache Size | Hit Rate | Epoch Zero | Train Time | Compaction | Total Time |
| ---------- | -------- | ---------- | ---------- | ---------- | ---------- |
| 100 %      | 100.00 % | 26.66 s    | 103.84 s   | 3.83 s     | 150.01 s   |
| 70 %       | 98.53 %  | 32.14 s    | 118.33 s   | 4.59 s     | 171.89 s   |
| 50 %       | 97.08 %  | 35.49 s    | 131.45 s   | 5.21 s     | 188.28 s   |
| 30 %       | 92.33 %  | 51.50 s    | 179.65 s   | 6.59 s     | 255.71 s   |
| 20 %       | 86.45 %  | 64.93 s    | 218.09 s   | 6.58 s     | 306.74 s   |
| 10 %       | 73.61 %  | 94.73 s    | 279.76 s   | 6.90 s     | 397.40 s   |
| 5 %        | 59.33 %  | 110.39 s   | 331.38 s   | 7.79 s     | 465.26 s   |
## 测试数据

```
-pg
./main
gprof -b main gmon.out > report.txt
```

```
dataset/taobao/raw_sample.csv
userid size: 1141729
adgroupid size: 846811
total time: 88.99227023124695 s
```

### 统计信息

#### 数据分布

![](fig/Figure_2.png)

#### LRU缓存策略的表现

![](fig/Figure_1.png)

#### LFU缓存策略的表现

![](fig/Figure_4.png)


#### 缓存大小和LRU命中率的关系

当Cache能放下x%的id时，最频繁出现的前x%的id占所有迭代id的比例是q%，命中率是p%，则`Hit Rate Ratio`定义为p/q

![](fig/Figure_3.png)

#### 缓存大小和LFU命中率的关系

![](fig/Figure_5.png)

#### LRU与LFU命中率的比较

![](fig/Figure_6.png)

#### LRU淘汰append写文件时，写入embedding的数量与id的数量的比值（只遍历一轮）的比较

![](fig/Figure_7.png)




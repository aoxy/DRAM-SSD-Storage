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
| 100 %      | 100.00 % | 37.87 s    | 127.85 s   | 4.49 s     | 184.46 s   |
| 70 %       | 93.61 %  | 52.75 s    | 186.55 s   | 5.44 s     | 258.92 s   |
| 50 %       | 86.15 %  | 70.91 s    | 240.58 s   | 6.54 s     | 332.26 s   |
| 40 %       | 80.31 %  | 84.57 s    | 280.51 s   | 7.48 s     | 386.77 s   |
| 30 %       | 72.26 %  | 102.57 s   | 327.15 s   | 7.83 s     | 451.73 s   |
| 20 %       | 60.92 %  | 120.59 s   | 388.04 s   | 8.32 s     | 531.20 s   |
| 10 %       | 43.25 %  | 147.39 s   | 469.81 s   | 10.27 s    | 641.72 s   |

##### LFU

| Cache Size | Hit Rate | Epoch Zero | Train Time | Compaction | Total Time |
| ---------- | -------- | ---------- | ---------- | ---------- | ---------- |
| 100 %      | 100.00 % | 28.28 s    | 98.06 s    | 5.20 s     | 145.77 s   |
| 70 %       | 95.91 %  | 39.11 s    | 135.53 s   | 5.58 s     | 194.40 s   |
| 50 %       | 90.49 %  | 50.28 s    | 168.12 s   | 6.37 s     | 238.99 s   |
| 40 %       | 86.16 %  | 59.20 s    | 194.89 s   | 6.60 s     | 274.92 s   |
| 30 %       | 79.74 %  | 69.15 s    | 231.06 s   | 7.74 s     | 322.18 s   |
| 20 %       | 69.98 %  | 89.61 s    | 287.03 s   | 8.48 s     | 399.33 s   |
| 10 %       | 54.04 %  | 114.77 s   | 354.46 s   | 9.13 s     | 492.54 s   |
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
| 100 %      | 100.00 % | 29.82 s    | 106.09 s   | 3.03 s     | 154.55 s   |
| 70 %       | 97.67 %  | 34.19 s    | 127.14 s   | 3.85 s     | 180.87 s   |
| 50 %       | 95.21 %  | 38.55 s    | 143.03 s   | 4.43 s     | 201.44 s   |
| 30 %       | 88.51 %  | 53.23 s    | 185.31 s   | 5.07 s     | 259.12 s   |
| 20 %       | 80.78 %  | 68.87 s    | 231.89 s   | 5.90 s     | 322.11 s   |
| 10 %       | 64.62 %  | 96.43 s    | 317.17 s   | 6.97 s     | 436.07 s   |
| 5 %        | 47.76 %  | 121.00 s   | 392.57 s   | 7.37 s     | 536.48 s   |

##### LFU

| Cache Size | Hit Rate | Epoch Zero | Train Time | Compaction | Total Time |
| ---------- | -------- | ---------- | ---------- | ---------- | ---------- |
| 100 %      | 100.00 % | 23.83 s    | 90.17 s    | 3.33 s     | 132.92 s   |
| 70 %       | 98.53 %  | 26.58 s    | 105.01 s   | 4.01 s     | 151.12 s   |
| 50 %       | 97.08 %  | 31.36 s    | 118.56 s   | 4.64 s     | 170.11 s   |
| 30 %       | 92.33 %  | 41.89 s    | 149.61 s   | 5.66 s     | 212.59 s   |
| 20 %       | 86.44 %  | 52.75 s    | 183.62 s   | 6.03 s     | 257.90 s   |
| 10 %       | 73.58 %  | 75.95 s    | 256.52 s   | 6.88 s     | 354.85 s   |
| 5 %        | 59.25 %  | 97.55 s    | 327.97 s   | 7.64 s     | 448.78 s   |
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




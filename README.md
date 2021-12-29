# DRAM-SSD 混合 embedding 存储 POC

## 存在的问题

1. 当前的 cache_manager 每 5 秒检查一次内存中实际存了的 embedding 数量，非常不准，系统中实际最多可以存放下的 embedding 数量远大于设定的值，导致缓存命中率计算错误
2. 多线程正确性

## 下一步

1. 用 embedding 池统一存储 embedding，管理内存使用
train time 细粒度
2. 上一步完成后改进 cache 策略的实现和设计更好的 cache 策略
3. 正确使用多线程

import collections


class Node:
    def __init__(self, key, val, pre=None, nex=None, freq=0):
        self.pre = pre
        self.nex = nex
        self.freq = freq
        self.val = val
        self.key = key

    def insert(self, nex):
        nex.pre = self
        nex.nex = self.nex
        self.nex.pre = nex
        self.nex = nex


def create_linked_list():
    head = Node(0, 0)
    tail = Node(0, 0)
    head.nex = tail
    tail.pre = head
    return (head, tail)


class LFUCache:
    def __init__(self, capacity: int):
        self.capacity = capacity
        self.size = 0
        self.minFreq = 0
        self.freqMap = collections.defaultdict(create_linked_list)
        self.keyMap = {}

    def delete(self, node):
        if node.pre:
            node.pre.nex = node.nex
            node.nex.pre = node.pre
            if node.pre is self.freqMap[node.freq][0] and node.nex is self.freqMap[node.freq][-1]:
                self.freqMap.pop(node.freq)
        return node.key

    def increase(self, node):
        node.freq += 1
        self.delete(node)
        self.freqMap[node.freq][-1].pre.insert(node)
        if node.freq == 1:
            self.minFreq = 1
        elif self.minFreq == node.freq - 1:
            head, tail = self.freqMap[node.freq - 1]
            if head.nex is tail:
                self.minFreq = node.freq

    def get(self, key: int) -> int:
        if key in self.keyMap:
            self.increase(self.keyMap[key])
            return self.keyMap[key].val
        return -1

    def put_ori(self, key: int, value: int) -> None:
        if self.capacity != 0:
            if key in self.keyMap:
                node = self.keyMap[key]
                node.val = value
            else:
                node = Node(key, value)
                self.keyMap[key] = node
                self.size += 1
            if self.size > self.capacity:
                self.size -= 1
                deleted = self.delete(self.freqMap[self.minFreq][0].nex)
                self.keyMap.pop(deleted)
            self.increase(node)

    def put(self, key: int, value: int) -> None:
        if self.capacity != 0:
            if key in self.keyMap:
                node = self.keyMap[key]
                node.val = value
            elif self.size == self.capacity:
                deleted = self.delete(self.freqMap[self.minFreq][0].nex)
                self.keyMap.pop(deleted)
                node = Node(key, value)
                self.keyMap[key] = node
            else:
                node = Node(key, value)
                self.keyMap[key] = node
                self.size += 1
            self.increase(node)


def lfu_hit_rate_2(visit_list, capacity_percent, visit_dup_size):
    visit_count = 0
    hit_count = 0
    miss_count = 0
    capacity = int(capacity_percent * visit_dup_size / 100)
    cache = LFUCache(capacity)
    value = 123
    # 预先加载一些数据进内存
    for key in visit_list:
        if cache.size >= cache.capacity:
            break
        elif cache.get(key) > 0:
            pass
        else:
            cache.put(key, value)
    # 正式开始迭代
    for key in visit_list:
        visit_count += 1
        if cache.get(key) > 0:
            hit_count += 1
        else:
            miss_count += 1
            cache.put(key, value)

    print(
        "LFUCache[{}]({} %) {} visit, {} miss, {} hit({} %)".format(
            capacity,
            capacity_percent,
            visit_count,
            miss_count,
            hit_count,
            "%.6g" % (hit_count * 100 / visit_count),
        )
    )
    return hit_count * 100 / visit_count


def lfu_hit_rate(visit_list, capacity_percent, visit_dup_size):
    visit_count = 0
    hit_count = 0
    miss_count = 0
    capacity = int(capacity_percent * visit_dup_size / 100)
    cache = LFUCache(capacity)
    value = 123
    # 预先加载一些数据进内存
    for key in visit_list:
        if key in cache.keyMap:
            node = cache.keyMap[key]
            node.val = value
        elif cache.size >= cache.capacity:
            break
        else:
            node = Node(key, value)
            cache.keyMap[key] = node
            cache.size += 1
        cache.increase(node)

    # 正式开始迭代
    for key in visit_list:
        visit_count += 1
        if key in cache.keyMap:
            hit_count += 1
            node = cache.keyMap[key]
            node.val = value
        elif cache.size >= cache.capacity:
            miss_count += 1
            deleted = cache.delete(cache.freqMap[cache.minFreq][0].nex)
            cache.keyMap.pop(deleted)
            node = Node(key, value)
            cache.keyMap[key] = node
        else:
            miss_count += 1
            node = Node(key, value)
            cache.keyMap[key] = node
            cache.size += 1
        cache.increase(node)

    print(
        "LFUCache[{}]({} %) {} visit, {} miss, {} hit({} %)".format(
            capacity,
            capacity_percent,
            visit_count,
            miss_count,
            hit_count,
            "%.6g" % (hit_count * 100 / visit_count),
        )
    )
    return hit_count * 100 / visit_count


def main():
    lfu = LFUCache(5)
    print(lfu.get(12))
    print(lfu.keyMap.keys())
    lfu.put(12, 1)
    print(lfu.keyMap.keys())
    lfu.put(14, 1)
    print(lfu.keyMap.keys())
    lfu.put(13, 1)
    print(lfu.keyMap.keys())
    lfu.put(15, 1)
    print(lfu.keyMap.keys())
    lfu.put(16, 1)
    print(lfu.keyMap.keys())
    lfu.put(12, 1)
    print(lfu.keyMap.keys())
    lfu.put(14, 1)
    print(lfu.keyMap.keys())
    lfu.put(19, 1)
    print(lfu.keyMap.keys())
    print(lfu.get(12))
    print(lfu.keyMap.keys())
    lfu.put(11, 1)
    print(lfu.keyMap.keys())
    lfu.put(17, 1)
    print(lfu.keyMap.keys())
    lfu.put(12, 1)
    print(lfu.keyMap.keys())


if __name__ == "__main__":
    main()

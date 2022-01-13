
## 每次写embedding都写一个打开关闭文件一次

```
user data size: 1141729
ad data size: 846811
writing user embedding... 100.00 %
writing adgroup embedding... 100.00 %
read id time: 0.46 s
wirte user embedding time: 11.75 s
wirte adgroup embedding time: 8.55 s
```

Mac

```
# aoxuyang @ macbook in ~/Documents/code/DRAM-SSD-Storage on git:main x [23:19:44] 
$ ./write_file_and_ssd_map 
user data size: 1141729
ad data size: 846811
writing user embedding... 100.00 %
writing adgroup embedding... 100.00 %
read id time: 0.50 s
wirte user embedding time: 216.61 s
wirte adgroup embedding time: 182.87 s
```

```
user data size: 1141729
ad data size: 846811
reading user embedding... 100.00 %
reading adgroup embedding... 100.00 %
read id time: 1.98 s
read user embedding time: 17.42 s
read adgroup embedding time: 13.06 s
```

## 写embedding时只打开一次文件，最后统一关闭

```
user data size: 1141729
ad data size: 846811
writing user embedding... 100.00 %
writing adgroup embedding... 100.00 %
read id time: 0.46 s
wirte user embedding time: 5.21 s
wirte adgroup embedding time: 2.96 s
```

Mac
```
$ ./write_file_and_ssd_map        
user data size: 1141729
ad data size: 846811
writing user embedding... 100.00 %
writing adgroup embedding... 100.00 %
read id time: 0.48 s
wirte user embedding time: 23.99 s
wirte adgroup embedding time: 17.15 s
```

```
root@11881896465a:/home/code/DRAM-SSD-Storage# ls -lh storage/ad/
total 655M
-rwxrwxrwx 1 root root 8.3M Jan 10 09:17 adgroupid_count.txt
-rwxrwxrwx 1 root root 5.6M Jan 10 09:17 all_adgroupid.txt
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb0.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb1.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb10.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb11.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb12.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb13.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb14.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb15.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb2.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb3.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb4.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb5.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb6.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb7.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb8.hdss
-rw-r--r-- 1 root root 6.9M Jan 11 15:38 emb9.hdss
-rwxrwxrwx 1 root root 116M Jan 11 14:31 embeddings.txt
-rw-r--r-- 1 root root  12M Jan 11 15:38 offset.txt
-rw-r--r-- 1 root root   64 Jan 11 08:03 offset_save.txt
-rw-r--r-- 1 root root 404M Jan 11 14:25 offset_saved.txt
```

```
root@11881896465a:/home/code/DRAM-SSD-Storage# ls -lh storage/user/
total 775M
-rwxrwxrwx 1 root root 7.7M Jan 10 09:17 all_userid.txt
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb0.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb1.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb10.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb11.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb12.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb13.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb14.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb15.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb2.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb3.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb4.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb5.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb6.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb7.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb8.hdss
-rw-r--r-- 1 root root 9.3M Jan 11 15:38 emb9.hdss
-rwxrwxrwx 1 root root 160M Jan 11 14:31 embeddings.txt
-rw-r--r-- 1 root root  17M Jan 11 15:38 offset.txt
-rw-r--r-- 1 root root 432M Jan 11 14:20 offset_saved.txt
-rwxrwxrwx 1 root root  12M Jan 10 09:17 userid_count.txt
```





敢开始
```
$ ls -lh storage/ad
total 282424
-rw-r--r--  1 aoxuyang  staff   5.5M  1 12 02:03 all_ids.txt
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb0.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb1.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb10.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb11.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb12.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb13.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb14.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb15.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb2.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb3.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb4.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb5.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb6.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb7.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb8.hdss
-rw-r--r--  1 aoxuyang  staff   6.9M  1 12 02:08 emb9.hdss
-rw-r--r--  1 aoxuyang  staff   8.3M  1 12 02:03 id_count.txt
-rw-r--r--  1 aoxuyang  staff    12M  1 12 02:08 offset.txt
```

1
```
$ ls -lh storage/ad
total 511800
-rw-r--r--  1 aoxuyang  staff   5.5M  1 12 02:03 all_ids.txt
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb0.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb1.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb10.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb11.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb12.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb13.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb14.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb15.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb2.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb3.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb4.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb5.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb6.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb7.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb8.hdss
-rw-r--r--  1 aoxuyang  staff    14M  1 12 02:10 emb9.hdss
-rw-r--r--  1 aoxuyang  staff   8.3M  1 12 02:03 id_count.txt
-rw-r--r--  1 aoxuyang  staff    12M  1 12 02:08 offset.txt
-rw-r--r--  1 aoxuyang  staff     0B  1 12 02:10 offset_saved.txt
```

2
```
$ ls -lh storage/ad
total 767808
-rw-r--r--  1 aoxuyang  staff   5.5M  1 12 02:03 all_ids.txt
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb0.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb1.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb10.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb11.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb12.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb13.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb14.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb15.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb2.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb3.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb4.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb5.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb6.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb7.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb8.hdss
-rw-r--r--  1 aoxuyang  staff    21M  1 12 02:19 emb9.hdss
-rw-r--r--  1 aoxuyang  staff   8.3M  1 12 02:03 id_count.txt
-rw-r--r--  1 aoxuyang  staff    12M  1 12 02:08 offset.txt
-rw-r--r--  1 aoxuyang  staff    13M  1 12 02:19 offset_saved.txt
```

begin make run var="user 70" 
```
$ ls -lh storage/user 
total 401784
-rw-r--r--  1 aoxuyang  staff   7.7M  1 12 02:03 all_ids.txt
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb0.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb1.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb10.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb11.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb12.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb13.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb14.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb15.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb2.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb3.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb4.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb5.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb6.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb7.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb8.hdss
-rw-r--r--  1 aoxuyang  staff   9.3M  1 12 02:07 emb9.hdss
-rw-r--r--  1 aoxuyang  staff    11M  1 12 02:03 id_count.txt
-rw-r--r--  1 aoxuyang  staff    16M  1 12 02:07 offset.txt
```
1 make run var="user 70"
```
$ ls -lh storage/user
total 2731664
-rw-r--r--  1 aoxuyang  staff   7.7M  1 12 02:03 all_ids.txt
-rw-r--r--  1 aoxuyang  staff    69M  1 12 02:27 emb0.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb1.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb10.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb11.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb12.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb13.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb14.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb15.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb2.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb3.hdss
-rw-r--r--  1 aoxuyang  staff    69M  1 12 02:27 emb4.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb5.hdss
-rw-r--r--  1 aoxuyang  staff    69M  1 12 02:27 emb6.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb7.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb8.hdss
-rw-r--r--  1 aoxuyang  staff    68M  1 12 02:27 emb9.hdss
-rw-r--r--  1 aoxuyang  staff    11M  1 12 02:03 id_count.txt
-rw-r--r--  1 aoxuyang  staff    16M  1 12 02:07 offset.txt
-rw-r--r--  1 aoxuyang  staff    17M  1 12 02:27 offset_saved.txt
```


make run var="user 100"
```
$ ls -lh storage/user
total 2732416
-rw-r--r--  1 aoxuyang  staff   7.7M  1 12 02:03 all_ids.txt
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb0.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb1.hdss
-rw-r--r--  1 aoxuyang  staff    77M  1 12 02:31 emb10.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb11.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb12.hdss
-rw-r--r--  1 aoxuyang  staff    77M  1 12 02:31 emb13.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb14.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb15.hdss
-rw-r--r--  1 aoxuyang  staff    77M  1 12 02:31 emb2.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb3.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb4.hdss
-rw-r--r--  1 aoxuyang  staff    77M  1 12 02:31 emb5.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb6.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb7.hdss
-rw-r--r--  1 aoxuyang  staff    77M  1 12 02:31 emb8.hdss
-rw-r--r--  1 aoxuyang  staff    78M  1 12 02:31 emb9.hdss
-rw-r--r--  1 aoxuyang  staff    11M  1 12 02:03 id_count.txt
-rw-r--r--  1 aoxuyang  staff    16M  1 12 02:07 offset.txt
-rw-r--r--  1 aoxuyang  staff    17M  1 12 02:31 offset_saved.txt
```
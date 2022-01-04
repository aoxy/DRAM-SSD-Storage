import matplotlib.pyplot as plt
import numpy as np

x = [1, 2, 3, 4, 5]
y = [1, 4, 9, 16, 25]
# plt.plot(x,y)
# plt.show()

cap_perc = [1, 5, 10, 20, 30, 50, 70, 100]

user_data = [
    (cap_perc, [7.99985, 28.9187, 43.6927, 61.157, 72.4288, 86.2242, 93.6659, 100]),
    (cap_perc, [7.99983, 28.9223, 43.6926, 61.1669, 72.4383, 86.2248, 93.6606, 100]),
    (cap_perc, [7.99867, 28.9174, 43.6895, 61.1708, 72.4364, 86.2309, 93.6696, 100]),
    (cap_perc, [8.00056, 28.9269, 43.6925, 61.1595, 72.4363, 86.2292, 93.6587, 100]),
    (cap_perc, [7.99964, 28.9264, 43.6869, 61.1704, 72.4436, 86.2316, 93.6613, 100]),
    (cap_perc, [7.99677, 28.9128, 43.6888, 61.1556, 72.4278, 86.2234, 93.6606, 100]),
]


ad_data = [
    (cap_perc, [20.09, 48.9124, 65.2088, 81.0437, 88.6488, 95.2525, 97.6852, 100]),
    (cap_perc, [20.0971, 48.9118, 65.2206, 81.0412, 88.6403, 95.2569, 97.6879, 100]),
    (cap_perc, [20.1036, 48.9074, 65.2075, 81.0423, 88.6498, 95.2578, 97.687, 100]),
    (cap_perc, [20.1025, 48.8989, 65.2182, 81.0491, 88.6483, 95.2511, 97.6853, 100]),
    (cap_perc, [20.0995, 48.9255, 65.2224, 81.0434, 88.6495, 95.2574, 97.6852, 100]),
    (cap_perc, [20.0979, 48.9093, 65.2215, 81.0467, 88.6445, 95.2498, 97.6853, 100]),
]

fig1 = plt.figure()
for i in range(len(user_data)):
    ax = fig1.add_subplot(6, 2, 2 * i + 1)
    ax.plot(user_data[i][0], user_data[i][1])
for i in range(len(ad_data)):
    ax = fig1.add_subplot(6, 2, 2 * i + 2)
    ax.plot(ad_data[i][0], ad_data[i][1])
plt.show()

res1 = np.zeros_like(np.array(user_data[0][1]))
res2 = np.zeros_like(np.array(ad_data[0][1]))
for i in user_data:
    res1 += np.array(i[1])
for i in ad_data:
    res2 += np.array(i[1])

avg_data = [
    (cap_perc, res1 / len(user_data)),
    (cap_perc, res2 / len(ad_data)),
]

fig2 = plt.figure()
ax1 = fig2.add_subplot(1, 2, 1)
ax1.plot(avg_data[0][0], avg_data[0][1])

ax2 = fig2.add_subplot(1, 2, 2)
ax2.plot(avg_data[1][0], avg_data[1][1])

plt.show()

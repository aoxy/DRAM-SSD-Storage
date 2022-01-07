import numpy as np

per_points = [1, 5, 10, 20, 30, 50, 70, 100]

u_result_lfu = [
    [8.31166970988473, 29.17531206556106, 44.07373743790045, 61.82663646505091, 73.27945846445064, 87.25451475736409, 94.70817055571398, 100.0],
    [8.208088715846822, 29.138671451471744, 43.97157221520131, 61.682811417638575, 73.19872184464764, 87.2184615377664, 94.71807342438676, 100.0],
    [8.156292570803911, 29.190546668850068, 44.09853979377408, 61.68831259297353, 73.22263934343454, 87.2392236738355, 94.72739266391723, 100.0],
    [8.123326937636515, 29.07229963926824, 44.047583321626234, 61.74038737386503, 73.19329221094948, 87.24046247375692, 94.7158744603925, 100.0],
    [8.253638146392339, 29.15351445843301, 44.05279833041399, 61.77196359313879, 73.2674959497079, 87.2308947211723, 94.71360772011074, 100.0],
]
a_result_lfu = [
    [20.967050143646194, 49.15865717251411, 65.4729404866586, 81.43481346327755, 89.13149620183567, 95.8766111600209, 98.44784770939305, 100.0],
    [20.950460014607295, 49.268225071947356, 65.4557817898746, 81.37434195343536, 89.08962175221208, 95.88315533711342, 98.45056629159144, 100.0],
    [20.986407804424445, 49.07355274751703, 65.54535568449701, 81.44642580053491, 89.13836043361913, 95.88545596553892, 98.45185780640314, 100.0],
    [21.032875980200437, 49.22075907860547, 65.48875495374061, 81.37481262209852, 89.10984920867983, 95.88195419068505, 98.45021987945536, 100.0],
    [21.152647976250886, 49.281490397549724, 65.5677519821646, 81.48462903458591, 89.147314434267, 95.87586562085846, 98.45134195354832, 100.0],
]

u_result_lru = [
    [7.99985, 28.9187, 43.6927, 61.157, 72.4288, 86.2242, 93.6659, 100],
    [7.99983, 28.9223, 43.6926, 61.1669, 72.4383, 86.2248, 93.6606, 100],
    [7.99867, 28.9174, 43.6895, 61.1708, 72.4364, 86.2309, 93.6696, 100],
    [8.00056, 28.9269, 43.6925, 61.1595, 72.4363, 86.2292, 93.6587, 100],
    [7.99964, 28.9264, 43.6869, 61.1704, 72.4436, 86.2316, 93.6613, 100],
    [7.99677, 28.9128, 43.6888, 61.1556, 72.4278, 86.2234, 93.6606, 100],
]


a_result_lru = [
    [20.09, 48.9124, 65.2088, 81.0437, 88.6488, 95.2525, 97.6852, 100],
    [20.0971, 48.9118, 65.2206, 81.0412, 88.6403, 95.2569, 97.6879, 100],
    [20.1036, 48.9074, 65.2075, 81.0423, 88.6498, 95.2578, 97.687, 100],
    [20.1025, 48.8989, 65.2182, 81.0491, 88.6483, 95.2511, 97.6853, 100],
    [20.0995, 48.9255, 65.2224, 81.0434, 88.6495, 95.2574, 97.6852, 100],
    [20.0979, 48.9093, 65.2215, 81.0467, 88.6445, 95.2498, 97.6853, 100],
]


def plot_lru_lfu():
    import matplotlib.pyplot as plt

    res1 = np.zeros_like(np.array(u_result_lfu[0]))
    res2 = np.zeros_like(np.array(a_result_lfu[0]))
    for i in u_result_lfu:
        res1 += np.array(i)
    for i in a_result_lfu:
        res2 += np.array(i)
    avg_data_lfu = [
        (per_points, res1 / len(u_result_lfu)),
        (per_points, res2 / len(a_result_lfu)),
    ]
    res3 = np.zeros_like(np.array(u_result_lru[0]))
    res4 = np.zeros_like(np.array(a_result_lru[0]))
    for i in u_result_lru:
        res3 += np.array(i)
    for i in a_result_lru:
        res4 += np.array(i)
    avg_data_lru = [
        (per_points, res3 / len(u_result_lru)),
        (per_points, res4 / len(a_result_lru)),
    ]

    fig2 = plt.figure()
    ax1 = fig2.add_subplot(1, 2, 1)
    ax1.plot(avg_data_lru[0][0], avg_data_lru[0][1], color="red", label="LRU")
    ax1.plot(avg_data_lfu[0][0], avg_data_lfu[0][1], color="skyblue", label="LFU")
    ax1.set_title("user")
    ax1.set_xlabel("LFU Cache Size(%)")
    ax1.set_ylabel("Hit Rate (%)")
    ax1.grid(True)
    ax1.legend()

    ax2 = fig2.add_subplot(1, 2, 2)
    ax2.plot(avg_data_lru[1][0], avg_data_lru[1][1], color="red", label="LRU")
    ax2.plot(avg_data_lfu[1][0], avg_data_lfu[1][1], color="skyblue", label="LFU")
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LFU Cache Size(%)")
    ax2.set_ylabel("Hit Rate (%)")
    ax2.grid(True)
    ax2.legend()
    plt.show()


def plot_avg_lfu():
    import matplotlib.pyplot as plt

    res1 = np.zeros_like(np.array(u_result_lfu[0]))
    res2 = np.zeros_like(np.array(a_result_lfu[0]))
    for i in u_result_lfu:
        res1 += np.array(i)
    for i in a_result_lfu:
        res2 += np.array(i)
    avg_data = [
        (per_points, res1 / len(u_result_lfu)),
        (per_points, res2 / len(a_result_lfu)),
    ]

    # avg_data
    # [8.21060322, 29.14606886, 44.04884622, 61.74202229, 73.23232156, 87.23671143, 94.71662376, 100.0]
    # [21.01788838, 49.20053689, 65.50611698, 81.42300457, 89.12332841, 95.88060845, 98.45036673, 100.0]

    fig2 = plt.figure()
    ax1 = fig2.add_subplot(1, 2, 1)
    ax1.plot(avg_data[0][0], avg_data[0][1])
    ax1.set_title("user")
    ax1.set_xlabel("Cache Size(%)")
    ax1.set_ylabel("Hit Rate (%)")
    ax1.grid(True)

    ax2 = fig2.add_subplot(1, 2, 2)
    ax2.plot(avg_data[1][0], avg_data[1][1])
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("Cache Size(%)")
    ax2.set_ylabel("Hit Rate (%)")
    ax2.grid(True)
    plt.show()


# plot_avg_lfu()
plot_lru_lfu()

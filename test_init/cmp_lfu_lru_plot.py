import numpy as np

per_points = [1, 5, 10, 20, 30, 50, 70, 100]

u_result_lfu = [
    [17.694547408967125, 40.61199954318782, 53.713427020997585, 69.7419391496207, 79.5804165839388, 90.5226647482463, 95.94674079083104, 100.0],
]
a_result_lfu = [
    [31.43179177046009, 59.408672224497955, 73.53103274758179, 86.40466035777371, 92.32975754426329, 97.10975929213843, 98.54443644977113, 100.0],
]

u_result_lru = [
    [7.993177638900818, 28.910928064093476, 43.6831050395774, 61.14668592216097, 72.43348613999396, 86.21781242919967, 93.66259329923709, 100.0],
]
a_result_lru = [
    [20.10309451090767, 48.92210286776157, 65.21746530164721, 81.05090221346435, 88.65144805356104, 95.25646942549542, 97.68756343907576, 100.0],
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
    ax1.set_xlabel("Cache Size(%)")
    ax1.set_ylabel("Hit Rate (%)")
    ax1.grid(True)
    ax1.legend()

    ax2 = fig2.add_subplot(1, 2, 2)
    ax2.plot(avg_data_lru[1][0], avg_data_lru[1][1], color="red", label="LRU")
    ax2.plot(avg_data_lfu[1][0], avg_data_lfu[1][1], color="skyblue", label="LFU")
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("Cache Size(%)")
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

    fig2 = plt.figure()
    ax1 = fig2.add_subplot(1, 2, 1)
    ax1.plot(avg_data[0][0], avg_data[0][1])
    ax1.set_title("user")
    ax1.set_xlabel("LFU Cache Size(%)")
    ax1.set_ylabel("Hit Rate (%)")
    ax1.grid(True)

    ax2 = fig2.add_subplot(1, 2, 2)
    ax2.plot(avg_data[1][0], avg_data[1][1])
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LFU Cache Size(%)")
    ax2.set_ylabel("Hit Rate (%)")
    ax2.grid(True)
    plt.show()


def plot_avg_lru():
    import matplotlib.pyplot as plt

    res1 = np.zeros_like(np.array(u_result_lru[0]))
    res2 = np.zeros_like(np.array(a_result_lru[0]))
    for i in u_result_lru:
        res1 += np.array(i)
    for i in a_result_lru:
        res2 += np.array(i)
    avg_data = [
        (per_points, res1 / len(u_result_lru)),
        (per_points, res2 / len(a_result_lru)),
    ]

    fig2 = plt.figure()
    ax1 = fig2.add_subplot(1, 2, 1)
    ax1.plot(avg_data[0][0], avg_data[0][1])
    ax1.set_title("user")
    ax1.set_xlabel("LRU Cache Size(%)")
    ax1.set_ylabel("Hit Rate (%)")
    ax1.grid(True)

    ax2 = fig2.add_subplot(1, 2, 2)
    ax2.plot(avg_data[1][0], avg_data[1][1])
    ax2.set_title("adgroup_id")
    ax2.set_xlabel("LRU Cache Size(%)")
    ax2.set_ylabel("Hit Rate (%)")
    ax2.grid(True)
    plt.show()


plot_avg_lfu()
plot_avg_lru()
plot_lru_lfu()

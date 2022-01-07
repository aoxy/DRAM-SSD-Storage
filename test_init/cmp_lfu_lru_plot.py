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
    [8.003991722105473, 28.902817501689984, 43.59804203342267, 60.90771049780516, 71.93986014212462, 85.08940501870606, 91.7139949109798, 95.7009915030751],
    [7.999108064056574, 28.90027965625825, 43.60070037003217, 60.90220179177159, 71.93060114818303, 85.08874984792695, 91.70762017460602, 95.7009915030751],
    [7.992337966005748, 28.89309160443454, 43.595666098011066, 60.89404227982713, 71.93160273109822, 85.08671279395281, 91.70844102075456, 95.7009915030751],
    [7.993102331914713, 28.893358944235214, 43.59696514352137, 60.88485482752234, 71.92157560589835, 85.08989827946506, 91.70783479951642, 95.7009915030751],
    [8.004044436995747, 28.900851989352645, 43.60188268971402, 60.902054943148684, 71.93590276000481, 85.09484971380145, 91.71376899002149, 95.7009915030751],
]


a_result_lru = [
    [20.094965121757653, 48.85595697651638, 65.11339481219963, 80.77834363865509, 88.16598156763617, 94.28962938834047, 96.21897931094936, 96.81146078947853],
    [20.083164517035023, 48.874693354659264, 65.11239322928444, 80.77240944815003, 88.16029965553454, 94.28772035624272, 96.22027459111037, 96.81146078947853],
    [20.09264190123632, 48.85762879160791, 65.11040135950196, 80.76603094642695, 88.16293540004821, 94.28789732766006, 96.2187872781348, 96.81146078947853],
    [20.089991095325427, 48.869915126390914, 65.11426460788914, 80.77710860408297, 88.16395580970995, 94.28689574474487, 96.21982651454304, 96.81146078947853],
    [20.080099522700557, 48.86891354347572, 65.1031078778977, 80.7666446983637, 88.16409512763424, 94.28624810466437, 96.21769909218558, 96.81146078947853],
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

    # avg_data
    # [8.21060322, 29.14606886, 44.04884622, 61.74202229, 73.23232156, 87.23671143, 94.71662376, 100.0]
    # [21.01788838, 49.20053689, 65.50611698, 81.42300457, 89.12332841, 95.88060845, 98.45036673, 100.0]

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

    # avg_data
    # [7.9985169, 28.89807994, 43.59865127, 60.89817287, 71.93190848, 85.08992313, 91.71033198, 95.7009915]
    # [20.08817243, 48.86542156, 65.11071238, 80.77210747, 88.16345351, 94.28767818, 96.21911336, 96.81146079]

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


# plot_avg_lfu()
plot_avg_lru()
# plot_lru_lfu()

import numpy as np


feb_list = [71, 76, 162, 161, 160, 163, 164, 165, 72, 75]

front_top = np.array([5.1, -4.1, -4.2, -8.6, -4.4, 4.4, 8.5, 2.4, (1.5 + (1.0/16))*2.54])

front_bottom = np.array([33.1, -2.4, -2.4, -10.0, -5.0, 4.2, 9.2, (1.0 + (11.0/32))*2.54, (1.0 - (5.0/32))*2.54])

back_bottom = np.array([33.2, -1.8, -1.8, -10.1, -4.3, 4.8, 10.0, (1.0 + (11.0/32))*2.54, (1.0 - (5.0/32))*2.54])

back_top = np.array([5.4, -4.1, -4.2, -8.5, -4.5, 4.6, 8.6, 4.3, 2.4])




z_avg_bottom = (front_bottom + back_bottom)/2.0
z_avg_top = (front_top + back_top)/2.0



z_avg_bottom[1] = z_avg_bottom[1] + z_avg_bottom[3]
z_avg_bottom[2] = z_avg_bottom[2] + z_avg_bottom[3]

z_avg_bottom[-1] = z_avg_bottom[-1] + z_avg_bottom[-3]
z_avg_bottom[-2] = z_avg_bottom[-2] + z_avg_bottom[-3]


z_avg_top[1] = z_avg_top[1] + z_avg_top[3]
z_avg_top[2] = z_avg_top[2] + z_avg_top[3]

z_avg_top[-1] = z_avg_top[-1] + z_avg_top[-3]
z_avg_top[-2] = z_avg_top[-2] + z_avg_top[-3]


print("Bottom", z_avg_bottom)
print("Top", z_avg_top)



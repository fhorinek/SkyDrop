import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
import math

def add_line(name, x, index):
    item = {}
    item["name"] = name
    item["data"] = np.zeros(len(x))
    item["index"] = index
    item["axis"] = False
    
    return item

time = np.arange(2000)
y = []
# y.append(add_line("qw", time, 0))
# y.append(add_line("qx", time, 1))
# y.append(add_line("qy", time, 2))
# y.append(add_line("qz", time, 3))

# y.append(add_line("acc_x", time, 4))
# y.append(add_line("acc_y", time, 5))
# y.append(add_line("acc_z", time, 6))

# y.append(add_line("mag_x", time, 7))
# y.append(add_line("mag_y", time, 8))
# y.append(add_line("mag_z", time, 9))

# y.append(add_line("gyro_x", time, 10))
# y.append(add_line("gyro_y", time, 11))
# y.append(add_line("gyro_z", time, 12))

# y.append(add_line("zTrack", time, 13))
y.append(add_line("vTrack", time, 14))

index = 0


s = serial.Serial("/dev/ttyUSB0", 921600)

fig = plt.figure(1, figsize=(15,13))
ax = fig.add_subplot(111)

leg = []
for line in y:
    line["axis"], = ax.plot(time, line["data"])
    leg.append(line["name"])
    
plt.ion()
plt.legend(leg)
plt.show()



    
while True:
    line = s.readline()
    data = line.split(";")
#     print data
    
    for line in y:
        val = 0
        try:
            tmp = data[line["index"]]
            val = float(tmp)
        except:
            print "Err"
        line["data"][index] = val


    '''draw'''
    if index % 15 == 0:
        v_min = 100000
        v_max = -100000
        
        for line in y:
            line["axis"].set_ydata(line["data"])
            if max(line["data"]) > v_max:
                v_max = max(line["data"])
            if min(line["data"]) < v_min:
                v_min = min(line["data"])
            
        ax.set_ylim([v_min, v_max])
        ax.figure.canvas.draw()
        
        
        
    index = (index + 1) % len(time)
    

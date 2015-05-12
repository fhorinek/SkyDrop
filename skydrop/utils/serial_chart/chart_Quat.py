import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np
from math import atan2, sin, cos, asin, pi, sqrt, acos

def add_line(name, x, index):
    item = {}
    item["name"] = name
    item["data"] = np.zeros(len(x))
    item["index"] = index
    item["axis"] = False
    
    return item

time = np.arange(2)
y = []
y.append(add_line("qw", time, 0))
y.append(add_line("qx", time, 1))
y.append(add_line("qy", time, 2))
y.append(add_line("qz", time, 2))
index = 01


s = serial.Serial("/dev/ttyUSB0", 921600)

fig = plt.figure(1, figsize=(15,13))
axs = fig.add_subplot(111, projection='3d')

axis_x, = axs.plot(time, [0,0], [0,0], "r")
axis_y, = axs.plot(time, [0,0], [0,0], "g")
axis_z, = axs.plot(time, [0,0], [0,0], "b")
single, = axs.plot(time, [0,0], [0,0], "k-o", lw=5, zs = [0,0])
axs.set_autoscale_on(True)
    
leg = ["x", "y", "z", "sum"]
plt.legend(leg)
    
plt.ion()
plt.show()

v_min = 1
v_max = -1

skip = 0
    
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
    if skip % 20 == 0:
        qw = y[0]["data"][index]
        qx = y[1]["data"][index]
        qy = y[2]["data"][index]
        qz = y[3]["data"][index]

        qmax = sqrt(qw*qw + qx*qx + qy*qy + qz*qz)
        qw /= qmax
        qx /= qmax
        qy /= qmax
        qz /= qmax
        
        angle = 2 * acos(qw);
        qs = sqrt(1 - qw * qw);
        if (s < 0.001):
            vx = qx
            vy = qy
            vz = qz
        else:
            vx = qx / qs
            vy = qy / qs
            vz = qz / qs
        
        ax = [0, vx]
        ay = [0, vy]
        az = [0, vz]

        
        single.set_xdata(ax)
        single.set_ydata(ay)
        single.set_3d_properties(zs = az)

        x_data = ax
        y_data = [0, 0]

        axis_x.set_xdata(x_data)
        axis_x.set_ydata(y_data)
        axis_x.set_3d_properties(zs = [0,0])
        
        x_data = [ax[1], ax[1]]
        y_data = ay
        
        axis_y.set_xdata(x_data)
        axis_y.set_ydata(y_data)
        axis_y.set_3d_properties(zs = [0,0])
        
        x_data = [ax[1], ax[1]]
        y_data = [ay[1], ay[1]]
        
        axis_z.set_xdata(x_data)
        axis_z.set_ydata(y_data)
        axis_z.set_3d_properties(zs = az)
            
        axs.set_ylim([-v_max, v_max])
        axs.set_xlim([-v_max, v_max])
        axs.set_zlim([-v_max, v_max])
        axs.figure.canvas.draw()
        
        
    skip += 1
#     index = 1
    

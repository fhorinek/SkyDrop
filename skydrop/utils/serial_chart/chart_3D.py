import serial
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

def add_line(name, x, index):
    item = {}
    item["name"] = name
    item["data"] = np.zeros(len(x))
    item["index"] = index
    item["axis"] = False
    
    return item

time = np.arange(2)
y = []
y.append(add_line("x", time, 3))
y.append(add_line("y", time, 4))
y.append(add_line("z", time, 5))
index = 01


s = serial.Serial("/dev/ttyUSB0", 921600)

fig = plt.figure(1, figsize=(15,13))
ax = fig.add_subplot(111, projection='3d')

axis_x, = ax.plot(time, y[0]["data"], y[1]["data"], "r")
axis_y, = ax.plot(time, y[0]["data"], y[1]["data"], "g")
axis_z, = ax.plot(time, y[0]["data"], y[1]["data"], "b")
single, = ax.plot(time, y[0]["data"], y[1]["data"], "k-o", lw=5, zs = y[2]["data"])
ax.set_autoscale_on(True)
    
leg = ["x", "y", "z", "sum"]
plt.legend(leg)
    
plt.ion()
plt.show()

v_min = 100000
v_max = -100000

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
        if val > v_max:
            v_max = val
        if val < v_min:
            v_min = val

    '''draw'''
    if skip % 20 == 0:
        single.set_xdata(y[0]["data"])
        single.set_ydata(y[1]["data"])
        single.set_3d_properties(zs = y[2]["data"])

        x_data = [0, y[0]["data"][1]]
        y_data = [0, 0]

        axis_x.set_xdata(x_data)
        axis_x.set_ydata(y_data)
        axis_x.set_3d_properties(zs = [0,0])
        
        x_data = [y[0]["data"][1], y[0]["data"][1]]
        y_data = [0, y[1]["data"][1]]
        
        axis_y.set_xdata(x_data)
        axis_y.set_ydata(y_data)
        axis_y.set_3d_properties(zs = [0,0])
        
        x_data = [y[0]["data"][1], y[0]["data"][1]]
        y_data = [y[1]["data"][1], y[1]["data"][1]]
        
        axis_z.set_xdata(x_data)
        axis_z.set_ydata(y_data)
        axis_z.set_3d_properties(zs = y[2]["data"])
            
        ax.set_ylim([-v_max, v_max])
        ax.set_xlim([-v_max, v_max])
        ax.set_zlim([-v_max, v_max])
        ax.figure.canvas.draw()
        
        
    skip += 1
#     index = 1
    

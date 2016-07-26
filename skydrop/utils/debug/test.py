#!/usr/bin/python
import matplotlib.pyplot as plt

f = open("bat.log", "r")

size = 0
offset = False
x = []
y = []
per = []
per2 = []

ofs = 0

max_adc = 3692
min_adc = 3200
adc_dif = max_adc - min_adc

for line in f.readlines():
    size += 1
    data = line.replace("\n", "").split(" ")
    time = int(data[2])
    adc = int(data[4])
    val = int(data[3])
    
    if ofs == 0:
        ofs = time
    time -= ofs
    x.append(time / (1000.0 * 60 * 60))
    y.append(adc)
    per.append(val)
    
    
    val = ((adc - min_adc) * 105) / adc_dif;
    if val > 100:
        val = 100
    if val < 0:
        val = 0
    per2.append(val)
    
        
f.close()

# plt.plot(x, y)
# plt.hold(1)
# plt.plot(x, per)
# plt.show()

fig = plt.figure()
ax = fig.add_subplot(111)
ax.plot(x, y, '-b')
ax2 = ax.twinx()
ax2.plot(x, per, '-r')
ax2.plot(x, per2, '-g')
ax.grid()
plt.show()



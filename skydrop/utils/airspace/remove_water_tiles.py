#!/usr/bin/python3

import os

f = open("agl_tiles.list", "r")
data = f.readlines()
f.close()

agl_list = []

for a in data:
    a = a.split(".")[0] + ".AIR"
    agl_list.append(a)
                
#load files
for fname in os.listdir("data"):
    if fname not in agl_list:
        print("%s not in agl data" % fname)
        os.remove("data/%s" % fname)


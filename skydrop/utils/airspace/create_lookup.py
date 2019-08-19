#!/usr/bin/python3

import os

table = {}

#load files
for fname in os.listdir("lists"):
    f = open("lists/" + fname, "r")
    fname = fname.replace(".list", "")
    for line in f.readlines():
        line = line.split()[0]
        
        if line in table:
            table[line].append(fname)
        else:
            table[line] = [fname]
        
    f.close()

if not os.path.isdir("lookup"):
                os.mkdir("lookup")    

for key in table:
    f = open("lookup/%s" % key, "w")
    for k in table[key]:
        f.write(k + "\n")
    f.close()


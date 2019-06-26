#!/usr/bin/python3

import os
import sys

table = {}

filename = sys.argv[1]

dst = os.path.basename(filename).replace("openaip_airspace_", "").replace(".aip", "").replace(".list", "") + ".zip"

if not os.path.isdir("dist"):
    os.mkdir("dist")

os.system("rm dist/%s" % dst)

#load files
f = open(filename, "r")

for line in f.readlines():
    line = line.split()[0]
    
    os.system("zip -g -j dist/%s data/%s" % (dst, line))
    
f.close()

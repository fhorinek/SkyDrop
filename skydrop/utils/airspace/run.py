#!/usr/bin/python3

import os

res = []
ssize = 0
isize = []
iall = []

for fname in os.listdir("lists"):
    f = open("lists/" + fname, "r")
    fname = fname.replace(".list", "")
    
    cnt = 0
    done = 0
    csize = 0
    working = 0
    
    for line in f.readlines():
        line = line.split()[0]

        if line not in iall:
            iall.append(line)
            
        cnt += 1
        if os.path.exists("data/%s" % line):
            size = os.path.getsize("data/%s" % line)
            if size == 0:
                working += 1
                continue

            done += 1
            csize += size
            
            if line not in isize:
                isize.append(line)
                ssize += size
            
        
    f.close()

    data = {}
    data["name"] = fname
    data["done"] = done
    data["cnt"] = cnt
    data["size"] = csize / (1024 * 1024)
    data["working"] = working
    
    res.append(data)


def score(a):
    if a["cnt"] == 0:
        return 0
        
    return a["cnt"] * 10000 + a["cnt"]

    
res.sort(key = score, reverse = False)    

for data in res:
    os.system("./convert.py source/" + data["name"])


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
        
    return (a["done"] / a["cnt"] * 1000000000) + a["done"] * 10000 + a["cnt"]

    
res.sort(key = score, reverse = True)    

for data in res:
    fname = data["name"]
    done = data["done"]
    cnt = data["cnt"]
    size = data["size"]
    working = data["working"]
    
    dst = fname.replace("openaip_airspace_", "").replace(".aip", "") + ".zip"
    zipped = os.path.exists("dist/%s" % dst)
    if zipped:
        zsize = os.path.getsize("dist/%s" % dst) / (1024 * 1024)

    print(" %-55s %4u / %4u   %3.0f %% %8.1fMB  %s%s%s" % 
        (fname, done, cnt, 
        (done * 100) / cnt if done > 0 else 0, size, 
        "done!" if cnt == done else "", 
        ("working (%u)..." % working) if working > 0 else "",
        ("  ZIP (%5.1fMB)" % zsize) if zipped else ""))
        
print("-" * 100)
print(" "* 50, "Total %4u / %4u   %3.0f %% %8.1fMB" % (len(isize), len(iall), (len(isize) * 100)/ len(iall), ssize / (1024 * 1024)))


#!/usr/bin/python

f = open("bt_dbg.log", "r")

stats = {}
stats["same"] = 0
stats["total"] = 0
stats["cnt"] = {}

for line in f.readlines():
    if line[:4] == "old ":
        old = int(line[4:])
    if line[:4] == "act ":
        act = int(line[4:])
        
        stats["total"] += 1
        
        if old == act:
            stats["same"] += 1
            
        
f.close()

print stats
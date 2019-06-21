#!/usr/bin/python3

import os

for d in os.listdir("data"):
    for f in os.listdir("data/%s" % d):
        fil = open("data/%s/%s" % (d, f), "r")
        
        print("%s/%s:" % (d, f))
        time = None
        
        for line in fil.readlines():
            
            if line.find("HFDTE") == 0:
                date = line[5:7] + "." + line[7:9] + ".20" + line[9:11]
                print("  Date: %s" % date)
                
            if line.find("B") == 0 and time is None:
                time = line[1:3] + ":" + line[3:5] + ":" + line[5:7]
                print("  Start time: %s" % time)

            if line.find("LXSB  SKYDROP-DURATION-ms") == 0:
                sec = int(line[27:]) / 1000
                
                hour = sec / (60 * 60)
                sec %= 60 * 60
                mins = sec / 60
                sec %= 60
                
                
                print("  Duration: %02u:%02u:%02u" % (hour, mins, sec))
                
                
        print()
        fil.close()

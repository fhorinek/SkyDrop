#!/usr/bin/python3

import os

path = "../../Release/src"

to_analyse = []

def scan(path):
    print(path)
    for file in os.scandir(path):
        
        if file.is_dir():
            scan(os.path.join(path, file.name))
        
        if file.name[-2:] == ".o":
            to_analyse.append(os.path.join(path, file.name))
        

scan(path)        
print(to_analyse)

result = {}

print ("   text       data        bss        dec        hex    filename")
for file in to_analyse:
    data = os.popen("avr-size %s" % file).read().split("\n")[1]
    print(data)
    

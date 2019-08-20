#!/usr/bin/python3

import os

def write_to_file(f, d):
    f = open(f, "w")
    for item in d:
        f.write(" %06x  %5u %3s '%s'\n" % (item[0], item[1], item[2], item[3]))
    f.close()

data = os.popen("avr-nm -S -C --size-sort -r ../../Release/skydrop.elf").read()

flash = []
ram = []

flash_used = 0
ram_used = 0

for line in data.split("\n"):
    fields = line.split(maxsplit = 3)
    
    if len(fields) != 4:
#        print(len(fields), fields)
        continue
    
    addr = int(fields[0], 16)
    size = int(fields[1], 16)
    name = fields[3]
    
    t = fields[2]
    
    
    if t in "tT": #text
        row  = [addr, size, "TEXT    ", name]

        flash.append(row)    
        flash_used += size

    elif t in "dD": #data
        row  = [addr, size, "    DATA", name]

        flash.append(row)        
        flash_used += size
        ram.append(row)
        ram_used += size

    elif t in "bB": #bss
        row  = [addr, size, "BSS     ", name]
        ram.append(row)
        ram_used += size
  


write_to_file("ram_usage.txt", ram)
write_to_file("flash_usage.txt", flash)

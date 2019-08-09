#!/usr/bin/python3
#
# This file reads a BAT-CAL.RAW file and prints the content to stdout
# as a CSV file. This can be used to draw a diagram of the battery
# discharge.
#
# usage:
#   % python3 bat-cal-to-csv.py BAT-CAL.RAW > bat.csv
#   % ooffice bat.csv 
#
# 2019-08-03: tilmann@bubecks.de
#

import sys

def usage():
    print("bat-cal-to-csv.py BAT-CAL.RAW\n")
    
if len(sys.argv) != 2:
    usage()
    sys.exit(1)

f = open(sys.argv[1], "rb")
while True:
    chunk = f.read(2)
    if len(chunk) == 0:
        break
    val = int.from_bytes(chunk, 'little', signed=False)
    print(val)
    

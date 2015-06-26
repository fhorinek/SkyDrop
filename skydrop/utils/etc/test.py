#!/usr/bin/python

file = open("TEST.CSV", "r")
lines = file.read().split("\n")
file.close()

def print_bites(val):
    s = ""
    for i in range(8):
        if (val & 1 << (7-i)):
            s += "1"
        else:
            s += "."
    return s

def print_bites2(val):
    s = ""
    for i in range(8):
        if (i == 4):
            s += " "
        if (val & 1 << (7-i)):
            s += "1"
        else:
            s += "."
    return s

def cmp_r(a,b):
    a, b = map(print_bites, [a, b])
    for i in range(8):
        if a[i] != b[7-i]:
            return False
    return True

for line in lines:
    if (line == ""):
        break;
    a, b = map(int, line.split(";"))
    if (not cmp_r(a, b)):
        print "%03d %03d %s | %s" % (a, b, print_bites2(a), print_bites2(b))
    
    
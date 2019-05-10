#!/usr/bin/python

import sys
import struct

mul = 10000000
div = 8340
width = 1201

AGL_PATH = "/media/horinek/f60241c8-9f0b-4a61-85c4-f8baff2b2043/srtm_hgt/hgt/"

def get_alt_point(point):
    lat = int(point.y * mul)
    lon = int(point.x * mul)
    return get_alt(lat, lon)

def get_alt(lat, lon):
    if lat >= 0:
        lat_c = "N"
    else:
        lat_c = "S"

    if lon >= 0:
        lon_c = "E"

    else:
        lon_c = "W"
     
    lat_n = abs(lat / mul)
    lon_n = abs(lon / mul)

    y = (lat % mul) / div
    x = (lon % mul) / div

    name = "%c%02u%c%03u.HGT" % (lat_c, lat_n, lon_c, lon_n)

    pos1 = int(x + width * ((width - y) - 1))
    pos2 = int(x + width * ((width - y) - 2))
    
    
    pos1 *= 2
    pos2 *= 2

    f = None
    raw1 = 'hovno'
    raw2 = 'hovno'
    
    f = open("%s/%s" % (AGL_PATH, name), "rb")

    f.seek(pos1)
    raw1 = f.read(4)
    alt11, alt21 = struct.unpack(">hh", raw1)
    
    f.seek(pos2)
    raw2 = f.read(4)
    alt12, alt22 = struct.unpack(">hh", raw2)
    f.close()
    
    
    #get displacement
    lat_dr = ((lat % mul) % div) / float(div)
    lon_dr = ((lon % mul) % div) / float(div)

    
    alt1 = alt11 + (alt12 - alt11) * lat_dr
    alt2 = alt21 + (alt22 - alt21) * lat_dr
    
    alt = alt1 + (alt2 - alt1) * lon_dr


    f.close()

    return alt




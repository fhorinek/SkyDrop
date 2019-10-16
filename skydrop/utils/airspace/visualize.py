#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# This script can be used to visualize the content of an AIR file
# containing airspace data. It will draw a map showing the contents of
# the file as a raster with points and arrows.
#
# By using the right button on the map, the user can see the underlying
# data. It is read from data directory, where the AIR data files from
# SkyDrop must be accessible.
#
# It will draw arrows to the stored points and airspaces and log some
# output to stdout.
#
# 20.12.2018: tilmann@bubecks.de
#
# Fedora: dnf install python3-pyserial python3-shapely python3-gdal python3-matplotlib

from math import sqrt,cos,atan2,floor,sin,asin
import re
import os
import numpy
import sys
from PIL import Image, ImageDraw
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrow
import getopt

import shapely
import shapely.ops
import shapely.geometry
from Airspace import Airspace
import openaip

import numpy as np

from const import *
from gps_calc import *

from math import sqrt

# Compute a point P2 which is distance_m meter in angle "angle" away from
# given point P.
#
# @param P the point to displace
# @param angle the angle in degree
# @param distance_m the distance in meter
#
# return new Point
#

def displacePoint(P, angle, distance_m):
    # taken from https://gis.stackexchange.com/questions/5821/calculating-latitude-longitude-x-miles-from-point
    lon1 = P.x
    lat1 = P.y

    angle = numpy.radians(-angle)
    lat1Radians = lat1 * (numpy.pi / 180.0)
    lon1Radians = lon1 * (numpy.pi / 180.0)
    distanceRadians = (distance_m / 1000) / 6371
    lat = asin(sin(lat1Radians)*cos(distanceRadians)+cos(lat1Radians)*sin(distanceRadians)*cos(angle));
    if cos(lat) == 0:
        lon = lon1Radians;
    else:
        lon = numpy.fmod((lon1Radians - asin(sin(angle) * sin(distanceRadians) / cos(lat)) + numpy.pi), (2 * numpy.pi)) - numpy.pi

    newLat = lat * (180 / numpy.pi)
    newLon = lon * (180 / numpy.pi)

    p2 =  shapely.geometry.Point(newLon, newLat)
    return p2


def get_point(lat, lon):
    mul = 10000000
    debug = False

    p = shapely.geometry.Point(lon, lat)

    print("Point (lat, lon)", lat, lon)

    lat = int(lat * mul)
    lon = int(lon * mul)

    print("Point (lat, lon)", lat, lon)

    lat_n = int((lat / mul))
    lon_n = int((lon / mul))

    if lat >= 0:
        lat_c = "N"
    else:
        lat_c = "S"
        lat_n -= 1

    if lon >= 0:
        lon_c = "E"
    else:
        lon_c = "W"
        lon_n -= 1

    name = "data/%c%02u%c%03u.AIR" % (lat_c, abs(lat_n), lon_c, abs(lon_n))

    print(" filename is '%s'" % name)

    if not os.path.exists(name):
        return

    
    filesize = os.path.getsize(name)
    f = open(name, "rb")

    numPoints = 200

    x = int((lon % mul) * numPoints / mul)
    y = int((lat % mul) * numPoints / mul)

    print("x, y", x, y)

    offset_point = int((x * numPoints + y) * DATA_LEVELS * DATA_LEVEL_SIZE)
    offset_index = int((numPoints * numPoints) * DATA_LEVELS * DATA_LEVEL_SIZE)

    print("offset_point", offset_point)


    global patch
    global ax        

    for pt in patch:
        pt.remove()

    patch = []

    for i in range(DATA_LEVELS):    
        f.seek(offset_point + DATA_LEVEL_SIZE * i, os.SEEK_SET)    
        
        index = int.from_bytes(f.read(1), 'little', signed=False)
        a =     int.from_bytes(f.read(1), 'little', signed=False)
        b =     int.from_bytes(f.read(1), 'little', signed=False)

        inside = bool(index & 0x80)
        
        if index == 0x7F:
            print ("  level", i, "---")
            continue        

        index &= 0x7F
        
        mode = (a & 0x80) >> 6 | (b & 0x80) >> 7

        origin = shapely.geometry.Point(lon_n + ((x + 0.5) / float(numPoints)), 
                                        lat_n + ((y + 0.5) / float(numPoints)))
                                        
        plt.plot(origin.x, origin.y, '.', color="red")
        
        if debug:
            print("origin lat", origin.y * 10000000)
            print("origin lon", origin.x * 10000000)        
        
        if mode == 0:
            mul = OFFSET_MUL_0    
        elif mode == 1:
            mul = OFFSET_MUL_1    
        elif mode == 2:
            mul = OFFSET_MUL_2    

        lat_offset = a & 0x7F
        long_offset = b & 0x7F


        if lat_offset & 0x40:
            lat_offset = -(lat_offset & 0x3F)
        if long_offset & 0x40:
            long_offset = -(long_offset & 0x3F)

        if debug:
            print("lat_offset", lat_offset)
            print("long_offset", long_offset)

        if mode < 3:
            target = shapely.geometry.Point(origin.x + long_offset * mul, origin.y + lat_offset * mul)
        else:        
            target = origin
            
        if debug:
            print("otarget lat", target.y)
            print("otarget lon", target.x)

        if long_offset == 0:
            ty = target.y
            tx = p.x
        elif lat_offset == 0:
            tx = target.x
            ty = p.y
        else:
            k = lat_offset / long_offset
            kn = -long_offset / lat_offset
            
            if debug:
                print("k", k)
                print("kn", kn)

            q1 = p.y - k * p.x
            q2 = target.y - kn * target.x

            #print("q1", q1)
            #print("q2", q2)
            
            tx = (-q1 + q2) / (k - kn)
            ty = k * tx + q1

        #print("target x", tx)
        #print("target y", ty)

        p2 = shapely.geometry.Point(tx, ty)

        dx = p2.x - p.x
        dy = p2.y - p.y

        #print("dx", dx)
        #print("dy", dy)

        if dx * long_offset < 0 or dy * lat_offset < 0:
            inside = not inside

        
        distance_km = gps_distance_2d_shapely(p, p2) / (100.0 * 1000.0) 
        angle = gps_bearing_shapely(p, p2)

        plt.plot(p.x, p.y, 'o', color="magenta")
        plt.plot(p2.x, p2.y, 'x', color="green")
        plt.plot(origin.x, origin.y, 'x', color="red")
        
        plt.plot(target.x, target.y, 'x', color="blue")

        if inside:
            color = "red"
            if mode == "angle":
                color = "darkred"
        else:
            color = "green"
            if mode == "angle":
                color = "lightgreen"

        if (abs(long_offset) == 63 or abs(lat_offset) == 63) and mode == 0:
            distance_km = 999;

        print ("  %u %3udeg %8.3fkm %2s mode %d" % (i, angle, distance_km, "IN" if inside else "", mode))
        
        f.seek(offset_index + DATA_INDEX_SIZE * index, os.SEEK_SET)   
        
        floor       = int.from_bytes(f.read(2), 'little', signed=False)
        ceil        = int.from_bytes(f.read(2), 'little', signed=False)        
        class_index = int.from_bytes(f.read(1), 'little', signed=False)        
        name        = f.read(50).decode("ascii").split("\0")[0]
       
        #print("floor", floor) 
        #print("ceil", ceil) 
        #print("class_index", class_index) 
        #print("name", name) 
       
        floor_mode = "AGL" if floor & 0x8000 else "MSL"
        ceil_mode  = "AGL" if ceil & 0x8000 else "MSL"
        
        floor &= 0x7FFF
        ceil &= 0x7FFF

        #reverse
        class_dict = {v: k for k, v in CLASS_DICT.items()}
        class_name = class_dict[class_index]
        
        print("   i =%3u %6u %s - %6u %s [%10s] %s" % (index, floor, floor_mode, ceil, ceil_mode, class_name, name))    

        lwid = 3 - i * 0.5
        arrow = FancyArrow(p.x, p.y, p2.x - p.x, p2.y - p.y, length_includes_head=True, linewidth=lwid, color=color, head_width=0.007, alpha=1.0)
           
        patch.append(ax.add_patch(arrow))

                
    fig.canvas.draw_idle()
    print()

airspaces = []

def getBoundingBox(airspaces):
    bb = [+100, +200, -100, -200]
    for airspace in airspaces:
        bb2 = airspace.getBoundingBox()
        bb[0] = min(bb[0], bb2[0])
        bb[1] = min(bb[1], bb2[1])
        bb[2] = max(bb[2], bb2[2])
        bb[3] = max(bb[3], bb2[3])
    return bb


def ev(event):
    lat = event.ydata
    lon = event.xdata
    if lat is None or lon is None:
        return
    if event.button == 3:        # right mouse button
        get_point(lat, lon)
        spf.send_point(lat, lon, galt)

def usage():
    print ('visualize.py [-c 49.5,8.9] openairspace-file')

from spoof import GPS_Spoof      

spf = GPS_Spoof("/dev/ttyUSB1")

level = 0
path = "."
galt = 200

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
patch = []
checkPoint = None

try:
    opts, args = getopt.getopt(sys.argv[1:],"hc:",["help", "check="])
except getopt.GetoptError:
    usage()
    sys.exit(1)

for opt, arg in opts:
    if opt in ("-h", "--help"):
        usage()
        sys.exit(0)
    elif opt in ("-c", "--check"):
        m = re.match('([-+]?\d*\.\d+|\d+),([-+]?\d*\.\d+|\d+)', arg)
        if m != None:
            checkPoint = shapely.geometry.Point(float(m.group(2)), float(m.group(1)))
        else:
            print ("argument '" + arg + "' is invalid for --check.")
            usage()
            sys.exit(1)

if len(args) != 1:
    usage()
    sys.exit(1)

for oas in openaip.load_file(args[0]):
    if CLASS_FILTER[oas.category] and not oas.invalid:
        airspaces.append(Airspace(oas))

if checkPoint != None:
    get_point(checkPoint.y, checkPoint.x)

for airspace in airspaces:
    airspace.draw()

a, = plt.plot([], [])
a.figure.canvas.mpl_connect('motion_notify_event', ev)
a.figure.canvas.mpl_connect('button_press_event', ev)

plt.title(args[0])
plt.xlabel('Longitude')
plt.ylabel('Latitude')
ax.axis('equal')

plt.subplots_adjust(left=0.05, right=1.0, top=1.0, bottom=0.05)
plt.show()



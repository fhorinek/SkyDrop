#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# This script can be used to visualize the content of an AIR file
# containing airspace data. It will draw a map showing the contents of
# the file as a raster with points and arrows.
#
# It takes two arguments:
#   1. a filename of an AIR file
#   2. [optional] a level to visualize, which can be number from 0-4.
#
# 20.12.2018: tilmann@bubecks.de

from math import sqrt,cos,atan2,floor,sin,asin
import re
import os
import numpy
import sys
from PIL import Image, ImageDraw
import matplotlib.pyplot as plt
import airspaces_data

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
    lon1 = P[0]
    lat1 = P[1]

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

    p2 = numpy.array([newLon, newLat])
    return p2

# Visualize the given file and the given elevation level.
#
# @param filename the name of the file to show
# @param the level inside that file (0-4)
#
def visualize(filename, level):

    basename = os.path.basename(filename)
    m = re.search('(N|S)(\d\d)([EW])(\d\d\d)', basename)
    if m != None:
        lat = int(m.group(2))
        lon = int(m.group(4))
        if m.group(1) == "S":
            lat = -lat
        if m.group(3) == "W":
            lon = -lon
    else:
        print ("Filename ", basename, "is invalid")
        sys.exit(1)
        
    f = open(filename, 'rb')

    # each point has "levels" elevation levels
    levels = 5

    # The size of 1 level in bytes in the file
    sizeof_level = 4
    
    filesize = os.path.getsize(filename)

    # The number of points in 1 line or row in the file
    numPoints = int(sqrt(filesize / (levels * sizeof_level)))
    print (filename, numPoints, "x", numPoints)

    # The number of points to draw on screen for 1 line or row
    numDrawPoints = 30

    skip = 1/numDrawPoints
    line = 0
    for lat_i in numpy.arange(lat, lat + 1, skip):
        line = line + 1
        col = 0
        for lon_i in numpy.arange(lon, lon + 1, skip):
            col = col + 1
            #if lon_i > lon + 1 or lat_i > lat + 1:
            #    continue
            
            p = numpy.array([lon_i, lat_i])
            #print (p)
            
            y = (lat_i - lat) * numPoints;
            x = (lon_i - lon) * numPoints;
            
            offset = (int(numPoints - y - 1) * numPoints + int(x)) * (levels * sizeof_level) + level * sizeof_level 
            f.seek(offset, os.SEEK_SET)
            if offset > filesize:
                print ("Point", p, "x=", x, "y=", y)
                print ("Unable to seek to", offset, ". File size is", filesize)
                sys.exit(1)
                
            height   = int.from_bytes(f.read(2), 'little', signed=False)
            angle    = int.from_bytes(f.read(1), 'little', signed=False)
            distance = int.from_bytes(f.read(1), 'little', signed=False)

            if level == 0:
                minHeight = 0
            else:
                # read lower height:
                f.seek(- 2 * sizeof_level, os.SEEK_CUR)
                minHeight = int.from_bytes(f.read(2), 'little', signed=False)
            
            distance_m = distance * 64
            
            #print (p, height, angle, distance)
            
            if angle >= 128:
                forbidden = True
                angle = angle - 128
            else:
                forbidden = False

            angle = angle * 3

            if forbidden:
                color = "red"
                if distance == 255:
                    color = "darkred"
            else:
                color = "green"
                angle = angle + 180
                if distance == 255:
                    color = "lightgreen"

            plt.plot(p[0], p[1], '.', color=color)
            if height == 0:
                continue
            
            p2 = displacePoint(p, angle, distance_m)

            if line % 2 == 0 and col % 2 == 0:
                plt.text(p[0], p[1], str(minHeight) + "-" + str(height) , fontsize=6, rotation=30, verticalalignment='bottom', horizontalalignment='left')
            
            if distance > 0 and distance <= 255:
                plt.arrow(p[0], p[1], p2[0] - p[0], p2[1]-p[1], length_includes_head=True, color=color, head_width=0.01, alpha=1.0)
                
if len(sys.argv) == 2:
    visualize(sys.argv[1], 0)
elif len(sys.argv) == 3:
    visualize(sys.argv[1], int(sys.argv[2]))
else:
    print ("Usage: visualize-AIR.py AIR-filename [layernum]")
    sys.exit(1)
    
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.axis('equal')
plt.show()

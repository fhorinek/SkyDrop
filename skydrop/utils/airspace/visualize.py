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
from matplotlib.patches import FancyArrow

import shapely
import shapely.ops
import shapely.geometry
from osgeo import ogr
from Airspace import Airspace

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

    p = shapely.geometry.Point(lon, lat)

    lat = int(lat * mul)
    lon = int(lon * mul)

    print("Point (lat, lon)", lat, lon)

    if lat >= 0:
        lat_c = "N"
    else:
        lat_c = "S"

    if lon >= 0:
        lon_c = "E"

    else:
        lon_c = "W"
     
    lat_n = int(abs(lat / mul))
    lon_n = int(abs(lon / mul))


    name = "%c%02u%c%03u.air" % (lat_c, lat_n, lon_c, lon_n)

    print(" filename is '%s'" % name)

    if not os.path.exists(name):
        return

    
    filesize = os.path.getsize(name)
    f = open(name, "rb")

    # The number of points in 1 line or row in the file
    numPoints = int.from_bytes(f.read(2), 'little', signed=False)

    x = int((lon % mul) * numPoints / mul)
    y = int((lat % mul) * numPoints / mul)

    offset_point = int(2 + (x * numPoints + y) * DATA_LEVELS * DATA_LEVEL_SIZE)
    offset_index = int(2 + (numPoints * numPoints) * DATA_LEVELS * DATA_LEVEL_SIZE)

    global patch
    global ax        

    for pt in patch:
        pt.remove()

    patch = []

    for i in range(DATA_LEVELS):    
        f.seek(offset_point + DATA_LEVEL_SIZE * i, os.SEEK_SET)    
        
        index = int.from_bytes(f.read(1), 'little', signed=False)
        a = f.read(1)
        b = f.read(1)

        print(ord(a), ord(b))

        inside = bool(index & 0x80)
        
        if index == 0x7F:
            print ("  level", i, "---")
            continue        

        index &= 0x7F
        
        if ord(a) & 0x80 and ord(b) & 0x80:
            mode = "normal"
        elif ord(a) & 0x80:
            mode = "offset"
        elif ord(b) & 0x80:
            mode = "close"
        else:
            mode = "angle"

        origin = shapely.geometry.Point(lon_n + ((x + 0.5) / float(numPoints)), lat_n + ((y + 0.5) / float(numPoints)))
        plt.plot(origin.x, origin.y, '.', color="red")
        
        if mode == "angle":
            #continue
            distance_raw = int.from_bytes(a, 'little', signed=False) 
            angle_raw = int.from_bytes(b, 'little', signed=False)

            angle = angle_raw * 3
            distance_km = (distance_raw & 0x7F) / 2.0

            p2 = displacePoint(p, angle, distance_km * 1000.0)

        else:
            lat_offset = int.from_bytes(a, 'little', signed=False) & 0x7F
            long_offset = int.from_bytes(b, 'little', signed=False) & 0x7F

            if lat_offset & 0x40:
                lat_offset = -(lat_offset & 0x3F)
            if long_offset & 0x40:
                long_offset = -(long_offset & 0x3F)

            print(lat_offset, long_offset)

            if mode == "offset":
                target = shapely.geometry.Point(origin.x + long_offset * OFFSET_MUL, origin.y + lat_offset * OFFSET_MUL)
            
            if mode == "close":
                target = shapely.geometry.Point(origin.x + long_offset * OFFSET_MUL_FINE, origin.y + lat_offset * OFFSET_MUL_FINE)

            if mode == "normal":
                target = origin
                

            plt.plot(target.x, target.y, 'x', color="blue")

            if long_offset == 0:
                ty = target.y
                tx = p.x
            elif lat_offset == 0:
                tx = target.x
                ty = p.y
            else:
                k = lat_offset / long_offset
                kn = -long_offset / lat_offset
                
                q1 = p.y - k * p.x
                q2 = target.y - kn * target.x
                
                tx = (-q1 + q2) / (k - kn)
                ty = k * tx + q1

            p2 = shapely.geometry.Point(tx, ty)

            dx = p2.x - p.x
            dy = p2.y - p.y

            if dx * long_offset < 0 or dy * lat_offset < 0:
                inside = not inside

            
            plt.plot(p.x, p.y, 'o', color="magenta")
            plt.plot(p2.x, p2.y, 'x', color="green")
            
            distance_km = gps_distance_2d_shapely(p, p2) / (100.0 * 1000.0) 
            angle = gps_bearing_shapely(p, p2)
        
        if inside:
            color = "red"
            if mode == "angle":
                color = "darkred"
        else:
            color = "green"
            if mode == "angle":
                color = "lightgreen"

        print ("  %u %3udeg %8.3fkm %2s %s" % (i, angle, distance_km, "IN" if inside else "", mode))
        
        f.seek(offset_index + DATA_INDEX_SIZE * index, os.SEEK_SET)   
        
        floor       = int.from_bytes(f.read(2), 'little', signed=False)
        ceil        = int.from_bytes(f.read(2), 'little', signed=False)        
        class_index = int.from_bytes(f.read(1), 'little', signed=False)        
        name        = f.read(50).decode("ascii").split("\0")[0]
        
       
        floor_mode = "AGL" if floor & 0x8000 else "MSL"
        ceil_mode  = "AGL" if ceil & 0x8000 else "MSL"
        
        floor &= 0x7FFF
        ceil &= 0x7FFF
            
        class_dict = {
            "R" : 0,    #restricted
            "Q" : 1,    # danger
            "P" : 2,    # prohibited
            "A" : 3,    # Class A
            "B" : 4,    # Class B
            "C" : 5,    # Class C
            "D" : 6,    # Class D 
            "GP" : 7,   # glider prohibited 
            "CTR" : 8,  # CTR
            "W" : 9    # Wave Window
        }

        #reverse
        class_dict = {v: k for k, v in class_dict.items()}
        class_name = class_dict[class_index]
        
        print("   i =%3u %6u %s - %6u %s [%4s] %s" % (index, floor, floor_mode, ceil, ceil_mode, class_name, name))    

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

#**********************************************************************
#                           ReadLayer()
#**********************************************************************

def ReadLayer( poLayer ):

    poDefn = poLayer.GetLayerDefn()

# --------------------------------------------------------------------
#      Read, and dump features.
# --------------------------------------------------------------------
    poFeature = poLayer.GetNextFeature()
    while poFeature is not None:
        ReadFeature(poFeature)
        poFeature = poLayer.GetNextFeature()
        
    return

def ReadField(poFeature, fieldName):
        poDefn = poFeature.GetDefnRef()
        iField = poDefn.GetFieldIndex(fieldName)
        poFDefn = poDefn.GetFieldDefn(iField)
        if poFeature.IsFieldSet( iField ):
                value = poFeature.GetFieldAsString( iField ).strip();
        else:
                value = None

        return value

def ReadAltFt( poFeature, fieldName ):
    poDefn = poFeature.GetDefnRef()
    alt = None
    level = None
    unit = None
    iField = poDefn.GetFieldIndex(fieldName)
    poFDefn = poDefn.GetFieldDefn(iField)
    if poFeature.IsFieldSet( iField ):
        value = poFeature.GetFieldAsString( iField ).strip();
        alt = value
        if alt == "GND" or alt == "SFC":
            alt = 0
            unit = "ft"
            level = "AGL"
        elif alt == "UNLTD":
            alt = 99999
            unit = "ft"
            level = "MSL"
        else:
            # "8000 ft AMSL"
            m = re.match('(\d+)\s+([a-z]+)\s+(\w+)', alt)
            if m != None:
                alt = int(m.group(1))
                unit = m.group(2)
                level = m.group(3)
            else:
                # "2200ft MSL"
                m = re.match('(\d+)([a-z]+)\s+(\w+)', alt)
                if m != None:
                    alt = int(m.group(1))
                    unit = m.group(2)
                    level = m.group(3)
                else:
                    m = re.match('(\d+)\s*(\w+)', alt)
                    if m != None:
                        alt = int(m.group(1))
                        level = m.group(2)
                        if level == "ft":
                            unit = level
                            level = "MSL"
                        else:
                            unit = "ft"
                    else:
                        # "FL80"
                        m = re.match('FL\s*(\d+)', alt)
                        if m != None:
                            alt = int(m.group(1)) * 100
                            level = "MSL"
                            unit = "ft"
                        else:
                            print ("Unknown alt grammar '"+value+"'")
                            sys.exit(1)

        if unit == "m":
            alt = alt * 3.28084    # convert meter to feet
            unit = "ft"
            
        if unit != "ft":
            print("airspace #%ld: %s" % (poFeature.GetFID(), value))
            print ("Unknown unit", unit)
            sys.exit(1)
            
        if level == "AMSL":
            level = "MSL"
            
        if level != "MSL" and level != "AGL":
            print("airspace #%ld: %s" % (poFeature.GetFID(), value))
            print ("Unknown level", level)
            sys.exit(1)

    return alt, (level == "AGL")
        
def ReadFeature( poFeature ):

    poDefn = poFeature.GetDefnRef()
    # print("OGRFeature(%s):%ld" % (poDefn.GetName(), poFeature.GetFID() ))

    classAir = ReadField(poFeature, "CLASS")
   
    name = ReadField(poFeature, "NAME")
    floor, floorAGL = ReadAltFt(poFeature, "FLOOR")
    ceiling, ceilingAGL = ReadAltFt(poFeature, "CEILING")

    nGeomFieldCount = poFeature.GetGeomFieldCount()
    if nGeomFieldCount > 0:
        for iField in range(nGeomFieldCount):
            poGeometry = poFeature.GetGeomFieldRef(iField)
            if poGeometry is not None:
                geometryName = poGeometry.GetGeometryName()
                if geometryName != "POLYGON":
                    print ("Unknown geometry: ", geometryName)
                    sys.exit(1)
                geometryCount = poGeometry.GetGeometryCount()
                if geometryCount != 1:
                    print("GeometryCount != 1")
                    sys.exit(1)
                        
                ring = poGeometry.GetGeometryRef(0)
                points = ring.GetPointCount()
                p = []
                for i in range(points):
                    lon, lat, z = ring.GetPoint(i)
                    p.append((lon, lat))
                polygon = shapely.geometry.Polygon(p)
                airspace = Airspace()
                airspace.setName(name)
                airspace.setMinMax(floor, floorAGL, ceiling, ceilingAGL)
                airspace.setPolygon(polygon)
                airspaces.append(airspace)
        
    return

def draw_airspace(pszDataSource):
    # --------------------------------------------------------------------
    #      Open data source.
    # --------------------------------------------------------------------
        poDS = None
        poDriver = None

        poDS = ogr.Open( pszDataSource)

    # --------------------------------------------------------------------
    #      Report failure.
    # --------------------------------------------------------------------
        if poDS is None:
            print( "FAILURE:\n"
                    "Unable to open datasource `%s' with the following drivers." % pszDataSource )

            for iDriver in range(ogr.GetDriverCount()):
                print( "  -> %s" % ogr.GetDriver(iDriver).GetName() )

            return 1

        poDriver = poDS.GetDriver()

    # --------------------------------------------------------------------
    #      Some information messages.
    # --------------------------------------------------------------------
        print( "INFO: Open of `%s'\n"
                "      using driver `%s' successful." % (pszDataSource, poDriver.GetName()) )

        poDS_Name = poDS.GetName()
        if str(type(pszDataSource)) == "<type 'unicode'>" and str(type(poDS_Name)) == "<type 'str'>":
            poDS_Name = poDS_Name.decode("utf8")
        if pszDataSource != poDS_Name:
            print( "INFO: Internal data source name `%s'\n"
                    "      different from user name `%s'." % (poDS_Name, pszDataSource ))

        #gdal.Debug( "OGR", "GetLayerCount() = %d\n", poDS.GetLayerCount() )

        # --------------------------------------------------------------------
        #      Process specified data source layers.
        # --------------------------------------------------------------------
        poLayer = poDS.GetLayerByName("airspaces")

        if poLayer is None:
            print( "FAILURE: Couldn't fetch requested layer %s!" % papszIter )
            return 1

        ReadLayer( poLayer )

    # --------------------------------------------------------------------
    #      Close down.
    # --------------------------------------------------------------------
        poDS.Destroy()

        for airspace in airspaces:
            airspace.draw()
            
        
galt = 200            

def ev(event):
    lat = event.ydata
    lon = event.xdata
    if lat is None or lon is None:
        return
    if event.button == 1:
        get_point(lat, lon)
        spf.send_point(lat, lon, galt)
        
from spoof import GPS_Spoof      

spf = GPS_Spoof("/dev/ttyUSB1")

level = 0
path = "."

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
patch = []

draw_airspace(sys.argv[1])


a, = plt.plot([], [])
a.figure.canvas.mpl_connect('motion_notify_event', ev)
a.figure.canvas.mpl_connect('button_press_event', ev)

plt.title(sys.argv[1])
plt.xlabel('Longitude')
plt.ylabel('Latitude')
ax.axis('equal')

plt.subplots_adjust(left=0.05, right=1.0, top=1.0, bottom=0.05)
#plt.savefig(sys.argv[1] + ".png", dpi=300, bbox_inches='tight')
plt.show()



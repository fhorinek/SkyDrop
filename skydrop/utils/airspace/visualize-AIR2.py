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

import shapely
import shapely.ops
import shapely.geometry
from osgeo import ogr
from Airspace import Airspace

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
    numDrawPoints = 50

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

            plt.plot(p[0], p[1], '.', color=color, markersize=0.5)
            if height == 0:
                continue
            
            p2 = displacePoint(p, angle, distance_m)

            if line % 2 == 0 and col % 2 == 0:
                plt.text(p[0], p[1], str(minHeight) + "-" + str(height) , fontsize=6, rotation=30, verticalalignment='bottom', horizontalalignment='left')
            
            if distance > 0 and distance <= 255:
                plt.arrow(p[0], p[1], p2[0] - p[0], p2[1]-p[1], length_includes_head=True, linewidth=0.3, color=color, head_width=0.007, alpha=1.0)
                

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
            
        if level != "MSL" and level != "GND" and level != "AGL" and level != "AMSL":
            print("airspace #%ld: %s" % (poFeature.GetFID(), value))
            print ("Unknown level", level)
            sys.exit(1)

    return alt
        
def ReadFeature( poFeature ):

    poDefn = poFeature.GetDefnRef()
    # print("OGRFeature(%s):%ld" % (poDefn.GetName(), poFeature.GetFID() ))

    classAir = ReadField(poFeature, "CLASS")
    if classAir in ["RMZ", "TMZ", "Q", "W", "G"]:
            # R restricted
            # Q danger
            # P prohibited
            # A Class A
            # B Class B
            # C Class C
            # D Class D
            # GP glider prohibited
            # CTR CTR
            # W Wave Window
            #
            # These airspaces will be skipped
            return
    
    name = ReadField(poFeature, "NAME")
    floor = ReadAltFt(poFeature, "FLOOR")
    ceiling = ReadAltFt(poFeature, "CEILING")

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
                airspace.setMinMax(floor, ceiling)
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

level = 0
path = "."

plt.figure(figsize=(60,30))

draw_airspace(sys.argv[1])


for file in os.scandir(path):
    if file.name[-4:] == ".air":
        visualize(file.name, level)
    
plt.title(sys.argv[1])
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.axis('equal')
plt.savefig(sys.argv[1] + ".png", dpi=300, bbox_inches='tight')
#plt.show()



#!/usr/bin/env python
# -*- mode: python-mode; python-indent-offset: 4 -*-
#*****************************************************************************
# dnf install python3-shapely python3-gdal
#
# This program is used to read a "Open-Airspace-file" containing a number of
# airspaces and then computing a raster of positions around these airspaces.
# Each raster point has an angle and distance showing to the nearest airspace.
#
# This can be used to generate AIR files used by SkyDrop variometer to help
# the pilot avoid flying into forbidden airspaces.
#
# 2018-12-23, tilmann@bubecks.de

import sys
import re
import os

from Airspace import Airspace
from AirspaceVector import AirspaceVector
from pprint import pprint
from osgeo import gdal
from osgeo import ogr
import shapely
import shapely.ops
import shapely.geometry
import matplotlib.pyplot as plt
import numpy
import time
import multiprocessing
import getopt

bDraw = False
bVerbose = 0
wantedResolution = 300
latOnly = lonOnly = None
force = False
checkPoint = None

# each point has "levels" elevation levels
levels = 5

# The size of 1 level in bytes in the file
sizeof_level = 4

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
    
def findAirspacesInHeight(airspaceVectors, height):
    matchingAirspaceVectors = []
    for airspaceVector in airspaceVectors:
        if height >= airspaceVector.airspace.getMin() and height < airspaceVector.airspace.getMax():
            matchingAirspaceVectors.append(airspaceVector)

    return matchingAirspaceVectors

def findAirspacesInside(airspaceVectors):
    matchingAirspaceVectors = []
    for airspaceVector in airspaceVectors:
        if airspaceVector.inside:
            matchingAirspaceVectors.append(airspaceVector)

    return matchingAirspaceVectors

def findAirspacesNotInside(airspaceVectors):
    matchingAirspaceVectors = []
    for airspaceVector in airspaceVectors:
        if not airspaceVector.inside:
            matchingAirspaceVectors.append(airspaceVector)

    return matchingAirspaceVectors

def findLowestHeightInAirspaces(airspace_vectors):
    minHeight = None
    for airspace_vector in airspace_vectors:
        if minHeight == None or airspace_vector.airspace.getMin() <= minHeight:
            minHeight = airspace_vector.airspace.getMin()

    return minHeight

def findHighestHeightInAirspaces(airspace_vectors):
    maxHeight = None
    for airspace_vector in airspace_vectors:
        if maxHeight == None or airspace_vector.airspace.getMax() > maxHeight:
            maxHeight = airspace_vector.airspace.getMax()

    return maxHeight

# Find the smallest height which is >= h.
def findNextHeightInAirspaces(airspace_vectors, h):
    nextHeight = None
    for airspace_vector in airspace_vectors:
        newHeight = airspace_vector.airspace.getMin()
        if newHeight > h:
            if nextHeight == None:
                nextHeight = newHeight
            else:
                nextHeight = min(nextHeight, newHeight)
        newHeight = airspace_vector.airspace.getMax()
        if newHeight > h:
            if nextHeight == None:
                nextHeight = newHeight
            else:
                nextHeight = min(nextHeight, newHeight)

    return nextHeight

def findNearestAirspace(airspaceVectors):
    nearestAirspace = None
    for airspaceVector in airspaceVectors:
        if nearestAirspace == None or airspaceVector.distance < nearestAirspace.distance:
            nearestAirspace = airspaceVector

    return nearestAirspace

def findFarestAirspace(airspaceVectors):
    farestAirspace = None
    for airspaceVector in airspaceVectors:
        if farestAirspace == None or airspaceVector.distance > farestAirspace.distance:
            farestAirspace = airspaceVector

    return farestAirspace

# Entering a point here, would help debugging stuff inside dump.
checkpoints = numpy.array([
    #[9.0, 48.1],
    #[8.8, 48.9]
    ])
    
def dumpPoint(output, offset, p, airspaces, draw=False):
    global bVerbose
    global bDraw
    
    inside = False
            
    #print (p)
    check = False
    for c in checkpoints:
        # print (p[0], c[0], p[1], c[1])
        if abs(p.x - c[0]) < 0.001 and abs(p.y - c[1]) < 0.001:
            check = True
            print (p)

    if bVerbose > 1:
        check = True
        
    avs = []
    for airspace in airspaces:
        if airspace.getDistanceToCenter(p) / (100*1000) < 100:
            av = airspace.getAirspaceVector(p, draw)
            if not av.isTooFar():
                avs.append(av)

    if bVerbose > 0:
        print(p, len(avs), "airspaces here")
    #pprint ("All airspaces:")
    #pprint (avs)

    sortedAirspaces = {}
            
    # We start at lowest height
    hMin = findLowestHeightInAirspaces(avs)
    hMax = findHighestHeightInAirspaces(avs)
    h = hMin
    while h != None and h < hMax:
        if check:
            print ("Height:", h)
        nearestAirspace = None
        airspacesInThisHeight = findAirspacesInHeight(avs, h)
        if check:
            print ("airspacesInThisHeight:")
            print (airspacesInThisHeight)
        airspacesInside = findAirspacesInside(airspacesInThisHeight)
        if len(airspacesInside) != 0:
            inside = True
            if check:
                print ("INSIDE:")
            nearestAirspace = findNearestAirspace(airspacesInside)
        else:
            nearestAirspace = findNearestAirspace(airspacesInThisHeight)

        if check:
            print ("nearestAirspace")
            print (nearestAirspace)

        # avs.sort(key=lambda x: x.distance, reverse=False)

        #pprint(nearestAirspace)

        if nearestAirspace != None:
            sortedAirspaces[h] = nearestAirspace

        h = findNextHeightInAirspaces(avs, h)

    if check:
        print ("\nsortedAirspaces")
        print (sortedAirspaces)

    if inside:
        if bVerbose > 0:
            print ("INSIDE")
                
    # Eliminate all subsequent identical airspaces:
    compactAirspaces = {}
    heights = sorted(sortedAirspaces.keys())
    if len(heights) > 0:
        height1 = heights[0]
        compactAirspaces[height1] = sortedAirspaces[height1]

        for i in range(len(heights)-1):
            height1 = heights[i]
            height2 = heights[i+1]
            if bVerbose > 1:
                print ("comparing airspace in", height1, "and", height2)
            if sortedAirspaces[height1].airspace != sortedAirspaces[height2].airspace:
                if bVerbose > 1:
                    print("  different airspaces. Adding airspace from", height2)
                compactAirspaces[height2] = sortedAirspaces[height2]
            else:
                if bVerbose > 1:
                    print ("  identical airspaces. Skipping airspace from", height2)
    
    if bVerbose > 1:
        print ("compactAirspaces(without subsequent identical)")
        print (compactAirspaces)

    sortedAirspaces = compactAirspaces
    
    # Eliminate all subsequent airspaces with same angle/distance:
    compactAirspaces = sortedAirspaces
    heights = sorted(sortedAirspaces.keys())
    if len(heights) > 5:
        compactAirspaces = {}
        height1 = heights[0]
        compactAirspaces[height1] = sortedAirspaces[height1]

        for i in range(len(heights)-1):
            height1 = heights[i]
            height2 = heights[i+1]
            if bVerbose > 1:
                print ("comparing airspace in", height1, "and", height2, "for identical distance/angle.")
                print("  distance:", sortedAirspaces[height1].getDistanceAsByte(), sortedAirspaces[height2].getDistanceAsByte())
                print ("  angle:", sortedAirspaces[height1].getAngleAsByte(), sortedAirspaces[height2].getAngleAsByte())
                        
            if sortedAirspaces[height1].getDistanceAsByte() != sortedAirspaces[height2].getDistanceAsByte() or abs(sortedAirspaces[height1].getAngleAsByte() - sortedAirspaces[height2].getAngleAsByte()) > 5:
                if bVerbose > 1:
                    print("  different distance/angle. Adding airspace from", height2)
                compactAirspaces[height2] = sortedAirspaces[height2]
            else:
                if bVerbose > 1:
                    print ("  identical distance/angle. Skipping airspace from", height2)
    
    if check:
        print ("compactAirspaces (without identical distance/angle)")
        print (compactAirspaces)
 
    # Delete all airspaces which are far away
    while len(compactAirspaces) > 5:
                
        outsideAirspaces = findAirspacesNotInside(compactAirspaces.values())
        farestAirspace = findFarestAirspace(outsideAirspaces)
        if farestAirspace == None:
            print ("Too much airspaces and none is far away at", p)
            sys.exit(1)
        if check:
            print("Deleting ", farestAirspace)
                
        compactAirspaces2 = {}
        for height in compactAirspaces.keys():
            if compactAirspaces[height] != farestAirspace:
                compactAirspaces2[height] = compactAirspaces[height]
            #else:
            #    print (p, " removed airspace ", farestAirspace)

        compactAirspaces = compactAirspaces2

    #names = []
    heights = sorted(compactAirspaces.keys())
    for i in range(len(heights)):
        height1 = heights[i]
        if i+1 < len(compactAirspaces):
            height2 = heights[i+1]
            name = compactAirspaces[height1].airspace.getName()
        else:
            height2 = None
            name = None
        for byte in compactAirspaces[height1].getBytes(height2):
            output[offset] = byte
            offset = offset + 1
        if bVerbose > 0:
            print("    up to ",height2, ": ", name)
                
    # Fill up with empty Airspaces
    av = AirspaceVector()
    for i in range(5 - len(compactAirspaces)):
        for byte in av.getBytes():
            output[offset] = byte
            offset = offset + 1
        
def dump(lon, lat, airspaces, draw=False):

    global levels
    global sizeof_level
    global wantedResolution
    
    filename = f"N{lat:02d}E{lon:03d}.air"
    
    if draw:
        numPoints = 10
    else:
        numPoints = wantedResolution

    filesize = numPoints * numPoints * levels * sizeof_level
    if os.path.isfile(filename) and not force:
        print (filename, "exists, skipping...")
        return
    
    f = open(filename, 'wb')
    print (filename, "computing (" + str(numPoints) + "x" + str(numPoints) + ")")

    output = bytearray(filesize)
    
    try:
        # Quickcheck for emptyness
        isEmpty = True
        for lat_i in numpy.arange(lat, lat + 1, 1/10):
            for lon_i in numpy.arange(lon, lon + 1, 1/10):
                p = shapely.geometry.Point(lon_i, lat_i)
                offset = 0
                dumpPoint(output, offset, p, airspaces, False)
                for offset in range(levels * sizeof_level):
                    if output[offset] != 0:
                        isEmpty = False

        if not isEmpty:
            for lat_i in numpy.arange(lat, lat + 1, 1/numPoints):
                for lon_i in numpy.arange(lon, lon + 1, 1/numPoints):
                    p = shapely.geometry.Point(lon_i, lat_i)
                    y = (lat_i - lat) * numPoints;
                    x = (lon_i - lon) * numPoints;
                    offset = (int(numPoints - 1 - y) * numPoints + int(x)) * (levels * sizeof_level)
                    dumpPoint(output, offset, p, airspaces, draw)
                    
    except (KeyboardInterrupt, SystemExit):
        print("Exiting...")
        f.close()
        os.remove(filename)
        sys.exit(1)

    isEmpty = True
    for byte in output:
        if byte != 0:
            isEmpty = False
            break
    if isEmpty:
        print (filename, "is empty")
    else:
        f.write(bytes(output))
        print (filename, "saved")
    f.close()

#**********************************************************************
#                                main()
#**********************************************************************

def usage():
    print ('convert.py openairspace-file [lat] [lon]')
    
def main(argv = None):

    global bVerbose
    global bDraw
    global latOnly, lonOnly
    global wantedResolution
    global force
    global checkPoint
    global levels, sizeof_level
    
    if argv is None:
        argv = sys.argv

    try:
        opts, args = getopt.getopt(argv,"hvqdr:fc:",["help", "resolution=","quiet","verbose","draw", "force", "check"])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
      
# --------------------------------------------------------------------
#      Processing command line arguments.
# --------------------------------------------------------------------
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        elif opt in ("-q", "--quiet"):
            bVerbose = 0
        elif opt in ("-v", "--verbose"):
            bVerbose = bVerbose + 1
        elif opt in ("-d", "--draw"):
            bDraw = True
        elif opt in ("-r", "--resolution"):
            wantedResolution = int(arg)
        elif opt in ("-f", "--force"):
            force = True
        elif opt in ("-c", "--check"):
            m = re.match('([-+]?\d*\.\d+|\d+),([-+]?\d*\.\d+|\d+)', arg)
            if m != None:
                checkPoint = shapely.geometry.Point(float(m.group(2)), float(m.group(1)))
            else:
                print (arg, "is invalid for --check. Use e.g. 48.5,10.2")
                sys.exit(1)
                
    if len(args) == 1:
        pszDataSource = args[0]
    elif len(args) == 3:
        pszDataSource = args[0]
        latOnly = int(args[1])
        lonOnly = int(args[2])
    else:
        usage()
        sys.exit(1)
        
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
    if bVerbose > 0:
        print( "INFO: Open of `%s'\n"
                "      using driver `%s' successful." % (pszDataSource, poDriver.GetName()) )

    poDS_Name = poDS.GetName()
    if str(type(pszDataSource)) == "<type 'unicode'>" and str(type(poDS_Name)) == "<type 'str'>":
        poDS_Name = poDS_Name.decode("utf8")
    if bVerbose > 0 and pszDataSource != poDS_Name:
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

    boundingBox = getBoundingBox(airspaces)
    print("BoundingBox:", boundingBox)

    if checkPoint != None:
        output = bytearray(levels * sizeof_level)
        print (checkPoint)
        dumpPoint(output, 0, checkPoint, airspaces, False)
        for level in range(levels):
            offset = level * sizeof_level
            height   = int.from_bytes([output[offset], output[offset+1]], 'little', signed=False)
            angle    = int.from_bytes([output[offset+2]], 'little', signed=False)
            distance = int.from_bytes([output[offset+3]], 'little', signed=False)
            print ("  level", level, "height=" + str(height),"angle="+str(angle),"distance="+str(distance))
        sys.exit(0)
            
    count = 0
    if bDraw:
        print ("DRAW")
        for airspace in airspaces:
            if True or "Stuttgart" in airspace.getName(): 
                    airspace.draw()
        plt.plot(boundingBox[0], boundingBox[1], '.', color="black")
        plt.plot(boundingBox[2], boundingBox[3], '.', color="black")
        
    try:
        procs = []
        if latOnly != None and lonOnly != 0:
            p = multiprocessing.Process(target=dump, args=(lonOnly,latOnly,airspaces, bDraw))
            procs.append(p)
        else:
            for lat in range(int(boundingBox[1])-1,int(boundingBox[3])+2):
                for lon in range(int(boundingBox[0])-1,int(boundingBox[2])+2):
                    p = multiprocessing.Process(target=dump, args=(lon,lat,airspaces, bDraw))
                    procs.append(p)

        running = []
        parallelism = multiprocessing.cpu_count()    # set to "1" for sequential
        while len(procs) > 0 or len(running) > 0:
            # Start as much processes as we have CPUs
            while len(running) < parallelism and len(procs) > 0:
                p = procs.pop(0)
                p.start()
                running.append(p)
            for i in range(len(running)):
                if not running[i].is_alive():
                    running[i].join()
                    del running[i]
                    # "i" is now wrong, break out and restart
                    break      
            time.sleep(1)

    except (KeyboardInterrupt, SystemExit):
        print("Exiting (main)...")
        sys.exit(1)
            
    if bDraw:
        plt.axis('equal')
        plt.show()

    return 0

#**********************************************************************
#                               Usage()
#**********************************************************************

def Usage():

    print( "Usage: convert [-q|-v] datasource_name")
    return 1

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

        if unit != "ft":
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

if __name__ == '__main__':
    version_num = int(gdal.VersionInfo('VERSION_NUM'))
    if version_num < 1800: # because of ogr.GetFieldTypeName
        print('ERROR: Python bindings of GDAL 1.8.0 or later required')
        sys.exit(1)

    sys.exit(main(sys.argv[1:]))

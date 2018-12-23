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
from multiprocessing import Process

bVerbose = False
bSummaryOnly = False
nFetchFID = ogr.NullFID
papszOptions = None

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

    inside = False
            
    #print (p)
    check = False
    for c in checkpoints:
        # print (p[0], c[0], p[1], c[1])
        if abs(p.x - c[0]) < 0.001 and abs(p.y - c[1]) < 0.001:
            check = True
            print (p)
            
    avs = []
    for airspace in airspaces:
        if airspace.getDistanceToCenter(p) / (100*1000) < 100:
            av = airspace.getAirspaceVector(p, draw)
            if not av.isTooFar():
                avs.append(av)

    if bVerbose:
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
        if bVerbose:
            print ("INSIDE")
                
    # Eliminate all subsequent identical airspaces:
    compactAirspaces = {}
    heights = sorted(sortedAirspaces.keys())
    for i in range(len(heights)-1):
        height1 = heights[i]
        height2 = heights[i+1]
        if sortedAirspaces[height1].airspace != sortedAirspaces[height2].airspace:
            if check:
                print(sortedAirspaces[height1].getDistanceAsByte(), sortedAirspaces[height2].getDistanceAsByte())
                print (sortedAirspaces[height1].getAngleAsByte(), sortedAirspaces[height2].getAngleAsByte())
                        
            if sortedAirspaces[height1].getDistanceAsByte() != sortedAirspaces[height2].getDistanceAsByte() or abs(sortedAirspaces[height1].getAngleAsByte() - sortedAirspaces[height2].getAngleAsByte()) > 5:
                compactAirspaces[height1] = sortedAirspaces[height1]

    if len(heights) > 0:
        height = heights[len(heights)-1]
        compactAirspaces[height] = sortedAirspaces[height]
            
    if check:
        print ("compactAirspaces")
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
        #print("Writing up to height: ", height2)
        for byte in compactAirspaces[height1].getBytes(height2):
            output[offset] = byte
            offset = offset + 1
        #f.write(bytes(compactAirspaces[height1].getBytes(height2)))
        if bVerbose:
            print("    up to ",height2, ": ", name)
                
    # Fill up with empty Airspaces
    av = AirspaceVector()
    for i in range(5 - len(compactAirspaces)):
        for byte in av.getBytes():
            output[offset] = byte
            offset = offset + 1
        #f.write(bytes(av.getBytes()))
        
def dump(lon, lat, airspaces, draw=False):

    global levels
    global sizeof_level
    
    filename = f"N{lat:02d}E{lon:03d}-3.air"
    print (filename, "computing...")
    if draw:
        numPoints = 10
    else:
        numPoints = 1200
        numPoints = 30
    skip = 1/numPoints

    output = bytearray(numPoints * numPoints * levels * sizeof_level)

    try:
        for lat_i in numpy.arange(lat + 1 + skip, lat, -skip):
            for lon_i in numpy.arange(lon, lon + 1 + skip, skip):
                p = shapely.geometry.Point(lon_i, lat_i)
                y = (lat_i - lat) * numPoints;
                x = (lon_i - lon) * numPoints;
                offset = (int(numPoints - 1 - y) * numPoints + int(x)) * (levels * sizeof_level)
                dumpPoint(output, offset, p, airspaces, draw)
    except (KeyboardInterrupt, SystemExit):
        print("Exiting...")
        sys.exit(1)

    isEmpty = True
    for byte in output:
        if byte != 0:
            isEmpty = False
            break
    if isEmpty:
        print (filename, "is empty")
    else:
        f = open(filename, 'wb')
        f.write(bytes(output))
        f.close()
        print (filename, "saved")

def EQUAL(a, b):
    return a.lower() == b.lower()

#**********************************************************************
#                                main()
#**********************************************************************

def main(argv = None):

    global bVerbose
    global bSummaryOnly
    global nFetchFID
    global papszOptions

    pszWHERE = None
    pszDataSource = None
    papszLayers = None
    poSpatialFilter = None
    nRepeatCount = 1
    bAllLayers = False
    pszSQLStatement = None
    pszDialect = None
    options = {}
    pszGeomField = None

    if argv is None:
        argv = sys.argv

    argv = ogr.GeneralCmdLineProcessor( argv )

# --------------------------------------------------------------------
#      Processing command line arguments.
# --------------------------------------------------------------------
    if argv is None:
        return 1

    nArgc = len(argv)

    iArg = 1
    while iArg < nArgc:

        if EQUAL(argv[iArg],"--utility_version"):
            print("%s is running against GDAL %s" %
                   (argv[0], gdal.VersionInfo("RELEASE_NAME")))
            return 0

        elif EQUAL(argv[iArg],"-q") or EQUAL(argv[iArg],"-quiet"):
            bVerbose = False
        elif EQUAL(argv[iArg],"-v") or EQUAL(argv[iArg],"-verbose"):
            bVerbose = True

        elif argv[iArg][0] == '-':
            return Usage()

        elif pszDataSource is None:
            pszDataSource = argv[iArg]

        iArg = iArg + 1

    if pszDataSource is None:
        return Usage()

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
    if bVerbose:
        print( "INFO: Open of `%s'\n"
                "      using driver `%s' successful." % (pszDataSource, poDriver.GetName()) )

    poDS_Name = poDS.GetName()
    if str(type(pszDataSource)) == "<type 'unicode'>" and str(type(poDS_Name)) == "<type 'str'>":
        poDS_Name = poDS_Name.decode("utf8")
    if bVerbose and pszDataSource != poDS_Name:
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

    ReadLayer( poLayer, pszWHERE, pszGeomField, poSpatialFilter, options )

# --------------------------------------------------------------------
#      Close down.
# --------------------------------------------------------------------
    poDS.Destroy()

    boundingBox = getBoundingBox(airspaces)
    #pprint(boundingBox)

    draw = True

    count = 0
    if draw:
        for airspace in airspaces:
            if True or "Stuttgart" in airspace.getName(): 
                    airspace.draw()
        plt.plot(boundingBox[0], boundingBox[1], '.', color="black")
        plt.plot(boundingBox[2], boundingBox[3], '.', color="black")
        
    try:
        procs = []
        if False:
            for lat in range(int(boundingBox[1]),int(boundingBox[3])+1):
                for lon in range(int(boundingBox[0]),int(boundingBox[2])+1):
                    p = Process(target=dump, args=(lon,lat,airspaces, draw))
                    p.start()
                    procs.append(p)

        for p in procs:
            p.join()

    except (KeyboardInterrupt, SystemExit):
        print("Exiting (main)...")
        sys.exit(1)
            
    if draw:
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

def ReadLayer( poLayer, pszWHERE, pszGeomField, poSpatialFilter, options ):

    poDefn = poLayer.GetLayerDefn()

# --------------------------------------------------------------------
#      Read, and dump features.
# --------------------------------------------------------------------
    poFeature = poLayer.GetNextFeature()
    while poFeature is not None:
        ReadFeature(poFeature, options)
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
        if alt == "GND":
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
        
def ReadFeature( poFeature, options = None ):

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

    sys.exit(main( sys.argv ))

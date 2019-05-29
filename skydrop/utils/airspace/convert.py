#!/usr/bin/env python3
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
# Various checkpoints for using with "-c":
#  * Möhringen: 48.723957,9.153292
#  * Nabern: 48.614241,9.475000
#  * Grabenstetten: 48.536363,9.437542
#  * Bad Urach: 48.490105,9.394959
#  * Degerloch: 48.745936,9.169557
#  * Kornwestheim: 48.864396,9.223579
#  * Markgroenningen: 48.908029,9.085469
#  * Boenigheim: 49.039651,9.095502
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
import struct

import cProfile

from const import *
from gps_calc import *

bVerbose = 0
wantedResolution = 200
latOnly = lonOnly = None
force = False
checkPoint = None


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

# Entering a point here, would help debugging stuff inside dump.
checkpoints = numpy.array([
    #[9.0, 48.1],
    #[8.8, 48.9]
    ])
    
    
def dumpPoint(output, offset, p, airspaces, check=False):

    def av_score(av):
        class_dict = {
            "A" : 1,    # Class A
            "B" : 1,    # Class B
            "C" : 1,    # Class C
            "D" : 1,    # Class D 
            "CTR" : 1,  # CTR
            "Q" : 1,    # danger
            "P" : 1,    # prohibited
            "R" : 0.5,    #restricted
            "GP" : 0.25,   # glider prohibited 
            "W" : 0.25    # Wave Window
        }      
    
        score = 0
        
        if av.isInside():
            dist_score = 1
        else:
            dist_score = 1 - min(av.getDistance() / (33.0), 1)

        score += dist_score * 10000   
            
        alt_score = 1 - min(av.airspace.getMin()[0] / 10000, 1)            
        score += alt_score * 1000
        
        class_score = class_dict[av.airspace.getClass()]
        score += class_score * 100
        
        av.scores = {}
        av.scores["D"] = dist_score
        av.scores["A"] = alt_score
        av.scores["C"] = class_score
        av.scores["T"] = score
        
        return score

    global bVerbose
    
    inside = False

    avs = []
    
    #get all airspaces in proximity
    for airspace in airspaces:
        if airspace.isNear(p):
            av = airspace.getAirspaceVector(p)
            avs.append(av)

    #sort by importance
    avs.sort(key = av_score, reverse = True)

    if check and bVerbose > 1:
        print ("Airspaces here:")
        i = 0
        for av in avs:
            air = av.airspace
            if i == DATA_LEVELS:
                print("-" * 120)
            i += 1
            
            str_score = ""
            for key in av.scores:
                str_score += "%s %0.3f " % (key, av.scores[key])            
            str_score = str_score[:-1]
            
            print("%50s %5u - %-5u %2s %5.2fkm %4s  %s" % 
                (air.getName(), air.getMin()[0], air.getMax()[0], "IN" if av.isInside() else "", av.getDistance(), air.getClass(), str_score))
 
    #cut to max levels
    avs = avs[:DATA_LEVELS]

    #sort by min alt
    avs.sort(key = lambda av: av.airspace.getMin())

    if output is not None:
        #store to data file
        for av in avs:
            for byte in av.getBytes():
                output[offset] = byte
                offset += 1
                    
        # Fill up with empty Airspaces
        av = AirspaceVector()
        for i in range(DATA_LEVELS - len(avs)):
            for byte in av.getBytes():
                output[offset] = byte
                offset += 1
                
    return len(avs)
      
class Indexer(object):
    def __init__(self):
        self.num = 0
        self.list = {}
        
    def getNext(self, airspace):
        old = self.num
        self.num += 1
        
        #print("Indexing: %3u" % old, airspace.getName())
        
        self.list[old] = airspace
        
        return old
        
    def printIndex(self, index):  
        airspace = self.list[index]
        hmin, hmin_agl = airspace.getMin()
        hmax, hmax_agl = airspace.getMax()
        class_name = airspace.getClass()
        name = airspace.getName()    
        
        hmin_mode = "AGL" if hmin_agl else "MSL"
        hmax_mode = "AGL" if hmax_agl else "MSL"
        
        print("   i =%3u %6u %s - %6u %s [%4s] %s" % (index, hmin, hmin_mode, hmax, hmax_mode, class_name, name))    
        
    def dumpIndex(self):
        buff = bytes()
        
    
        for i in self.list:
            airspace = self.list[i]
            hmin, hmin_agl = airspace.getMin()
            hmax, hmax_agl = airspace.getMax()
            class_name = airspace.getClass()
            name = airspace.getName()
                        
            hmin = min(hmin, 0x7FFF) + (0x8000 if hmin_agl else 0)
            hmax = min(hmax, 0x7FFF) + (0x8000 if hmax_agl else 0)
                        
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
            
            class_index = class_dict[class_name]
            if len(name) > 49:
                name = name[:49]
                
            name += '\0'
            #floor      2   0
            #ceil       2   2
            #class      1   4
            #name       50  5                       
            #reserved   9   55
                        
            line = (struct.pack("HHB", hmin, hmax, class_index) + bytes(name, "ascii"))
            buff += line + bytes('\0', "ascii") * (64 - len(line))
            
        return buff
    
        
def dump(lon, lat, airspaces):

    global wantedResolution
    
    profile = False
    
    if profile:
        pr = cProfile.Profile()
        pr.enable() 
    
    lat_n = abs((lat * HGT_COORD_MUL) / HGT_COORD_MUL);
    lon_n = abs((lon * HGT_COORD_MUL) / HGT_COORD_MUL);

    if lat >= 0:
        lat_c = 'N'
    else:
        lat_c = 'S'
        lat_n += 1
        
    if (lon >= 0):
        lon_c = 'E'
    else:
        lon_c = 'W'
        lon_n += 1

    filename = "%c%02u%c%03u.AIR" % (lat_c, lat_n, lon_c, lon_n)
    
    numPoints = wantedResolution

    filesize = numPoints * numPoints * DATA_LEVELS * DATA_LEVEL_SIZE
    if os.path.isfile(filename) and not force:
        print (filename, "exists, skipping...")
        return
    
    print (filename, "computing (" + str(numPoints) + "x" + str(numPoints) + ")")

    output = bytearray(filesize)
    
    f = None
    
    indexer = Indexer()
    
    for airspace in airspaces:
        airspace.setIndexer(indexer)
    
    try:
        # Quickcheck for emptyness
        isEmpty = True
        for lat_i in numpy.arange(lat, lat + 1, 1/4):
            for lon_i in numpy.arange(lon, lon + 1, 1/4):
                p = shapely.geometry.Point(lon_i, lat_i)
                if dumpPoint(None, 0, p, airspaces) > 0:
                    isEmpty = False
                if not isEmpty:
                    break
            if not isEmpty:
                break
                    

        if not isEmpty:
            pos = 0
            f = open(filename, 'wb')
            last_per = 999
            
            for lat_i in numpy.arange(lat, lat + 1, 1 / numPoints):
                pos += 1
                per = int((pos * 100) / numPoints)
                if per != last_per:
                    print ("%s: %u %%" % (filename, per))
                    last_per = per
                    
                for lon_i in numpy.arange(lon, lon + 1, 1 / numPoints):
                    off = (1 / numPoints) * 0.5
                    p = shapely.geometry.Point(lon_i + off, lat_i + off)
                    
                    mul = HGT_COORD_MUL
                    x = int(round((abs(lon_i) * mul % mul) * numPoints / mul))
                    y = int(round((abs(lat_i) * mul % mul) * numPoints / mul))
                    
                    offset = int((x * numPoints + y) * DATA_LEVELS * DATA_LEVEL_SIZE)
                    dumpPoint(output, offset, p, airspaces)
                    
    except (KeyboardInterrupt, SystemExit):
        print("Exiting...")
        if f:
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
        f.write(bytes(indexer.dumpIndex()))
               
        print (filename, "saved")
        f.close()
        
    if profile:        
        pr.disable()
        pr.print_stats(sort = "cumtime")

#**********************************************************************
#                                main()
#**********************************************************************

def usage():
    print ('convert.py openairspace-file [lat] [lon]')
    
def main(argv = None):

    global bVerbose
    global latOnly, lonOnly
    global wantedResolution
    global force
    global checkPoint
    
    if argv is None:
        argv = sys.argv

    try:
        opts, args = getopt.getopt(argv,"hvqdr:fc:",["help", "resolution=","quiet","verbose", "force", "check"])
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
        
    print("Resolution table")
    print("Max distance %7.3fkm" % (OFFSET_BASE * 111))
    print("Mode 0 %0.10f deg -> %7.3fkm  %7.3fkm" % (OFFSET_MUL_0, OFFSET_MUL_0 * 111, OFFSET_MUL_0 * 111 * 64))
    print("Mode 1 %0.10f deg -> %7.3fkm  %7.3fkm" % (OFFSET_MUL_1, OFFSET_MUL_1 * 111, OFFSET_MUL_1 * 111 * 64))
    print("Mode 2 %0.10f deg -> %7.3fkm  %7.3fkm" % (OFFSET_MUL_2, OFFSET_MUL_2 * 111, OFFSET_MUL_2 * 111 * 64))
    print("Mode 3 normal vector")
        
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
    
        indexer = Indexer()
        
        for airspace in airspaces:
            airspace.setIndexer(indexer)    
    
        output = bytearray(DATA_LEVELS * DATA_LEVEL_SIZE)
        dumpPoint(output, 0, checkPoint, airspaces, True)
        
        print()
        for level in range(DATA_LEVELS):
            offset = level * DATA_LEVEL_SIZE
            index = int.from_bytes([output[offset+0]], 'little', signed=False)
            a = output[offset+1]
            b = output[offset+2]

            inside = bool(index & 0x80)
            index &= 0x7F
            
            mode = (a & 0x80) >> 6 | (b & 0x80) >> 7           

            lat_offset = a & 0x7F
            long_offset = b & 0x7F
            if lat_offset & 0x40:
                lat_offset = -(lat_offset & 0x3F)
            if long_offset & 0x40:
                long_offset = -(long_offset & 0x3F)           


            if index == 0x7F:
                print ("---")
                continue
            
            print ("mode %d lat %d lon %d" % (mode, lat_offset, long_offset))
            indexer.printIndex(index)
            

    count = 0

    if checkPoint == None:   
        try:
            procs = []
            if latOnly != None and lonOnly != 0:
                p = multiprocessing.Process(target=dump, args=(lonOnly,latOnly,airspaces))
                procs.append(p)
            else:
                for lat in range(int(boundingBox[1])-1,int(boundingBox[3])+2):
                    for lon in range(int(boundingBox[0])-1,int(boundingBox[2])+2):
                        p = multiprocessing.Process(target=dump, args=(lon,lat,airspaces))
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
    #print("OGRFeature(%s):%ld" % (poDefn.GetName(), poFeature.GetFID() ))

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
                airspace.setClass(classAir)
                airspaces.append(airspace)
        
    return

if __name__ == '__main__':
    version_num = int(gdal.VersionInfo('VERSION_NUM'))
    if version_num < 1800: # because of ogr.GetFieldTypeName
        print('ERROR: Python bindings of GDAL 1.8.0 or later required')
        sys.exit(1)

    sys.exit(main(sys.argv[1:]))

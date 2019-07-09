#!/usr/bin/env python3
# -*- mode: python-mode; python-indent-offset: 4 -*-
#*****************************************************************************
# dnf install python3-shapely python3-gdal python3-matplotlib
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
import openaip

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
mk_list = None
DataSource = None
inspect = False

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

   
def dumpPoint(output, offset, p, airspaces, check=False):

    def av_score(av):
        score = 0
        
        if av.isInside():
            dist_score = 1
        else:
            dist_score = 1 - min(av.getDistance() / (33.0), 1)

        score += dist_score * 10000   
            
        alt_score = 1 - min(av.airspace.getMin()[0] / 10000, 1)            
        score += alt_score * 1000
        
        class_score = CLASS_SCORE[av.airspace.getClass()]
        score += class_score * 100
        
        av.scores = {}
        av.scores["D"] = dist_score
        av.scores["A"] = alt_score
        av.scores["C"] = class_score
        av.scores["T"] = score
        
        return score

    global bVerbose
    global mk_list

    avs = []
    
    #get all airspaces in proximity
    for airspace in airspaces:
        if airspace.isNear(p):
            av = airspace.getAirspaceVector(p)
            avs.append(av)
            
            if mk_list:
                return True

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
            
            print("%50s %5u - %-5u %2s %5.2fkm %10s  %s" % 
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
        
        print("   i =%3u %6u %s - %6u %s [%10s] %s" % (index, hmin, hmin_mode, hmax, hmax_mode, class_name, name))    
        
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

            class_index = CLASS_DICT[class_name]
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

    #print("lon, lat", lon, lat)

    global wantedResolution
    global mk_list
    global DataSource
    
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
        
    if (lon >= 0):
        lon_c = 'E'
    else:
        lon_c = 'W'

    filename = "%c%02u%c%03u.AIR" % (lat_c, lat_n, lon_c, lon_n)

    if lat < LAT_MINIMUM:
        print (filename, "too low to care, skipping...")
        return    
    
    if os.path.isfile("data/" + filename) and not force and not mk_list:
        print (filename, "exists, skipping...")
        return    
    
    #do we need to add aditional airspace?
    if os.path.exists("lookup/" + filename) and not mk_list:
        f = open("lookup/" + filename, "r")
        data = f.readlines()
        f.close()
        
        #print(data)
        
        needed_as = []
        
        for a in data:
            a = a.split()[0]
            needed_as.append(a)
        
        if DataSource not in needed_as:
            print("%s Not needed for this airspace, skipping..." % filename)
            return
            
        for a in needed_as:
            if a == DataSource:
                continue
            load_airspace(a)
   
    #is this airspace over water?
    if os.path.exists("agl_tiles.list"):
        do_not_generate = True
        
        f = open("agl_tiles.list", "r")
        data = f.readlines()
        f.close()
        
        needed_as = []
        
        for a in data:
            a = a.split(".")[0] + ".AIR"
            if a == filename:
                do_not_generate = False
                
        if do_not_generate:
            print(filename, "over water, skipping...")
            return
            
    
    numPoints = wantedResolution
    filesize = numPoints * numPoints * DATA_LEVELS * DATA_LEVEL_SIZE

    output = bytearray(filesize)
    
    f = None
    
    indexer = Indexer()
    
    for airspace in airspaces:
        airspace.setIndexer(indexer)
    
    try:
        # Quickcheck for emptyness
        isEmpty = True
        delta = AIRSPACE_BORDER / 2
        
        print (filename, "Checking...")
        for lat_i in numpy.arange(lat + delta / 2, lat + 1, delta):
            for lon_i in numpy.arange(lon + delta / 2, lon + 1, delta):
                p = shapely.geometry.Point(lon_i, lat_i)
                if dumpPoint(None, 0, p, airspaces) > 0:
                    isEmpty = False
                if not isEmpty:
                    break
            if not isEmpty:
                break

        if not isEmpty:
            if mk_list:
                print (filename, "added to list")
                f1 = open("lists/%s.list" % os.path.basename(DataSource), "a")
                f1.write("%s\n" % filename)
                f1.close()
                return 

            print (filename, "computing (%ux%u)" % (numPoints, numPoints))

        
            pos = 0
            if not os.path.isdir("data"):
                os.mkdir("data")

            f = open("data/" + filename, 'wb')
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
                    x = int(round(((lon_i) * mul % mul) * numPoints / mul))
                    y = int(round(((lat_i) * mul % mul) * numPoints / mul))
                    
                    offset = int((x * numPoints + y) * DATA_LEVELS * DATA_LEVEL_SIZE)
                    dumpPoint(output, offset, p, airspaces)
                    
    except (KeyboardInterrupt, SystemExit):
        print("Exiting...")
        if f:
            f.close()
            os.system("rm data/" + filename)

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

def load_airspace(filename):
    filename = "source/" + filename
    print("Loading %s" % filename)

    #load airspaces
    classes = {}    
    skipped = []
    invalid = []
    
    global bVerbose    
    global airspaces
    global inspect


    for oas in openaip.load_file(filename):
    
        if oas.invalid:
            invalid.append(oas)
            continue
    
        for class_name in CLASS_DICT.keys():
            #do not change classes for single letter
            if len(class_name) == 1:
                continue
        
            #change class if starting as CTR, TMA ...
            if oas.name.find(class_name) == 0 and oas.category != class_name:
                if (bVerbose > 1):
                    print(oas.name)
                    print(" %10s --> %10s" % (oas.category, class_name))
                oas.category = class_name
            
        #do we skip this category?
        if CLASS_FILTER[oas.category] and oas.bottom.value <= MAX_ALTITUDE:
            airspaces.append(Airspace(oas))
            
            if oas.category not in classes:
                classes[oas.category] = [oas]
            else:   
                classes[oas.category].append(oas)
            
        else:
            skipped.append(oas)
            
    if inspect:
        for key in classes:
            print("---- %10s (%2u) ----" % (key, len(classes[key])))
            for n in classes[key]:
                print("  %s" % n)
            print()
        
        print("---- %10s (%2u) ----" % ("SKIPPED", len(skipped)))
        for s in skipped:
            print(" ", s)
        print()            

        print("---- %10s (%2u) ----" % ("INVALID", len(invalid)))
        for s in invalid:
            print(" ", s)
        print()
        
        return

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
    global mk_list
    global DataSource
    global inspect
    
    if argv is None:
        argv = sys.argv

    try:
        opts, args = getopt.getopt(argv,"hr:qvlfc:i",["help", "resolution=","quiet","verbose", "list", "force", "check", "inspect"])
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
        elif opt in ("-l", "--list"):
            mk_list = True
        elif opt in ("-i", "--inspect"):
            inspect = True
        elif opt in ("-c", "--check"):
            m = re.match('([-+]?\d*\.\d+|\d+),([-+]?\d*\.\d+|\d+)', arg)
            if m != None:
                checkPoint = shapely.geometry.Point(float(m.group(2)), float(m.group(1)))
                print(checkPoint)
            else:
                print (arg, "is invalid for --check. Use e.g. 48.5,10.2")
                sys.exit(1)
                
    if len(args) == 1:
        DataSource = args[0]
    elif len(args) == 3:
        DataSource = args[0]
        latOnly = int(args[1])
        lonOnly = int(args[2])
    else:
        usage()
        sys.exit(1)
       
    if (bVerbose > 1):
        print("Resolution table")
        print("Max distance %7.3fkm" % (OFFSET_BASE * 111))
        print("Mode 0 %0.10f deg -> %7.3fkm  %7.3fkm" % (OFFSET_MUL_0, OFFSET_MUL_0 * 111, OFFSET_MUL_0 * 111 * 64))
        print("Mode 1 %0.10f deg -> %7.3fkm  %7.3fkm" % (OFFSET_MUL_1, OFFSET_MUL_1 * 111, OFFSET_MUL_1 * 111 * 64))
        print("Mode 2 %0.10f deg -> %7.3fkm  %7.3fkm" % (OFFSET_MUL_2, OFFSET_MUL_2 * 111, OFFSET_MUL_2 * 111 * 64))
        print("Mode 3 normal vector")
        
        
    DataSource = os.path.basename(DataSource)
    load_airspace(DataSource)

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
                print ("%u ---" % level)
                continue
            
            print ("%u mode %d lat %d lon %d" % (level, mode, lat_offset, long_offset))
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
            if mk_list:
                #if we are making list use only one, since we are writing the result to single file
                parallelism = 1 
                os.system("rm lists/%s.list" % os.path.basename(DataSource))

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
                #time.sleep(0.1)

        except (KeyboardInterrupt, SystemExit):
            print("Exiting (main)...")
            sys.exit(1)

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

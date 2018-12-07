#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# This script creates AIR files for SkyDrop. It takes some airspace definitions and iterates over 
# a given raster to compute angle and distance to the airspace in various heights. This data is stored
# in the AIR files and copied onto SD card in SkyDrop. The device then simply uses that information to
# display angle and distance to CTR.
#
# 02.12.2018: tilmann@bubecks.de
#

from math import sqrt,cos,atan2,floor
import numpy
import sys
from PIL import Image, ImageDraw
import matplotlib.pyplot as plt
import airspaces_data

 #
 # Returns the bearing from lat1/lon1 to lat2/lon2. All parameters
 # must be given as fixed integers multiplied with GPS_MULT.
 #
 # \param lat1 the latitude of the 1st GPS point
 # \param lon1 the longitude of the 1st GPS point
 # \param lat2 the latitude of the 2nd GPS point
 # \param lon2 the longitude of the 2nd GPS point
 #
 # \return the bearing in degrees (0-359, where 0 is north, 90 is east, ...).
 #
def gps_bearing(P1, P2):
    d = P2 - P1
    return (numpy.degrees(atan2(d[0],d[1])) + 360) % 360

 #
 # Compute the distance between two GPS points in 2 dimensions
 # (without altitude). Latitude and longitude parameters must be given as fixed integers
 # multiplied with GPS_MULT.
 #
 # \param lat1 the latitude of the 1st GPS point
 # \param lon1 the longitude of the 1st GPS point
 # \param lat2 the latitude of the 2nd GPS point
 # \param lon2 the longitude of the 2nd GPS point
 #
 # \return the distance in cm.
 #
def gps_distance_2d(P1, P2):

    # Compute the average lat of lat1 and lat2 to get the width of a
    # 1 degree cell at that position of the earth:
    lat = (P1[1] + P2[1]) / 2 * (numpy.pi / 180.0)

    # 111.3 km (in cm) is the width of 1 degree
    dx = cos(lat) * 11130000 * abs(P1[0] - P2[0])
    dy = 1.0      * 11130000 * abs(P1[1] - P2[1])

    return sqrt(dx * dx + dy * dy)

def draw_p(p, color="black"):
    plt.plot(p[0], p[1], '.', color=color)

def draw_vector(p1, p2, color="black", alpha=1.0):
    plt.arrow(p1[0], p1[1], p2[0] - p1[0], p2[1]-p1[1], length_includes_head=True, color=color, head_width=0.01, alpha=alpha)

def unit_vector(vector):
    """ Returns the unit vector of the vector.  """
    return vector / numpy.linalg.norm(vector)

def angle_between(v1, v2):
    """ Returns the angle in radians between vectors 'v1' and 'v2'::

            >>> angle_between((1, 0, 0), (0, 1, 0))
            1.5707963267948966
            >>> angle_between((1, 0, 0), (1, 0, 0))
            0.0
            >>> angle_between((1, 0, 0), (-1, 0, 0))
            3.141592653589793
    """
    v1_u = unit_vector(v1)
    v2_u = unit_vector(v2)
    return numpy.arccos(numpy.clip(numpy.dot(v1_u, v2_u), -1.0, 1.0))


# http://www.uni-protokolle.de/foren/viewt/15542,0.html
# Bestimme den Lotfusspunkt des Punktes p auf der Geraden durch A und B.
# Punkt P=(x1, y1) 
def lotfuss(A, B, P):
    # u=[(y1-y2)*(y3-y2)+(x2-x1)*(x2-x3)]/[(y3-y2)*(y3-y2)-(x3-x2)*(x2-x3)]
    u = ((P[1]-A[1])*(B[1]-A[1])+(A[0]-P[0])*(A[0]-B[0]))/((B[1]-A[1])*(B[1]-A[1])-(B[0]-A[0])*(A[0]-B[0]))
    return numpy.array([A[0]+u*(B[0]-A[0]), A[1]+u*(B[1]-A[1])])

#
# Return the distance between the line between p1 and p2 and p.
#http://www.matheboard.de/archive/116361/thread.html
def distancePointToLine(p1, p2, p):
    m = (p2[1] - p1[1]) / (p2[0] - p1[0])
    n = p1[1] - m * p1[0]
    d = abs((m * p[0] - p[1] + n) / sqrt(m * m + 1))
    return d

# Bestimme den zu P nächstgelegenen Punkt auf der Linue von A nach B
def closestPointToLineSegment(A, B, P, debug=False):
    L = lotfuss(A, B, P)
    if debug:
        draw_vector(A, B, "red")
        draw_p(L, "black")
        #draw_vector(A, L)
        #draw_vector(L, P)

        # (1) L ist links von AB
    AB = unit_vector(B - A)
    AL = unit_vector(L - A)
    #if debug:
    #    print (AB, AL)

    if not numpy.allclose(AB, AL):
        # L is left of A, therefore A is closest
        return A

    # (2) L ist rechts von AB
    BA = unit_vector(A - B)
    BL = unit_vector(L - B)
    if not numpy.allclose(BA, BL):
        # L is right of B, therefore B is closest
        #return numpy.linalg.norm(P - B)
        return B

    return L

def meter_to_feet(m):
    return m * 3.2808399

class Airspace:
    def __init__(self):
        self.points = None

    def setPoints(self, p):
        self.points = p
        
    def printReversePoints(self, p):
        self.points = numpy.array([]);
        for i in range(0, len(p) - 1, +2):
            p1 = p[i]
            p2 = p[i + 1]
            print ( "[" , p1, ", ", p2, "], ")
        programmendehier

    def setMinMax(self, minFt, maxFt):
        self.minFt = minFt
        self.maxFt = maxFt

    def getMin(self):
        return self.minFt
    
    def getMax(self):
        return self.maxFt
    
    def append(self, p):
        self.points.append(p)

    def closestPointTo(self, p):
        closestP = None
        distanceP = None
        for i in range(len(self.points) - 1):
            p1 = self.points[i]
            p2 = self.points[i + 1]
            c = closestPointToLineSegment(p1, p2, p, False)
            distance = numpy.linalg.norm(c - p)
            if distanceP == None or distance < distanceP:
                distanceP = distance
                closestP = c

        return closestP
    
    def draw(self):
        for i in range(len(self.points) - 1):
            p1 = self.points[i]
            p2 = self.points[i + 1]
            draw_vector(p1, p2, "blue")
            
    def is_inside(self, p):
        count = 0
        for i in range(len(self.points) - 1):
            p1 = self.points[i]
            p2 = self.points[i + 1]
            
            g = p2 - p1
            v = p - p1
#            https://www.c-plusplus.net/forum/266934-full
#Klar: Sei (g1,g2) der Richtungsvektor der Geraden, (s1,s2) der Stützvektor und (p1,p2) der Punkt. Dann ist g2*(p1-s1)+g1*(p2-s2) entweder positiv oder negativ (oder Null). Null heißt, der Punkt ist auf der Geraden, positiv heißt, er ist in Richtung des Richtungsvektors aus gesehen rechts, negativ heißt links.
            result = g[1] * (p[0]-p1[0]) - g[0] * (p[1]-p1[1])
            if result > 0:
                count = count + 1
            else:
                count = count - 1

        return (abs(count) == len(self.points) - 1)
            
    def __repr__(self):
        return "".join(["Polygon(", str(self.points), ")"])

    def getBoundingBox(self):
        bb = [+100, +200, -100, -200]
        for p in self.points:
            bb[0] = min(p[0], bb[0])
            bb[2] = max(p[0], bb[2])
            bb[1] = min(p[1], bb[1])
            bb[3] = max(p[1], bb[3])
        return bb
            
    def getAngleAndDistance(self, p, draw=False):
        c = self.closestPointTo(p)
        distance_cm = gps_distance_2d(p,c)
        angle = gps_bearing(p,c)
        if self.is_inside(p):
            forbidden = True
            if draw:
                draw_p(p, "red")
                draw_vector(p, c, "red", 0.2)
        else:
            forbidden = False
            # Show arrow away from airspace
            angle = angle - 180
            if angle < 0:
                angle = angle + 360
            if draw:
                draw_p(p, "green")
                draw_vector(p, c, "black", 0.2)
                    
        angle_byte = int(angle / 3)
        if forbidden:
            angle_byte = angle_byte + 128;      # highest bit shows FORBIDDEN
        distance_m = distance_cm / 100
        distance_byte = int(distance_m / 64)
        if distance_byte > 255:
            distance_byte = 255
        distance_byte = distance_byte.to_bytes(1, 'little', signed=False)[0]
        angle_byte = angle_byte.to_bytes(1, 'little', signed=False)[0]

        result = [angle_byte, distance_byte]
        return result


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
    [9.375, 48.75],
    [9.375, 48.625]
    ])
    
def dump(lon, lat, airspaces, draw=False):
    filename = f"N{lat:02d}E{lon:03d}.air"
    print (filename)
    f = open(filename, 'wb')
    if draw:
        numPoints = 8
    else:
        numPoints = 1200
    skip = 1/numPoints
    for y in numpy.arange(lat + 1 + skip, lat, -skip):
        for x in numpy.arange(lon, lon + 1 + skip, skip):
            p = numpy.array([x,y])

            check = False
            for c in checkpoints:
                if p[0] == c[0] and p[1] == c[1]:
                    check = True
                    print (p)

            airspace_count = 0
            for airspace in airspaces:
                airspace_count = airspace_count + 1
                height = airspace.getMax()
                angleAndDistance = airspace.getAngleAndDistance(p, draw)
                f.write(height.to_bytes(2, 'little', signed=False))
                f.write(bytes(angleAndDistance))
                if check:
                    print ("Height:  ", height, "Angle,Dist: ", angleAndDistance) 

            while airspace_count < 5:
                airspace_count = airspace_count + 1
                height = 0
                angleAndDistance = [0, 0]
                f.write(height.to_bytes(2, 'little', signed=False))
                f.write(bytes(angleAndDistance))
            
    f.close()

stuttgart_1 = Airspace()
stuttgart_1.setPoints(airspaces_data.stuttgart)
stuttgart_1.setMinMax(0, 3500)

stuttgart_2 = Airspace()
stuttgart_2.setPoints(airspaces_data.stuttgart_2)
stuttgart_2.setMinMax(3500, 4500)

stuttgart_3 = Airspace()
stuttgart_3.setPoints(airspaces_data.stuttgart_3)
stuttgart_3.setMinMax(4500, 5500)

stuttgart_4 = Airspace()
stuttgart_4.setPoints(airspaces_data.stuttgart_4)
stuttgart_4.setMinMax(5500, 7500)

stuttgart_5 = Airspace()
stuttgart_5.setPoints(airspaces_data.stuttgart_5)
stuttgart_5.setMinMax(7500, 10000)

mallorca_1 = Airspace()
mallorca_1.setPoints(airspaces_data.mallorca)
mallorca_1.setMinMax(0, 1000)

# airspaces = [ stuttgart_1, stuttgart_2, stuttgart_3, stuttgart_4, stuttgart_5 ]
airspaces = [ mallorca_1 ]

boundingBox = getBoundingBox(airspaces)

draw = True

if draw:
    for airspace in airspaces:
        airspace.draw()

if len(sys.argv) > 1:
    dump(int(sys.argv[2]), int(sys.argv[1]))
else:
    for lat in numpy.arange(floor(boundingBox[1]), floor(boundingBox[3]) + 1, +1):
        for lon in numpy.arange(floor(boundingBox[0]), floor(boundingBox[2]) + 1, +1):
            dump(lon, lat, airspaces, draw)

if draw:
    plt.axis('equal')
    plt.show()

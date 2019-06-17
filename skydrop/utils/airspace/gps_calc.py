import numpy
from math import cos, sqrt, atan2

def gps_bearing_shapely(P1, P2):
    p1 = numpy.array([P1.x,P1.y])
    p2 = numpy.array([P2.x,P2.y])

    d = p2 - p1
    return (numpy.degrees(atan2(d[0],d[1])) + 360) % 360

def gps_distance_2d_shapely(P1, P2):
    lat = (P1.y + P2.y) / 2 * (numpy.pi / 180.0)

    # 111.3 km (in cm) is the width of 1 degree
    dx = cos(lat) * 11130000 * abs(P1.x - P2.x)
    dy = 1.0      * 11130000 * abs(P1.y - P2.y)

    return sqrt(dx * dx + dy * dy)


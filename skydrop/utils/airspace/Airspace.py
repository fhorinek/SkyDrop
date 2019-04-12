import matplotlib.pyplot as plt
import shapely
import shapely.ops
import shapely.geometry
import sys
from pprint import pprint
import numpy
from math import sqrt,cos,atan2,floor
from AirspaceVector import AirspaceVector


def gps_bearing_shapely(P1, P2):
    p1 = numpy.array([P1.x,P1.y])
    p2 = numpy.array([P2.x,P2.y])

    return gps_bearing(p1,p2)

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

def gps_distance_2d_shapely(P1, P2):
    return gps_distance_2d([P1.x, P1.y], [P2.x, P2.y])

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
    plt.plot(p.x, p.y, '.', color=color)

def draw_vector(p1, p2, color="black", alpha=1.0):
    dx = p2.x - p1.x
    dy = p2.y - p1.y
    if dx != 0 and dy != 0:
        #print (p1[0], p1[1], dy, dy)
        plt.arrow(p1.x, p1.y, dx, dy, length_includes_head=True, color=color, head_width=0.01, alpha=alpha)

def draw_line(p1, p2, color="black", alpha=1.0):
    plt.plot([p1.x, p2.x], [p1.y, p2.y], color=color, alpha=alpha)

# This is a airspace, which is forbidden to fly into.
# It consists of a self.polygon describing the outside boundary and a minimum
# and maximum height of this airspace.
#
class Airspace:
    def __init__(self):
        self.minFt = None
        self.maxFt = None
        self.name = None
        self.polygon = None
        
    def setName(self, name):
        self.name = name
        
    def getName(self):
        return self.name
        
    def setPolygon(self, polygon):
        self.polygon = polygon
        self.bb = self.polygon.bounds
        self.center = shapely.geometry.Point( (self.bb[0]+self.bb[2])/2, (self.bb[1]+self.bb[3])/2 )

    def setMinMax(self, minFt, maxFt):
        self.minFt = minFt
        self.maxFt = maxFt

    def getMin(self):
        return self.minFt
    
    def getMax(self):
        return self.maxFt
    
    def draw(self):
        print ("draw airspace \""+self.name+"\"")
        coords = list(self.polygon.exterior.coords)
        for i in range(len(coords)-1):
            p1 = shapely.geometry.Point(coords[i])
            p2 = shapely.geometry.Point(coords[i+1])
            draw_line(p1, p2, "blue")

    def is_inside(self, p):
        return 

    def __repr__(self):
        from pprint import pformat
        #return pformat(vars(self))
        return self.name+" BB: "+pformat(self.getBoundingBox())+", min: "+pformat(self.minFt)+", max: "+pformat(self.maxFt)

    def getBoundingBox(self):
        return self.bb

    def getCenter(self):
        return self.center

    def getDistanceToCenter(self, point):
        return gps_distance_2d_shapely(point, self.center)
        
    # Compute a vector from the given point to this airspace.
    # The point is meant as the pilots position and the vector
    # is pointing to the boundary of the airspace.
    #
    # So this is used to compute the angle and distance of the
    # pilot to get into or out of the airspace.
    #
    def getAirspaceVector(self, point, draw=False):
        av = AirspaceVector()
        av.setAirspace(self)
        av.setInside(point.within(self.polygon))

        nearest_points = shapely.ops.nearest_points(self.polygon.boundary, point)
        if len(nearest_points) != 2:
            print(len(nearest_points),"nearest points to",p)
            for np in nearest_points:
                pprint(np.wkt)
            sys.exit(1)
        c = nearest_points[0]
        
        distance_cm = gps_distance_2d_shapely(point,c)
        angle = gps_bearing_shapely(point,c)

        av.setDistance(distance_cm)

        if not av.isInside():
            # Show arrow away from airspace
            angle = angle - 180
            if angle < 0:
                angle = angle + 360

        av.setAngle(angle)

        if draw:
            if av.isInside():
                draw_p(point, "red")
                draw_vector(point, c, "red", 0.2)
            else:
                draw_p(point, "green")
                draw_vector(point, c, "black", 0.2)

        return av

import matplotlib.pyplot as plt
import shapely
import shapely.ops
import shapely.geometry
import sys
from pprint import pprint
import numpy
from math import sqrt,cos,atan2,floor
from AirspaceVector import AirspaceVector


from const import *

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
    def __init__(self, a):
        self.minFt = a.bottom.value
        self.minAGL = a.bottom.agl
        
        self.maxFt = a.top.value
        self.maxAGL = a.top.agl
        
        self.name = a.name
        
        self.setPolygon(shapely.geometry.Polygon(a.coordinates))
        self.class_name = a.category
        
        self.index = None
        self.indexer = None
        
    def setIndexer(self, indexer):
        self.indexer = indexer
        
    def getIndex(self):
        if self.index == None:
            self.index = self.indexer.getNext(self)
            
        return self.index
        
    def getClass(self):
        return self.class_name
                
    def getName(self):
        return self.name
        
    def setPolygon(self, polygon):
        self.polygon = polygon
        self.bb = self.polygon.bounds
        
        ex = AIRSPACE_BORDER      
        self.near_box = shapely.geometry.box(self.bb[0] - ex, self.bb[1] - ex, self.bb[2] + ex, self.bb[3] + ex)

    def getMin(self):
        return self.minFt, self.minAGL
    
    def getMax(self):
        return self.maxFt, self.maxAGL
    
    def draw(self):
        print ("draw airspace \""+self.name+"\"")
        coords = list(self.polygon.exterior.coords)
        for i in range(len(coords)-1):
            p1 = shapely.geometry.Point(coords[i])
            p2 = shapely.geometry.Point(coords[i+1])
            draw_line(p1, p2, "blue")

    def __repr__(self):
        from pprint import pformat
        return self.name+" BB: "+pformat(self.getBoundingBox())+", min: "+pformat(self.minFt)+", max: "+pformat(self.maxFt)+", class: "+ self.class_name

    def getBoundingBox(self):
        return self.bb

    def isNear(self, point):
        return point.within(self.near_box)

        
    # Compute a vector from the given point to this airspace.
    # The point is meant as the pilots position and the vector
    # is pointing to the boundary of the airspace.
    #
    # So this is used to compute the angle and distance of the
    # pilot to get into or out of the airspace.
    #
    def getAirspaceVector(self, point):
    
        nearest_points = shapely.ops.nearest_points(self.polygon.boundary, point)
        if len(nearest_points) != 2:
            print(len(nearest_points),"nearest points to",p)
            for np in nearest_points:
                pprint(np.wkt)
            sys.exit(1)
        target = nearest_points[0]
            
        av = AirspaceVector(self, point, target)

        return av

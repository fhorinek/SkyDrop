import random
import struct

import shapely.geometry
from const import *
from gps_calc import *

from math import sqrt


class AirspaceVector:

    def __init__(self, airspace = None, point = None, target = None):
        self.airspace = airspace
        self.point = point
        self.target = target
        self.too_far = False
        
        if self.airspace:
            self.inside = point.within(self.airspace.polygon)
            self.distance_km = gps_distance_2d_shapely(self.point, self.target) / (100 * 1000)

    def getDistance(self):
        return self.distance_km

    def isInside(self):
        return self.inside

    def __repr__(self):
        from pprint import pformat
        return pformat(vars(self))
    
    def getBytes(self):
        if self.airspace != None:
            index = self.airspace.getIndex()
        else:
            return struct.pack("BBB", 0x7F, 0, 0)      
            
        if self.inside:
            index |= 0x80
            
        #index
        #   1000 0000 - inside (128, 0x80)
        #   -III IIII - 0 - 125 index
        #             - 126 no airspace (126, 0x7F)  
        #a  1000 0000 - mode A
        #b  1000 0000 - mode B
        
        # A B  
        # 0 0 mode0 offset with mul OFFSET_MUL_0
        # 0 1 mode1 offset with mul OFFSET_MUL_1
        # 1 0 mode2 offset with mul OFFSET_MUL_2
        # 1 1 mode3 offset with normalised vector
        mode = 0
            
        offset_long = round((self.target.x - self.point.x) / OFFSET_MUL_0)
        offset_lat = round((self.target.y - self.point.y) / OFFSET_MUL_0)
        
        if abs(offset_long) > 63 or abs(offset_lat) > 63: 
            if abs(offset_long) > abs(offset_lat):
                ratio = 63 / abs(offset_long)
            else:
                ratio = 63 / abs(offset_lat)

            offset_lat = round(offset_lat * ratio)
            offset_long = round(offset_long * ratio)
        
        if abs(offset_long) < MIN_RES and abs(offset_lat) < MIN_RES:
            mode = 1
        
            offset_long = round((self.target.x - self.point.x) / OFFSET_MUL_1)
            offset_lat = round((self.target.y - self.point.y) / OFFSET_MUL_1)

            if abs(offset_long) < MIN_RES and abs(offset_lat) < MIN_RES:
                mode = 2
                
                offset_long = round((self.target.x - self.point.x) / OFFSET_MUL_2)
                offset_lat = round((self.target.y - self.point.y) / OFFSET_MUL_2)       
    
                if abs(offset_long) < MIN_RES and abs(offset_lat) < MIN_RES:
                    mode = 3
                
                    off_x = self.target.x - self.point.x
                    off_y = self.target.y - self.point.y
                    dist = sqrt(off_x ** 2 + off_y ** 2)
                    
                    if dist == 0:
                    
                        #Zero distance condition! 
                        #Point 20.20249999999996 49.522500000000264
                        #Target 20.20249999999996 49.522500000000264

                    
                        print("Zero distance condition! ")
                    
                        print("Point", self.point.x, self.point.y)
                        print("Target", self.target.x, self.target.y)

                        x = self.point.x + [-1, 1][random.randint(0, 1)] * random.uniform(0.00001, 0.0001)
                        y = self.point.y + [-1, 1][random.randint(0, 1)] * random.uniform(0.00001, 0.0001)
    
                        new_point = shapely.geometry.Point(x, y)

                        new_av = self.airspace.getAirspaceVector(new_point) 
                        return new_av.getBytes()         
                    
                    mul = 60 / dist
                    
                    offset_long = round(off_x * mul)
                    offset_lat = round(off_y * mul)      
                    
        a = int(abs(offset_lat))
        b = int(abs(offset_long))
        
        if offset_lat < 0:
            a |= 0x40
        if offset_long < 0:
            b |= 0x40

        assert abs(offset_lat) <= 63, "offset_lat %d!!! mode %d dist %0.2fkm " % (offset_lat, mode, self.distance_km) + str(self.point)
        assert abs(offset_long) <= 63, "offset_long %d!!! mode %d dist %0.2fkm " % (offset_long, mode, self.distance_km) + str(self.point)
        assert not (a == 0 and b == 0), "A,B == 0" + str(self.target) + " " + str(self.point)
        
        #add sign
        if mode == 0:
            a |= 0
            b |= 0
        elif mode == 1:
            a |= 0
            b |= 0x80
        elif mode == 2:
            a |= 0x80
            b |= 0
        elif mode == 3:
            a |= 0x80
            b |= 0x80
        
        bytes = struct.pack("BBB", index, a, b)      
            
        return bytes


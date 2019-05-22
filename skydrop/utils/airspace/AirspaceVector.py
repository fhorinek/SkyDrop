import sys
from agl import get_alt_point

import struct

from const import *
from gps_calc import *

from math import sqrt


class AirspaceVector:

    def __init__(self, airspace = None, point = None, target = None):
        self.airspace = airspace
        self.point = point
        self.target = target

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
        # 0 0 Angle mode
        # 1 0 offset with mul 0.001         ~111m
        # 0 1 offset with mul 0.0001        ~11m
        # 1 1 offset with normalised vector +/- 22m

        # Offset mode
        #a  -LLL LLLL - latitude offset  
        #b  -LLL LLLL - longitude offset 

        # Angle mode
        #a  -DDD DDDD - distance *500m
        #b  -AAA AAAA - angle / 3
            
        offset_long = round((self.target.x - self.point.x) / OFFSET_MUL)
        offset_lat = round((self.target.y - self.point.y) / OFFSET_MUL)

        if (abs(offset_lat) > 63 or abs(offset_long) > 63):
            angle = gps_bearing_shapely(self.point, self.target)

            a = min(int(self.distance_km * 2), 127)
            b = int(angle / 3)
            
            bytes = struct.pack("BBB", index, a, b)      
        else:
            too_close = (abs(offset_long) < 6 and abs(offset_lat) < 6)

            if too_close:
                offset_long = round((self.target.x - self.point.x) / OFFSET_MUL_FINE)
                offset_lat = round((self.target.y - self.point.y) / OFFSET_MUL_FINE)            
        
            assert abs(offset_long) <= 64, "offset_long %f" % offset_long
            assert abs(offset_lat) <= 64,  "offset_lat %f" % offset_lat
        
            still_too_close = abs(offset_long) < 3 and abs(offset_long) < 3
        
            if still_too_close:
            
                off_x = self.target.x - self.point.x
                off_y = self.target.y - self.point.y
                dist = sqrt(off_x ** 2 + off_y ** 2)
                
                mul = 60 / dist
                
                offset_long = round(off_x * mul)
                offset_lat = round(off_y * mul)      
                
                assert abs(offset_long) <= 64, "%f %f %f %f %f %f" % (off_x, off_y, dist, mul, offset_long, offset_lat)
                assert abs(offset_lat) <= 64, "%f %f %f %f %f %f" % (off_x, off_y, dist, mul, offset_long, offset_lat)
        
            a = int(abs(offset_lat))
            b = int(abs(offset_long))
            
            assert not (a == 0 and b == 0), "A,B == 0" + str(self.target) + str(self.point)
            
            if offset_lat < 0:
                a |= 0x40
            if offset_long < 0:
                b |= 0x40
                
            if still_too_close:
                a |= 0x80
                b |= 0x80   
            elif too_close:                            
                a |= 0
                b |= 0x80
            else:
                a |= 0x80
                b |= 0
                
            bytes = struct.pack("BBB", index, a, b)      
            
        return bytes


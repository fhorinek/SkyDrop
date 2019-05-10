import sys
from agl import get_alt_point

# This describes the distance from an airspace to a point.
# It mainly contains a distance and a angle.
#

FEET_STEP = 250
FEET_STEP_in_M = 76.196281622

class AirspaceVector:

    def __init__(self):
        self.distance = 0
        self.angle = 0
        self.airspace = None
        self.inside = False
        self.point = None

    def setPoint(self, point):
        self.point = point

    def setDistance(self, distance):
        self.distance = distance

    def isTooFar(self):
        if self.inside:
            return False
        return self.distance / (100*1000) > 20     # 20 km

    def getDistanceAsByte(self):
        distance_m = self.distance / 100
        distance_byte = int(distance_m / 64)
        if distance_byte > 255:
            distance_byte = 255
        return distance_byte
    
    def setAngle(self, angle):
        self.angle = angle

    def getAngleAsByte(self):
        a = self.angle
        if not self.inside:
            a += 180
            a %= 360
                    
        angle_byte = int(a / 3)
        if self.inside:
            angle_byte = angle_byte + 128;      # highest bit shows FORBIDDEN

        return angle_byte
    
    def setAirspace(self, airspace):
        self.airspace = airspace

    def setInside(self, inside):
        self.inside = inside

    def isInside(self):
        return self.inside

    def __repr__(self):
        from pprint import pformat
        return pformat(vars(self))
    
    def getBytes(self):
        angle_byte = self.getAngleAsByte()
        distance_byte = self.getDistanceAsByte()

        if self.airspace != None:
            agl = None
            
            hfloor, hfloorAGL = self.airspace.getMin()
            hfloor /= FEET_STEP
            
            hfloor = min(hfloor, 127)
            
            if hfloorAGL:
                if hfloor > 0:
                    agl = get_alt_point(self.point) / FEET_STEP_in_M
                    hfloor += agl
                    
                    
                hfloor = int(hfloor) + 0x80
            
            hceil, hceilAGL =  self.airspace.getMax()
            hceil /= FEET_STEP

            hceil = min(hceil, 127)            

            if hceilAGL:
                if not agl:
                    agl = get_alt_point(self.point) / FEET_STEP_in_M
                hceil = int(hceil + agl) + 0x80
        else:
            hfloor = 0
            hceil = 0
         

            
        result = [int(hfloor), int(hceil), angle_byte, distance_byte]
        return result


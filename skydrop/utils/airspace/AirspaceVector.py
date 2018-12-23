import sys

# This describes the distance from an airspace to a point.
# It mainly contains a distance and a angle.
#
class AirspaceVector:

    def __init__(self):
        self.distance = 0
        self.angle = 0
        self.airspace = None
        self.inside = False

    def setDistance(self, distance):
        self.distance = distance

    def isTooFar(self):
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
    
    def getBytes(self, height = None):
        angle_byte = self.getAngleAsByte()
        distance_byte = self.getDistanceAsByte()

        if height == None:
            if self.airspace != None:
                height = self.airspace.getMax()
            else:
                height = 0

        if height > 65535:
            height = 65535
            
        bytes = height.to_bytes(2, 'little', signed=False)

        result = [bytes[0], bytes[1], angle_byte, distance_byte]
        return result


#!/usr/bin/env python

import math
import serial


from datetime import datetime
from time import time


class GPS_Spoof(object):
    
    def __init__(self, port):
        self.altitiude = 200
        self.heading = 0
        self.speed = 0
        self.last_point = None

        try:
            self.port = serial.Serial(port, 9600)
        except serial.serialutil.SerialException as se:
            print (se)
            print ("No GPS spoofing to SkyDrop...")
            self.port = None

    def send_point(self, latitude, longitude, alt):
        utc = datetime.utcnow()
        time = utc.strftime("%H%M%S")
        date = utc.strftime("%d%m%y")
        
        lat_dec = (latitude - int(latitude)) * 60
        lat_str = "%02d%07.4f" % (abs(int(latitude)), abs(lat_dec))
        lat_c = "N" if latitude > 0 else "S"
        
        lon_dec = (longitude - int(longitude)) * 60
        lon_str = "%03d%07.4f" % (abs(int(longitude)), abs(lon_dec))
        lon_c = "E" if longitude > 0 else "W"
        
        if self.last_point:
            olatitude = self.last_point[0]
            olongitude = self.last_point[1]
                    
            R = 6371e3 # metres
            φ1 = math.radians(latitude)
            φ2 = math.radians(olatitude)
            Δφ = math.radians(olatitude - latitude);
            Δλ = math.radians(olongitude - longitude);
            
            a = math.sin(Δφ/2) * math.sin(Δφ/2) + math.cos(φ1) * math.cos(φ2) * math.sin(Δλ/2) * math.sin(Δλ/2)
            c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
            
            d = R * c
            self.speed = 1.94384 * d
            

            #hdg
            dx = longitude - olongitude
            dy = olatitude - latitude
            
            self.heading = math.degrees(math.atan2(dx, dy))
            if self.heading < 0:
                self.heading += 360
        else:
            self.heading = 0
            self.speed = 0
        
        self.last_point = latitude, longitude
        
        s = "GPRMC,%s.000,A,%s,%c,%s,%c,%.1f,%.1f,%s,0.4,E,A" % (time, lat_str, lat_c, lon_str, lon_c, self.speed, self.heading, date)

        cs = 0
        for c in s:
            n = ord(c)
            cs ^= n 
            
        line = bytes("$%s*%02X\r\n" % (s, cs), "utf-8")
        if self.port != None:
            self.port.write(line)

        s = "GPGGA,,,,,,,5,1,%d,,10" % alt
        cs = 0
        for c in s:
            n = ord(c)
            cs ^= n 
            
        line = bytes("$%s*%02X\r\n" % (s, cs), "utf-8")
        if self.port != None:
            self.port.write(line)

        s = "GPGSA,,3"
        cs = 0
        for c in s:
            n = ord(c)
            cs ^= n 
            
        line = bytes("$%s*%02X\r\n" % (s, cs), "utf-8")
        
        if self.port != None:
            self.port.write(line)
 
        
    

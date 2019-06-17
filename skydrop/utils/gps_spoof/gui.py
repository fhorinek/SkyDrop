#!/usr/bin/env python

import math
import pygame
import serial
from pygame.locals import *

from datetime import datetime
from time import time


class GPS_Spoof(object):
    
    black = 0, 0, 0
    
    win_size = (800, 600)
    done = False
    click = False
    last_point = 0
    
    points = []
    point_radius = 2
    point_color = 0, 100, 255
    point_color_used = 200, 200, 200
    point_timer = 0
    point_period = 100
    #point_min_dist = 15
    point_min_time = 0.05
    point_index = 0
    
    last_point = None
    last_time = None   
    
    lat_start = 48.1485965
    lon_start = 17.1077477
    
    lat = 0
    lon = 0
    
    gain = 0.000005

    waypoints = []
    wpt_color = 0, 255, 0
    wpt_radius = 4
    
    need_redraw = True
    
    def add_waypoints(self):
        self.waypoints.append([48.1492265, 17.1082577])
        self.waypoints.append([48.150286, 17.110068])
    
    def main(self, port):
        self.add_waypoints()
        
        pygame.init()
        self.screen = pygame.display.set_mode(self.win_size, RESIZABLE)
        self.clock = pygame.time.Clock()
        
        pygame.font.init()
        self.font = pygame.font.Font(pygame.font.get_default_font(), 20)
        
        self.altitiude = 200
        self.heading = 0
        self.speed = 0
        
        self.port = serial.Serial(port, 9600)
        
        while not self.done:
            self.event()
            self.draw()
            self.clock.tick(30)

    def draw_text(self, text, x, y):
        tmp = self.font.render(text, True, (255, 10, 10))
        self.screen.blit(tmp,  [x, y])
    
                    
    def add_point(self, pos, alt):
#         if len(self.points):
#             last_point = self.points[-1]
#             dist = math.sqrt((last_point[0] - pos[0]) ** 2 + (last_point[1] - pos[1]) ** 2)
#             if dist < self.point_min_dist:
#                 return

        if self.last_time:
            if time() - self.last_time < self.point_min_time:
                return

        self.last_time = time()
        
        a = pos[0], pos[1], alt
        
        self.points.append(a)
        
    def coord_to_point(self, lat, lon):
        y = int((lat - self.lat_start) / self.gain)
        x = int((lon - self.lon_start) / self.gain)       

        return x, y
            
                    
    def send_point(self, point):
        x, y, alt = point
        
        utc = datetime.utcnow()
        time = utc.strftime("%H%M%S")
        date = utc.strftime("%d%m%y")
        
        latitude = self.lat_start + y * self.gain
        longitude = self.lon_start + x * self.gain
        
        self.lat = latitude
        self.lon = longitude
        
        lat_dec = (latitude - int(latitude)) * 60
        lat_str = "%02d%07.4f" % (abs(int(latitude)), lat_dec)
        lat_c = "N" if latitude > 0 else "S"
        
        lon_dec = (longitude - int(longitude)) * 60
        lon_str = "%03d%07.4f" % (abs(int(longitude)), lon_dec)
        lon_c = "E" if longitude > 0 else "W"
        
        if self.last_point:
            ox, oy = self.last_point[0:2]

            olatitude = self.lat_start + oy * self.gain
            olongitude = self.lon_start + ox * self.gain 
                    
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
            dx = x - ox
            dy = oy - y
            
            self.heading = math.degrees(math.atan2(dx, dy))
            if self.heading < 0:
                self.heading += 360
        else:
            self.heading = 0
            self.speed = 0
        
        self.last_point = point
        
        s = "GPRMC,%s.000,A,%s,%c,%s,%c,%.1f,%.1f,%s,0.4,E,A" % (time, lat_str, lat_c, lon_str, lon_c, self.speed, self.heading, date)

        cs = 0
        for c in s:
            n = ord(c)
            cs ^= n 
            
        line = bytes("$%s*%02X\r\n" % (s, cs), "utf-8")
        self.port.write(line)

        s = "GPGGA,,,,,,,5,1,%d,,10" % alt
        cs = 0
        for c in s:
            n = ord(c)
            cs ^= n 
            
        line = bytes("$%s*%02X\r\n" % (s, cs), "utf-8")

        s = "GPGSA,,3"
        cs = 0
        for c in s:
            n = ord(c)
            cs ^= n 
            
        line = bytes("$%s*%02X\r\n" % (s, cs), "utf-8")
        
        self.port.write(line)

    def event(self):
        for e in pygame.event.get():
            
            if e.type == QUIT or (e.type == KEYUP and e.key == K_ESCAPE):
                self.done = True
                break

            elif e.type == MOUSEBUTTONDOWN and e.button == 1:
                self.add_point(e.pos, self.altitiude)
                self.click = True
                self.need_redraw = True
                
            elif e.type == MOUSEBUTTONDOWN and e.button == 3:
                self.points = []
                self.point_timer = 0
                self.point_index = 0   
                self.need_redraw = True             

            elif e.type == MOUSEBUTTONDOWN and e.button == 4:
                self.altitiude += 10
                self.need_redraw = True

            elif e.type == MOUSEBUTTONDOWN and e.button == 5:
                self.altitiude -= 10
                self.need_redraw = True

            elif e.type == MOUSEBUTTONUP and e.button == 1:
                self.click = False
                
            elif e.type == MOUSEMOTION and self.click:
                self.add_point(e.pos, self.altitiude)
                self.need_redraw = True
                
            elif e.type == pygame.VIDEORESIZE:
                self.screen = pygame.display.set_mode((e.w, e.h), RESIZABLE)
                self.need_redraw = True
                
            
          
        if len(self.points) > 0 and pygame.time.get_ticks() > self.point_timer and self.point_index < len(self.points):
            self.point_timer = pygame.time.get_ticks() + self.point_period
            self.send_point(self.points[self.point_index])
            self.point_index += 1
            
            self.need_redraw = True
                    
    def draw(self):
        if not self.need_redraw:
            return
        self.need_redraw = False
        
        self.screen.fill(self.black)
        
        last_point = None
        for index in range(len(self.points)):
            point = self.points[index][0:2]
            color = self.point_color if index > self.point_index else self.point_color_used
            
            pygame.draw.circle(self.screen, color, point, self.point_radius)
            if last_point:
                pygame.draw.line(self.screen, color, last_point, point)
            last_point = point
            
        for wpt in self.waypoints:
            point = self.coord_to_point(*wpt)
            pygame.draw.circle(self.screen, self.wpt_color, point, self.wpt_radius)
            
            
        self.draw_text("Alt: %d" % self.altitiude, 0, 0)
        self.draw_text("Hdg: %d" % self.heading, 0, 20)
        self.draw_text("Spd: %d" % (self.speed * 1.852), 0, 40)
            
        self.draw_text("Lat: %0.7f" % self.lat, 0, 60)
        self.draw_text("Lon: %0.7f" % self.lon, 0, 80)
        
        pygame.display.update()     
        
        
if __name__ == '__main__':
    o = GPS_Spoof()
    o.main("/dev/ttyUSB1")
    
    
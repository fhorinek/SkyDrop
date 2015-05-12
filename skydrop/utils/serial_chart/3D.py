import serial
import pygame
from math import *
import numpy as np

from OpenGL.GL import * #@UnusedWildImport
from OpenGL.GLU import * #@UnusedWildImport
from OpenGL.GLUT import * #@UnusedWildImport

  


def add_line(name, x, index):
    item = {}
    item["name"] = name
    item["data"] = np.zeros(len(x))
    item["index"] = index
    item["axis"] = False
    
    return item

time = range(2000)
y = []
y.append(add_line("qw", time, 0))
y.append(add_line("qx", time, 1))
y.append(add_line("qy", time, 2))
y.append(add_line("qz", time, 3))

y.append(add_line("acc_x", time, 4))
y.append(add_line("acc_y", time, 5))
y.append(add_line("acc_x", time, 6))

y.append(add_line("mag_x", time, 7))
y.append(add_line("mag_y", time, 8))
y.append(add_line("mag_x", time, 9))

y.append(add_line("gyro_x", time, 10))
y.append(add_line("gyro_y", time, 11))
y.append(add_line("gyro_x", time, 12))



index = 0


s = serial.Serial("/dev/ttyUSB0", 921600)

   
pygame.init()
surface = pygame.display.set_mode((640, 480), pygame.OPENGL | pygame.DOUBLEBUF, 16)
glutInit()     

glEnable(GL_DEPTH_TEST)
glDepthFunc(GL_LESS)  
    
    
while True:
    line = s.readline()
    data = line.split(";")
#     print data
    
    for line in y:
        val = 0
        try:
            tmp = data[line["index"]]
            val = float(tmp)
        except:
            print "Err"
        line["data"][index] = val


    '''draw'''
    if index % 4 == 0: #25 fps
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glClearColor(0.5,0.5,0.75,0.0)

        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()       

        q0 = y[0]["data"][index]
        q1 = y[1]["data"][index]
        q2 = y[2]["data"][index]
        q3 = y[3]["data"][index]

        try:
            yaw = atan2(2*q1*q2 - 2*q0*q3, 2*q0*q0 + 2*q1*q1 - 1)
            pitch = -asin(2*q1*q3 + 2*q0*q2)
            roll = atan2(2*q2*q3 - 2*q0*q1, 2*q0*q0 + 2*q3*q3 - 1)
        except:
            print "Math err"
            continue

        yaw *= 180.0 / pi
        pitch *= 180.0 / pi
        roll *= 180.0 / pi

        print "ANG: %02f  %02f  %02f" % (pitch, yaw, roll)
            

        acc_x = y[4]["data"][index]
        acc_y = y[5]["data"][index]
        acc_z = y[6]["data"][index]

        acc_s = sqrt(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z)
        print "ACC: %02f  %02f  %02f  %02f" % (acc_x, acc_y, acc_z, acc_s)
        

        mag_x = y[7]["data"][index]
        mag_y = y[8]["data"][index]
        mag_z = y[9]["data"][index]

        mag_s = sqrt(mag_x * mag_x + mag_y * mag_y + mag_z * mag_z)
        print "MAG: %02f  %02f  %02f  %02f" % (mag_x, mag_y, mag_z, mag_s)

        gyro_x = y[10]["data"][index]
        gyro_y = y[11]["data"][index]
        gyro_z = y[12]["data"][index]

        print "GYR: %02f  %02f  %02f" % (gyro_x, gyro_y, gyro_z)


        print


        yaw += 180
#         roll = index

        glLineWidth(5)    


        glTranslate(-0.5, -0.8, -0.5)
        glRotatef(roll, 0, 0, 1)
        glColor3f(1, 0, 0)
        glBegin(GL_LINES)
        glVertex3f(-0.25, 0, 0)
        glVertex3f(0.25, 0, 0)
        glEnd()      
 
 
        glLoadIdentity() 
        glTranslate(0, -0.8, -0.5)
        glRotatef(yaw, 0, 0, 1)
        glColor3f(0, 1, 0)
        glBegin(GL_LINES)
        glVertex3f(-0.25, 0, 0)
        glVertex3f(0.25, 0, 0)
        glEnd()      
 
        glLoadIdentity() 
        glTranslate(0.5, -0.8, -0.5)
        glRotatef(pitch, 0, 0, 1)
        glColor3f(0, 0, 1)
        glBegin(GL_LINES)
        glVertex3f(-0.25, 0, 0)
        glVertex3f(0.25, 0, 0)
        glEnd()      




        glLoadIdentity() 
        glRotatef(yaw, 0, -1, 0)
        glRotatef(roll, 1, 0, 0)
        glRotatef(pitch, 0, 0, -1)
               
    
        #X
        glColor3f(1, 0, 0)
        glBegin(GL_LINES)
        glVertex3f(0, 0, 0)
        glVertex3f(1, 0, 0)
        glEnd()        
    
        glColor3f(0, 1, 0)
        glBegin(GL_LINES)
        glVertex3f(0, 0, 0)
        glVertex3f(0, 1, 0)
        glEnd()        
          
        glColor3f(0, 0, 1)
        glBegin(GL_LINES)
        glVertex3f(0, 0, 0)
        glVertex3f(0, 0, 1)
        glEnd()   
        
#         glRotatef(90, 0, 0, 0);
        
        glColor3f(1, 1, 1)
        glutSolidTeapot(0.3)  
    
        glLineWidth(1)  
        glColor3f(0, 0, 0)
        glutWireTeapot(0.31)  


        
        
        pygame.display.flip()
        
        
    index = (index + 1) % len(time)
    

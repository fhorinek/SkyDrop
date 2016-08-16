#!/usr/bin/env python

import threading
import serial
import struct
import cProfile

def calc_crc(sum, key, data):
    for i in range(0, 8):
        if ((data & 0x01)^(sum & 0x01)):
            sum = (sum >> 1) % 0x100 
            sum = (sum ^ key) % 0x100
        else:
            sum = (sum >> 1) % 0x100
        data = (data >> 1) % 0x100
        
    #print "crc -> %02X" % sum
    return sum
            

class SerialStream(threading.Thread):
    
    def __init__(self, cqin, cqout):
        threading.Thread.__init__(self)
        
        self.handle = False
        self.cqin = cqin
        self.cqout = cqout
        self.status = False
        
    def open(self, filename, speed):
        try:
            self.handle = serial.Serial(filename, speed, timeout = 0)
        except:
            self.cqout.Write("message")
            self.cqout.Write("Unable to open " + filename)
            return False
        else:
            return True
            
    def close(self):
        self.handle.close()
        self.handle = False
        self.status = "Closed"
            
    def write(self, data):
        crc_key = 0xD5
        
        if (len(data) == 0):
            return
        
        to_send = ""
        
        to_send += chr(0xC0)
        to_send += chr((len(data) & 0x00FF) >> 0)
        crc = calc_crc(0x00, crc_key, (len(data) & 0x00FF) >> 0)
        to_send += chr((len(data) & 0xFF00) >> 8)
        crc = calc_crc(crc, crc_key, (len(data) & 0xFF00) >> 8)

        to_send += chr(crc)

        for byte in data:
            to_send += chr(byte)
            crc = calc_crc(crc, crc_key, byte)
        to_send += chr(crc)
        
#         print "data len", len(to_send)
        self.handle.write(to_send)      
        
            
    def run(self):
        cProfile.runctx('self.inf_loop()', globals(), locals())
        
    def inf_loop(self):
        #states
        #0 = waiting for startbyte (0xC0)
        #1 = waiting for length1
        #2 = waiting for length2
        #3 = waiting for length bytes
        #4 = waiting for crc
          
        state = 0
        length1 = 0
        length2 = 0
        to_read = 0
        byte = 0
        bytes = ()
        crc = 0
        crc_key = 0xD5 #CRC-8
        start_byte = 0xC0
        
        if (not self.handle):
            return False
        
        while (True):
            
            if (self.cqin.HaveEvent()):
                command = self.cqin.Read()
                if (command):
                    if (command == "quit"):
                        self.cqout.Write("quit")
                        self.close()
                        break
                    
                    if (command == "send"):
                        to_send = self.cqin.Read()
#                         print "sending data, len", len(to_send)
#                         print to_send
                        self.write(to_send)
            
            bytes_buffer = self.handle.read(size = 1024)
            if (len(bytes_buffer) == 0):
                continue

            for byte in bytes_buffer:
                byte = ord(byte)
                
                if (state == 0):
                    if (byte == start_byte):
    #                     print "Got head"
                        state += 1
                    continue
                    
                if (state == 1):
                    length1 = byte
                    state += 1
                    crc = calc_crc(0x00, crc_key, byte)
                    continue
                
                if (state == 2):
                    length2 = byte
    
                    state += 1
                    to_read = length1 | (length2 << 8)
                    bytes = []
                    crc = calc_crc(crc, crc_key, byte)
                    continue
                
                if (state == 3):
                    if crc == byte:
                        state += 1
                    else:
                        state = 0
                    continue                
                
                if (state == 4):
                    bytes.append(byte)
                    crc = calc_crc(crc, crc_key, byte)
                    to_read -= 1;
                    if (to_read == 0):
                        state += 1
                    continue
                
                if (state == 5):
                    if (crc == byte):
                        self.cqout.Write("msg", False)
                        self.cqout.Write(bytes)
#                         print "CRC ok %02X" % crc
#                         print bytes
                    else:
                        print "CRC fail %X <> %X" % (crc, byte)
                        print bytes
                                                                       
                    state = 0
        print "Serialstream done"    
                    
                    
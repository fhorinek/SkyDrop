
import threading
import select
import sys
import os
import struct
import time

from glue import CommandQuee
from serial_stream import SerialStream


'''modules'''
import file
import info
import special

S_IDLE = 0
S_PUSH = 1
S_PULL = 2
    
STREAM_OVERHEAD = 7 + 4 + 10
    
FILE_CHUNK_RX = 500 - STREAM_OVERHEAD
FILE_CHUNK_TX = 500 - STREAM_OVERHEAD
    
def int_16(c):
    return int(c, 16)    
    
class Interface(threading.Thread):
    def __init__(self, cin, cout):
        threading.Thread.__init__(self)
        self.cin = cin
        self.cout = cout
        self.state = S_IDLE
        
        self.modules = {0x00: info,
                        0x03: file,
                        0xFF: special}

    def get_modules(self):
        for key in self.modules:
            print "%02X - %s" % (key, self.modules[key].info())

    def get_hint(self, op):
        if op in self.modules:
            self.modules[op].hint()
            
        print
            
    def get_commnad(self, op_hi, op_lo):
        data = False

        if op_hi in self.modules:
            data = self.modules[op_hi].command(op_lo)
            
        if data == False:
            line = raw_input("data:")
            data = map(int_16, line.split())
            
        data = [op_hi, op_lo] + data
        
        #print "TX data", data
        
        self.cout.Write("send", False)
        self.cout.Write(data)

    def get_meaning(self, op_hi, op_lo, data):
        print
        if op_hi in self.modules:
            self.modules[op_hi].parse(op_lo, data)

    def run(self):
        self.get_modules()
        while(1):

            while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
                try:
                    line = sys.stdin.readline()
                    if line:
                        cmd = map(int_16, line.split())
                        if len(cmd) > 0:
                            op_hi = cmd[0]
                        if len(cmd) > 1:
                            op_lo = cmd[1]
    
                        if len(cmd) == 1:
                            self.get_hint(op_hi)
                            op_lo = int(raw_input("op lo:"), 16)
                
                        self.get_commnad(op_hi, op_lo)
                             
                    else: # an empty line means stdin has been closed
                        print 'Connection closed'
                        self.cout.Write("quit")
                        return
                except:
                    print
                    print "Input error"
                    print "Enter high op code to begin in hex"
                    self.get_modules()
            else:
                if self.cin.HaveEvent():
                    type = self.cin.Read()
                    if type == "msg": 
                        data = self.cin.Read()
                        #print "RX data", data
                        
                        op_hi = data[0]
                        op_lo = data[1]
                        print "op_high %02X" % op_hi
                        print "op_low  %02X" % op_lo
                        
                        if op_lo == 0xFF:
                            print "Unknown command"
                            print "Enter high op code to begin in hex"
                            self.get_modules()                            
                            continue
                        
                        data = data[2:]
                        print data
                        
                        self.get_meaning(op_hi, op_lo, data)

   
if __name__ == "__main__":
    from_ui = CommandQuee()
    to_ui = CommandQuee()

    stream = SerialStream(from_ui, to_ui)
    stream.open("/dev/rfcomm0", 115200)
    stream.start()
    
    ui = Interface(to_ui, from_ui)
    ui.start()
    

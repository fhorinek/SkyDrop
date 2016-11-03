#!/usr/bin/python

import bluepy.btle as ble
import struct
import sys

class Delegate(ble.DefaultDelegate):
    def __init__(self):
        ble.DefaultDelegate.__init__(self)
        self.buffer = ""

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print " discovered device", dev.addr
        elif isNewData:
            print " deceived new data from", dev.addr   
    
    def handleNotification(self, cHandle, data):
        print "len %03d <<%s>>" % (len(data), data)
        self.buffer += data
        parts = self.buffer.split("\r\n")
        
        i = 0
        for line in parts:
            if i == len(parts) - 1:
                self.buffer = line
            else:
                print "%3i: %s" % (i, line)
            i += 1
            
        ble.DefaultDelegate.handleNotification(self, cHandle, data)

# adr = sys.argv[1]
adr = "4a:37:61:f0:e6:0e"

print "Connecting to %s" % adr
p = ble.Peripheral(adr, ble.ADDR_TYPE_RANDOM).withDelegate(Delegate())

if False:
    print
    print "Getting services"
    services = p.getServices()
    for ser in services:
        common = ser.uuid.getCommonName()
        if (common == str(ser.uuid)):
            common = ""
        print " %s %s" % (ser.uuid, common)
        
        chars = ser.getCharacteristics()
        for char in chars:
            hnd = char.getHandle()
            print "  ", char.uuid, char.uuid.getCommonName()
            print "   handle %04X" % hnd
            print "   properties", char.propertiesToString()  
            if char.supportsRead():
                val = char.read()  
                txt = ""
                for c in val:
                    txt += "%02X " % ord(c)
                txt = "(" + txt[:-1] + ")"
                print "   value", val, txt
            print
            
        print 


# print ">> %04X" % struct.unpack('<h', p.readCharacteristic(0x0212))
# print p.getCharacteristics(uuid=0x2902)
#p.writeCharacteristic(0x0211, "test", withResponse=True)
p.writeCharacteristic(0x0212, struct.pack('<h', 0x001), withResponse=True)
print ">> %04X" % struct.unpack('<h', p.readCharacteristic(0x0212))

    
while True:
    p.waitForNotifications(1.0)
    continue
    
    

    
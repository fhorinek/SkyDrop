#!/usr/bin/python

import bluepy.btle as ble

class Delegate(ble.DefaultDelegate):
    def __init__(self):
        ble.DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print " discovered device", dev.addr
        elif isNewData:
            print " deceived new data from", dev.addr   

print "Starting BLE scan"

a = ble.Scanner().withDelegate(Delegate())
devices = a.scan(10)

print

for dev in devices:
    print "Device %s (%s), RSSI=%d dB" % (dev.addr, dev.addrType, dev.rssi)
    for (adtype, desc, value) in dev.getScanData():
        print " [%02X] %s = %s" % (adtype, desc, value)
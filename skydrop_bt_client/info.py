def info():
    return "Device info module"

def hint():
    print "00 - Get device string" 
    print "01 - Get device ID" 
    print "02 - Get hardware version" 
    print "03 - Get software version" 

def command(op):
    data = False
    
    if op == 0x00 or op == 0x01 or op == 0x02 or op == 0x03:
        data = []
        
    return data

def parse(op, data):
    if op == 0x80:
        slen = data[0]
        name = "".join(map(chr, data[1:1 + slen]))
        print "Device string: '%s'" % name
        
    if op == 0x81:
        slen = data[0]
        name = "".join(map(chr, data[1:1 + slen]))
        print "Device ID: '%s'" % name
        
    if op == 0x82:
        slen = data[0]
        name = "".join(map(chr, data[1:1 + slen]))
        print "HW version: '%s'" % name
        
    if op == 0x83:
        slen = data[0]
        name = "".join(map(chr, data[1:1 + slen]))
        print "SW version: '%s'" % name
                                
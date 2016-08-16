def info():
    return "Device info module"

def hint():
    print "00 - Get device string" 
    print "01 - Get device id" 

def command(op):
    data = False
    
    if op == 0x00 or op == 0x01:
        data = []
        
    return data

def parse(op, data):
    if op == 0x80:
        slen = data[0]
        name = "".join(map(chr, data[1:1 + slen]))
        print "Device string: '%s'" % name
        
    if op == 0x81:
        slen = data[0]
        print "Device ID:",  
        for c in data[1:1 + slen]:
            print "%02X" % c,
        print
                                
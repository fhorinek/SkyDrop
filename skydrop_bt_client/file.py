import struct

def info():
    return "File operations module"

def hint():
    print "00 - Check storage availability" 
    print "01 - List dir" 
    print "02 - Open file for writing" 
    print "03 - Open file for reading" 
    print "04 - Close file" 
    print "05 - Delete file or dir" 
    print "06 - Push data to file" 
    print "07 - Pull data from file" 
    print "08 - Create dir" 

def command(op):
    data = False
    
    if op == 0x00 or op == 0x04:
        data = []
    
    if op == 0x01:
        path = raw_input("path to list:")
        data = [len(path)]
        data += map(ord, path)

    if op == 0x02:
        print "open file for writing"
        path = raw_input("remote:")
        data = [len(path)]
        data += map(ord, path)    
        
    if op == 0x03:
        print "open file for reading"
        path = raw_input("remote:")
        data = [len(path)]
        data += map(ord, path)
        
    if op == 0x05:
        path = raw_input("file or dir to remove:")
        data = [len(path)]
        data += map(ord, path)       
        
    if op == 0x08:
        path = raw_input("create dir:")
        data = [len(path)]
        data += map(ord, path)      
        
    return data

def parse(op, data):
    if op == 0x80:
        if data[0]:
            print "Storage is available"
        else:
            print "Storage is not available"
        
    if op == 0x81:
        print "listing dir"
        file_count = data[0] | (data[1] << 8)
        print " files:", file_count
        index = 2
        for i in range(file_count): 
            size, = struct.unpack("<L", "".join(map(chr, data[index:index + 4])))
            slen = data[index + 4]
            name = "".join(map(chr, data[index + 5:index + 5 + slen]))
            if size == 0xFFFFFFFF:
                print " %13s   [dir]" % (name)
            else:
                print " %13s   %ub" % (name, size)
            index += 5 + slen
            
    if op == 0x82:
        if data[0]:
            print "File opened for writing"
        else:
            print "Fail to open file for writing"
            
    if op == 0x83:
        if data[0]:
            print "File opened for reading"
        else:
            print "Fail to open file for reading"      
            
    if op == 0x84:
        if data[0]:
            print "File closed"
        else:
            print "Fail to close file"      
            
    if op == 0x85:
        if data[0]:
            print "File or dir deleted"
        else:
            print "Fail to delete file or dir"

    if op == 0x86:
        if data[0]:
            print "Data pushed sucessfully"
        else:
            print "Fail to push data"
            
    if op == 0x87:
        if data[0]:
            print "Data pulled sucessfully"
            slen = data[1] | (data[2] << 8)
            for i in range(slen):
                print "%02X" % data[3 + i],
            print
        else:
            print "Fail to push data"
          
    if op == 0x88:
        if data[0]:
            print "Directory created sucessfully"
        else:
            print "Fail to create diretory"            
                                

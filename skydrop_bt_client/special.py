def info():
    return "Special command module"
    
def hint():
    print "00 - Get login status" 
    print "01 - Login request" 
    print "02 - Logout request" 
    print "03 - Change password" 

def command(op):
    data = False
    
    if op == 0x00 or op == 0x02:
        data = []
    
    if op == 0x01:
        path = raw_input("password:")
        data = [len(path)]
        data += map(ord, path)

    if op == 0x03:
        path = raw_input("new password:")
        data = [len(path)]
        data += map(ord, path)
        
    return data

def parse(op, data):
    if op == 0x80 or op == 0x81:
        if data[0] == 0:
            print "Not logged in"
        else:
            print "Log in level", data[0]

    if op == 0x82:
        print "Logged out"

    if op == 0x83:
        if data[0]:
            print "Password changed"
        else:
            print "Failed to change password"      
   
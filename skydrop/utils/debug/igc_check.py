
filename = "03-1339.IGC"

f = open(filename, "r")

lines = f.readlines()
f.close()

stamps = []

for line in lines:
    t = line[0]
    
    if t == "B":
        dte = line[1:7]
        h = int(dte[0:2])
        m = int(dte[2:4])
        s = int(dte[4:6])
        stamp = h * 3600 + m * 60 + s
        if stamp in stamps:
            print "Error", line
        else:
            stamps.append(stamp)
            if min(stamps) == stamp and len(stamps) > 1:
                print "Error2" , line    
            
import sys
import struct

if len(sys.argv) > 1:
    infile = sys.argv[1]
else:
    infile = "in.RAW"

if len(sys.argv) > 2:
    outfile = sys.argv[2]
else:
    outfile = "out.csv"

file = open(infile, "rb")

data = file.read()
file.close()

file = open(outfile, "w")

index = 0
start_byte = chr(0xAA)

strings = ["time(ms)", "acc X", "acc Y", "acc Z", "gyro X", "gyro Y", "gyro Z", "mag X", "mag Y", "mag Z", "alt(m)", "lat", "lon"]
file.write(", ".join(strings) + "\n")

while index < len(data):
    if data[index] != start_byte:
        index += 1
    else:
        line = data[index + 1:index + 35]
        index += 35;
        
        values = struct.unpack("<Ihhhhhhhhhfii", line)
        strings = map(str, values)
        
        file.write(", ".join(strings) + "\n")
        
file.close()
        
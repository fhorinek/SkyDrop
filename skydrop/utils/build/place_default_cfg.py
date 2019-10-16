#!/usr/bin/python

import struct

placeholder = struct.pack("I", 0xAABBCCDD) + "*** Default configuration placeholder ***" 

input_fw = open("UPDATE.FW", "rb").read()
input_cfg = open("UPDATE.EE", "rb").read()


index = input_fw.find(placeholder)

print("Firmware length      %6u" % len(input_fw))
print("Configuration length %6u" % len(input_cfg))
print("Placeholder index    0x%04x" % index)

placeholder = placeholder + "\0" * (len(input_cfg) - len(placeholder))

output_fw = input_fw.replace(placeholder, input_cfg)

f = open("UPDATE.FW", "wb")
f.write(output_fw)
f.close()
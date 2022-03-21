#!/usr/bin/env python3

import os
import struct

CRC_KEY = 0x9B

def calc_crc(sum, key, data):
    for i in range(0, 8):
        if ((data & 0x01)^(sum & 0x01)):
            sum = (sum >> 1) % 0x100 
            sum = (sum ^ key) % 0x100
        else:
            sum = (sum >> 1) % 0x100
        data = (data >> 1) % 0x100
    return sum

def get_crc(data):
    crc = 0
    
    for c in data:
        crc = calc_crc(crc, CRC_KEY, c)
        
    return crc





out_file = open("SKYDROP.FW", "wb")

in_ee = open("UPDATE.EE", "rb").read()
in_fw = open("UPDATE.FW", "rb").read()

# struct update_header
# {
#     uint32_t build_number;    4
#     uint32_t file_size;       4
#     uint32_t eeprom_size;     4
#     uint8_t eeprom_crc;       1
#     uint32_t flash_size;      4
#     uint8_t flash_crc;        1
# };

f = open("../utils/build/build_number.txt", "r")
build_number = int(f.readline())
f.close()

eeprom_size = len(in_ee)
flash_size = len(in_fw)
file_size = 18 + eeprom_size + flash_size
eeprom_crc = get_crc(in_ee)
flash_crc = get_crc(in_fw)

header = struct.pack("<LLLBLB", build_number, file_size, eeprom_size, eeprom_crc, flash_size, flash_crc)

out_file.write(header)
out_file.write(in_ee)
out_file.write(in_fw)
out_file.close()

#!/usr/bin/python

import CParser
import json
import os
import shutil
from collections import OrderedDict

files = ["../src/gui/widgets/widgets.h",
         "../src/gui/widgets/layouts/layouts.h",
         "../src/common.h",
         "../src/fc/fc.h",
         "../src/fc/conf.h"]
p = CParser.CParser(files)
p.processAll()

# p.printAll()

sizes = {
    "uint8_t": 1,
    "int8_t": 1,
    "uint16_t": 2,
    "int16_t": 2,
    "uint32_t": 4,
    "int32_t": 4,
    "float": 4,
    "char": 1,
     }

# print sizes

def map_struct(parser, path, map_path):
    global map
    global mem_index
    
    for struct in path:
#         print struct
        struct_name = struct[0]
        struct_type = struct[1][0]
        if len(struct[1]) == 2:
            struct_len = struct[1][1][0]
        else:
            struct_len = 1
        
        if struct_type in sizes:
            var_size = sizes[struct_type]
            
            if struct_type == "char":
                map_name = map_path + "_" + struct_name
                map[map_name] =  [mem_index, struct_len, "char"]
                mem_index += struct_len
                
            else:                
                for i in range(struct_len):
                    if struct_len > 1: 
                        map_name = map_path + "_" + struct_name + "_" + str(i)
                    else:
                        map_name = map_path + "_" + struct_name
                    map[map_name] =  [mem_index, var_size, struct_type]
                    mem_index += var_size
        else:
            for i in range(struct_len):
                next_struct = parser.defs["structs"][struct_type]["members"]
                if struct_len > 1: 
                    map_name = map_path + "_" + struct_name + "_" + str(i)
                else:
                    map_name = map_path + "_" + struct_name
                
                map_struct(parser, next_struct, map_name)
        
        
print "------------------------------"
mem_index = 0
map = OrderedDict()
map_struct(p, p.defs["structs"]["cfg_t"]["members"], "cfg")

# print p.defs["macros"]

# for item in map:
#     name, index, size, var_type = item
#     print "%04X %04d %s %s (%d)" % (index, index, var_type, name, size)

f = open("../utils/build/build_number.txt", "r")
number = int(f.readline())
f.close()    
    

path = "../../skydrop_configurator/app/fw/%08d/" % number

try:
    os.makedirs(path)
except:
    pass

f = open(os.path.join(path, "ee_map.json"), "w")
f.write(json.dumps({"map": map, "macros": p.defs["macros"]}))
f.close()

#firmware image
shutil.copyfile("UPDATE.FW", os.path.join(path, "UPDATE.FW"))
#eeprom image
shutil.copyfile("UPDATE.EE", os.path.join(path, "UPDATE.EE"))
#disassembly
shutil.copyfile("skydrop.lss", os.path.join(path, "skydrop.lss"))

#copy last update file
shutil.copyfile("UPDATE.EE", os.path.join("../../skydrop_configurator/app/", "UPDATE.EE"))
print "done"

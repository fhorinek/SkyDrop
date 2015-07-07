#!/usr/bin/python

import CParser
import json

files = ["../src/gui/widgets/widgets.h",
         "../src/common.h",
         "../src/gui/widgets/widget.h",
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
            for i in range(struct_len):
                if struct_len > 1: 
                    map_name = map_path + "_" + struct_name + "_" + str(i)
                else:
                    map_name = map_path + "_" + struct_name
                map.append([map_name, mem_index, var_size, struct_type])
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
map = []
map_struct(p, p.defs["structs"]["cfg_t"]["members"], "cfg")

# for item in map:
#     name, index, size, var_type = item
#     print "%04X %04d %s %s (%d)" % (index, index, var_type, name, size)

f = open("../utils/build/build_number.txt", "r")
number = int(f.readline())
f.close()    
    

f = open("../../skydrop_configurator/app/ref/ee_maps/ee_%05d.json" % number, "w")
f.write(json.dumps({"build_number": number, "ee_map": map}))
f.close()

print "done"

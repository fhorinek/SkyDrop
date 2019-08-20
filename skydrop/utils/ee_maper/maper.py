#!/usr/bin/python2

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

layout_file = "../src/gui/widgets/layouts/layouts.cpp"

p = CParser.CParser(files)

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

def to_int(s):
    if s[:2] == "0x":
        return int(s[2:], 16)
    if s[:2] == "0b":
        return int(s[2:], 2)
    return int(s)

def parse_layouts(filename, p):
    f = open(filename, "r")
    data = f.readlines()
    f.close()
    
    layouts_files = []
    layouts_map = OrderedDict()
    layouts_macro_name = {}
    layouts_name_map = {}
    
    for name in p.defs["macros"]:
        if name[:7] == "LAYOUT_":
            layouts_macro_name[to_int(p.defs["macros"][name])] = name
            
    for line in data:
        line = line.split()
        if len(line) == 0:
            continue
        
        if line[0] == "#include":
            name = line[1].split('"')[1]
            if name[-4:] == ".inc":
                layouts_files.append(name)
        
        if line[0] == "const": 
            layout_pos = 0

        if line[0] == "(layout_desc": 
            layout_name = line[2].replace("&", "").replace(",", "")
            
            layouts_map[layouts_macro_name[layout_pos]] = {"widgets": [], "number_of_widgets": False, "id": layout_pos}
            layouts_name_map[layout_name] = layouts_macro_name[layout_pos]
            layout_pos += 1
    
       
    base = os.path.dirname(filename)
    
    for filename in layouts_files:
        layout_name = False
        
        f = open(os.path.join(base, filename), "r")
        data = f.readlines()
        f.close()        
        for line in data:
            line = line.split()
            
            if len(line) == 0:
                continue
            
            w = []
            a = "".join(line).split("{")
            if len(a) == 2:
                a = a[1].split("}")
                if len(a) == 2:
                    a = a[0].split(",")
                    w = map(to_int, a)
                    if len(w) == 4:
                        layouts_map[layout_name]["widgets"].append(w)
                    
            n = "".join(line).split(",")
            if len(n) == 2:
                n = to_int(n[0])
                layouts_map[layout_name]["number_of_widgets"] = n

            if len(line) == 5:
                if line[1][:11] == "layout_desc":
                    layout_name = layouts_name_map[line[2]]

    return layouts_map
                
                
            
        

def map_struct(parser, path, map_path):
    global ee_map
    global mem_index
    
    for struct in path:
#         print struct
        struct_name = struct[0]
        struct_type = struct[1][0]
        if len(struct[1]) == 2:
            struct_len = struct[1][1][0]
        else:
            struct_len = 1
        
        arr_type = False
        if struct_type[-4:] == "_arr":
            struct_type = struct_type[:-4]
            arr_type = True
        
        if struct_type in sizes:
            var_size = sizes[struct_type]
            
            if struct_type == "char" or arr_type:
                map_name = map_path + "_" + struct_name
                ee_map[map_name] =  [mem_index, struct_len, struct_type]
                mem_index += struct_len
                
            else:                
                for i in range(struct_len):
                    if struct_len > 1: 
                        map_name = map_path + "_" + struct_name + "_" + str(i)
                    else:
                        map_name = map_path + "_" + struct_name
                    ee_map[map_name] =  [mem_index, var_size, struct_type]
                    mem_index += var_size
        else:
            for i in range(struct_len):
                next_struct = parser.defs["structs"][struct_type]["members"]
                if struct_len > 1: 
                    map_name = map_path + "_" + struct_name + "_" + str(i)
                else:
                    map_name = map_path + "_" + struct_name
                
                map_struct(parser, next_struct, map_name)
        
        
mem_index = 0
ee_map = OrderedDict()

f = open("../utils/build/build_number.txt", "r")
number = int(f.readline())
f.close()   

print "Configuration mapper (Build %u)" % number
print "-----------------------------------------"

print "Parsing headers...",
p.processAll()
print "ok"

print "Parsing cfg...",
map_struct(p, p.defs["structs"]["cfg_t"]["members"], "cfg")
print "ok"

print "Parsing layouts...",
layouts = parse_layouts(layout_file, p)
print "ok"

path = "../../skydrop_configurator/app/fw/%08d/" % number

try:
    os.makedirs(path)
except:
    pass

print "Writing description file...",
f = open(os.path.join(path, "ee_map.json"), "w")
f.write(json.dumps({"map": ee_map, "macros": p.defs["macros"], "layouts": layouts}))
f.close()
print "ok"

print "Providing firmware, configuration and description files to configurator...",
#firmware image
shutil.copyfile("UPDATE.FW", os.path.join(path, "UPDATE.FW"))
#eeprom image
shutil.copyfile("UPDATE.EE", os.path.join(path, "UPDATE.EE"))
#disassembly
shutil.copyfile("skydrop.lss", os.path.join(path, "skydrop.lss"))

#copy last update file
shutil.copyfile("UPDATE.EE", os.path.join("../../skydrop_configurator/app/", "UPDATE.EE"))
print "ok"

print "-----------------------------------------"


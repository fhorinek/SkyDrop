import PIL
import PIL.Image
import PIL.ImageDraw
import PIL.ImageFont

import os.path

        
def u16to8(val):
    b = (val & 0xFF00) >> 8
    a = (val & 0x00FF) >> 0
    return (a, b)

def u8to16(a, b):
    return (b << 8) | a

class FontChar:
    def __init__(self, parent, c, height, force_width = False):
        self.parent = parent
        self.c = c
        self.height = height
        
        self.size = parent.font.getsize(c)
        if force_width is not False:
            self.width = force_width
        else:
            self.width = self.size[0]

        self.im = PIL.Image.new('1', (self.width, self.height), 0)
        self.draw = PIL.ImageDraw.Draw(self.im)

        self.draw.text((0,0), c, font = self.parent.font, fill = 1)
        
        self.im.save("chr.png", "PNG")
        
        box = self.im.getbbox()
        if (box == None):
            box = (0, 0, self.im.size[0], self.im.size[1]) 
        
        if force_width is not False:
            box = (0, 0, force_width, self.height)
        else:
            box = (box[0], 0, box[2], self.height)
            
        self.im = self.im.crop(box)
        self.width = box[2] - box[0]
        print " width", self.width
        
    def copy(self, x, y, im):
        box = (x, y)
        self.x = x
        im.paste(self.im, box)
        return self.width
    
    def GetHeight(self):
        box = self.im.getbbox()
        return box[3] - box[1]
        
    
class FontConvertor:
    def __init__(self, fontname, fontsize, spacing, start, end):
        
        self.fontname = fontname
        self.fontsize = fontsize
        
        height = fontsize * 3
        
        self.font = PIL.ImageFont.truetype(fontname, fontsize)
        self.buffer = PIL.Image.new('1', (2048, height), 0)
        self.draw = PIL.ImageDraw.Draw(self.buffer)
        self.chars = []
        self.width = 0
        self.height = height
        self.spacing = spacing
        self.start = start
        self.end = end
        
    def Generate(self, force_size = {}, move_xy = {}):
        for i in range(self.start, self.end):
            print "generating %d %c" % (i, chr(i))
            c = chr(i)
            if i in force_size:
                item = FontChar(self, c, self.height, force_size[i])
            else:
                item = FontChar(self, c, self.height)
                
            if i in move_xy:
                y = move_xy[i][1]
            else:
                y = 0
                
            self.width += item.copy(self.width, y, self.buffer)
            self.chars.append(item)
            
        #get char A height
        tmp = FontChar(self, 'A', self.height)
        self.A_height = tmp.GetHeight()
            
        self.buffer = self.buffer.crop(self.buffer.getbbox())
        self.height = self.buffer.size[1]

        self.w, self.h = self.buffer.size
        if (self.h % 8 <> 0):
            self.h = ((self.h / 8 + 1) * 8) 
        
        self.im = PIL.Image.new('1', (self.w, self.h), 0)
        self.im.paste(self.buffer, (0, 0))
        self.im.save("generated_%s_%d.png" % (os.path.basename(self.fontname), self.fontsize))
       
        self.n = self.h / 8
        

    def StoreBuffer(self, filename):
        im = PIL.Image.new('1', (self.w + len(self.chars), self.h), 0)
        width = 0
        i = 0
        for char in self.chars:
            width += char.copy(width, im) + 1
            i += 1
            char.x -= i
        im.save(filename, "PNG")
            
        
    def LoadBuffer(self, filename):
        im = PIL.Image.open(filename)
        width = 0
        i = 0
        for char in self.chars:
            box = (char.x, 0)
            self.im.paste(im , box)
            w, h = im.size
            box = (char.width + 1, 0, w, h)
#             print box
            im = im.crop(box)
            i += 1
            
        self.im.save("loaded.png", "PNG")
        
    def Convert(self): 
        print "Converting"
        data = []
        
        #header
        #character height
        data.append(self.height)
        #A height
        data.append(self.A_height)
        #characte spacing
        data.append(self.spacing)
        #character lines
        data.append(self.n)
        #font start
        data.append(self.start)
        #font end
        data.append(self.end - 1)
        
        print "header", len(data)
        
        for char in self.chars:
            data += u16to8(char.x)
        
        data += u16to8(self.chars[-1].x + self.chars[-1].width)

        print "table", len(data)
        
        pixels = list(self.im.getdata())
        width, height = self.im.size
        pixels = [pixels[i * width:(i + 1) * width] for i in xrange(height)]
       
        img_data = []
       
        for x in range(self.w):
            for line in range(self.n):
                y_base = line * 8;
                item = 0
                for y in range(8): 
                    if pixels[y_base + y][x]:
                        a = 1
                    else:
                        a = 0
                    item |= a << y
                    
                img_data.append(item)
         
#         print img_data
         
        self.data = data + img_data      
        print "data size", len(data)

    def Save(self):
        filename = os.path.basename(self.fontname).lower()
        filename = filename.replace(".", "_")
        filename = filename.replace("-", "_")
        filename = "font_%s_%d.h" % (filename, self.fontsize)
        
        print "Generating file", filename
        print
        
        file = open(filename, "w")
        
        file.write("//This file is automatically generated\n\n")
        file.write("// Font name:         %s\n" % self.fontname)
        file.write("// Font size:         %dpt\n" % self.fontsize)
        file.write("// Font height:       %dpx\n" % self.data[0])
        file.write("// A height:          %dpx\n" % self.data[1])
        file.write("// Character spacing: %d\n" % self.data[2])
        file.write("// Bytes per line:    %d\n" % self.data[3])
        file.write("// Character start:   %d\n" % self.data[4])
        file.write("// Character end:     %d\n" % self.data[5])
        file.write("//\n");
        file.write("// data size:         %0.1fkB (%db)\n\n" % (len(self.data) / 1024.0, len(self.data)))
        
        file.write("//Available characters:")
        i = 0
        for c in range(self.data[4], self.data[5]):
            if (i % 5 == 0):
                file.write("\n// ")
            else:
                file.write("    ")

            file.write("%3d 0x%02X = '%c'" % (c, c, chr(c)))
            i += 1
                
        file.write("\n\n")
        
        filename = filename.replace(".h", "")
        
        file.write("const uint8_t %s[] PROGMEM = {\n" % (filename));
        
        cnt = 0
        for byte in self.data:
            cnt += 1
            file.write("0x%02X, " % byte)
            if (cnt == 10):
                cnt = 0
                file.write("\n"); 
        
        file.write("\n};\n\n");

        file.close()

'''Convert font'''

tiny = FontConvertor("source/6px-Normal.ttf", 8, 1, 33, 127)
tiny.Generate()
tiny.Convert()
tiny.Save()
 
values_36 = FontConvertor("source/Arial_Bold.ttf", 36, 1, 42, 59)
values_36.Generate({49: 17, 52: 17}) #number must have fixed width for values
values_36.Convert()
values_36.Save()
 
values_16 = FontConvertor("source/Arial_Bold.ttf", 16, 1, 42, 59)
values_16.Generate({49: 8}) #number must have fixed width for values
values_16.Convert()
values_16.Save()
  
values_10 = FontConvertor("source/Arial_Bold.ttf", 10, 1, 42, 59)
values_10.Generate({49: 5}) #number must have fixed width for values
values_10.Convert()
values_10.Save()

text_L = FontConvertor("source/Arial.ttf", 12, 1, 33, 127)
text_L.Generate({49: 5, 45:3}, {116: [0, 1]})
text_L.Convert()
text_L.Save()

text_M = FontConvertor("source/Arial.ttf", 10, 1, 33, 127)
text_M.Generate({49: 5, 45:2, 52: 5}, {81: [0, -1]})
text_M.Convert()
text_M.Save()

# data = conv.data
# 
# for a in data:
#     for i in range(8):
#         if (a & (1 << (7 - i))):
#             print "X",
#         else:
#             print " ",
#     print    



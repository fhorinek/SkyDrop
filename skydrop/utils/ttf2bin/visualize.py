#!/usr/bin/python3
# 
# This programm takes a source file containing a font as input, and draws
# this font into "drawn.png".
#
# This can be used to check previously converted fonts, how they look, e.g.
#    python3 ./visualize.py ../../src/data/font_6px_normal_ttf_8.h
#
# 2019-01-09, tilmann@bubecks.de
#

import PIL
import PIL.Image
import PIL.ImageDraw
import PIL.ImageFont

import os.path
import sys
        
def u16to8(val):
    b = (val & 0xFF00) >> 8
    a = (val & 0x00FF) >> 0
    return (a, b)

def u8to16(a, b):
    return (b << 8) | a

class FontConvertor:

    def drawFont(self):

        self.buffer = PIL.Image.new('1', (2048, 48), 0)

        text_x = 1
        text_y = 1

        # Header: 76, Skip character address table
        font_adr_start = 7 + (self.end - self.start + 2)
        print(self.font_9bit)

        for ascii in range(self.start, self.end):
                
            c_index = ascii - self.start
            adr = 7 + c_index;
            start = self.data[adr + 0] + (0xFF if c_index >= self.font_9bit else 0)
            width = self.data[adr + 1] + (0xFF if c_index + 1 >= self.font_9bit else 0) - start;

            print ("  drawing %3d 0x%02x = '%c'     start=%3d, width=%d" % (ascii, ascii, chr(ascii), start, width))

            adr = font_adr_start + start * self.font_lines;

            for x in range(width):
                index = adr + x * self.font_lines;
                for n in range(self.font_lines):
                    data = self.data[index + n]
                    for a in range(8):
                        if data & (1 << a):
                            self.buffer.putpixel((text_x, text_y + a + n * 8), 1)

                text_x = text_x + 1

            text_x = text_x + self.font_spacing;

        # auto crop the image:
        imageBox = self.buffer.getbbox()
        self.buffer = self.buffer.crop(imageBox)

    def readSource(self, filename):
        bytes = []

        with open(filename, "r") as ins:
            for line in ins:
                if line.startswith('0x'):
                    numbers = line.split(',')
                    for number in numbers:
                        number = number.strip()
                        if number.startswith("0x"):
                            bytes.append(int(number,0))
                        
        return bytearray(bytes)

    def readFontSource(self, filename):
        self.data = self.readSource(filename)
        # Font height
        self.height = self.data[0]
        #A height
        self.A_height = self.data[1]
        #characte spacing
        self.font_spacing = self.data[2]
        #character lines
        self.font_lines = self.data[3]
        #font start
        self.start = self.data[4]
        #font end
        self.end = self.data[5]
        self.font_9bit = self.data[6]

# Check usage:
if len(sys.argv) != 2:
    print ("usage: visualize.py source-with-font.h")
    print ("  show the embedded font")
    sys.exit(1)

fc = FontConvertor()
fc.readFontSource(sys.argv[1])
fc.drawFont()
fc.buffer.save("%s.png" % sys.argv[1], "PNG")
print ("Font saved to %s.png" % sys.argv[1])
#fc.buffer.show()

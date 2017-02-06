#!/usr/bin/python
#
# Convert an 84x48 image into a bitmap suitable for use on SkyDrop.
# The image must be in an arbitrary format understood by ImageMagick and
# in b/w format. Anything in "white" will be transparent and any other
# color will be "black".
#
# Copyright 2017 by Dr. Tilmann Bubeck <tilmann@bubecks.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import sys
import subprocess

def getPixel(x, y):
    output = subprocess.check_output(["convert", sys.argv[1] + "[1x1+" + str(x) + "+" + str(y) + "]", "txt:"])
    if "#FFFFFF" in output:
        return 0
    else:
        return 1
    
def putPixel(x, y, color):
    index = ((y / 8) * width) + (x % width)
    if color != 0:
        img[index] |= (1 << (y % 8));
    else:
	img[index] &= ~(1 << (y % 8));

# Check usage:
if len(sys.argv) != 3:
    print "usage: convert-image-to-LOGO.py input-file-of-logo output"
    print "  convert the image into a LOGO file."
    sys.exit(1)

# These are the dimensions of the image (and the LCD):
width  = 84
height = 48

# Create a memory array which holds the data and is organized as the LCD memory:
img = []
for i in range(width * height / 8):
    img.append(0)

# Fill memory array
for y in range(height):
    for x in range(width):
        putPixel(x, y, getPixel(x, y))

# Convert into byte array:
img = map(chr, img)
img = "".join(img)

# Write out byte array
f = open(sys.argv[2], "wb");
f.write(img)
f.close()

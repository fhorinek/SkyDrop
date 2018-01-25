#!/usr/bin/python
#
# Convert a LOGO in SkyDrop format into an image file, e.g. a PNG file.
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

import PIL
import PIL.Image

import sys

def getPixel(x, y):
    index = ((y // 8) * width) + (x % width)
    return (img[index] & ( 1 << (y % 8)) > 0)

# Check usage:
if len(sys.argv) != 3:
    print ("usage: convert-LOGO-to-image.py input-file-of-logo output")
    print ("  convert the LOGO to an standard image file.")
    print ("  The output format to use is determined from the filename extension.")
    print ("  e.g. convert-LOGO-to-image.py LOGO mylogo.png")
    sys.exit(1)

# Read image from file
f = open(sys.argv[1], "rb");
img = bytearray(f.read())
f.close()

# These are the dimensions of the image (and the LCD):
width  = 84
height = 48

# Create a white image:
image = PIL.Image.new("1", (width, height), 255)

# Convert image
for y in range(height):
    for x in range(width):
        if getPixel(x, y):
            # Set a black pixel
            image.putpixel((x, y), 0)

# Save image to a file and choose format based on the filename
image.save(sys.argv[2])

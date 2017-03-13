#!/usr/bin/python
#
# Convert a LOGO in SkyDrop format into a PNG file.
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
    index = ((y / 8) * width) + (x % width)
    return (img[index] & ( 1 << (y % 8)) > 0)

# Check usage:
if len(sys.argv) != 3:
    print "usage: convert-LOGO-to-png.py input-file-of-logo output.png"
    print "  convert the LOGO to a PNG image"
    sys.exit(1)

# Read image from file
f = open(sys.argv[1], "rb");
binary = f.read()
f.close()

# convert image into array of integers:
img = map(ord, binary)

# These are the dimensions of the image (and the LCD):
width  = 84
height = 48

# Create a white PNG file:
subprocess.call(["convert", "-size", str(width) + "x" + str(height), "xc:#ffffff", sys.argv[2]])

# Convert image
for y in range(height):
    for x in range(width):
        if getPixel(x, y):
            # Set a black pixel
            subprocess.call(["convert", sys.argv[2], "-fill", "black", "-draw", "color " + str(x) + "," + str(y) + " point", sys.argv[2] ])



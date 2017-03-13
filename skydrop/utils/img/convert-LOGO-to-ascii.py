#!/usr/bin/python
#
# Dump a LOGO in SkyDrop format in human readable form as a ASCII art
# to stdout.
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

def getPixel(x, y):
    index = ((y / 8) * width) + (x % width)
    return (img[index] & ( 1 << (y % 8)) > 0)

# Check usage:
if len(sys.argv) != 2:
    print "usage: convert-LOGO-to-ascii.py input-file-of-logo"
    print "  dump the logo to stdout"
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

# Dump image
for y in range(height):
    for x in range(width):
        if getPixel(x, y) :
            sys.stdout.write('X')
        else:
            sys.stdout.write('.')
    print




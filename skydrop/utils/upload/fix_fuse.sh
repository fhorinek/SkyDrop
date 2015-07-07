#!/bin/bash

avrdude -p x192a3 -P usb -c avrisp2 -e -U fuse1:w:0x00:m -U fuse2:w:0xBF:m -U fuse4:w:0xFF:m -U fuse5:w:0xF8:m -F

echo "done"


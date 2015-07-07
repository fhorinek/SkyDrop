#!/bin/bash

echo " *** PRE BUILD ***"

#refresh any files containing BUILD_NUMBER
rm src/common.o
rm src/gui/usb.o
rm src/gui/splash.o

#increment build number
../utils/build/inc_build_number.py

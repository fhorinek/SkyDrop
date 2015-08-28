#!/bin/bash

echo " *** PRE BUILD ***"

#refresh any files containing BUILD_NUMBER
rm src/fc/conf.o
rm src/common.o
rm src/gui/usb.o
rm src/gui/splash.o
rm src/gui/update.o

#increment build number
../utils/build/inc_build_number.py

echo " *** PRE BUILD DONE ***"

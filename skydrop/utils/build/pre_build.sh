#!/bin/bash

echo " *** PRE BUILD ***"

#refresh any files containing BUILD_NUMBER
rm src/common.o
rm src/debug.o

rm src/fc/conf.o
rm src/fc/logger/igc.o
rm src/fc/logger/kml.o

rm src/fc/protocols/skybean/info.o

rm src/gui/usb.o
rm src/gui/splash.o
rm src/gui/update.o

#increment build number
../utils/build/inc_build_number.py

#suffle the key
if [ -f "../../../SkyDrop_vali/skydrop_vali/src/p_key_prep.py" ]; then 
    ../../../SkyDrop_vali/skydrop_vali/src/p_key_prep.py
else
    echo "The validator private key is closed source. Tracklogs will be generated without G-Record."
fi

echo " *** PRE BUILD DONE ***"

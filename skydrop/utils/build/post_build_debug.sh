#!/bin/bash

echo " *** POST BUILD ***"

#create IMAGES
../utils/hex2bin/main.py skydrop.hex UPDATE.FW
../utils/hex2bin/main.py skydrop.eep UPDATE.EE ee

#pack to SKYBEAN.FW
../utils/hex2bin/pack.py

echo " *** POST BUILD DONE ***"

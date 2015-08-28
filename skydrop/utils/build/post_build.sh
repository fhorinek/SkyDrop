#!/bin/bash

echo " *** POST BUILD ***"

#create IMAGES
../utils/hex2bin/main.py skydrop.hex UPDATE.FW
../utils/hex2bin/main.py skydrop.eep UPDATE.EE ee

#create ee_map and copy files to ref
../utils/ee_maper/maper.py

#pack to SKYBEAN.FW
../utils/hex2bin/pack.py

echo " *** POST BUILD DONE ***"

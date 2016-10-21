#!/bin/bash

echo " *** POST BUILD ***"

#create IMAGES
echo " * Creating application image"
../utils/hex2bin/main.py skydrop.hex UPDATE.FW
echo

echo " * Creating configuration image"
../utils/hex2bin/main.py skydrop.eep UPDATE.EE ee
echo

#pack to SKYBEAN.FW
echo " * Packing update files"
../utils/hex2bin/pack.py
echo

#create ee_map and copy files to ref
echo " * Running configuration mapper"
../utils/ee_maper/maper.py
echo

echo " *** POST BUILD DONE ***"

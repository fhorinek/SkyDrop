#!/bin/bash

echo " *** POST BUILD FACTORY TEST ***"
echo

#create IMAGES
echo " * Creating application image"
../utils/hex2bin/main.py skydrop_factory_test.hex UPDATE.FW
echo

echo " * Creating configuration image"
../utils/hex2bin/main.py skydrop_factory_test.eep UPDATE.EE ee
echo

#pack to SKYBEAN.FW
echo " * Packing update files"
../utils/hex2bin/pack.py
echo

echo " *** POST BUILD DONE ***"

#!/bin/bash

echo " *** POST BUILD FACTORY TEST ***"
echo

#create IMAGES
echo " * Creating application image"
../utils/hex2bin/main.py skydrop_factory_test.hex UPDATE.FW
echo

echo " *** POST BUILD DONE ***"

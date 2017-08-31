#!/bin/bash

function size_of()
{
    size=$(avr-size --format=avr --mcu=atxmega192a3u skydrop.elf | grep "$1" | grep -oP '\d+\.\d*\%')
    echo ${size%%.*}
}

echo " *** POST BUILD ***"

echo " * Checking sizes"
data_size=$(size_of "^Data:")
if [ "$data_size" -gt 90 ]; then
    echo "Data size is $data_size %. This is too much."
    exit
else
	echo "Sizes ok"
fi
echo

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

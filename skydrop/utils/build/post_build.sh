#!/bin/bash

echo " *** POST BUILD ***"
echo

echo " * Checking sizes"
sizes=($(avr-size --format=avr --mcu=atxmega192a3u skydrop.elf | grep bytes | tr -s " " | cut -f2 -d " "))

echo -e "Mem\tsize\tlimit"

flash_limit=196608
echo -e "Flash\t${sizes[0]}\t$flash_limit"
if [ "${sizes[0]}" -gt "$flash_limit" ]; then #192kB
    echo "Program size 192kB exceeded!"
    exit
fi

ram_limit=13107
echo -e "RAM\t${sizes[1]}\t$ram_limit"
if [ "${sizes[1]}" -gt "$ram_limit" ]; then #80%
    echo "Data size 80% exceeded!"
    exit
fi

eeprom_limit=2048
echo -e "EEPROM\t${sizes[2]}\t$eeprom_limit"
if [ "${sizes[2]}" -gt "$eeprom_limit" ]; then #2kB
    echo "Eeprom size 2kB exceeded!"
    exit
fi

echo "Size OK"
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

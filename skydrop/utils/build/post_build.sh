#!/bin/bash

AVR="$(dirname $(dirname $(dirname $(readlink -e $0))))/avr/bin/"
if [ ! -d ${AVR} ]; then
    $AVR=""
fi

# print the size in bytes of the given section
function size_sect()
{
    # skydrop.elf  :
    # section                      size      addr
    # .fw_info                       32   8454144
    # .eeprom                       647   8454176
    # .cfg_ro                       391   8455808
    # [...]
    "${AVR}avr-size" --format=sysv skydrop.elf | grep $1 | awk '{ print $2 }'
}

echo " *** POST BUILD ***"
echo

echo " * Checking sizes"

echo -e "Mem\tsize\tlimit"

flash_limit=196608
flash_size=$(( $(size_sect .text) + $(size_sect .data) ))
echo -e "Flash\t$flash_size\t$flash_limit"
if [ "$flash_size" -gt "$flash_limit" ]; then #192kB
    echo "Program size 192kB exceeded!"
    exit
fi

ram_limit=13107
ram_size=$(( $(size_sect .data) + $(size_sect .bss) + $(size_sect .noinit) ))
echo -e "RAM\t$ram_size\t$ram_limit"
if [ "$ram_size" -gt "$ram_limit" ]; then #80%
    echo "Data size 80% exceeded!"
    exit
fi

eeprom_limit=2048
eeprom_size=$(size_sect .eeprom)
echo -e "EEPROM\t$eeprom_size\t$eeprom_limit"
if [ "$eeprom_size" -gt "$eeprom_limit" ]; then #2kB
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

echo " * Injecting default configuration"
../utils/build/place_default_cfg.py
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

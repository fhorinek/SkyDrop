avrdude -p x192a3 -P usb -c avrisp2 -e -V -U application:w:skydrop.hex -U eeprom:w:skydrop.eep

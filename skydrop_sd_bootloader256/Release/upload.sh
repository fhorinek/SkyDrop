#cp skydrop_sd_bootloader.hex upload.hex
tail -n +2 skydrop_sd_bootloader256.hex > upload.hex
avrdude -c dragon_pdi -P usb -p x256a3u -U boot:w:upload.hex -U fuse2:w:0xBF:m -U fuse4:w:0xFF:m -U fuse5:w:0xF7:m -e
#avrdude -c dragon_pdi -P usb -p x256a3u -U boot:w:upload.hex


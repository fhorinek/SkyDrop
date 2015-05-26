SkyDrop
=====

<b>skydrop</b><br>
SkyDrop vario source code<br>
<b>skydrop_sd_bootloader</b><br>
Bootloader inside the devices<br>

External library and code
=====

We are using following libraries:

<b>FatFs</b> - (C)ChaN (http://elm-chan.org/fsw/ff/00index_e.html)<br>
/src/drivers/storage/FatFs<br>
<b>LUFA</b> - (C)Dean Camera (www.lufa-lib.org)<br>
/src/tasks/task_usb/LUFA<br>
<b>Kalman filter</b> - (C)Alistair Dickie (https://github.com/alistairdickie/BlueFlyVario_Android)<br>
/src/fc/kalman.cc<br>



Buid info
=====

Tools we are using:

Eclipse IDE for C/C++ Developers<br>
https://eclipse.org/downloads/packages/eclipse-ide-cc-developers/lunasr2<br>
AVR Eclipse plugin<br>
http://avr-eclipse.sourceforge.net/wiki/index.php/The_AVR_Eclipse_Plugin<br>
PyDev Eclipse plugin<br>
http://pydev.org/<br>
Atmel AVG GCC Toolchain:<br>
http://www.atmel.com/tools/ATMELAVRTOOLCHAINFORLINUX.aspx<br>

Programming
=====

<ul>
<li>Run python script <tt>skydrop/utils/hex2bin/main.py <i>[HEX file]</i> UPDATE.BIN</tt></li>
<li>Copy UPDATE.BIN to root directory on skydrop SD</li>
</ul>


Datasheets
=====

All datasheets needed for development can be found in doc directory




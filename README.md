SkyDrop
=====

<b> !!! If you are looking for updates go to this site (or download files from github as RAW) !!!</b>
http://vps.skybean.eu:8080/repo/updates/

I want to ...
* [... update SkyDrop firmware](http://skybean.eu/page/support)
* [... report bug/suggest feature/ask for help](https://github.com/fhorinek/SkyDrop/issues/new)
* [... see what features are implemented in lastest release](https://github.com/fhorinek/SkyDrop/blob/master/updates/changelog.txt)
* [... look for solution to my problem](https://github.com/fhorinek/SkyDrop/issues?utf8=%E2%9C%93&q=is%3Aissue+label%3Aquestion)
* [... see what features will be implemented in next release](https://github.com/fhorinek/SkyDrop/milestones)
* [... browse issues](https://github.com/fhorinek/SkyDrop/issues)

Folder structure
=====

<b>skydrop</b><br>
SkyDrop vario source code<br>
<b>skydrop_sd_bootloader</b><br>
Bootloader inside the devices<br>
<b>updates</b><br>
Official firmware updates<br>


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
<li>To create firmware update run python script <tt>skydrop/utils/hex2bin/main.py <i>[HEX file]</i> UPDATE.FW</tt></li>
<li>To create eeprom update run python script <tt>skydrop/utils/hex2bin/main.py <i>[EEP file]</i> UPDATE.EE ee</tt></li>
<li>Copy UPDATE.FW and UPDATE.EE to root directory on skydrop SD</li>
</ul>


Datasheets
=====

All datasheets needed for development can be found in doc directory




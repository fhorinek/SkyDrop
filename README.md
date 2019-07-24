SkyDrop
=====

<b> !!! If you are looking for updates go to this site (or download files from github as RAW) !!!</b>
[http://vps.skybean.eu/repo/updates/](http://vps.skybean.eu/repo/updates/)

I want to ...
* [... update SkyDrop firmware](http://skybean.eu/support)
* [... configure SkyDrop via configurator](http://vps.skybean.eu/configurator/)
* [... report bug/suggest feature/ask for help](https://github.com/fhorinek/SkyDrop/issues/new)
* [... see what features are implemented in lastest release](https://github.com/fhorinek/SkyDrop/blob/master/updates/changelog.txt)
* [... look for solution to my problem](https://github.com/fhorinek/SkyDrop/issues?utf8=%E2%9C%93&q=is%3Aissue+label%3Aquestion)
* [... see what features will be implemented in next release](https://github.com/fhorinek/SkyDrop/milestones)
* [... browse issues](https://github.com/fhorinek/SkyDrop/issues)

Bug reporting etiquette
=====
* Search issues before posting new bug report/feature request
* Post Debug.log with the problem (in Debug first Enable debug.log, Clear the log and then recreate the issue)
* Tell us what was the trigger of the bug (opening menu, landing, certain settings)

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

We are standing on shoulders these giants:

<b>SkyDrop</b>

<b>FatFs</b> - (C)ChaN (http://elm-chan.org/fsw/ff/00index_e.html)<br>
/src/drivers/storage/FatFs<br>
<b>LUFA</b> - (C)Dean Camera (www.lufa-lib.org)<br>
/src/tasks/task_usb/LUFA<br>
<b>Kalman filter</b> - (C)Alistair Dickie (https://github.com/alistairdickie/BlueFlyVario_Android)<br>
/src/fc/kalman.cc<br>
<b>Intel HEX file format reader and converter</b> - (C)Alexander Belchenko (bialix@ukr.net)<br>
/utils/hex2bin/intelhex.py<br>
<b>CParser.py</b> - (C)Luke Campagnola<br>
/utils/ee_maper/CParser.py<br>

<b>[Configurator](https://vps.skybean.eu/configurator/#/)</b>

<b>Angular.js (angular, angular-animate, angular-route)</b> - (C)Google (https://angularjs.org/)<br>
app/bower_components/angular, app/bower_components/angular-animate, app/bower_components/angular-route, <br>
<b>jQuery</b> - (C) The jQuery Foundation (https://jquery.com/)<br>
app/bower_components/jquery<br>
<b>UI Bootstrap</b> - (C) Angular-UI team (https://angular-ui.github.io/bootstrap/)<br>
app/bower_components/angular-bootstrap<br>
<b>Bootstrap</b> - (C) Mark Otto, Jacob Thornton, https://github.com/orgs/twbs/people (http://getbootstrap.com/)<br>
app/bower_components/bootstrap<br>
<b>Angular Chart</b> - (C) Jerome Touffe-Blin (http://jtblin.github.io/angular-chart.js/)<br>
app/bower_components/angular-chart.js<br>
<b>Chart.js</b> - (C) Nick Downie (http://www.chartjs.org/)<br>
app/bower_components/chart.js<br>
<b>seiyria-bootstrap-slider, angular-bootstrap-slider</b> - (C) Kyle J. Kemp (https://github.com/seiyria)<br>
app/bower_components/seiyria-bootstrap-slider, app/bower_components/angular-bootstrap-slider<br>
<b>FileSaver.js</b> - (C) Teleborder, Inc (https://github.com/Teleborder)<br>
app/bower_components/filesaver<br>
<b>angular-ui-switch</b> - (C) Kristijan Sedlak (https://github.com/xpepermint)<br>
app/bower_components/angular-ui-switch<br>



Tools info
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

Build info
=====

<ul>
<li>Open & build project in Eclipse IDE (a directory Release should appear)</li>
<li>cd skydrop/Release</li>
<li>Run ../utils/build/pre_build.sh</li>
<li>make</li>
<li>Run ../utils/build/post_build.sh and you can see message  *** POST BUILD DONE ***, if everything went good</li>
<li>Copy UPDATE.FW and UPDATE.EE from Release dir to root directory on skydrop SD</li>
<li><i> --- OR ---</i></li>
<li>Copy SKYDROP.FW from Release dir to root directory on skydrop SD</li>
</ul>


Datasheets
=====

All datasheets needed for development can be found in doc directory

Bluetooth connection
=====

pan1026 clasic SPP
-----
```bash
$ hcitool scan
Scanning ...
	00:13:43:0E:F5:1E	SkyDrop SPP
	
$ sdptool records 00:13:43:0E:F5:1E
Service Name: Serial Port
Service RecHandle: 0x10000
Service Class ID List:
  "Serial Port" (0x1101)
Protocol Descriptor List:
  "L2CAP" (0x0100)
  "RFCOMM" (0x0003)
    Channel: 5

^C
$ sudo rfcomm connect 0 00:13:43:0E:F5:1E 5
Connected /dev/rfcomm0 to 00:13:43:0E:F5:1E on channel 5
Press CTRL-C for hangup
```

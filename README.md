SkyDrop
=====

<b> !!! If you are looking for updates go to this site (or download files from github as RAW) !!!</b>
[http://vps.skybean.eu:8080/repo/updates/](http://vps.skybean.eu:8080/repo/updates/)

I want to ...
* [... update SkyDrop firmware](http://skybean.eu/page/support)
* [... configure SkyDrop via configurator](http://vps.skybean.eu:8080/configurator/#/advanced)
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

<b>SkyDrop</b>

<b>FatFs</b> - (C)ChaN (http://elm-chan.org/fsw/ff/00index_e.html)<br>
/src/drivers/storage/FatFs<br>
<b>LUFA</b> - (C)Dean Camera (www.lufa-lib.org)<br>
/src/tasks/task_usb/LUFA<br>
<b>Kalman filter</b> - (C)Alistair Dickie (https://github.com/alistairdickie/BlueFlyVario_Android)<br>
/src/fc/kalman.cc<br>

<b>[Configurator](http://vps.skybean.eu:8080/configurator/)</b>

<b>Angular.js</b> - (C)Google (https://angularjs.org/)<br>
app/angular<br>
<b>jQuery</b> - (C) The jQuery Foundation (https://jquery.com/)<br>
app/jquery<br>
<b>UI Bootstrap</b> - (C) Angular-UI team (https://angular-ui.github.io/bootstrap/)<br>
app/bootstrap<br>
<b>Chart.js</b> - (C) Nick Downie (http://www.chartjs.org/)<br>
app/app/chart.js<br>
<b>Angular Chart</b> - (C) Jerome Touffe-Blin (http://jtblin.github.io/angular-chart.js/)<br>
app/angular/angular-chart.js<br>


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
<li>Run utils/build/pre_build.sh (use Release as working directory)</li>
<li>Build project using make</li>
<li>Run utils/build/post_build.sh (use Release as working directory)</li>
<li>Copy UPDATE.FW and UPDATE.EE from Release dir to root directory on skydrop SD</li>
<li><i> --- OR ---</i></li>
<li>Copy SKYDROP.FW from Release dir to root directory on skydrop SD</li>
</ul>


Datasheets
=====

All datasheets needed for development can be found in doc directory




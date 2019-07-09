<p style="text-align: center;font-size:36pt; font-weight: bold;" >

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/SkyBean_bean_black.png" style="vertical-align: text-bottom;height:1.5cm;" />SkyDrop user guide
</p>

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/skydrop-render-front.png" style="width:70%;display: block; margin-left: auto;margin-right: auto;" />

<p style="text-align:right;" >
**1701**
</p>

<p style="page-break-after: always;" />

## SkyDrop - combined variometer

**main features**

- **analog/digital variometer** - 0.1m/s accuracy
- **5 altimeters** - 1m accuracy, real time refresh rate
- **GPS recording** - 1Hz sampling
- **bluetooth &amp; USB connectivity**
- **motion &amp; environment sensors**
- **light weight &amp; compact size** - 68g, 98 x 58 x 20 mm

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/drop_controls.png" style="width:100%;" />

**RGB LED** - multicolor signal light

**LCD** - main interface display

**buttons** - user controls

**buzzer** - audio transducer

**USB port** - communication &amp; charge interface

**screws** - used for assembly/disassembly

<p style="page-break-after: always;" />

## buttons

**the middle button is used to confirm selection**

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/button-middle.png" align="left" style="width:2.35cm;" />

<div style="margin-left: 2cm;">

* **press &amp; hold for 1s** to turn on, pull up menu bar, move to upper level in menu, toggle widget value, start/stop flight stopwatch

* **short press** to confirm selection, list adjustable widgets on home screen, turn off device, if menu bar is pulled up

* **press &amp; hold for 5s** to turn device off
</div>

**the left arrow button is used for left move**

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/button-left.png" align="left" style="width:2.35cm;" />

<div style="margin-left: 2cm;">

* **short press** to scroll between home screens to the left, select widget menu if menu bar is pulled up, scroll up in menu, lower value during setting parameter

* **press &amp; hold** to rapid value lowering
</div>

**the right button is used for right move**

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/button-right.png" align="left" style="width:2.35cm;" />

<div style="margin-left: 2cm;">

* **short press** to scroll between home screens to the right, select settings menu if menu bar is pulled up, scroll down in menu, raise value during setting parameter
* **press &amp; hold** to rapid value raising
</div>

## important note - please read

SkyDrop is in silent mode after start-up, so if you want to hear acoustic feedback, please blow/suck shortly to/from buzzer grid. This feature will help you to be polite to others before take-off. Of course, sound is enabled automatically just after take-off. To disable this function, please uncheck **Logger suppress audio** checkbox.

## operation of the SkyDrop variometer

During startup animation, firmware version in bottom left and hardware revision in bottom right corner are shown. The device will start on the last used home screen. You can scroll between home screens by pressing the left or the right arrows. If adjustable/switchable widgets are on current home screen, you can press middle button to scroll between them. If adjustable widget is highlighted, you can change its value by arrow buttons. If switchable widget is highlighted, you can toggle its state by long press of the middle button. To enter widget or settings menu, you need to pull up the menu bar from the bottom of any home screen by long press of the middle button, until menu bar appears. After that, you can enter into widget menu by pressing the left arrow button, or enter into settings menu pressing the right arrow button, or you can turn off the device by short press of the middle button again.

## multi-color light signalization

* **yellow light** - device is flashed with new firmware

* **red light** - device is charging

* **green light** - device is fully charged

* **red flash** - battery is less than 20%

* **yellow flash** - searching for GPS fix

* **green flash** - GPS 3D fix is reached

* **cyan flash** - bluetooth is enabled, but not connected

* **blue flash** - bluetooth is enabled &amp; connected

## widget menu

**Change widgets** selection allow to assign different widget options on current home screen. Use arrow buttons to scroll between fields on the screen, use middle button to select field for widget change, which will be chosen from widget list in next step. Several widgets are adjustable (e.g. altimeter widgets **Alt**) or can enable/disable specific function can be toggled (e.g. **beep** or **weak**).

**You can chose from following widgets:**

**Vario bar** shown graphically value of analog vario, each step represents 1m/s. The vario bar increases from middle to top when vario shows 0 to 3 m/s, then decreases from middle to top when vario shows 3 to 6 m/s. The bar shows sinking the same way, so the total range of vario bar is -6 to 6 m/s.

**Vario** shows value of digital variometer.

**AVG vario** (**Avg** on LCD) shows averaged digital variometer during time period set in **Average vario integration int.**

**Vario history** shown graphically relative vario value during time period set in **Average vario integration int.**

**Altitude 1 to 5** (**Alt** on LCD) shows altimeter (1 - 5) value. This widget can be adjusted.

**Height AGL** shows the height above the ground. This needs HAGL data files stored in SkyDrop. Please follow https://github.com/fhorinek/SkyDrop/wiki/HAGL-data-files for instructions on how to put them onto device.

**Ground Level** shows the height above sea level of the ground at the current position. Taken from HAGL data.

**Airspace Distance** (**AS dist** on LCD) shows the distance to the closest airspace border from your current position. If the pilot is inside the airspace, it has to move this distance to leave the airspace. If he is outside the airspace, then this is the remaining distance before entering the airspace.

**Airspace Arrow** (**AS near** on LCD) shows the angle to the closest airspace border from your current position. If the pilot is inside the airspace, it has to follow the arrow to leave the airspace. If he is outside the airspace, then this is the arrow pointing to the airspace. The pilot has **Airspace distance** until reaching the airspace.

**Airspace Name** (**AS Name** on LCD) shows the name of the closest airspace.

**Airspace Info** (**AS Info** on LCD) shows the lowest and highest value of the airspace at the current position. The values are shown in imperial or meter, depending on the setting of Altimeter 1. If the value is followed by "A", it is meant as "AGL". Followed by "M" means "main sea level".

**Airspace Limits** (**AS Limit** on LCD) shows the lowest and highest height allowed to be used at the current position. The values are shown in imperial or meter, depending on the setting of Altimeter 1.

**Ground speed** (**GSpd** on LCD) shows ground speed obtained from GPS receiver.

**Glide ratio** (**Glide** on LCD) shows ratio between actual ground speed and sinking rate.

**GPS heading** (**GHdg** on LCD) shows heading obtained from GPS receiver.

**GPS HDG arrow** shows graphically heading obtained from GPS receiver - north is up.

**GPS HDG points** shows the cardinal direction of the pilots movement as 'N', 'E', 'W', ...

**GPS position** (**GPos** on LCD) shows GPS coordinates obtained from GPS receiver.

**Thermal time** (**TTime** on LCD) shows the time since the pilot started circling.

**Thermal gain** (**TGain** on LCD) shows the meter/feet gained since circling.

**Flight time** (**FTime** on LCD) shows stopwatch of current flight. You can manually start/stop this stopwatch by long press the middle button when highlighted.

**Time** shows actual clock in current time zone.

**Date** shows actual date.

**Odometer** shows the total amount of km/miles flown so far.

**Home Arrow** shows the direction to _HOME_.

**Home Distance** shows the distance in miles/km to _HOME_.

**Home Time** shows the time needed to get to _HOME_. This only shows up, if you fly towards _HOME_.

**Home Info** Some textual information about _HOME_, taken from the home data file.

**Waypoint arrow** shows the direction, where the next waypoint is.

**Waypoint distance** shows the distance in miles/km to the next waypoint.

**Waypoint time** shows the time needed to get to the next waypoint. This only shows up, if you fly towards it.

**Waypoint info** shows the number of waypoints and allows to go to previous or next waypoints.

**Wind direction** shows the direction of the wind in degree: 0 is north, 180 is south...

**Wind arrow** shows the direction of the wind relative to the pilot as an arrow.

**Wind dir points** shows the cardinal direction of the wind as 'N', 'E', 'W', ...

**Wind speed** shows the speed of the wind in km/h or mph.

**Accelerometer** shows acceleration of the pilot averaged through all directions

**Compass heading** shows the heading of the pilot in degree: 0 is north, 180 is south...

**Compass arrow** shows the direction of north

**Compass points** shows the cardinal direction of the pilots as 'N', 'E', 'W', ...

**Battery** shows remaining battery power in %.

**Temperature** (**Temp** on LCD) shows air temperature and humidity.

**Weeklift ON/OFF** (**weak** on LCD) switch on/off attention to light thermal (<1m/s) around.

**Audio ON/OFF** (**beep** on LCD) can be toggled by long press to mute/unmute vario sound.

**empty** leaves current field empty.

**Change layout** entry in widget menu allows to change widget layout on current home screen. You can chose from following options.

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/01.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/02.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/03.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/04.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/05.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/06.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/07.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/08.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/09.gif" style="width:30%;" />
<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/10.gif" style="width:30%;" />

**Pages count** entry defines number of home screens (1 to 5).

You can adjust all screens, layouts and widgets easily through online drag&amp;drop configurator at [vps.skybean.eu/configurator](http://vps.skybean.eu/configurator)

## description of menu functions

The menu is accessible by long pressing the middle button and the select the right menu entry with the right button. Here is a description of the available menu entries.

### Vario

**Lift/Sink threshold** defines the vertical speed value when sound is activated (in 0.1m/s steps).

**Weak lift** enables continuous sound before lift threshold is reached to let pilot know &quot;something is in the air&quot; (in 0.1m/s steps). Frequency will change linearly from low to high inside weak range.

**Fluid audio** allows sound tone change during actual beep.

**Digital vario int.** suppress very fast vario changes and makes displayed digital vario value more stable in 0.1s steps (won&#39;t affect vario bar graph or sound output).

**Average vario int.** defines time used to compute average vario value, used for overall thermal power overview (in 0.1s steps).

**Units** can by metric or imperial (meters or feets per second).

**Use accel** checkbox allow barometer and motion sensors (9D IMU) signal fusion to accelerate vario response to small quick altitude changes.

**Vario demo** can simulate device behavior to achieve desired settings (in 0.1m/s steps). Frequency, length and pause of beep is displayed.

### Altimeters

**Mode** can by absolute related to QNH1 or QNH2, relative to other altimeter (with offset adjustable though home screen widget), or following GPS altitude.

**Zero at takeoff** checkbox only in altimeter relative mode set altimeter value to zero right after takeoff (automatic or manual).

**Units** can by metric or imperial (meters or feets).

**Get from GPS** set altimeter value equal to actual GPS value. Then is works independently from GPS. If in QNH dependent mode, this choice will change also QNH value. In relative mode QNH value stays, just offset is changed. **Note** : Altimeter value is based on atmospheric pressure, which varies during weather changes, therefore altimeter should be calibrated prior to takeoff. Altimeter calibrating can be done by entering the known altitude of location or setting the known QNH value valid for current area or get from stabilized GPS value (GPS altitude is changing slower within fast altitude changes, so it can take some time for stabilization, usually several minutes). If HAGL data is available it is preferred over GPS height value as long as you are on ground.

**Alarms** Allows you to set various alarms, that will be triggered when the entered height is reached. There is a LOW, LOWEST and HIGH alarm. Lower alarms will be raised when the height of the pilot is lower than the entered value, which is typically used by acro pilots. For high alarms the height must be higher, which can be used if you fly inside restricted areas near airports. The reset value is the distance to the alarm limit, where the alarm will stop.

### Logger

**Enabled** checkbox enables or disables GPS tracking.

**Total time**

**Format** of output file can be chosen IGC, KML or RAW.

**Auto start/land** submenu offers **State** can be waiting for takeoff, flying or landed. **Start/Land threshold** in meters defines altitude change needed for takeoff/landing detection for **Timeout** period (in seconds). Long press of middle button when **FTime** widget is highlighted is used to manual start/stop flight time stopwatch. **Suppress audio** checkbox mutes the device when waiting for takeoff, so launch area is not saturated by random mix of annoying sounds generated by many waiting varios.

**Record always** checkbox disables automatic start/stop of logging. Logger will start recording when device is turned on until it is turned off.

**Pilot name, Glider type** and **Glider id** fields are used to be written in recorded track log.

### Flight logs
contains all recorded logs showing basic flight statistics.

### Navigation

There are two different kinds of navigation: HOME and WAYPOINT.

**HOME** Select "Set as Takeoff" to automatically set HOME to the position where you started or load a home file from the SD card. More information here https://github.com/fhorinek/SkyDrop/wiki/Navigation-and-aerodrome-files

**WAYPOINT** You can load a file from SD card which contains a number of waypoints to follow. Use the waypoint widgets to get directions for the next waypoint.

### GPS

**Enable GPS** checkbox enables/disables onboard GPS receiver (significant for battery power).

**Status** shows ratio of fixed/available satellites.

**GPS time** shows actual GMT time obtained from GPS receiver.

**GPS date** shows actual date obtained from GPS receiver.

**Speed units** , used for ground speed widget **GSpd,** can be chosen from - m/s, km/h, mph or knots.

**Format** of coordinates can be chosen from - DD.dddddd, DDº MM.mmm? or DDº MM? SS?.

### Bluetooth

**Bluetooth module** shows used type onboard.

**Enabled** checkbox enables/disables bluetooth function (using this function significant for battery power).

**Protocol** selection changes devices communication between protocols (apps) - DigiFly (XCSoar), LK8EX1 (XCtrack), BlueFly (FlyMe) or SkyBean. **SkyDrop SPP** Bluetooth device must be chosen when pairing with your phone or tablet.

**Forward GPS** checkbox will enable forwarding GPS coordinates to connected device, so there is possibility to turn off its internal GPS receiver to save power.

### Settings

**Time** item in settings menu allow adjust on-board clock.

**Date** item in settings menu allow adjust on-board date.

**Time zone** entry is useful with GMT time obtained from GPS receiver to set correct local time.

**DST** checkbox means daylight saving time option if applicable.

**Sync with GPS** checkbox allows to get accurate time from GPS.

**Display contrast** adjustment tunes LCD screen contrast.

**Display backlight** adjustment controls backlight level (in 20% steps).

**Display backlight timeout** defines time interval after last button press when backlight remains turned on.

**Invert display** checkbox switch dark and empty pixels on LCD.

**Flip orientation** checkbox turn over LCD screen, so SkyDrop can be operated in upside-down position.

**Animation** checkbox enables smooth switching between home screens.

**Vario volume** adjust vario sound level (in 10% steps).

**Vario mute** checkbox switch off beeping of vario. This can be also done by **beep** widget.

**Alert volume** adjustment set sound level of pop-up info messages.

**Menu volume** adjustment set sound levels of menu sounds.

**On/Off sound** checkbox play sound at startup or power off.

**Page sound** checkbox informs which home screen is currently selected by 1 to 5 fast beeps.

**Button sound** checkbox enables beep when any buttons is pressed.

**GPS ready** checkbox play sound when GPS 3D fix is achieved.

**Auto power-off** entry defines time to automatic switch-off of device, if no operation occurred. Auto power-off cannot occur during flight.

**Device id**

**Mass storage** checkbox will switch device from active to PC mode after charger connection (do not use while charging during flight).

**Uart function** can be used for data streaming by UART interface. User can choose from several baud rates - 9600, 19200, 38400, 57600 or 115200. UART is using same protocol as Bluetooth. Debug msg. option runs at baud rate 921600 and is used for development.

**Format SD** function will erase and format inserted micro SD card - recommended if any mass storage connection problem occurs.

### Calibration

**Reset**

**Firmware**

**ADC raw**

**Board rev.**

**Debug log** checkbox activates log records used for debugging and can be checked if technical support ask for it. There are some other service information in debug menu.

**Clear log** This will clear the debug log from previous log messages.

**Debug GPS** All messages received from the GPS receiver will be logged to the debug log.

**WDT last PC**

**Record screen** The device will make a dump of the screen every second to the SD card into the directory "REC". This directory must already exist otherwise no dump will be made. Use https://github.com/fhorinek/SkyDrop/blob/master/skydrop/utils/img/convert-LOGO-to-image.py to convert each dump.

**Acc bias**

**Acc sens**

**Mag bias**

**Mag sens**

**Gyro bias**

## power management

Since SkyDrop is portable device with rechargeable Li-Poly battery onboard, it needs time to time recharge its battery. You can use any micro USB wall charger or another USB host device, even another portable device with active OTG function.

## technical specification

**weight &amp; dimensions** - 68g, 98 x 58 x 20 mm (3.8? x 2.3? x 0.8?)

**pressure sensor** - MS5611-01BA03, 10cm resolution, -1400 to 25000 m altitude (-4600 to 82000 feet)

**GPS receiver** - L80M39 22/68 channel, position \&lt;2.5m CEP, velocity accuracy \&lt;0.1m/s, sensitivity -165dBm

**flight logger memory** - 8 GB flash (58 116 hours of IGC log)

**Bluetooth transceiver** - PAN1026, 2.1 SPP, BLE 4.0

**USB interface** - micro USB 2.0, device only

**battery** - 900 mAh

**buzzer** - magnetic transducer 95 dB @ 10cm

**operation temperature range** - device -40 to 85 ºC (-40 to 185 ºF), battery -40 to 45 ºC (-40 to 113 ºF), LCD -20 to 45 (-4 to 158 ºF)

**power consumption** - depends on activated functions. Several operation modes are listed:

audio mode only: 13h

audio + altitude logger, without GPS: 12h

audio + Bluetooth: 10h

audio + GPS logger: 9h

audio + GPS logger + Bluetooth: 7.5h

## package includes

SkyDrop variometer with strap, raiser mount, USB cable, stickers, user guide.

## device update instructions

Watch video tutorial on SkyBean YouTube channel [youtube.com/c/SkyBeanVarios](http://www.youtube.com/c/SkyBeanVarios) or follow these instructions:

1. Download update file from SkyBean webpage or repository[skybean.eu/repo/updates](http://vps.skybean.eu/repo/updates/) the last folder contains the newest firmware version.
2. Connect SkyDrop (powered off) to the computer via micro USB cable.
3. Wait until USB mode is enabled and device folder will show in computer.
4. Copy file **SKYDROP.FW** to the device. File name must stay exactly the same, so be careful if you download the file multiple times.
5. Eject the device from system toolbar or just unplug from PC.
6. Follow instruction on LCD and confirm update, SkyDrop LED will lid yellow during the update.

Please note, this update procedure will reset all settings to default factory state. If you want to save your specific settings, please update through online configurator at [vps.skybean.eu/configurator](http://vps.skybean.eu/configurator). Alternatively you can find other advanced information or recently solved software issues on our GitHub account [github.com/fhorinek/SkyDrop](https://github.com/fhorinek/SkyDrop)

## important information

By purchasing this device user agrees with terms and conditions of device operation. SkyDrop variometer is not designed for flying under instrument flight rules (IFR) in any case. Manufacturer is not responsible for any accidents or injuries caused by reliance on information provided by SkyDrop variometer. We highly recommend to use both velcro and safety strap when flying with SkyDrop. Security strap is not a regular mount. However we did our best to protect device against destruction during water landing, it is not guaranteed and warranty will void. If device is broken of any reason, do not hesitate and contact us to solve it.

## FAQ / Troubleshooting

If there is something wrong with the device or if you need more detailed explanation of any function, please visit our FAQ section at [skybean.eu/page/skydrop-frequently-asked-questions](http://skybean.eu/page/skydrop-frequently-asked-questions)

If you cannot find the answer, do not hesitate to contact us via email or Facebook.

## contact information

<img src="https://github.com/fhorinek/SkyDrop/raw/master/skydrop/doc/manual/images/qr.gif" align="right" style="width:40%;" />

email:

[info@skybean.eu](mailto:info@skybean.eu)

website:

[skybean.eu](http://skybean.eu/)

facebook page:

[facebook.com/varioskybean](https://www.facebook.com/varioskybean)


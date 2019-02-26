The feature is called "airspace": Show the pilot, if he is inside of
a "forbidden" airspace, which is typically near an airport. Additionally
show him the shortest way to allowed airspace to get out. If he is outside
the forbidden airspace, show him, in which direction the airspace starts
and how far away he is to help him to avoid the forbidden area.

This is especially helpful for pilots flying near airports (like me). 

There could be these widgets:
 1. An arrow pointing away from the forbidden area. Pilots should follow this
    direction.
 2. A distance (in ft and km) to the border of the forbidden area.
 3. A height indicator showing the height, where the forbidden airspace starts.
    It will flash, if the pilot is too high.

Here are two screenshots taken on the device. The first shows a pilot outside
of the CTR and the second inside CTR.

![Outside CTR](https://github.com/bubeck/SkyDrop/raw/airspace/skydrop/utils/airspace/airspace-ok.png)
![Inside CTR](https://github.com/bubeck/SkyDrop/raw/airspace/skydrop/utils/airspace/airspace-forbidden.png)

+++ Implementation +++

The implementation should be fast and compact. To do so, I would propose to 
pre-compute a map for a given area (e.g. germany) outside the skydrop and
store it on the SD card. This map contains a grid with 90m distance
between the points. It is similar to the HAGL files. Each point stores
the distance and direction to the nearest forbidden area. SkyDrop then
reads the file at the currect GPS position to show the information stored
there. This is fast and compact in PROGMEM.

Here is an example of an area and the points in the grid. For each
point you see an arrow pointing to the forbidden area, which will be
stored efficiently inside the file. This file will then be stored on
the SD and used by the code.

![airspace example](https://github.com/bubeck/SkyDrop/raw/airspace/skydrop/utils/airspace/de-stuttgart.png)

++++ Data files ++++

A file called "tile" which holds data for a single 1 degree x 1 degree in GPS.
The naming of that file is identical to the names of the HAGL data but with a ".AIR" extension,
e.g. N46E016.AIR and also stored in "/AIR/" subdirectory. The resolution of a file is
1201 x 1201 points (3 arc/90m resolution) and the organisation is identical to HAGL files.
However, instead of storing a single 16 bit value for HAGL it contains 5 _slices_.

Each slice describes a vertical area where the same airspace data is used:

    uint16_t up;     // This is the upper ceiling in ft of this "slice". "0" means not used.
    uint8_t dir;     // direction in degree / 3, where 0 is north, if bit 7 is set, this indicates inside CTR
    uint8_t dist;    // distance / 64m
            
So in this example, where P1 is the pilot we will have

                                            +--1500ft--
                                            |          
                                            |  Air D
                                            |
                                  +--1000ft-+----------
                                  |
                           P1     |   Air D
                                  |
                   +--500ft-AGL---+--------------------
                   |                
                   |          Airspace C   
      ____ground___|___________________________________
                                                       
                   <-1km-> <-1km->
                           <-----2km-------->


    500     // 500ft (height)
    158     // 30+128=158, 30*3 = 90 degree = east of pilot, 128 = INSIDE
    16      // ~1km distance

    1000    // 1000ft (height)
    30      // 90 degree = east of pilot, currently OUTSIDE
    16      // ~1km distance

    1500    // 1500ft (height)
    30      // 90 degree = east of pilot, currently OUTSIDE
    31      // ~2km distance

    0       // not used
    0
    0

    0       // not used
    0
    0

The size for 1 point is 5 x 4 bytes = 20 bytes.
With 1201 x 1201 points, we have a file size of 28848020 bytes = 27 MB.

+++ PROGMEM size +++

gcc version 7.2.0 (Fedora 7.2.0-1.fc28) 

    Mem     size-before  size    incr    limit
    Flash   186514       188604  +2090   196608
    RAM     11922        12027   +105    13107
    EEPROM  773          773     +0      2048

+++ Map generation +++

Use "convert.py" to generate AIR files for a given region. This
program is used to read a "Open-Airspace-file" containing a number of
airspaces and then computing a raster of positions around these
airspaces.  Each raster point has an angle and distance showing to the
nearest airspace. Open-Airspace files can be downloaded e.g. from http://soaringweb.org/Airspace/

    python3 ./convert.py Germany_CW17_2018.txt

This generates all tiles for germany. A picture of the airspaces based
upon the Open-Airspace files are e.g.

![Open Airspace of Germany](https://github.com/bubeck/SkyDrop/raw/airspace/skydrop/utils/airspace/germany-airspaces.png)

![Open Airspace of Slovakia](https://github.com/bubeck/SkyDrop/raw/airspace/skydrop/utils/airspace/slovakia-airspaces.png)



# How to generate data for airspace

Main problem with the data generation are places when airspaces from different files (countries)
overlap. It is expesive to hold all airspaces in memory.

## Data source

Data used comes from openAIP. Visit openAIP.net and contribute to better aviation data, free for everyone to use and share.
www.openaip.net

## Folder structure

This new method requires pre-computed meta data about the source file. 
 * source - OpenAIP airspace source files
 * lists - list of AIR files that will be generated for each source file
 * lookup - inverted lists directory: list of source files for each AIR file
 * data - generated AIR files for SkyDrop
 * dist - bundled zip files
 * agl_tiles.list - contain list of agl data file, it is used to exclude airpsaces over water

## Prerequisites ##

This is how to install the minimum required packets on a debian system to create airspace files
 * apt-get install -y python3 python3-pip watch && \
 * pip3 install matplotlib shapely lxml 

The data generation now need additional steps, if you want to use multiple files as sources.
This can be skipped and generate datafiles for only single source file, like in **step 3**.
 1. **Create file list for every source file.**

    eg. `./convert.py -l source/openaip_airspace_slovakia_sk.aip` will create list file for this airspace
    lists are required for final airspace packing as border data files will be shared for more than one country
    they are also used for creating lookup lists
    Generaly this method is not very intense on the CPU, you can run it in paralel. See `./create_lists.sh`
 2. **Create lookup data.**

    Use `./create_lookup.py` to invert the lists data
    Thees files will be later used for loading additional airspaces when generating border data
 3. **Generate data file.**

    `./convert.py source/openaip_airspace_slovakia_sk.aip` Will generate data for entered single airspace file.
    The convert script will look for files in lookup directory.
    If the file exist it will be used to check if the AIR file needs to be generated for this airspace 
    (skiping Quickcheck for emptyness) and also if there is a need to load another airspace.
 4. **Bundle airspace data to zip**

    Use `./create_zip.py lists/openaip_airspace_slovakia_sk.aip.list` to create zip file of the data for the airspace 
    in dist directory
 
## Notes

Few notes on how we handle airspaces, since we do not want to edit the files, because of possible updates.

 * Tiles over water will not be generated. Conversion will skip any tile not in `agl_tiles.list` if the file exists.
 * Tiles under latitude -60 will not be generated.
 * Airspaces classes will be changed based on the its name prefix (eg. if airspace named "TMA Bratislava" is class C,
   it will be changed to class TMA)
 * Airspaces classes that have `false` in `CLASS_FILTER` inside `const.py` will be skipped
 * Airspaces without class will go into OTH class
 * Airspaces that starts higher than 32767ft / 9987m (0x7FFF) will be skipped
 * Airspaces that have same top and bottom will be skipped.
 
 
 
 
 
 
 
 

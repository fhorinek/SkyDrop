# Skydrop Firmware

## Build instructions

### avr-toolchain

skydrop uses v7.2 of the avr-gcc toolchain.
Newer toolchains tend to produce bigger binaries that won't fit on skydrops ATMEGA.

On linux you can use the provided script to set up the toolchain

``` bash
cd utils/avr-toolchain
./build.sh
```


### Build the firmware

You can use either eclipse as build system or cmake.

``` bash
mkdir build
cd build
cmake ..
make -j $(nproc)
```

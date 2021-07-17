# Skydrop Firmware

## Build instructions

### avr-toolchain

skydrop uses v7.2 of the avr-gcc toolchain.
Newer toolchains tend to produce bigger binaries that won't fit on skydrops ATMEGA.

On linux you can use the provided script to set up the toolchain

```
cd utils/avr-toolchain
./build.sh
```


### Build the firmware

You can use either eclipse as build system or cmake.

```
mkdir build
cd build
cmake .. -DIGC=OFF
make -j $(nproc)
```

`-DIGC=OFF` disables use of the igc-private-key
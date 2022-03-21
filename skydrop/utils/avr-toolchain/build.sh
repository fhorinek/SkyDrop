#!/bin/bash

#Source http://blog.zakkemble.net/avr-gcc-builds/
# http://www.nongnu.org/avr-libc/user-manual/install_tools.html

# For optimum compile time this should generally be set to the number of CPU cores your machine has
JOBCOUNT=$(nproc)

# Build Linux toolchain
BUILD_LINUX=1

# Build AVR-LibC
BUILD_LIBC=1

# Output locations for built toolchains
# ../../avr
PREFIX_LINUX=$(dirname -- $(dirname -- $(dirname -- "$(realpath -- "$BASH_SOURCE")")))/avr
PREFIX_LIBC=$PREFIX_LINUX

# Install packages
#apt-get install wget make mingw-w64 gcc g++ bzip2

rm -rf linux

NAME_BINUTILS="binutils-2.32"
NAME_GCC="gcc-7.2.0"
NAME_LIBC="avr-libc-2.0.0"


OPTS_BINUTILS="
	--target=avr
	--disable-nls
"

OPTS_GCC="
    --target=avr 
    --enable-languages=c,c++ 
    --disable-nls 
    --disable-libssp 
    --with-system-zlib 
    --enable-version-specific-runtime-libs 
"

OPTS_LIBC=""

TIME_START=$(date +%s)

makeDir()
{
	rm -rf "$1/"
	mkdir -p "$1"
}

fixGCCAVR()
{
	# In GCC 7.1.0 there seems to be an issue with INT8_MAX and some other things being undefined in /gcc/config/avr/avr.c when building for Windows.
	# Adding '#include <stdint.h>' doesn't fix it, but manually defining the values does the trick.

	echo "Fixing missing defines..."

	DEFSFIX="
		#if (defined _WIN32 || defined __CYGWIN__)
		#define INT8_MIN (-128)
		#define INT16_MIN (-32768)
		#define INT8_MAX 127
		#define INT16_MAX 32767
		#define UINT8_MAX 0xff
		#define UINT16_MAX 0xffff
		#endif
	"

	ORIGINAL=$(cat ../gcc/config/avr/avr.c)
	echo "$DEFSFIX" > ../gcc/config/avr/avr.c
	echo "$ORIGINAL" >> ../gcc/config/avr/avr.c
}

echo "Clearing output directories..."
[ $BUILD_LINUX -eq 1 ] && makeDir "$PREFIX_LINUX"
[ $BUILD_LIBC -eq 1 ] && makeDir "$PREFIX_LIBC"

PATH="$PATH":"$PREFIX_LINUX"/bin
export PATH

CC=""
export CC

echo "Downloading sources..."
rm -f $NAME_BINUTILS.tar.xz
rm -rf $NAME_BINUTILS/
wget ftp://ftp.mirrorservice.org/sites/ftp.gnu.org/gnu/binutils/$NAME_BINUTILS.tar.xz
rm -f $NAME_GCC.tar.xz
rm -rf $NAME_GCC/
wget ftp://ftp.mirrorservice.org/sites/sourceware.org/pub/gcc/releases/$NAME_GCC/$NAME_GCC.tar.gz

if [ $BUILD_LIBC -eq 1 ]; then
	rm -f $NAME_LIBC.tar.bz2
	rm -rf $NAME_LIBC/
	wget ftp://ftp.mirrorservice.org/sites/download.savannah.gnu.org/releases/avr-libc/$NAME_LIBC.tar.bz2
fi

confMake()
{
	../configure --prefix=$1 $2 $3 $4
	make -j $JOBCOUNT
	make install-strip
	rm -rf *
}

# Make AVR-Binutils
echo "Making Binutils..."
echo "Extracting..."
tar xf $NAME_BINUTILS.tar.xz
mkdir -p $NAME_BINUTILS/obj-avr
cd $NAME_BINUTILS/obj-avr
[ $BUILD_LINUX -eq 1 ] && confMake "$PREFIX_LINUX" "$OPTS_BINUTILS"
cd ../../

# Make AVR-GCC
echo "Making GCC..."
echo "Extracting..."
tar xzf $NAME_GCC.tar.gz
mkdir -p $NAME_GCC/obj-avr
cd $NAME_GCC
chmod +x ./contrib/download_prerequisites
./contrib/download_prerequisites
cd obj-avr
# fixGCCAVR
[ $BUILD_LINUX -eq 1 ] && confMake "$PREFIX_LINUX" "$OPTS_GCC"
cd ../../

# Make AVR-LibC
if [ $BUILD_LIBC -eq 1 ]; then
	echo "Making AVR-LibC..."
	echo "Extracting..."
	bunzip2 -c $NAME_LIBC.tar.bz2 | tar xf -
	mkdir -p $NAME_LIBC/obj-avr
	cd $NAME_LIBC/obj-avr
	confMake "$PREFIX_LIBC" "$OPTS_LIBC" --host=avr --build=`../config.guess`
	cd ../../
fi

TIME_END=$(date +%s)
TIME_RUN=$(($TIME_END - $TIME_START))

echo ""
echo "Done in $TIME_RUN seconds"

exit 0

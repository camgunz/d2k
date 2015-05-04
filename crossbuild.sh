#!/bin/sh

set -e

BASE_DIR="`pwd`"
CROSSDEPS="${BASE_DIR}/crossdeps"
CROSSLIBS="${CROSSDEPS}/bin"
BUILD_DIR="crossbuild"
BUILD_TYPE="Debug"
DIST_DIR="${CROSSDEPS}/d2k"
WAD_DIR="${BASE_DIR}/data"
WAD_PATH="${WAD_DIR}/d2k.wad"

if [ ! -d "$BUILD_DIR" ]
then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

CPPFLAGS="-I${CROSSDEPS}/include" \
LDFLAGS="-L${CROSSDEPS}/lib" \
SDLDIR="$CROSSDEPS" \
PKG_CONFIG_PATH="${CROSSDEPS}/lib/pkgconfig" \
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
         -DLGI_LIBRARIES="${CROSSDEPS}/lib/lua/5.2/lgi/corelgilua51.dll" \
         -DCMAKE_TOOLCHAIN_FILE=mingw32-toolchain.cmake

clear
make

if [ ! -d "$DIST_DIR" ]
then
    mkdir "$DIST_DIR"
fi

cp d2k.exe \
   libcairo-2.dll \
   libexpat-1.dll \
   libffi-6.dll \
   libfluidsynth.dll \
   libfontconfig-1.dll \
   libfreetype-6.dll \
   libgcc_s_sjlj-1.dll \
   libgio-2.0-0.dll \
   libgirepository-1.0-1.dll \
   libglib-2.0-0.dll \
   libgmodule-2.0-0.dll \
   libgobject-2.0-0.dll \
   libintl-8.dll \
   libpango-1.0-0.dll \
   libpangocairo-1.0-0.dll \
   libpangoft2-1.0-0.dll \
   libpangowin32-1.0-0.dll \
   libpcre-1.dll \
   libpcreposix-0.dll \
   libpixman-1-0.dll \
   libpng16-16.dll \
   libportmidi.dll \
   lua52.dll \
   zlib1.dll
   "$DIST_DIR"

cp -R "$SCRIPTS_DIR" "$FONTS_DIR" "$DIST_DIR"

cd "$BASE_DIR"

./build_wad.sh

if [ -f "$WAD_PATH" ]
then
    cp $WAD_PATH $DIST_DIR
fi


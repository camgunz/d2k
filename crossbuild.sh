#!/bin/sh

set -e

BASE_DIR="`pwd`"
CROSSDEPS="${BASE_DIR}/crossdeps"
CROSSLIBS="${CROSSDEPS}/bin"
BUILD_DIR="crossbuild"
BUILD_TYPE="Release"

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
         -DLUA_LIBRARIES="${CROSSDEPS}/bin/lua52.dll" \
         -DLGI_LIBRARIES="${CROSSDEPS}/lib/lua/5.2/lgi/corelgilua51.dll" \
         -DFFI_INCLUDE_DIR="${CROSSDEPS}/lib/libffi-3.2.1/include" \
         -DCMAKE_TOOLCHAIN_FILE=mingw32-toolchain.cmake

clear

make


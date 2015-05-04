#!/bin/sh

set -e

CROSSDEPS="`pwd`/crossdeps"
CROSSLIBS="$CROSSDEPS/bin"
BUILD_DIR="crossbuild"
BUILD_TYPE="Debug"

if [ ! -d "$BUILD_DIR" ]
then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR" && \
    CPPFLAGS="-I$CROSSDEPS/include" \
    LDFLAGS="-L$CROSSDEPS/lib" \
    SDLDIR="$CROSSDEPS" \
    PKG_CONFIG_PATH="$CROSSDEPS/lib/pkgconfig" \
    cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
             -DLGI_LIBRARIES="$CROSSDEPS/lib/lua/5.2/lgi/corelgilua51.dll" \
             -DCMAKE_TOOLCHAIN_FILE=mingw32-toolchain.cmake && \
    clear && make


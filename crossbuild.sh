#!/bin/sh

CROSSDEPS="`pwd`/crossdeps"
CROSSLIBS="$CROSSDEPS/bin"
BUILD_DIR="crossbuild"

cd $BUILD_DIR && \
    CPPFLAGS="-I$CROSSDEPS/include" \
    LDFLAGS="-L$CROSSDEPS/lib" \
    SDLDIR="$CROSSDEPS" \
    PKG_CONFIG_PATH="$CROSSDEPS/lib/pkgconfig" \
    cmake .. -DCMAKE_BUILD_TYPE=Debug \
             -DCMAKE_TOOLCHAIN_FILE=mingw32-toolchain.cmake && \
    clear && make


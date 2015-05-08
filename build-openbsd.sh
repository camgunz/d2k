#!/bin/sh

BUILD_TYPE=Debug

if [ ! -d cbuild ]
then
  mkdir cbuild
fi

cd cbuild || exit 1

CC=`which clang` \
CXX=`which clang++` \
cmake .. \
    -DPROFILE=1 \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DLUA_LIBRARIES=/usr/local/lib/liblua5.2.so.5.2 \
    -DLUA_INCLUDE_DIR=/usr/local/include/lua-5.2 \
    -DLGI_LIBRARIES=/usr/local/lib/lua/5.2/lgi/corelgilua51.so \
    || exit 1

make


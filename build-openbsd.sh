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
    -DXDIFF_INCLUDE_DIR=~/local/usr/local/include \
    -DXDIFF_LIBRARIES=~/local/usr/local/lib/libxdiff.a \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    || exit 1

make -j 3 || exit 1


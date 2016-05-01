#!/bin/sh

BUILD_TYPE=Release
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
    || exit 1

make || exit 1


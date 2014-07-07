#!/bin/sh

if [ ! -d cbuild ]
then
  mkdir cbuild
fi

cd cbuild && CC=`which clang` CXX=`which clang++` cmake .. -DPROFILE=1 -DCMAKE_BUILD_TYPE=Debug && clear && make -j 3


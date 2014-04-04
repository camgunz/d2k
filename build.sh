#!/bin/sh

if [ ! -d cbuild ]
then
  mkdir cbuild
fi

cd cbuild && CC=`which clang` CXX=`which clang++` cmake -DCMAKE_BUILD_TYPE=Debug .. && clear && make


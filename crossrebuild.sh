#!/bin/sh

BUILD_DIR=crossbuild

rm -rf $BUILD_DIR

if [ ! -d $BUILD_DIR ]
then
  mkdir $BUILD_DIR
fi

./crossbuild.sh


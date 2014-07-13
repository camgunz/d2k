#!/bin/sh

BUILD_FOLDER=xcode

if [ -d $BUILD_FOLDER ]
then
    rm -rf $BUILD_FOLDER
fi

./build-mac.sh


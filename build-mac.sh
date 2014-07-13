#!/bin/sh

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/X11/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/opt/libffi/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/opt/fontconfig/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/opt/freetype/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/opt/pixman/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/opt/libpng/lib/pkgconfig
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/opt/cairo/lib/pkgconfig

BUILD_FOLDER=xcode

if [ ! -d $BUILD_FOLDER ]
then
  mkdir $BUILD_FOLDER
fi

( cd $BUILD_FOLDER && cmake .. -DCMAKE_BUILD_TYPE=Release && make )


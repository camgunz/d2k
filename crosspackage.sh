#!/bin/sh

set -e

BASE_DIR="`pwd`"
SCRIPTS_DIR="${BASE_DIR}/scripts"
FONTS_DIR="${BASE_DIR}/fonts"
CROSSDEPS="${BASE_DIR}/crossdeps"
SYSTEM_LIB_DIR='/usr/i686-w64-mingw32/bin'
LOCAL_LIB_DIR="${CROSSDEPS}/bin"
DIST_DIR="${BASE_DIR}/d2k-win"
D2K_WAD_DIR="${BASE_DIR}/data"
D2K_WAD_PATH="${D2K_WAD_DIR}/d2k.wad"

if [ ! -d "$DIST_DIR" ]
then
    mkdir -p "$DIST_DIR"
fi

cp "${BASE_DIR}/crossbuild/d2k.exe"             \
   "${SYSTEM_LIB_DIR}/libcairo-2.dll"           \
   "${SYSTEM_LIB_DIR}/libexpat-1.dll"           \
   "${SYSTEM_LIB_DIR}/libffi-6.dll"             \
   "${LOCAL_LIB_DIR}/libfluidsynth.dll"         \
   "${SYSTEM_LIB_DIR}/libfontconfig-1.dll"      \
   "${SYSTEM_LIB_DIR}/libfreetype-6.dll"        \
   "${SYSTEM_LIB_DIR}/libgcc_s_sjlj-1.dll"      \
   "${SYSTEM_LIB_DIR}/libgio-2.0-0.dll"         \
   "${LOCAL_LIB_DIR}/libgirepository-1.0.dll"   \
   "${SYSTEM_LIB_DIR}/libglib-2.0-0.dll"        \
   "${SYSTEM_LIB_DIR}/libgmodule-2.0-0.dll"     \
   "${SYSTEM_LIB_DIR}/libgobject-2.0-0.dll"     \
   "${SYSTEM_LIB_DIR}/libintl-8.dll"            \
   "${SYSTEM_LIB_DIR}/libmad-0.dll"             \
   "${SYSTEM_LIB_DIR}/libpango-1.0-0.dll"       \
   "${SYSTEM_LIB_DIR}/libpangocairo-1.0-0.dll"  \
   "${SYSTEM_LIB_DIR}/libpangoft2-1.0-0.dll"    \
   "${SYSTEM_LIB_DIR}/libpangowin32-1.0-0.dll"  \
   "${SYSTEM_LIB_DIR}/libpcre-1.dll"            \
   "${SYSTEM_LIB_DIR}/libpcreposix-0.dll"       \
   "${SYSTEM_LIB_DIR}/libpixman-1-0.dll"        \
   "${SYSTEM_LIB_DIR}/libpng16-16.dll"          \
   "${LOCAL_LIB_DIR}/libportmidi.dll"           \
   "${LOCAL_LIB_DIR}/lua53.dll"                 \
   "${SYSTEM_LIB_DIR}/zlib1.dll"                \
   "$DIST_DIR"

cp -a "${CROSSDEPS}/lib/girepository-1.0" "${DIST_DIR}/typelibs"

cp -R "$SCRIPTS_DIR" "$FONTS_DIR" "$DIST_DIR"

cp "${CROSSDEPS}/usr/local/lib/lua/5.3/lgi/corelgilua51.dll" "$SCRIPTS_DIR"

cd "$BASE_DIR"

if [ ! -f "$D2K_WAD_PATH" ]
then
    ./build_wad.sh
fi

if [ -f "$D2K_WAD_PATH" ]
then
    cp $D2K_WAD_PATH $DIST_DIR
fi


#!/bin/sh

set -e

BASE_DIR=$(pwd)
PACKAGE_DIR="${BASE_DIR}/d2k"
WIN_SKEL_DIR="${BASE_DIR}/d2k-win"
FREEDOOM_URL='http://static.totaltrash.org/freedoom/freedoom_and_freedm-0.9.tar.xz'
FREEDOOM_PACKAGE=$(basename "$FREEDOOM_URL")
FREEDOOM_TARBALL=$(basename "$FREEDOOM_PACKAGE" .xz)
FREEDOOM_FOLDER='freedoom-0.9'
FREEDM_FOLDER='freedm-0.9'

if [ ! -f ${BASE_DIR}/data/d2k.wad ]
then
    ./build_wad.sh
fi

if [ -d "$PACKAGE_DIR" ]
then
    rm -rf "$PACKAGE_DIR"
fi

mkdir "$PACKAGE_DIR"

cp ${BASE_DIR}/crossbuild/d2k.exe "${PACKAGE_DIR}/"

cp ${BASE_DIR}/data/d2k.wad "${PACKAGE_DIR}/"

curl -L -O "$FREEDOOM_URL"
xz -d "$FREEDOOM_PACKAGE"

tar xf "$FREEDOOM_TARBALL"
rm "$FREEDOOM_TARBALL"

cp "${FREEDOOM_FOLDER}/freedoom1.wad" \
   "${FREEDOOM_FOLDER}/freedoom2.wad" \
   "${FREEDM_FOLDER}/freedm.wad" "${PACKAGE_DIR}/"

rm -rf "$FREEDOOM_FOLDER"

rm -rf "$FREEDM_FOLDER"

mkdir -p "${PACKAGE_DIR}/etc/fonts"

cat > "${PACKAGE_DIR}/etc/fonts/fonts.conf" << 'EOF'
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<fontconfig>
  <dir prefix="default">fonts</dir>
  <cachedir>font_cache</cachedir>
</fontconfig>
EOF

cp "${BASE_DIR}/fonts" "${PACKAGE_DIR}/" -R

cp "${BASE_DIR}/crossdeps/lib/lua/5.2/lgi/corelgilua51.dll" \
    "${PACKAGE_DIR}/lgi.dll"

cp "${BASE_DIR}/scripts" "${PACKAGE_DIR}/" -R

cp -a "${BASE_DIR}/crossdeps/share/lua/5.2/"* \
      "${PACKAGE_DIR}/scripts/"

mkdir "${PACKAGE_DIR}/typelibs"

cp "${BASE_DIR}/crossdeps/lib/girepository-1.0/"* \
   "${PACKAGE_DIR}/typelibs/"

cp "${BASE_DIR}/COPYING" "${PACKAGE_DIR}/COPYING"

cp "${BASE_DIR}/crossdeps/bin/SDL.dll"                           \
   "${BASE_DIR}/crossdeps/bin/SDL_image.dll"                     \
   "${BASE_DIR}/crossdeps/bin/SDL_mixer.dll"                     \
   "${BASE_DIR}/crossdeps/bin/libFLAC-8.dll"                     \
   "${BASE_DIR}/crossdeps/bin/libcairo-2.dll"                    \
   "${BASE_DIR}/crossdeps/bin/libcairo-gobject-2.dll"            \
   "${BASE_DIR}/crossdeps/bin/libcairo-script-interpreter-2.dll" \
   "${BASE_DIR}/crossdeps/bin/libdumb.dll"                       \
   "${BASE_DIR}/crossdeps/bin/libexpat-1.dll"                    \
   "${BASE_DIR}/crossdeps/bin/libffi-6.dll"                      \
   "${BASE_DIR}/crossdeps/bin/libfluidsynth.dll"                 \
   "${BASE_DIR}/crossdeps/bin/libfontconfig-1.dll"               \
   "${BASE_DIR}/crossdeps/bin/libfreetype-6.dll"                 \
   "${BASE_DIR}/crossdeps/bin/libgcc_s_sjlj-1.dll"               \
   "${BASE_DIR}/crossdeps/bin/libgio-2.0-0.dll"                  \
   "${BASE_DIR}/crossdeps/bin/libgirepository-1.0-1.dll"         \
   "${BASE_DIR}/crossdeps/bin/libglib-2.0-0.dll"                 \
   "${BASE_DIR}/crossdeps/bin/libgmodule-2.0-0.dll"              \
   "${BASE_DIR}/crossdeps/bin/libgobject-2.0-0.dll"              \
   "${BASE_DIR}/crossdeps/bin/libharfbuzz-0.dll"                 \
   "${BASE_DIR}/crossdeps/bin/libiconv-2.dll"                    \
   "${BASE_DIR}/crossdeps/bin/libintl-8.dll"                     \
   "${BASE_DIR}/crossdeps/bin/libjpeg-62.dll"                    \
   "${BASE_DIR}/crossdeps/bin/liblzma-5.dll"                     \
   "${BASE_DIR}/crossdeps/bin/libmad-0.dll"                      \
   "${BASE_DIR}/crossdeps/bin/libmikmod-3.dll"                   \
   "${BASE_DIR}/crossdeps/bin/libogg-0.dll"                      \
   "${BASE_DIR}/crossdeps/bin/libpango-1.0-0.dll"                \
   "${BASE_DIR}/crossdeps/bin/libpangocairo-1.0-0.dll"           \
   "${BASE_DIR}/crossdeps/bin/libpangoft2-1.0-0.dll"             \
   "${BASE_DIR}/crossdeps/bin/libpangowin32-1.0-0.dll"           \
   "${BASE_DIR}/crossdeps/bin/libpcre-1.dll"                     \
   "${BASE_DIR}/crossdeps/bin/libpcreposix-0.dll"                \
   "${BASE_DIR}/crossdeps/bin/libpixman-1-0.dll"                 \
   "${BASE_DIR}/crossdeps/bin/libpng16-16.dll"                   \
   "${BASE_DIR}/crossdeps/bin/libportmidi.dll"                   \
   "${BASE_DIR}/crossdeps/bin/libssp-0.dll"                      \
   "${BASE_DIR}/crossdeps/bin/libtiff-5.dll"                     \
   "${BASE_DIR}/crossdeps/bin/libvorbis-0.dll"                   \
   "${BASE_DIR}/crossdeps/bin/libvorbisfile-3.dll"               \
   "${BASE_DIR}/crossdeps/bin/libwebp-5.dll"                     \
   "${BASE_DIR}/crossdeps/bin/libwinpthread-1.dll"               \
   "${BASE_DIR}/crossdeps/bin/lua52.dll"                         \
   "${BASE_DIR}/crossdeps/bin/zlib1.dll"                         \
   "${PACKAGE_DIR}/"


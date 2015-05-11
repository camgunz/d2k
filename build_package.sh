#!/bin/sh

set -e

PACKAGE_DIR='d2k'
FREEDOOM_URL='http://static.totaltrash.org/freedoom/freedoom-0.9.tar.xz'
FREEDOOM_PACKAGE=`basename "$FREEDOOM_URL"`
FREEDOOM_TARBALL=`basename "$FREEDOOM_PACKAGE" .xz`
FREEDOOM_FOLDER=`basename "$FREEDOOM_TARBALL" .tar`

if [ -d "$PACKAGE_DIR" ]
then
    rm -rf "$PACKAGE_DIR"
fi

mkdir "$PACKAGE_DIR"

pushd "$PACKAGE_DIR" > /dev/null
cp ../cbuild/d2k .
curl -L -O "$FREEDOOM_URL"
xz -d "$FREEDOOM_PACKAGE"
tar xf "$FREEDOOM_TARBALL"
rm "$FREEDOOM_TARBALL"
cp "${FREEDOOM_FOLDER}/freedoom1.wad" \
   "${FREEDOOM_FOLDER}/freedoom2.wad" \
   "${FREEDOOM_FOLDER}/COPYING" .
rm -rf "$FREEDOOM_FOLDER"
cp ../scripts . -R
cp ../fonts . -R
cp ~/wads/d2k.wad .
popd > /dev/null


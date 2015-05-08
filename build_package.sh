#!/bin/sh

set -e

PACKAGE_DIR='d2k'
FREEDOOM_URL='https://github.com/freedoom/freedoom/releases/download/v0.9/freedoom-0.9.zip'
FREEDOOM_PACKAGE=`basename "$FREEDOOM_URL"`
FREEDOOM_FOLDER=`basename "$FREEDOOM_PACKAGE" .zip`

if [ -d "$PACKAGE_DIR" ]
then
    rm -rf "$PACKAGE_DIR"
fi

mkdir "$PACKAGE_DIR"

pushd "$PACKAGE_DIR" > /dev/null
cp ../cbuild/d2k .
curl -L -O "$FREEDOOM_URL"
unzip -q "$FREEDOOM_PACKAGE"
cp "${FREEDOOM_FOLDER}/freedoom1.wad" \
   "${FREEDOOM_FOLDER}/freedoom2.wad" \
   "${FREEDOOM_FOLDER}/COPYING" .
rm "$FREEDOOM_PACKAGE"
rm -rf "$FREEDOOM_FOLDER"
cp ../scripts . -R
cp ../fonts . -R
cp ~/wads/d2k.wad .
popd > /dev/null


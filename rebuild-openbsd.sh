#!/bin/sh

if [ -d cbuild ]
then
    rm -rf cbuild
fi

./build-openbsd.sh


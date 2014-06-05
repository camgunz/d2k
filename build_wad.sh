#!/bin/sh

if [ ! `which deutex 2> /dev/null` ]
then
    echo 'Error: deutex not found, cannot build WAD'
    exit 1
fi

cd data && deutex -make doom2k.txt doom2k.wad


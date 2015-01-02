#!/bin/sh

# export LIBGL_ALWAYS_SOFTWARE=1
CMD="cbuild/d2k -solo-net -skill 5 -warp 1 -nomonsters -deathmatch -frags 5 -nomouse"

mkdir -p ~/.d2k/scripts
cp scripts/* ~/.d2k/scripts/

# ltrace $CMD 2> ltrace.log
# gdb --args $CMD
gdb -ex run --args $CMD
# CPUPROFILE=cpu.prof $CMD
# cbuild/d2k -skill 4 -warp 1 -file sunder.wad -warp 6
# cbuild/d2k -playdemo DEMO1
# $CMD
# cbuild/d2k -playdemo DEMO1


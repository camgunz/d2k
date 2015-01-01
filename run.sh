#!/bin/sh

# export LIBGL_ALWAYS_SOFTWARE=1
CMD="cbuild/doom2k -solo-net -skill 5 -warp 1 -nomonsters -deathmatch -frags 5 -nomouse"

cp scripts/* ~/.doom2k/scripts/

# ltrace $CMD 2> ltrace.log
# gdb --args $CMD
# gdb -ex run --args $CMD
# CPUPROFILE=cpu.prof $CMD
# cbuild/doom2k -skill 4 -warp 1 -file sunder.wad -warp 6
# cbuild/doom2k -playdemo DEMO1
$CMD
# cbuild/doom2k -playdemo DEMO1


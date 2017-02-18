#!/bin/sh

BASE_DIR="$(pwd)"

WAD=""
WAD="-iwad freedm.wad"
WAD="-file king1.wad"
WAD="-file judas23_.wad"
WAD="-file BOOMEDIT.WAD"
WAD="-file dwango5.wad"
WAD="-file heights.wad"
WAD="-file sunder.wad"

MAP="23"
MAP="2"
MAP="1"

CMD="cbuild/d2k -serve -nodaemon $WAD -skill 5 -warp $MAP -nomonsters"
CMD="cbuild/d2k -serve -nodaemon $WAD -skill 5 -warp $MAP -log /home/charlie/.d2k/log.txt"
CMD="cbuild/d2k -serve -nodaemon $WAD -skill 5 -warp $MAP -nomonsters -altdeath -frags 1 -log ~/.d2k/log.txt"
CMD="cbuild/d2k -serve -nodaemon $WAD -skill 5 -warp $MAP -deathmatch -frags 5"
CMD="cbuild/d2k -serve -nodaemon $WAD -skill 4 -warp $MAP"

mkdir -p ~/.d2k/scripts

cp "${BASE_DIR}/d2k_config_schema.json" ~/.d2k/
cp "${BASE_DIR}/scripts/"* ~/.d2k/scripts/

# gdb -ex 'source gdbserver.txt' --args $CMD
gdb -ex run --args $CMD
# gdb --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
# $CMD


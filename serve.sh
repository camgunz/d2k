#!/bin/sh

BASE_DIR="$(pwd)"

WAD="-file heights2.wad"
WAD=""
WAD="-iwad freedm.wad"
WAD="-file king1.wad"
WAD="-file sunder.wad"
WAD="-file judas23_.wad"
WAD="-file BOOMEDIT.WAD"
WAD="-file dwango5.wad"

MAP="2"
MAP="23"
MAP="1"

CMD="cbuild/d2k $WAD -skill 5 -warp $MAP -nomonsters -serve"
CMD="cbuild/d2k $WAD -skill 5 -warp $MAP -deathmatch -frags 5 -serve"
CMD="cbuild/d2k $WAD -skill 4 -warp $MAP -serve"
CMD="cbuild/d2k $WAD -skill 5 -warp $MAP -log /home/charlie/.d2k/log.txt -serve -nodaemon"
CMD="cbuild/d2k $WAD -skill 5 -warp $MAP -nomonsters -altdeath -frags 1 -log /home/charlie/.d2k/log.txt -serve -nodaemon"
CMD="cbuild/d2k $WAD -skill 5 -warp $MAP3 -nomonsters -altdeath -frags 1 -log /home/charlie/.d2k/log.txt -serve -nodaemon"

mkdir -p ~/.d2k/scripts

cp "${BASE_DIR}/d2k_config_schema.json" ~/.d2k/
cp "${BASE_DIR}/scripts/"* ~/.d2k/scripts/

# gdb -ex 'source gdbserver.txt' --args $CMD
gdb -ex run --args $CMD
# gdb --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
# $CMD


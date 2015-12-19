#!/bin/sh

BASE_DIR="$(pwd)"

CMD="${BASE_DIR}/cbuild/d2k -skill 5 -warp 1 -nomonsters -nomouse"
CMD="${BASE_DIR}/cbuild/d2k -iwad doom2.wad -file dwango5.wad -skill 5 -warp 1 -nomonsters -nomouse"

mkdir -p ~/.d2k/scripts
mkdir -p ~/.d2k/fonts

cp "${BASE_DIR}/scripts/"* ~/.d2k/scripts/
cp "${BASE_DIR}/fonts/"* ~/.d2k/fonts/

# gdb -ex 'source gdbserver.txt' --args $CMD
# gdb -ex run --args $CMD
# gdb --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
$CMD


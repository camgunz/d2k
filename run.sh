#!/bin/sh

BASE_DIR="$(pwd)"

CMD="${BASE_DIR}/cbuild/d2k -iwad doom2.wad -playdemo DEMO3 -nomouse"
CMD="${BASE_DIR}/cbuild/d2k -iwad freedm.wad -skill 5 -warp 1 -nomonsters -nomouse -deathmatch -frags 50 -timer 10"
CMD="${BASE_DIR}/cbuild/d2k -iwad doom2.wad -file gothicdm.wad -playdemo DEMO1 -dumpdemo d2kdump.bin"
CMD="${BASE_DIR}/cbuild/d2k -iwad doom2.wad -file dwango5.wad -skill 5 -warp 1 -nomonsters -nomouse -deathmatch -frags 50 -timer 10"
CMD="${BASE_DIR}/cbuild/d2k -skill 5 -warp 1 -nomonsters -nomouse"
CMD="${BASE_DIR}/cbuild/d2k -iwad freedoom2.wad -file sunder.wad -skill 4 -warp 2 -nomouse"

mkdir -p ~/.d2k/fonts

cp "${BASE_DIR}/d2k_config_schema.json" ~/.d2k/
cp "${BASE_DIR}/fonts/"* ~/.d2k/fonts/

${BASE_DIR}/install_scripts.sh

# gdb -ex 'source gdbserver.txt' --args $CMD
# gdb -ex run --args $CMD
# gdb -ex "source gdbdemo.txt" --args $CMD
# gdb --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
$CMD

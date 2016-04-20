#!/bin/sh

CMD="cbuild/d2k -iwad doom2.wad -file gothicdm.wad -nodraw -nosfx -nomouse -fastdemo DEMO1 -dumpdemo d2kdump.bin"

./build.sh || exit 1
$CMD
# gdb --args $CMD
# gdb -ex run --args $CMD
# gdb -ex "source gdbdemo.txt" --args $CMD

mv d2kdump.bin ../democomp/d2kdump.bin


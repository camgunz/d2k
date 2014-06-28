#!/bin/sh

# gdb -ex run --args cbuild/doom2k -file sunder.wad -warp 6 -nomouse
CPUPROFILE=cpu.prof cbuild/doom2k -file dwango5.wad -nomonsters


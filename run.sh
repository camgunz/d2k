#!/bin/sh

export LIBGL_ALWAYS_SOFTWARE=1

gdb -ex 'source gdbsetup.txt' --args cbuild/doom2k -nomouse -warp 1 -nomonsters
# CPUPROFILE=cpu.prof cbuild/doom2k
# cbuild/doom2k -skill 4 -warp 1 -file sunder.wad -warp 6
# cbuild/doom2k -playdemo DEMO1
# cbuild/doom2k -skill 4 -warp 1
# cbuild/doom2k -playdemo DEMO1


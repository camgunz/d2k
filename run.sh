#!/bin/sh

# gdb -ex run --args cbuild/doom2k -file sunder.wad -warp 6 -nomouse
# CPUPROFILE=cpu.prof cbuild/doom2k
cbuild/doom2k -skill 4 -warp 1 -file sunder.wad -warp 6


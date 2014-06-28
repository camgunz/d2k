WAD="-file dwango5.wad"
WAD=""

gdb -ex run --args cbuild/doom2k $WAD -warp 1 -deltaserve
# CPUPROFILE=cpu.prof cbuild/doom2k $WAD -nomonsters -warp 1 -deltaserve
# valgrind --leak-check=full --show-leak-kinds=all cbuild/doom2k $WAD -nomonsters -warp 1 -deltaserve
# cbuild/doom2k $WAD -nomonsters -warp 1 -deltaserve


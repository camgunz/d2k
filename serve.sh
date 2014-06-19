# gdb -ex run --args cbuild/doom2k -file heights.wad -nomonsters -warp 1 -deltaserve
# CPUPROFILE=cpu.prof cbuild/doom2k -file heights.wad -nomonsters -warp 1 -deltaserve
# valgrind --leak-check=full --show-leak-kinds=all cbuild/doom2k -file heights.wad -nomonsters -warp 1 -deltaserve
cbuild/doom2k -file heights.wad -nomonsters -warp 1 -deltaserve


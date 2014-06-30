WAD=""
WAD="-file dwango5.wad"
CMD="cbuild/doom2k $WAD -nomonsters -warp 1 -deltaserve"

# gdb -ex run --args cbuild/doom2k $WAD -warp 1 -deltaserve
# CPUPROFILE=cpu.prof cbuild/doom2k $WAD -nomonsters -warp 1 -deltaserve
valgrind --leak-check=full --show-leak-kinds=all $CMD
# cbuild/doom2k $WAD -skill 5 -warp 1 -nomonsters -file $WAD -deltaserve


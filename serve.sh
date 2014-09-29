WAD="-file dwango5.wad"
WAD=""
CMD="cbuild/doom2k $WAD -skill 4 -warp 1 -deltaserve"
CMD="cbuild/doom2k $WAD -skill 5 -warp 1 -nomonsters -file $WAD -deathmatch -frags 5 -deltaserve"

# gdb -ex 'source gdbserver.txt' --args $CMD
gdb -ex run --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
# $CMD


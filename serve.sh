WAD="-file heights2.wad"
WAD=""
WAD="-file dwango5.wad"
CMD="cbuild/d2k $WAD -skill 5 -warp 1 -nomonsters -deltaserve"
CMD="cbuild/d2k $WAD -skill 5 -warp 1 -deathmatch -frags 5 -deltaserve"
CMD="cbuild/d2k $WAD -skill 4 -warp 1 -deltaserve"
CMD="cbuild/d2k $WAD -skill 5 -warp 1 -nomonsters -deathmatch -frags 5 -deltaserve"

gdb -ex 'source gdbserver.txt' --args $CMD
# gdb -ex run --args $CMD
# gdb --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
# $CMD


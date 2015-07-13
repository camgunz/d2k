WAD="-file heights2.wad"
WAD=""
WAD="-file dwango5.wad"
CMD="cbuild/d2k $WAD -skill 5 -warp 1 -nomonsters -deltaserve"
CMD="cbuild/d2k $WAD -skill 5 -warp 1 -deathmatch -frags 5 -deltaserve"
CMD="cbuild/d2k $WAD -skill 4 -warp 1 -deltaserve"
CMD="cbuild/d2k $WAD -skill 5 -warp 1 -nomonsters -deathmatch -frags 5 -log /home/charlie/.d2k/log.txt -deltaserve"

cp scripts/* ~/.d2k/scripts/

# gdb -ex 'source gdbserver.txt' --args $CMD
gdb -ex run --args $CMD
# gdb --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
# $CMD


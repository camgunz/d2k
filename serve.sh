WAD="-file dwango5.wad"
WAD=""
CMD="cbuild/doom2k $WAD -skill 5 -warp 1 -nomonsters -file $WAD -deltaserve"
CMD="cbuild/doom2k $WAD -skill 4 -warp 1 -deltaserve"

# gdb -ex run --args $CMD
# CPUPROFILE=cpu.prof $CMD
# valgrind --leak-check=full --show-leak-kinds=all $CMD
$CMD


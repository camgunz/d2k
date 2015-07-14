# export LIBGL_ALWAYS_SOFTWARE=1
export SDL_AUDIODRIVER="pulse"

PORT=10667
PORT=10666
HOST=127.0.0.1

EXE="/home/charlie/code/d2k/cbuild/d2k"
CMD="$EXE -net $HOST:$PORT"
CMD="$EXE -file freedm.wad -nomouse -net $HOST:$PORT"

# gdb -ex "source gdbclient.txt" --args $CMD
gdb -ex run --args $CMD
# gdb --args $CMD
# valgrind --track-origins=yes --leak-check=yes $CMD
# CPUPROFILE=cpu.prof $CMD
# $CMD


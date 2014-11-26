# export LIBGL_ALWAYS_SOFTWARE=1
export SDL_AUDIODRIVER="pulse"

PORT=10666
PORT=10667
HOST=127.0.0.1

EXE="/home/charlie/code/d2k/cbuild/doom2k"
CMD="$EXE -net $HOST:$PORT"
CMD="$EXE -nomouse -net $HOST:$PORT"

# gdb -ex "source gdbclient.txt" --args $CMD
# gdb -ex run --args $CMD
# gdb --args $CMD
# valgrind --track-origins=yes --leak-check=yes $CMD
CPUPROFILE=cpu.prof $CMD
# $CMD


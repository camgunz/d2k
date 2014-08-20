# export LIBGL_ALWAYS_SOFTWARE=1
export SDL_AUDIODRIVER="pulse"

PORT=10666
HOST=127.0.0.1
HOST=totaltrash.org

CMD="cbuild/doom2k -net $HOST:$PORT"

# gdb -ex "source gdbclient.txt" --args $CMD
# gdb --args $CMD
# valgrind --track-origins=yes --leak-check=yes $CMD
# CPUPROFILE=cpu.prof $CMD
$CMD


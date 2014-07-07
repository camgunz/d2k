# export LIBGL_ALWAYS_SOFTWARE=1
export SDL_AUDIODRIVER="pulse"

PORT=10666
HOST=totaltrash.org
HOST=127.0.0.1

gdb -ex run --args cbuild/doom2k -nomouse -net $HOST:$PORT
# valgrind cbuild/doom2k -nomouse -net $HOST:10666
# CPUPROFILE=cpu.prof cbuild/doom2k -nomouse -net $HOST:$PORT
# cbuild/doom2k -nomouse -net $HOST:$PORT


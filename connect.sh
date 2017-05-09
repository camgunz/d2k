#!/bin/sh

# export LIBGL_ALWAYS_SOFTWARE=1
# export SDL_AUDIODRIVER="pulse"

BASE_DIR="$(pwd)"

PORT=10667
PORT=10666
HOST=45.33.60.48
HOST=127.0.0.1
HOST=totaltrash.org
HOST=97.107.132.133

CMD="${BASE_DIR}/cbuild/d2k -nomouse -connect ${HOST}:${PORT}"

mkdir -p ~/.d2k/scripts
mkdir -p ~/.d2k/fonts

cp "${BASE_DIR}/d2k_config_schema.json" ~/.d2k/
cp "${BASE_DIR}/scripts/"* ~/.d2k/scripts/
cp "${BASE_DIR}/fonts/"* ~/.d2k/fonts/

# gdb -ex "source gdb_dupesounds.txt" --args $CMD
gdb -ex run --args $CMD
# valgrind --track-origins=yes --leak-check=yes $CMD
# CPUPROFILE=cpu.prof $CMD
# $CMD


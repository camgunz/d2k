HOST=10.44.36.239
HOST=192.168.1.243
HOST=127.0.0.1

gdb -ex run --args cbuild/doom2k -nomouse -net $HOST:10666

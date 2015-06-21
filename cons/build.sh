#!/bin/sh

set -e

CC=gcc
GIO_LIBS=$(pkg-config --libs gio-unix-2.0)
GIO_CFLAGS=$(pkg-config --cflags gio-unix-2.0)
NCURSES_LIBS=$(pkg-config --libs ncursesw)
NCURSES_CFLAGS=$(pkg-config --cflags ncursesw)
LINEBREAK_LIBS=$(pkg-config --libs libunibreak)
LINEBREAK_CFLAGS=$(pkg-config --cflags libunibreak)
FLAGS='-g'

rm -f uds_server
$CC -o uds_server uds.c uds_server.c $GIO_LIBS $GIO_CFLAGS $FLAGS

rm -f uds_client
$CC -o uds_client uds.c uds_client.c $GIO_LIBS $GIO_CFLAGS $NCURSES_LIBS \
                                     $NCURSES_CFLAGS $LINEBREAK_LIBS \
                                     $LINEBREAK_CFLAGS $FLAGS
                                    


#!/bin/sh

set -e

CC=gcc
LIBS=$(pkg-config --libs gio-unix-2.0)
CFLAGS=$(pkg-config --cflags gio-unix-2.0)
FLAGS='-g'

rm -f uds_server
$CC -o uds_server uds.c uds_server.c $LIBS $CFLAGS $FLAGS

rm -f uds_client
$CC -o uds_client uds.c uds_client.c $LIBS $CFLAGS $FLAGS


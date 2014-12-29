#!/bin/sh

yaourt -S mingw-w64-gcc       \
          mingw-w64-enet      \
          mingw-w64-libxdiff  \
          mingw-w64-json-c    \
          mingw-w64-libwebp   \
          mingw-w64-dumb      \
          mingw-w64-libmad    \
          mingw-w64-sdl_mixer \
          mingw-w64-sdl_image \
          mingw-w64-gtk3

###
#
# Still requires:
#   - PolarSSL (https://polarssl.org)
#   - PortMidi (http://portmedia.sourceforge.net/portmidi)
#   - FluidSynth (http://www.fluidsynth.org)
#   - Pango (http://pango.org)
#     - Package is not built against a static GLib
#   - GObject Introspection (http://live.gnome.org/GObjectIntrospection)
#   - Lua 5.1 (http://www.lua.org)
#     - Package is built using g++ rather than GCC
#   - lgob (https://bitbucket.org/lucashnegri/lgib)
#
###


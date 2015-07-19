#!/bin/sh

yaourt --noconfirm -S \
                      # m4                  \
                      # autoconf            \
                      # automake            \
                      # mingw-w64-gcc       \
                      # mingw-w64-libxdiff  \
                      # mingw-w64-libmad    \
                      # mingw-w64-sdl_mixer \
                      mingw-w64-sdl_image \
                      # mingw-w64-enet      \
                      # mingw-w64-json-c    \
                      mingw-w64-dumb      \
                      # mingw-w64-xz        \
                      # mingw-w64-c-ares    \
                      # mingw-w64-curl

###
# Still requires:
#   - PortMidi (http://portmedia.sourceforge.net/portmidi)
#   - FluidSynth (http://www.fluidsynth.org)
###


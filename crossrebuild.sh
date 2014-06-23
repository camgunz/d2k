#!/bin/sh

CROSSDEPS="`pwd`/crossdeps"
CROSSLIBS="$CROSSDEPS/lib"
BUILD_DIR=crossbuild

rm -rf $BUILD_DIR

if [ ! -d $BUILD_DIR ]
then
  mkdir $BUILD_DIR
fi

# -DENET_INCLUDE_DIR="$CROSSINCS" \
# -DENET_LIBRARY="$CROSSLIBS/libenet.a" \
# -DSDL_INCLUDE_DIR="$CROSSDEPS/include/SDL" \
# -DSDLMIXER_INCLUDE_DIR="$CROSSDEPS/include/SDL" \
# -DSDLIMAGE_INCLUDE_DIR="$CROSSDEPS/include/SDL" \
# -DSDLMAIN_LIBRARY="$CROSSDEPS/lib/libSDLmain.a" \
# -DWEBP_INCLUDE_DIR="$CROSSDEPS/include" \
# -DPORTMIDI_INCLUDE_DIR="$CROSSDEPS/include" \
# -DOGG_INCLUDE_DIR="$CROSSDEPS/include" \
# -DVORBIS_INCLUDE_DIR="$CROSSDEPS/include" \

cd $BUILD_DIR && \
    CPPFLAGS="-I$CROSSDEPS/include" \
    LDFLAGS="-L$CROSSDEPS/lib" \
    SDLDIR="$CROSSDEPS" \
    PKG_CONFIG_PATH="$CROSSDEPS/lib/pkgconfig" \
    cmake .. -DCMAKE_BUILD_TYPE=Debug \
             -DPORTMIDI_LIBRARIES="$CROSSLIBS/libportmidi_s.a" \
             -DFLUIDSYNTH_LIBRARIES="$CROSSLIBS/libfluidsynth.a" \
             -DOGG_LIBRARIES="$CROSSLIBS/libogg.a" \
             -DVORBIS_LIBRARY="$CROSSLIBS/libvorbis.a" \
             -DVORBISFILE_LIBRARY="$CROSSLIBS/libvorbisfile.a" \
             -DFLAC_LIBRARIES="$CROSSLIBS/libFLAC.a" \
             -DMIKMOD_LIBRARIES="$CROSSLIBS/libmikmod.a" \
             -DSDLMIXER_LIBRARY="$CROSSLIBS/libSDL_mixer.a" \
             -DJPG_LIBRARIES="$CROSSLIBS/libjpeg.a" \
             -DPNG_LIBRARIES="$CROSSLIBS/libpng.a" \
             -DTIFF_LIBRARIES="$CROSSLIBS/libtiff.a" \
             -DWEBP_LIBRARIES="$CROSSLIBS/libwebp.a" \
             -DSDLIMAGE_LIBRARY="$CROSSLIBS/libSDL_image.a" \
             -DSDL_LIBRARY_TEMP="$CROSSLIBS/libSDL.a" \
             -DPCRE_LIBRARY="$CROSSLIBS/libpcre.a" \
             -DPCREPOSIX_LIBRARY="$CROSSLIBS/libpcreposix.a" \
             -DLZMA_LIBRARIES="$CROSSLIBS/liblzma.a" \
             -DGLIB_GIO_LIBRARIES="$CROSSLIBS/libgio-2.0.a" \
             -DGLIB_GMODULE_LIBRARIES="$CROSSLIBS/libgmodule-2.0.a" \
             -DGLIB_GOBJECT_LIBRARIES="$CROSSLIBS/libgobject-2.0.a" \
             -DGLIB_GTHREAD_LIBRARIES="$CROSSLIBS/libgthread-2.0.a" \
             -DGLIB_LIBRARY="$CROSSLIBS/libglib-2.0.a" \
             -DINTL_LIBRARY="$CROSSLIBS/libintl.a" \
             -DCMAKE_TOOLCHAIN_FILE=mingw32-toolchain.cmake && \
    clear && \
    make


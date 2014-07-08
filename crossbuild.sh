#!/bin/sh

CROSSDEPS="`pwd`/crossdeps"
CROSSLIBS="$CROSSDEPS/lib"
BUILD_DIR=crossbuild

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
             -DJPEG_LIBRARIES="$CROSSLIBS/libjpeg.a" \
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
             -DGLIB_LIBRARIES="$CROSSLIBS/libglib-2.0.a" \
             -DINTL_LIBRARIES="$CROSSLIBS/libintl.a" \
             -DFFI_LIBRARIES="$CROSSLIBS/libffi.a" \
             -DFONTCONFIG_LIBRARIES="$CROSSLIBS/libfontconfig.a" \
             -DEXPAT_LIBRARIES="$CROSSLIBS/libexpat.a" \
             -DFREETYPE_LIBRARIES="$CROSSLIBS/libfreetype.a" \
             -DFREETYPE_INCLUDE_DIRS="$CROSSDEPS/include/freetype2" \
             -DPIXMAN_LIBRARIES="$CROSSLIBS/libpixman-1.a" \
             -DCAIRO_LIBRARIES="$CROSSLIBS/libcairo.a" \
             -DCAIRO_INCLUDE_DIR="$CROSSDEPS/include/cairo" \
             -DPANGOFT2_LIBRARIES="$CROSSLIBS/libpangoft2-1.0.a" \
             -DPANGOWIN32_LIBRARIES="$CROSSLIBS/libpangowin32-1.0.a" \
             -DPANGOCAIRO_LIBRARIES="$CROSSLIBS/libpangocairo-1.0.a" \
             -DHARFBUZZ_LIBRARIES="$CROSSLIBS/libharfbuzz.a" \
             -DPANGO_LIBRARIES="$CROSSLIBS/libpango-1.0.a" \
             -DICONV_LIBRARIES="$CROSSLIBS/libiconv.a" \
             -DCMAKE_TOOLCHAIN_FILE=mingw32-toolchain.cmake && \
    clear && \
    VERBOSE=1 make


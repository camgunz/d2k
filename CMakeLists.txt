#===============================================================================
############################################################# License ##########
#===============================================================================
#
#  Copyright (c) 2010 Jamie Jones <jamie_jones_au@yahoo.com.au>
#  Copyright (C) 2013 David Hill
#  Copyright (C) 2014 Charles Gunyon <ladna@totaltrash.org>
#
#  This software is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the Free
#  Software Foundation; either version 2 of the License, or (at your option)
#  any later version.
#
#  This software is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU General Public License along with
#  this program; if not, write to the Free Software Foundation, Inc., 51
#  Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#===============================================================================

#===============================================================================
################################################# CMake Configuration ##########
#===============================================================================
PROJECT("PrBoom+" C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
IF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  MESSAGE(${CMAKE_SOURCE_DIR})
  MESSAGE(${CMAKE_BINARY_DIR})
  MESSAGE(FATAL_ERROR "In-tree Builds are NOT supported.")
ENDIF()

# Supported Build Types are:
#  * Debug (CMAKE_C_FLAGS_DEBUG)
#  * Release (CMAKE_C_FLAGS_RELEASE)
# If no build type requested, default to Release
IF(NOT DEFINED CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

# Default to not building shared libraries.
IF(NOT DEFINED BUILD_SHARED_LIBS)
  SET(BUILD_SHARED_LIBS OFF)
ENDIF()

# Default to not installing any sub-projects.
IF(NOT DEFINED SKIP_INSTALL_ALL)
  SET(SKIP_INSTALL_ALL ON)
ENDIF()

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCCompilerFlag)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

#===============================================================================
############################################################# Options ##########
#===============================================================================
OPTION(DOOMWADDIR, "The path where doom WADs are stored" "")
OPTION(
  CHECK_HEAP "Exhaustively run memory checks while the game is running" FALSE
)
OPTION(RANGECHECK "Enable internal range checking" FALSE)
OPTION(
  INSTRUMENTED
  "Enable real-time memory allocation statistics and enable extra debugging features"
  FALSE
)
OPTION(
  TIMEDIAG
  "Create timestamps whenever a lump is locked; report when lumps are locked for long periods of time"
  FALSE
)
OPTION(PROFILE "Enable profiling" FALSE)
OPTION(GL_DOOM "Build with OpenGL support" TRUE)
###
# TODO: How to make these dependent upon USE_GLU?
# OPTION(USE_GLU "Use the OpenGL Utility Library (libGLU)" FALSE)
###
OPTION(USE_GLU_IMAGESCALE "Use gluImageScale" FALSE)
OPTION(USE_GLU_MIPMAP "Use gluBuild2DMipmaps" FALSE)
OPTION(USE_GLU_TESS "Use gluTesselator" FALSE)
OPTION(HAVE_NET "Enable network game support" TRUE)
OPTION(USE_SDL_NET "Use SDL_net networking library" TRUE)
OPTION(DOGS "Enable MBF helper dogs" TRUE)
OPTION(HIGHRES "Enable high resolution support" TRUE)
OPTION(
  SIMPLECHECKS
  "Enable quick checks which only impose significant overhead if a possible error is detected"
  FALSE
)
OPTION(
  ZONEIDCHECK
  "Perform ID checks on zone blocks to detect corrupted and illegally freed blocks"
  FALSE
)
OPTION(
  ONLY_FREE_GRAPHICS "Build prboom.wad without non-free menu text lumps" TRUE
)

SET(VERSION "2.5.1.4-test")

IF(NOT DOOMWADDIR)
  SET(DOOMWADDIR "${CMAKE_INSTALL_PREFIX}/share/games/doom")
ENDIF()

#===============================================================================
##################################################### Platform Checks ##########
#===============================================================================
CHECK_INCLUDE_FILES("asm/byteorder.h" HAVE_ASM_BYTEORDER_H)
CHECK_INCLUDE_FILES("unistd.h" HAVE_UNISTD_H)
CHECK_INCLUDE_FILES("sys/wait.h" HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILES("netinet/in.h" HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILES("netinet/in6.h" HAVE_IPv6)
CHECK_FUNCTION_EXISTS("getopt" HAVE_GETOPT)
CHECK_FUNCTION_EXISTS("inet_ntoa" HAVE_INET_NTOA)
CHECK_FUNCTION_EXISTS("inet_aton" HAVE_INET_ATON)
CHECK_FUNCTION_EXISTS("inet_ntop" HAVE_INET_NTOP)
CHECK_FUNCTION_EXISTS("inet_pton" HAVE_INET_PTON)
CHECK_FUNCTION_EXISTS("mmap" HAVE_MMAP)
CHECK_FUNCTION_EXISTS("usleep" HAVE_USLEEP)
CHECK_FUNCTION_EXISTS("sched_setaffinity" HAVE_SCHED_SETAFFINITY)
CHECK_FUNCTION_EXISTS("min" MIN)
CHECK_FUNCTION_EXISTS("max" MAX)
CHECK_SYMBOL_EXISTS(sys_siglist signal.h HAVE_DECL_SYS_SIGLIST)
CHECK_SYMBOL_EXISTS(_stricmp string.h HAVE__STRICMP)
CHECK_SYMBOL_EXISTS(stricmp string.h HAVE_STRICMP)
CHECK_SYMBOL_EXISTS(strcasecmp string.h HAVE_STRCASECMP)
CHECK_SYMBOL_EXISTS(_strnicmp string.h HAVE__STRNICMP)
CHECK_SYMBOL_EXISTS(strnicmp string.h HAVE_STRNICMP)
CHECK_SYMBOL_EXISTS(strncasecmp string.h HAVE_STRNCASECMP)
CHECK_LIBRARY_EXISTS("m" "pow" "" HAVE_LIBM)
CHECK_LIBRARY_EXISTS("GL" "glFinish" "" HAVE_LIBGL)
CHECK_LIBRARY_EXISTS("GLU" "gluBuild2DMipmaps" "" HAVE_LIBGLU)
CHECK_LIBRARY_EXISTS("GLESv2" "glFinish" "" HAVE_LIBGLESv2)
CHECK_LIBRARY_EXISTS("SDL" "SDL_JoystickGetAxis" "" HAVE_SDL_JOYSTICKGETAXIS)

CHECK_C_SOURCE_COMPILES(
  "struct __attribute__((packed)) foo { int a; } b; int main() { return 0; }"
  HAVE_ATTRIBUTE_PACKED
)

IF(HAVE_ATTRIBUTE_PACKED)
  SET(PACKEDATTR "__attribute__((packed))")
ELSE()
  MESSAGE(FATAL_ERROR "Can't figure out how to pack structs with your compiler")
ENDIF()

IF(NOT HAVE_IPv6)
  IF(NOT HAVE_NETINET_IN_H)
    MESSAGE(FATAL_ERROR "Bad networking headers")
  ENDIF()
  SET(CMAKE_EXTRA_INCLUDE_FILES netinet/in.h)
  CHECK_TYPE_SIZE("struct sockaddr_in6" STRUCT_SOCKADDR_IN6_SIZE)
  SET(CMAKE_EXTRA_INCLUDE_FILES)
  IF(NOT STRUCT_SOCKADDR_IN6_SIZE EQUAL 0)
    SET(HAVE_IPv6 TRUE)
    MESSAGE(STATUS "Found IPv6 support")
  ELSE()
    MESSAGE(WARNING "Missing IPv6 support")
  ENDIF()
ENDIF()

IF(HAVE_STRICMP)
  SET(stricmp stricmp)
ELSE()
  IF(HAVE__STRICMP)
    SET(stricmp _stricmp)
  ELSE()
    IF(HAVE_STRCASECMP)
      SET(stricmp strcasecmp)
    ELSE()
      MESSAGE(FATAL_ERROR "Neither _stricmp, stricmp, or strcasecmp found")
    ENDIF()
  ENDIF()
ENDIF()

IF(HAVE_STRNICMP)
  SET(strnicmp strnicmp)
ELSE()
  IF(HAVE__STRNICMP)
    SET(strnicmp _strnicmp)
  ELSE()
    IF(HAVE_STRNCASECMP)
      SET(strnicmp strncasecmp)
    ELSE()
      MESSAGE(FATAL_ERROR "Neither _strnicmp, strnicmp, or strncasecmp found")
    ENDIF()
  ENDIF()
ENDIF()

#===============================================================================
########################################################### Functions ##########
#===============================================================================
FUNCTION(TRY_C_COMPILER_FLAG flag name)
  CHECK_C_COMPILER_FLAG("${flag}" ${name})
  IF(${name})
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
  ENDIF()
ENDFUNCTION(TRY_C_COMPILER_FLAG flag name)

#===============================================================================
################################################# Set Package Details ##########
#===============================================================================
SET(CMAKE_MFC_FLAG 2)
SET(CMAKE_INSTALL_MFC_LIBRARIES 1)

INCLUDE(InstallRequiredSystemLibraries)
SET(PACKAGE "PrBoom-Plus")
SET(PACKAGE_NAME "PrBoom-Plus")
SET(PACKAGE_VERSION "2.5.1.4")
# SET(PACKAGE_BUGREPORT "huh")
SET(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
SET(PACKAGE_TARNAME "prboom-plus")
SET(PACKAGE_URL "http://prboom-plus.sourceforge.net")

SET(CPACK_GENERATOR "DEB;RPM;STGZ;ZIP")
SET(CPACK_PACKAGE_VENDOR "Team TotalTrash")
SET(CPACK_PACKAGE_CONTACT "Team TotalTrash <ladna@totaltrash.org>")
SET(CPACK_PACKAGE_VERSION "2.5.1.4-test")
SET(CPACK_PACKAGE_NAME "PrBoom+")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(
  CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "PrBoom+ extends the original PrBoom engine while maintaining compatibility."
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.11.0), libgcc1 (>= 1:4.4.3), libsdl1.2debian (>=1.2.14), libsdl-mixer1.2 (>=1.2.8), libsdl-net1.2 (>=1.2.7), bash")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "freedoom, game-data-packager")

SET(BUILD_ARCH "${CMAKE_SYSTEM_PROCESSOR}")

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "i686")
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
  SET(CPACK_RPM_PACKAGE_ARCHITECTURE "i686" )
  SET(BUILD_ARCH "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
ENDIF()

IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
  SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64" )
  SET(BUILD_ARCH "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
ENDIF()

IF(WIN32)
  SET(BUILD_ARCH "windows-x86")
  IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "AMD64")
    SET(BUILD_ARCH "windows-$ENV{PROCESSOR_ARCHITECTURE}")
  ENDIF()
ENDIF()

SET(
  CPACK_PACKAGE_FILE_NAME "prboom-plus-${CPACK_PACKAGE_VERSION}_${BUILD_ARCH}"
)
SET(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_FILE_NAME}")
SET(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_FILE_NAME}")

SET(BIN_DIR bin)
SET(SHARE_DIR share/eternity)

IF(WIN32)
  SET(LIB_DIR ${BIN_DIR})
  SET(SHARE_DIR ${BIN_DIR})
ELSE()
  SET(LIB_DIR lib)
ENDIF()
INCLUDE(CPack)

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
  SET(GCC_OR_CLANG TRUE)
ELSE()
  SET(GCC_OR_CLANG FALSE)
ENDIF()

#===============================================================================
################################################## Compiler: Features ##########
#===============================================================================
IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  TRY_C_COMPILER_FLAG(--std=c99 FLAG_C_stdc99)
ENDIF()

#===============================================================================
################################################## Compiler: Warnings ##########
#===============================================================================
IF(GCC_OR_CLANG)
  TRY_C_COMPILER_FLAG(-Wall   FLAG_C_Wall)
ENDIF()

IF(CMAKE_C_COMPILER_ID STREQUAL "Intel")
  TRY_C_COMPILER_FLAG(-Wall              FLAG_C_Wall)
  TRY_C_COMPILER_FLAG(-Wcheck            FLAG_C_Wcheck)
  TRY_C_COMPILER_FLAG(-Wp64              FLAG_C_Wp64)
  TRY_C_COMPILER_FLAG(-Wshorten-64-to-32 FLAG_C_Wshorten_64_to_32)
ENDIF()

IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
ENDIF()

#===============================================================================
################################################# Compiler: Hardening ##########
#===============================================================================
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    TRY_C_COMPILER_FLAG(-pg FLAG_C_profile)
  ENDIF()
  IF(GCC_OR_CLANG)
    TRY_C_COMPILER_FLAG(-g FLAG_C_debug)
  ENDIF()
ELSE()
  IF(GCC_OR_CLANG)
    TRY_C_COMPILER_FLAG(-fstack-protector FLAG_C_fstack_protector)

    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    ###
    #   TRY_C_COMPILER_FLAG(-fPIC              FLAG_C_fPIC)
    #   TRY_C_COMPILER_FLAG(-pie               FLAG_C_pie)
    #   TRY_C_COMPILER_FLAG(-Wl,-z,relro       FLAG_C_Wl_z_relro)
    #   TRY_C_COMPILER_FLAG(-Wl,-z,now         FLAG_C_Wl_z_now)
    #   TRY_C_COMPILER_FLAG(-Wl,--as-needed    FLAG_C_Wl_as_needed)
    #   TRY_C_COMPILER_FLAG(-Wl,-z,noexecstack FLAG_C_Wl_z_noexecstack)
    ###
    ENDIF()
  ENDIF()

  IF(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    TRY_C_COMPILER_FLAG(-fstack-protector  FLAG_C_fstackprotector)
    TRY_C_COMPILER_FLAG(-fPIC              FLAG_C_fPIC)
    TRY_C_COMPILER_FLAG(-pie               FLAG_C_pie)
    TRY_C_COMPILER_FLAG(-Wl,-z,relro       FLAG_C_Wl_z_relro)
    TRY_C_COMPILER_FLAG(-Wl,-z,now         FLAG_C_Wl_z_now)
    TRY_C_COMPILER_FLAG(-Wl,--as-needed    FLAG_C_Wl_as_needed)
    TRY_C_COMPILER_FLAG(-Wl,-z,noexecstack FLAG_C_Wl_z_noexecstack)
  ENDIF()

  IF(MSVC)
    TRY_C_COMPILER_FLAG(/GS FLAG_C_GS)
  ENDIF()
ENDIF()

#===============================================================================
############################################## Compiler: Optimization ##########
#===============================================================================
IF(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR CMAKE_BUILD_TYPE STREQUAL "Release")
  IF(GCC_OR_CLANG)
    TRY_C_COMPILER_FLAG(-fomit-frame-pointer FLAG_C_fomit_frame_pointer)
  ENDIF()

  IF(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    TRY_C_COMPILER_FLAG(-fomit-frame-pointer FLAG_C_fomit_frame_pointer)
    TRY_C_COMPILER_FLAG(-ipo                 FLAG_C_ipo)
  ENDIF()

ENDIF()

#===============================================================================
################################################### Find Dependencies ##########
#===============================================================================
FIND_PACKAGE(OpenGL)

FIND_PACKAGE(SDL REQUIRED)
SET(HAVE_LIBSDL TRUE)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})

FIND_PACKAGE(SDL_net)
IF(SDLNET_FOUND)
  INCLUDE_DIRECTORIES(${SDLNET_INCLUDE_DIR})
  SET(HAVE_LIBSDL_NET TRUE)
ELSE()
  MESSAGE(WARNING "SDL_net not found")
  SET(HAVE_LIBSDL_NET FALSE)
ENDIF()

FIND_PACKAGE(SDL_image)
IF(SDLIMAGE_FOUND)
  INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})
  SET(HAVE_LIBSDL_IMAGE TRUE)
ELSE()
  MESSAGE(WARNING "SDL_image not found")
  SET(HAVE_LIBSDL_IMAGE FALSE)
ENDIF()

FIND_PACKAGE(SDL_mixer)
IF(SDLMIXER_FOUND)
  INCLUDE_DIRECTORIES(${SDLMIXER_INCLUDE_DIR})
  SET(HAVE_LIBSDL_MIXER TRUE)
ELSE()
  MESSAGE(WARNING "SDL_mixer not found")
  SET(HAVE_LIBSDL_MIXER FALSE)
ENDIF()

FIND_PACKAGE(FluidSynth)
IF(FLUIDSYNTH_FOUND)
  INCLUDE_DIRECTORIES(${FLUIDSYNTH_INCLUDE_DIR})
  SET(HAVE_LIBFLUIDSYNTH TRUE)
ELSE()
  MESSAGE(WARNING "FluidSynth not found")
ENDIF()

FIND_PACKAGE(PortMidi)
IF(PORTMIDI_FOUND)
  INCLUDE_DIRECTORIES(${PORTMIDI_INCLUDE_DIR})
  SET(HAVE_LIBPORTMIDI TRUE)
ELSE()
  MESSAGE(WARNING "PortMidi not found")
ENDIF()

FIND_PACKAGE(Ogg)
IF(OGG_FOUND)
  INCLUDE_DIRECTORIES(${OGG_INCLUDE_DIR})
ELSE()
  MESSAGE(WARNING "Ogg not found")
ENDIF()

FIND_PACKAGE(Vorbis)
IF(VORBIS_FOUND)
  INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR})
  SET(HAVE_LIBVORBISFILE TRUE)
ELSE()
  MESSAGE(WARNING "Vorbis not found")
ENDIF()

FIND_PACKAGE(Dumb)
IF(DUMB_FOUND)
  INCLUDE_DIRECTORIES(${DUMB_INCLUDE_DIR})
  SET(HAVE_LIBDUMB TRUE)
ELSE()
  MESSAGE(WARNING "Dumb not found")
ENDIF()

FIND_PACKAGE(Mad)
IF(MAD_FOUND)
  INCLUDE_DIRECTORIES(${MAD_INCLUDE_DIR})
  SET(HAVE_LIBMAD TRUE)
ELSE()
  MESSAGE(WARNING "Mad not found")
ENDIF()

FIND_PACKAGE(PCREPOSIX)
IF(PCRE_FOUND)
  INCLUDE_DIRECTORIES(${PCRE_INCLUDE_DIR})
  SET(HAVE_LIBPCREPOSIX TRUE)
ELSE()
  MESSAGE(WARNING "PCRE not found")
ENDIF()

FIND_PACKAGE(PNG)
IF(PNG_FOUND)
  INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
  SET(HAVE_LIBPNG TRUE)
ELSE()
  MESSAGE(WARNING "PNG not found")
ENDIF()

FIND_PACKAGE(XDiff)
IF(XDIFF_FOUND)
  INCLUDE_DIRECTORIES(${XDIFF_INCLUDE_DIR})
ELSE()
  MESSAGE(FATAL "XDiff not found")
ENDIF()

FIND_PACKAGE(ZLIB)
IF(ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ELSE()
  MESSAGE(WARNING "zlib not found")
ENDIF()

MARK_AS_ADVANCED(GPERF_LIBRARY)
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  MESSAGE(STATUS "Looking for Google Performance Tools")
  FIND_LIBRARY(GPERF_LIBRARIES profiler HINTS $ENV{GPERF_DIR})
  IF(${GPERF_LIBRARIES} STREQUAL "GPERF_LIBRARIES-NOTFOUND")
    MESSAGE(WARNING "Google Performance Tools not found")
  ELSE()
    MESSAGE(STATUS "Google Performance Tools found: ${GPERF_LIBRARIES}")
  ENDIF()
ENDIF()

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

## FIXME: This is an inelegant hack to find, and grab all needed
## .dll support files on windows. It works by looking for SDL.dll
## then taking every .dll file found in that directory from your SDK.
## This ensures you get sdl and all of it's needed .dll files, but may
## also grab unneeded .dll files.
IF(WIN32)
  FIND_PATH(
    SDL_SUPPORT_LIBS_DIR SDL.dll
    HINTS
    $ENV{SDLDIR}
    PATH_SUFFIXES lib64 lib
    PATHS
  )
  FILE(GLOB SDL_SUPPORT_LIBS ${SDL_SUPPORT_LIBS_DIR}/*.dll)
ENDIF()

#===============================================================================
################################################### Set Build Targets ##########
#===============================================================================

## FIXME: This is a continuation of the Windows SDL inelegant hack. This does
## the actual installation of those support libraries.
IF(WIN32)
  INSTALL(PROGRAMS ${SDL_SUPPORT_LIBS} DESTINATION ${LIB_DIR})
ENDIF()

#===============================================================================
################################################## Set Project Source ##########
#===============================================================================

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

IF(GCC_OR_CLANG)
  ADD_DEFINITIONS(-D_GNU_SOURCE)
ENDIF()

IF(APPLE)
  INCLUDE_DIRECTORIES(src/MAC)
  FILE(GLOB ARCH_SPECIFIC_SOURCES src/MAC/*.m)
  SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
    src/PCSOUND/pcsound_sdl.c
  )
ELSE()
  IF(UNIX)
    SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
      src/PCSOUND/pcsound_linux.c
    )
  ELSE()
    SET(ARCH_SPECIFIC_SOURCES src/SDL/SDL_win32_main.c)
    SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
      src/PCSOUND/pcsound_win32.c
    )
  ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(src/SDL)
SET(SDL_SOURCES
  src/SDL/i_joy.c
  src/SDL/i_network.c
  src/SDL/i_sound.c
  src/SDL/i_sshot.c
  src/SDL/i_video.c
)

INCLUDE_DIRECTORIES(src/PCSOUND)
SET(PCSOUND_SOURCES src/PCSOUND/pcsound.c)
IF(NOT WIN32)
  SET(PCSOUND_SOURCES ${PCSOUND_SOURCES} src/PCSOUND/pcsound_sdl)
ENDIF()

INCLUDE_DIRECTORIES(src/textscreen)
FILE(GLOB TEXTSCREEN_SOURCES src/TEXTSCREEN/*.c)

INCLUDE_DIRECTORIES(src/MUSIC)
FILE(GLOB MUSIC_SOURCES src/MUSIC/*.c)

INCLUDE_DIRECTORIES(src)
SET(PRBOOM_SOURCES
  src/am_map.c
  src/d_deh.c
  src/d_items.c
  src/d_main.c
  src/doomdef.c
  src/doomstat.c
  src/dstrings.c
  src/e6y.c
  src/e6y_launcher.c
  src/f_finale.c
  src/f_wipe.c
  src/g_game.c
  src/g_overflow.c
  src/hu_lib.c
  src/hu_stuff.c
  src/hu_tracers.c
  src/i_capture.c
  src/i_pcsound.c
  src/i_smp.c
  src/info.c
  src/m_bbox.c
  src/m_cheat.c
  src/m_delta.c
  src/m_menu.c
  src/m_misc.c
  src/m_random.c
  src/md5.c
  src/p_ceilng.c
  src/p_checksum.c
  src/p_doors.c
  src/p_enemy.c
  src/p_floor.c
  src/p_genlin.c
  src/p_inter.c
  src/p_lights.c
  src/p_map.c
  src/p_maputl.c
  src/p_mobj.c
  src/p_plats.c
  src/p_pspr.c
  src/p_saveg.c
  src/p_setup.c
  src/p_sight.c
  src/p_spec.c
  src/p_switch.c
  src/p_telept.c
  src/p_tick.c
  src/p_user.c
  src/r_bsp.c
  src/r_data.c
  src/r_demo.c
  src/r_draw.c
  src/r_filter.c
  src/r_fps.c
  src/r_main.c
  src/r_patch.c
  src/r_plane.c
  src/r_screenmultiply.c
  src/r_segs.c
  src/r_sky.c
  src/r_things.c
  src/s_advsound.c
  src/s_sound.c
  src/sc_man.c
  src/sounds.c
  src/st_lib.c
  src/st_stuff.c
  src/tables.c
  src/v_video.c
  src/version.c
  src/w_wad.c
  src/wi_stuff.c
  src/z_bmalloc.c
  src/z_zone.c
)

SET(SERVER_SOURCES
  src/POSIX/i_system.c
  src/SDL/i_network.c
  src/d_server.c
)

SET(CLIENT_SOURCES
  src/SDL/i_main.c
  src/SDL/i_system.c
  src/d_client.c
  src/lprintf.c
  src/m_argv.c
)

SET(GL_SOURCES
  src/gl_clipper.c
  src/gl_detail.c
  src/gl_drawinfo.c
  src/gl_fbo.c
  src/gl_gamma.c
  src/gl_hires.c
  src/gl_hqresize.c
  src/gl_light.c
  src/gl_main.c
  src/gl_map.c
  src/gl_missingtexture.c
  src/gl_opengl.c
  src/gl_preprocess.c
  src/gl_shader.c
  src/gl_shadow.c
  src/gl_sky.c
  src/gl_soft2gl.c
  src/gl_texture.c
  src/gl_vertex.c
  src/gl_wipe.c
)

IF(HAVE_MMAP)
  SET(WAD_SOURCES src/w_mmap.c)
ELSE()
  SET(WAD_SOURCES src/w_memcache.c)
ENDIF()

SET(MUS2MID_SOURCES src/memio.c src/mus2mid.c)

SET(PRBOOM_CLIENT_SOURCES
  ${ARCH_SPECIFIC_SOURCES}
  ${SDL_SOURCES}
  ${PCSOUND_SOURCES}
  ${TEXTSCREEN_SOURCES}
  ${MUSIC_SOURCES}
  ${PRBOOM_SOURCES}
  ${CLIENT_SOURCES}
  ${MUS2MID_SOURCES}
  ${WAD_SOURCES}
)

IF(GL_DOOM)
  SET(PRBOOM_CLIENT_SOURCES ${PRBOOM_CLIENT_SOURCES} ${GL_SOURCES})
ENDIF()

SET(PRBOOM_SERVER_SOURCES
  ${ARCH_SPECIFIC_SOURCES}
  ${SDL_SOURCES}
  ${PCSOUND_SOURCES}
  ${TEXTSCREEN_SOURCES}
  ${MUSIC_SOURCES}
  ${PRBOOM_SOURCES}
  ${SERVER_SOURCES}
  ${WAD_SOURCES}
)

IF(GL_DOOM)
  SET(PRBOOM_CLIENT_SOURCES ${PRBOOM_CLIENT_SOURCES} ${GL_SOURCES})
ENDIF()

ADD_EXECUTABLE(prboom-plus ${PRBOOM_CLIENT_SOURCES})

TARGET_LINK_LIBRARIES(prboom-plus
  ${SDL_LIBRARY}
  ${SDLMIXER_LIBRARY}
  ${SDLNET_LIBRARY}
  ${SDLIMAGE_LIBRARY}
  ${OPENGL_gl_LIBRARY}
  ${FLUIDSYNTH_LIBRARIES}
  ${PORTMIDI_LIBRARIES}
  ${OGG_LIBRARIES}
  ${VORBIS_LIBRARIES}
  ${MAD_LIBRARIES}
  ${DUMB_LIBRARIES}
  ${PCRE_LIBRARIES}
  ${PNG_LIBRARIES}
  ${XDIFF_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${GPERF_LIBRARIES}
)

INSTALL(
  TARGETS prboom-plus
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR}
)

# vi: ai:ts=2:sw=2:et


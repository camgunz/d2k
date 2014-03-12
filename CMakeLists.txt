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
PROJECT("PrBoom+" C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
IF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  MESSAGE(${CMAKE_SOURCE_DIR})
  MESSAGE(${CMAKE_BINARY_DIR})
  MESSAGE(FATAL_ERROR "In-tree Builds are NOT supported.")
ENDIF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

# Supported Build Types are:
#    * Debug (CMAKE_C_FLAGS_DEBUG)
#    * Release (CMAKE_C_FLAGS_RELEASE)
#    * RelWithDebInfo (CMAKE_C_FLAGS_RELWITHDEBINFO)
#    * MinSizeRel (CMAKE_C_FLAGS_MINSIZEREL)
# If no build type requested, default to Release
IF(NOT DEFINED CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF(NOT DEFINED CMAKE_BUILD_TYPE)

# Default to not building shared libraries.
IF(NOT DEFINED BUILD_SHARED_LIBS)
  SET(BUILD_SHARED_LIBS OFF)
ENDIF(NOT DEFINED BUILD_SHARED_LIBS)

# Default to not installing any sub-projects.
IF(NOT DEFINED SKIP_INSTALL_ALL)
  SET(SKIP_INSTALL_ALL ON)
ENDIF(NOT DEFINED SKIP_INSTALL_ALL)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)

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
# OPTION(USE_GLU "Use the OpenGL Utility Library (libGLU)" FALSE)
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

IF (NOT DOOMWADDIR)
  SET(DOOMWADDIR "${CMAKE_INSTALL_PREFIX}/share/games/doom")
ENDIF (NOT DOOMWADDIR)

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
ELSE(HAVE_ATTRIBUTE_PACKED)
  MESSAGE(FATAL_ERROR "Can't figure out how to pack structs with your compiler")
ENDIF(HAVE_ATTRIBUTE_PACKED)

IF(NOT HAVE_IPv6)
  IF(NOT HAVE_NETINET_IN_H)
    MESSAGE(FATAL_ERROR "Bad networking headers")
  ENDIF(NOT HAVE_NETINET_IN_H)
  SET(CMAKE_EXTRA_INCLUDE_FILES netinet/in.h)
  CHECK_TYPE_SIZE("struct sockaddr_in6" STRUCT_SOCKADDR_IN6_SIZE)
  SET(CMAKE_EXTRA_INCLUDE_FILES)
  IF(NOT STRUCT_SOCKADDR_IN6_SIZE EQUAL 0)
    SET(HAVE_IPv6 TRUE)
    MESSAGE(STATUS "Found IPv6 support")
  ELSE(NOT STRUCT_SOCKADDR_IN6_SIZE EQUAL 0)
    MESSAGE(WARNING "Missing IPv6 support")
  ENDIF(NOT STRUCT_SOCKADDR_IN6_SIZE EQUAL 0)
ENDIF(NOT HAVE_IPv6)

IF(HAVE_STRICMP)
  SET(stricmp stricmp)
ELSE(HAVE_STRICMP)
  IF(HAVE__STRICMP)
    SET(stricmp _stricmp)
  ELSE(HAVE__STRICMP)
    IF(HAVE_STRCASECMP)
      SET(stricmp strcasecmp)
    ELSE(HAVE_STRCASECMP)
      MESSAGE(FATAL_ERROR "Neither _stricmp, stricmp, or strcasecmp found")
    ENDIF(HAVE_STRCASECMP)
  ENDIF(HAVE__STRICMP)
ENDIF(HAVE_STRICMP)

IF(HAVE_STRNICMP)
  SET(strnicmp strnicmp)
ELSE(HAVE_STRNICMP)
  IF(HAVE__STRNICMP)
    SET(strnicmp _strnicmp)
  ELSE(HAVE__STRNICMP)
    IF(HAVE_STRNCASECMP)
      SET(strnicmp strncasecmp)
    ELSE(HAVE_STRNCASECMP)
      MESSAGE(FATAL_ERROR "Neither _strnicmp, strnicmp, or strncasecmp found")
    ENDIF(HAVE_STRNCASECMP)
  ENDIF(HAVE__STRNICMP)
ENDIF(HAVE_STRNICMP)

#===============================================================================
########################################################### Functions ##########
#===============================================================================
FUNCTION(TRY_C_COMPILER_FLAG flag name)
  CHECK_C_COMPILER_FLAG("${flag}" ${name})
  IF(${name})
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
  ENDIF(${name})
ENDFUNCTION(TRY_C_COMPILER_FLAG flag name)

FUNCTION(TRY_CXX_COMPILER_FLAG flag name)
   CHECK_CXX_COMPILER_FLAG(${flag} ${name})
   IF(${name})
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
   ENDIF(${name})
ENDFUNCTION(TRY_CXX_COMPILER_FLAG flag name)

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
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "i686")
IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
        SET(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64" )
        SET(BUILD_ARCH "${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
ENDIF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
IF(WIN32)
  SET(BUILD_ARCH "windows-x86")
  IF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "AMD64")
    SET(BUILD_ARCH "windows-$ENV{PROCESSOR_ARCHITECTURE}")
  ENDIF("$ENV{PROCESSOR_ARCHITECTURE}" STREQUAL "AMD64")
ENDIF(WIN32)
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
ELSE(WIN32)
  SET(LIB_DIR lib)
ENDIF(WIN32)
INCLUDE(CPack)

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_C_COMPILER_ID STREQUAL "Clang")
  SET(GCC_OR_CLANG TRUE)
ELSE(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
     CMAKE_C_COMPILER_ID STREQUAL "Clang")
  SET(GCC_OR_CLANG FALSE)
ENDIF(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
      CMAKE_C_COMPILER_ID STREQUAL "Clang")

IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(GXX_OR_CLANG TRUE)
ELSE(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
     CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(GXX_OR_CLANG FALSE)
ENDIF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
      CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    TRY_CXX_COMPILER_FLAG(-ferror-limit=8 FLAG_CXX_ERROR_LIMIT)
ENDIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

#===============================================================================
################################################## Compiler: Features ##########
#===============================================================================
IF(GXX_OR_CLANG)
  TRY_CXX_COMPILER_FLAG(-std=c++11 FLAG_CXX_stdcxx11)
ENDIF()

#===============================================================================
################################################## Compiler: Warnings ##########
#===============================================================================
IF(GCC_OR_CLANG)
  TRY_C_COMPILER_FLAG(-Wall   FLAG_C_Wall)
ENDIF()

IF(GXX_OR_CLANG)
  TRY_CXX_COMPILER_FLAG(-Wall   FLAG_CXX_Wall)
ENDIF()

IF(CMAKE_C_COMPILER_ID STREQUAL "Intel")
  TRY_C_COMPILER_FLAG(-Wall              FLAG_C_Wall)
  TRY_C_COMPILER_FLAG(-Wcheck            FLAG_C_Wcheck)
  TRY_C_COMPILER_FLAG(-Wp64              FLAG_C_Wp64)
  TRY_C_COMPILER_FLAG(-Wshorten-64-to-32 FLAG_C_Wshorten_64_to_32)
ENDIF()

IF(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  TRY_CXX_COMPILER_FLAG(-Wall              FLAG_CXX_Wall)
  TRY_CXX_COMPILER_FLAG(-Wcheck            FLAG_CXX_Wcheck)
  TRY_CXX_COMPILER_FLAG(-Wp64              FLAG_CXX_Wp64)
  TRY_CXX_COMPILER_FLAG(-Wshorten-64-to-32 FLAG_CXX_Wshorten_64_to_32)
ENDIF()

IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_WARNINGS)
ENDIF()

#===============================================================================
################################################# Compiler: Hardening ##########
#===============================================================================
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  IF(GCC_OR_CLANG)
    TRY_C_COMPILER_FLAG(-fstack-protector FLAG_C_fstack_protector)

    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      TRY_C_COMPILER_FLAG(-fPIC              FLAG_C_fPIC)
      TRY_C_COMPILER_FLAG(-pie               FLAG_C_pie)
      TRY_C_COMPILER_FLAG(-Wl,-z,relro       FLAG_C_Wl_z_relro)
      TRY_C_COMPILER_FLAG(-Wl,-z,now         FLAG_C_Wl_z_now)
      TRY_C_COMPILER_FLAG(-Wl,--as-needed    FLAG_C_Wl_as_needed)
      TRY_C_COMPILER_FLAG(-Wl,-z,noexecstack FLAG_C_Wl_z_noexecstack)
    ENDIF()
  ENDIF()

  IF(GXX_OR_CLANG)
    TRY_CXX_COMPILER_FLAG(-fstack-protector FLAG_CXX_fstack_protector)

    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      TRY_CXX_COMPILER_FLAG(-fPIC              FLAG_CXX_fPIC)
      TRY_CXX_COMPILER_FLAG(-pie               FLAG_CXX_pie)
      TRY_CXX_COMPILER_FLAG(-Wl,-z,relro       FLAG_CXX_Wl_z_relro)
      TRY_CXX_COMPILER_FLAG(-Wl,-z,now         FLAG_CXX_Wl_z_now)
      TRY_CXX_COMPILER_FLAG(-Wl,--as-needed    FLAG_CXX_Wl_as_needed)
      TRY_CXX_COMPILER_FLAG(-Wl,-z,noexecstack FLAG_CXX_Wl_z_noexecstack)
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

  IF(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    TRY_CXX_COMPILER_FLAG(-fstack-protector  FLAG_CXX_fstackprotector)
    TRY_CXX_COMPILER_FLAG(-fPIC              FLAG_CXX_fPIC)
    TRY_CXX_COMPILER_FLAG(-pie               FLAG_CXX_pie)
    TRY_CXX_COMPILER_FLAG(-Wl,-z,relro       FLAG_CXX_Wl_z_relro)
    TRY_CXX_COMPILER_FLAG(-Wl,-z,now         FLAG_CXX_Wl_z_now)
    TRY_CXX_COMPILER_FLAG(-Wl,--as-needed    FLAG_CXX_Wl_as_needed)
    TRY_CXX_COMPILER_FLAG(-Wl,-z,noexecstack FLAG_CXX_Wl_z_noexecstack)
  ENDIF()

  IF(MSVC)
    TRY_C_COMPILER_FLAG(/GS FLAG_C_GS)
    TRY_CXX_COMPILER_FLAG(/GS FLAG_CXX_GS)
  ENDIF()
ENDIF()

#===============================================================================
############################################## Compiler: Optimization ##########
#===============================================================================
IF(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR CMAKE_BUILD_TYPE STREQUAL "Release")
  IF(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    TRY_C_COMPILER_FLAG(-fomit-frame-pointer FLAG_C_fomit_frame_pointer)
  ENDIF()

  IF(GXX_OR_CLANG)
    TRY_CXX_COMPILER_FLAG(-fomit-frame-pointer FLAG_CXX_fomit_frame_pointer)
  ENDIF()

  IF(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    TRY_C_COMPILER_FLAG(-fomit-frame-pointer FLAG_C_fomit_frame_pointer)
    TRY_C_COMPILER_FLAG(-ipo                 FLAG_C_ipo)
  ENDIF()

  IF(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    TRY_CXX_COMPILER_FLAG(-fomit-frame-pointer FLAG_CXX_fomit_frame_pointer)
    TRY_CXX_COMPILER_FLAG(-ipo                 FLAG_CXX_ipo)
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
ELSE(SDLNET_FOUND)
  MESSAGE(WARNING "SDL_net not found")
  SET(HAVE_LIBSDL_NET FALSE)
ENDIF(SDLNET_FOUND)

FIND_PACKAGE(SDL_image)
IF(SDLIMAGE_FOUND)
  INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})
  SET(HAVE_LIBSDL_IMAGE TRUE)
ELSE(SDLIMAGE_FOUND)
  MESSAGE(WARNING "SDL_image not found")
  SET(HAVE_LIBSDL_IMAGE FALSE)
ENDIF(SDLIMAGE_FOUND)

FIND_PACKAGE(SDL_mixer)
IF(SDLMIXER_FOUND)
  INCLUDE_DIRECTORIES(${SDLMIXER_INCLUDE_DIR})
  SET(HAVE_LIBSDL_MIXER TRUE)
ELSE(SDLMIXER_FOUND)
  MESSAGE(WARNING "SDL_mixer not found")
  SET(HAVE_LIBSDL_MIXER FALSE)
ENDIF(SDLMIXER_FOUND)

FIND_PACKAGE(FluidSynth)
IF(FLUIDSYNTH_FOUND)
  INCLUDE_DIRECTORIES(${FLUIDSYNTH_INCLUDE_DIR})
  SET(HAVE_LIBFLUIDSYNTH TRUE)
ELSE(FLUIDSYNTH_FOUND)
  MESSAGE(WARNING "FluidSynth not found")
ENDIF(FLUIDSYNTH_FOUND)

FIND_PACKAGE(PortMidi)
IF(PORTMIDI_FOUND)
  INCLUDE_DIRECTORIES(${PORTMIDI_INCLUDE_DIR})
  SET(HAVE_LIBPORTMIDI TRUE)
ELSE(PORTMIDI_FOUND)
  MESSAGE(WARNING "PortMidi not found")
ENDIF(PORTMIDI_FOUND)

FIND_PACKAGE(Ogg)
IF(OGG_FOUND)
  INCLUDE_DIRECTORIES(${OGG_INCLUDE_DIR})
ELSE(OGG_FOUND)
  MESSAGE(WARNING "Ogg not found")
ENDIF(OGG_FOUND)

FIND_PACKAGE(Vorbis)
IF(VORBIS_FOUND)
  INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR})
  SET(HAVE_LIBVORBISFILE TRUE)
ELSE(VORBIS_FOUND)
  MESSAGE(WARNING "Vorbis not found")
ENDIF(VORBIS_FOUND)

FIND_PACKAGE(Dumb)
IF(DUMB_FOUND)
  INCLUDE_DIRECTORIES(${DUMB_INCLUDE_DIR})
  SET(HAVE_LIBDUMB TRUE)
ELSE(DUMB_FOUND)
  MESSAGE(WARNING "Dumb not found")
ENDIF(DUMB_FOUND)

FIND_PACKAGE(Mad)
IF(MAD_FOUND)
  INCLUDE_DIRECTORIES(${MAD_INCLUDE_DIR})
  SET(HAVE_LIBMAD TRUE)
ELSE(MAD_FOUND)
  MESSAGE(WARNING "Mad not found")
ENDIF(MAD_FOUND)

FIND_PACKAGE(PCREPOSIX)
IF(PCRE_FOUND)
  INCLUDE_DIRECTORIES(${PCRE_INCLUDE_DIR})
  SET(HAVE_LIBPCREPOSIX TRUE)
ELSE(PCRE_FOUND)
  MESSAGE(WARNING "PCRE not found")
ENDIF(PCRE_FOUND)

FIND_PACKAGE(PNG)
IF(PNG_FOUND)
  INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
  SET(HAVE_LIBPNG TRUE)
ELSE(PNG_FOUND)
  MESSAGE(WARNING "PNG not found")
ENDIF(PNG_FOUND)

FIND_PACKAGE(ZLIB)
IF(ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ELSE(ZLIB_FOUND)
  MESSAGE(WARNING "zlib not found")
ENDIF(ZLIB_FOUND)

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
ENDIF(WIN32)

#===============================================================================
################################################### Set Build Targets ##########
#===============================================================================

## FIXME: This is a continuation of the Windows SDL inelegant hack. This does
## the actual installation of those support libraries.
IF(WIN32)
  INSTALL(PROGRAMS ${SDL_SUPPORT_LIBS} DESTINATION ${LIB_DIR})
ENDIF(WIN32)

#===============================================================================
################################################## Set Project Source ##########
#===============================================================================

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

IF(APPLE)
  INCLUDE_DIRECTORIES(src/MAC)
  FILE(GLOB ARCH_SPECIFIC_SOURCES src/MAC/*.m)
  SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
    src/PCSOUND/pcsound_sdl.cpp
  )
ELSE(APPLE)
  IF(UNIX)
    INCLUDE_DIRECTORIES(src/POSIX)
    FILE(GLOB ARCH_SPECIFIC_SOURCES src/POSIX/*.cpp)
    SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
      src/PCSOUND/pcsound_linux.cpp
    )
  ELSE(UNIX)
    SET(ARCH_SPECIFIC_SOURCES src/SDL/SDL_win32_main.cpp)
    SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
      src/PCSOUND/pcsound_win32.cpp
    )
  ENDIF(UNIX)
ENDIF(APPLE)

INCLUDE_DIRECTORIES(src/SDL)
SET(SDL_SOURCES
  src/SDL/i_joy.cpp
  src/SDL/i_main.cpp
  src/SDL/i_network.cpp
  src/SDL/i_sound.cpp
  src/SDL/i_sshot.cpp
  src/SDL/i_system.cpp
  src/SDL/i_video.cpp
)

INCLUDE_DIRECTORIES(src/PCSOUND)
SET(PCSOUND_SOURCES src/PCSOUND/pcsound.cpp)

INCLUDE_DIRECTORIES(src/textscreen)
FILE(GLOB TEXTSCREEN_SOURCES src/TEXTSCREEN/*.c)

INCLUDE_DIRECTORIES(src/MUSIC)
FILE(GLOB MUSIC_SOURCES src/MUSIC/*.cpp)

INCLUDE_DIRECTORIES(src)
SET(PRBOOM_SOURCES
  src/am_map.cpp
  src/d_deh.cpp
  src/d_ipxgate.cpp
  src/d_items.cpp
  src/d_main.cpp
  src/d_server.cpp
  src/doomdef.cpp
  src/doomstat.cpp
  src/dstrings.cpp
  src/e6y.cpp
  src/e6y_launcher.cpp
  src/f_finale.cpp
  src/f_wipe.cpp
  src/g_game.cpp
  src/g_overflow.cpp
  src/hu_lib.cpp
  src/hu_stuff.cpp
  src/hu_tracers.cpp
  src/i_capture.cpp
  src/i_pcsound.cpp
  src/i_smp.cpp
  src/info.cpp
  src/lprintf.cpp
  src/m_argv.cpp
  src/m_bbox.cpp
  src/m_cheat.cpp
  src/m_menu.cpp
  src/m_misc.cpp
  src/m_random.cpp
  src/md5.cpp
  src/p_ceilng.cpp
  src/p_checksum.cpp
  src/p_doors.cpp
  src/p_enemy.cpp
  src/p_floor.cpp
  src/p_genlin.cpp
  src/p_inter.cpp
  src/p_lights.cpp
  src/p_map.cpp
  src/p_maputl.cpp
  src/p_mobj.cpp
  src/p_plats.cpp
  src/p_pspr.cpp
  src/p_saveg.cpp
  src/p_setup.cpp
  src/p_sight.cpp
  src/p_spec.cpp
  src/p_switch.cpp
  src/p_telept.cpp
  src/p_tick.cpp
  src/p_user.cpp
  src/r_bsp.cpp
  src/r_data.cpp
  src/r_demo.cpp
  src/r_draw.cpp
  src/r_filter.cpp
  src/r_fps.cpp
  src/r_main.cpp
  src/r_patch.cpp
  src/r_plane.cpp
  src/r_screenmultiply.cpp
  src/r_segs.cpp
  src/r_sky.cpp
  src/r_things.cpp
  src/s_advsound.cpp
  src/s_sound.cpp
  src/sc_man.cpp
  src/sounds.cpp
  src/st_lib.cpp
  src/st_stuff.cpp
  src/tables.cpp
  src/v_video.cpp
  src/version.cpp
  src/w_wad.cpp
  src/wi_stuff.cpp
  src/z_bmalloc.cpp
  src/z_zone.cpp
)

SET(SERVER_SOURCES src/d_server.cpp)

SET(CLIENT_SOURCES src/d_client.cpp)

SET(GL_SOURCES
  src/gl_clipper.cpp
  src/gl_detail.cpp
  src/gl_drawinfo.cpp
  src/gl_fbo.cpp
  src/gl_gamma.cpp
  src/gl_hires.cpp
  src/gl_hqresize.cpp
  src/gl_light.cpp
  src/gl_main.cpp
  src/gl_map.cpp
  src/gl_missingtexture.cpp
  src/gl_opengl.cpp
  src/gl_preprocess.cpp
  src/gl_shader.cpp
  src/gl_shadow.cpp
  src/gl_sky.cpp
  src/gl_soft2gl.cpp
  src/gl_texture.cpp
  src/gl_vertex.cpp
  src/gl_wipe.cpp
)

IF(HAVE_MMAP)
  SET(WAD_SOURCES src/w_mmap.cpp)
ELSE(HAVE_MMAP)
  SET(WAD_SOURCES src/w_memcache.cpp)
ENDIF(HAVE_MMAP)

SET(MUS2MID_SOURCES src/memio.cpp src/mus2mid.cpp)

SET(PRBOOM_CLIENT_SOURCES
  ${ARCH_SPECIFIC_SOURCES}
  ${SDL_SOURCES}
  ${PCSOUND_SOURCES}
  ${TEXTSCREEN_SOURCES}
  ${MUSIC_SOURCES}
  ${PRBOOM_SOURCES}
  ${CLIENT_SOURCES}
  ${WAD_SOURCES}
)

IF(GL_DOOM)
  SET(PRBOOM_CLIENT_SOURCES ${PRBOOM_CLIENT_SOURCES} ${GL_SOURCES})
ENDIF(GL_DOOM)

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
ENDIF(GL_DOOM)

ADD_EXECUTABLE(prboom-plus ${PRBOOM_CLIENT_SOURCES})
ADD_EXECUTABLE(prboom-plus-game-server ${PRBOOM_SERVER_SOURCES})

TARGET_LINK_LIBRARIES(prboom-plus
  ${SDL_LIBRARY}
  ${SDLMIXER_LIBRARY}
  ${SDLNET_LIBRARY}
  ${SDLIMAGE_LIBRARY}
  ${FLUIDSYNTH_LIBRARY}
  ${PORTMIDI_LIBRARY}
  ${OGG_LIBRARY}
  ${VORBIS_LIBRARY}
  ${MAD_LIBRARY}
  ${PCREPOSIX_LIBRARY}
  ${PNG_LIBRARY}
  ${ZLIB_LIBRARY}
)

TARGET_LINK_LIBRARIES(prboom-plus-game-server
  ${SDL_LIBRARY}
  ${SDLMIXER_LIBRARY}
  ${SDLNET_LIBRARY}
  ${SDLIMAGE_LIBRARY}
  ${FLUIDSYNTH_LIBRARY}
  ${PORTMIDI_LIBRARY}
  ${OGG_LIBRARY}
  ${VORBIS_LIBRARY}
  ${MAD_LIBRARY}
  ${PCREPOSIX_LIBRARY}
  ${PNG_LIBRARY}
  ${ZLIB_LIBRARY}
)

INSTALL(
  TARGETS prboom-plus prboom-plus-game-server
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR}
)

# vi: ai:ts=2:sw=2:et


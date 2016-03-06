#==============================================================================
############################################################# License #########
#==============================================================================
#
#  Copyright (c) 2010 Jamie Jones <jamie_jones_au@yahoo.com.au>
#  Copyright (C) 2013 David Hill
#  Copyright (C) 2014 Charles Gunyon <charles.gunyon@gmail.com>
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
#==============================================================================

#==============================================================================
################################################# CMake Configuration #########
#==============================================================================
PROJECT("D2K" C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)
IF(${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  MESSAGE(${CMAKE_SOURCE_DIR})
  MESSAGE(${CMAKE_BINARY_DIR})
  MESSAGE(FATAL_ERROR "In-tree Builds are NOT supported.")
ENDIF()

IF(WIN32)
  SET(CMAKE_MFC_FLAG 2)
  SET(CMAKE_INSTALL_MFC_LIBRARIES 1)
  INCLUDE(InstallRequiredSystemLibraries)
ENDIF()

###
# Supported Build Types are:
#  * Debug (CMAKE_C_FLAGS_DEBUG)
#  * Release (CMAKE_C_FLAGS_RELEASE)
# If no build type requested, default to Release
###
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
INCLUDE(CMakeDependentOption)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#==============================================================================
########################################################### Functions #########
#==============================================================================
FUNCTION(TRY_C_COMPILER_FLAG flag name)
  CHECK_C_COMPILER_FLAG("${flag}" ${name})
  IF(${name})
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}" PARENT_SCOPE)
  ENDIF()
ENDFUNCTION(TRY_C_COMPILER_FLAG flag name)

#==============================================================================
##################################################### Package Details #########
#==============================================================================
SET(VERSION "0")
SET(PACKAGE "D2K")
SET(PACKAGE_NAME "D2K")
SET(PACKAGE_VERSION "${VERSION}")
# SET(PACKAGE_BUGREPORT "huh")
SET(PACKAGE_STRING "${PACKAGE_NAME} v${PACKAGE_VERSION}")
SET(PACKAGE_TARNAME "d2k")
SET(PACKAGE_URL "http://github.com/camgunz/d2k")

SET(CPACK_GENERATOR "DEB;RPM;STGZ;ZIP")
SET(CPACK_PACKAGE_VENDOR "Team TotalTrash")
SET(CPACK_PACKAGE_CONTACT "Team TotalTrash <ladna@totaltrash.org>")
SET(CPACK_PACKAGE_VERSION "${VERSION}")
SET(CPACK_PACKAGE_NAME "D2K")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "D2K is a Doom Source Port for the 21st Century"
)
SET(CPACK_DEBIAN_PACKAGE_SECTION "games")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS
  "libc6 (>= 2.11.0),
   libgcc1 (>= 1:4.4.3),
   libglib2.0-0 (>= 2.32),
   libsdl1.2debian (>=1.2.14),
   libsdl-mixer1.2 (>=1.2.8),
   libsdl-net1.2 (>=1.2.7),
   libsdl-image1.2 (>=1.2.12),
   libpangocairo (>= 1.36),
   libportmidi0 (>= 1:184-2.1),
   libfluidsynth1 (>= 1.1.5-2),
   libdumb1 (>= 1:0.9.3-5.4),
   libgl1,
   libxdiff (>= 0.23),
   libenet1a (>= 1.3.3-2),
   bash"
)
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS
  "boom-wad or doom-wad,
   timidity,
   game-data-packager"
)

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
  CPACK_PACKAGE_FILE_NAME
  "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}_${BUILD_ARCH}"
)
SET(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_FILE_NAME}")
SET(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_FILE_NAME}")

INCLUDE(CPack)

IF(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  SET(CLANG TRUE)
ELSE()
  SET(CLANG FALSE)
ENDIF()

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  SET(GCC TRUE)
ELSE()
  SET(GCC FALSE)
ENDIF()

IF(CLANG OR GCC)
  SET(GCC_OR_CLANG TRUE)
ELSE()
  SET(GCC_OR_CLANG FALSE)
ENDIF()

#==============================================================================
##################################################### Platform Checks #########
#==============================================================================

IF(MINGW)
  SET(CMAKE_REQUIRED_DEFINITIONS
    ${CMAKE_REQUIRED_DEFINITIONS} "-D__USE_MINGW_ANSI_STDIO=1"
  )
ENDIF()

CHECK_INCLUDE_FILES("asm/byteorder.h" HAVE_ASM_BYTEORDER_H)
CHECK_INCLUDE_FILES("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES("stdbool.h" HAVE_STDBOOL_H)
CHECK_INCLUDE_FILES("stdint.h" HAVE_STDINT_H)
CHECK_INCLUDE_FILES("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES("sys/wait.h" HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILES("unistd.h" HAVE_UNISTD_H)

CHECK_SYMBOL_EXISTS(sys_siglist signal.h HAVE_DECL_SYS_SIGLIST)

CHECK_C_SOURCE_COMPILES(
  "struct __attribute__((packed)) foo { int a; } b; int main() { return 0; }"
  HAVE_ATTRIBUTE_PACKED
)
IF(HAVE_ATTRIBUTE_PACKED)
  SET(PACKEDATTR "__attribute__((packed))")
ELSE()
  MESSAGE(FATAL_ERROR "Can't figure out how to pack structs with your compiler")
ENDIF()

CHECK_FUNCTION_EXISTS("max" MAX)
CHECK_FUNCTION_EXISTS("min" MIN)
IF(MINGW OR WIN32)
  CHECK_SYMBOL_EXISTS(MapViewOfFile windows.h HAVE_MMAP)
ELSE()
  CHECK_FUNCTION_EXISTS("mmap" HAVE_MMAP)
ENDIF()
CHECK_FUNCTION_EXISTS("sched_setaffinity" HAVE_SCHED_SETAFFINITY)
CHECK_FUNCTION_EXISTS("usleep" HAVE_USLEEP)

CHECK_SYMBOL_EXISTS(_stricmp string.h HAVE__STRICMP)
CHECK_SYMBOL_EXISTS(stricmp string.h HAVE_STRICMP)
CHECK_SYMBOL_EXISTS(strcasecmp string.h HAVE_STRCASECMP)
CHECK_SYMBOL_EXISTS(_strnicmp string.h HAVE__STRNICMP)
CHECK_SYMBOL_EXISTS(strnicmp string.h HAVE_STRNICMP)
CHECK_SYMBOL_EXISTS(strncasecmp string.h HAVE_STRNCASECMP)
CHECK_LIBRARY_EXISTS("GL" "glFinish" "" HAVE_LIBGL)
CHECK_LIBRARY_EXISTS("GLU" "gluBuild2DMipmaps" "" HAVE_LIBGLU)
CHECK_LIBRARY_EXISTS("GLESv2" "glFinish" "" HAVE_LIBGLESv2)
CHECK_LIBRARY_EXISTS("SDL" "SDL_JoystickGetAxis" "" HAVE_SDL_JOYSTICKGETAXIS)

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

CHECK_SYMBOL_EXISTS(_snprintf string.h FOUND_string__SNPRINTF)
IF(FOUND_string_SNPRINTF)
  SET(snprintf _snprintf)
ELSE()
  CHECK_SYMBOL_EXISTS(snprintf string.h FOUND_string_SNPRINTF)
  IF(FOUND_string_SNPRINTF)
    SET(snprintf snprintf)
  ELSE()
    CHECK_SYMBOL_EXISTS(_snprintf stdio.h FOUND_stdio__SNPRINTF)
    IF(FOUND_stdio_SNPRINTF)
      SET(snprintf _snprintf)
    ELSE()
      CHECK_SYMBOL_EXISTS(snprintf stdio.h FOUND_stdio_SNPRINTF)
      IF(FOUND_stdio_SNPRINTF)
        SET(snprintf snprintf)
      ELSE()
        MESSAGE(FATAL_ERROR "Neither snprintf nor _snprintf found")
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()

CHECK_SYMBOL_EXISTS(_vsnprintf string.h FOUND_string__VSNPRINTF)
IF(FOUND_string_VSNPRINTF)
  SET(vsnprintf _vsnprintf)
ELSE()
  CHECK_SYMBOL_EXISTS(vsnprintf string.h FOUND_string_VSNPRINTF)
  IF(FOUND_string_VSNPRINTF)
    SET(vsnprintf vsnprintf)
  ELSE()
    CHECK_SYMBOL_EXISTS(_vsnprintf stdio.h FOUND_stdio__VSNPRINTF)
    IF(FOUND_stdio_VSNPRINTF)
      SET(vsnprintf _vsnprintf)
    ELSE()
      CHECK_SYMBOL_EXISTS(vsnprintf stdio.h FOUND_stdio_VSNPRINTF)
      IF(FOUND_stdio_VSNPRINTF)
        SET(vsnprintf vsnprintf)
      ELSE()
        MESSAGE(FATAL_ERROR "Neither vsnprintf nor _vsnprintf found")
      ENDIF()
    ENDIF()
  ENDIF()
ENDIF()

#==============================================================================
############################################################# Options #########
#==============================================================================
OPTION(DOOMWADDIR, "The path where Doom WADs are stored" "")
OPTION(
  CHECK_HEAP "Exhaustively run memory checks while the game is running" OFF
)
OPTION(HEAPDUMP "Dump the heap on a crash" OFF)
OPTION(RANGECHECK "Enable internal range checking" OFF)
OPTION(
  INSTRUMENTED
  "Enable real-time memory allocation statistics and enable extra debugging features"
  OFF
)
OPTION(
  TIMEDIAG
  "Create timestamps whenever a lump is locked; report when lumps are locked for long periods of time"
  OFF
)
OPTION(
  SIMPLECHECKS
  "Enable quick checks which only impose significant overhead if a possible error is detected"
  OFF
)
OPTION(
  ZONEIDCHECK
  "Perform ID checks on zone blocks to detect corrupted and illegally freed blocks"
  OFF
)
OPTION(GL_DOOM "Build with OpenGL support" ON)
CMAKE_DEPENDENT_OPTION(USE_GLU_IMAGESCALE "Use gluImageScale" OFF USE_GLU OFF)
CMAKE_DEPENDENT_OPTION(USE_GLU_MIPMAP "Use gluBuild2DMipmaps" OFF USE_GLU OFF)
CMAKE_DEPENDENT_OPTION(USE_GLU_TESS "Use gluTesselator" OFF USE_GLU OFF)
OPTION(DOGS "Enable MBF helper dogs" ON)
OPTION(HIGHRES "Enable high resolution support" ON)
OPTION(
  ONLY_FREE_GRAPHICS "Build prboom.wad without non-free menu text lumps" ON
)
OPTION(PROFILE "Enable profiling; requires a debug build" OFF)
OPTION(LEVELINFO_DEBUG "Debug using level info" OFF)
OPTION(DEBUG_WEAPON_CODEPOINTERS "Debug weapon codepointers" OFF)
OPTION(SANITIZE_ADDRESSES "Use Clang's address sanitizer, requires clang" OFF)
OPTION(ENABLE_OVERLAY "Enable new HUD widget overlay" OFF)

IF(NOT DOOMWADDIR)
  SET(DOOMWADDIR "${CMAKE_INSTALL_PREFIX}/share/games/doom")
ENDIF()

#==============================================================================
################################################## Compiler: Features #########
#==============================================================================
IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  TRY_C_COMPILER_FLAG(--std=c99 FLAG_C_stdc99)
  IF(PROFILE)
    TRY_C_COMPILER_FLAG(-pg FLAG_C_profile)
  ENDIF()
ENDIF()

#==============================================================================
################################################## Compiler: Warnings #########
#==============================================================================
IF(GCC_OR_CLANG)
  TRY_C_COMPILER_FLAG(-Wall FLAG_C_Wall)
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

#==============================================================================
################################################# Compiler: Hardening #########
#==============================================================================
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  ADD_DEFINITIONS(-DDEBUG)
  IF(CLANG AND SANITIZE_ADDRESSES)
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-optimize-sibling-calls")
    TRY_C_COMPILER_FLAG(-fno-omit-frame-pointer FLAG_C_NO_OMIT_FRAME_POINTER)
  ELSEIF(GCC)
    TRY_C_COMPILER_FLAG(-fstack-protector-strong FLAG_C_fstackprotector)
  ENDIF()
ELSE()
  IF(GCC_OR_CLANG)
    TRY_C_COMPILER_FLAG(-fstack-protector FLAG_C_fstack_protector)

    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      # TRY_C_COMPILER_FLAG(-fPIC              FLAG_C_fPIC)
      # TRY_C_COMPILER_FLAG(-pie               FLAG_C_pie)
      # TRY_C_COMPILER_FLAG(-Wl,-z,relro       FLAG_C_Wl_z_relro)
      # TRY_C_COMPILER_FLAG(-Wl,-z,now         FLAG_C_Wl_z_now)
      # TRY_C_COMPILER_FLAG(-Wl,--as-needed    FLAG_C_Wl_as_needed)
      # TRY_C_COMPILER_FLAG(-Wl,-z,noexecstack FLAG_C_Wl_z_noexecstack)
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

#==============================================================================
############################################## Compiler: Optimization #########
#==============================================================================
IF(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR
   CMAKE_BUILD_TYPE STREQUAL "Release")
  IF(GCC_OR_CLANG)
    TRY_C_COMPILER_FLAG(-fomit-frame-pointer FLAG_C_fomit_frame_pointer)
    TRY_C_COMPILER_FLAG(-O3 FLAG_C_optimizations)
  ENDIF()

  IF(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    TRY_C_COMPILER_FLAG(-fomit-frame-pointer FLAG_C_fomit_frame_pointer)
    TRY_C_COMPILER_FLAG(-ipo                 FLAG_C_ipo)
  ENDIF()

ENDIF()

#==============================================================================
################################################### Lua Configuration #########
#==============================================================================
# IF(WIN32)
#   ADD_DEFINITIONS(-DLUA_USE_WINDOWS)
# ELSEIF(APPLE)
#   IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
#     ADD_DEFINITIONS(-DLUA_USE_MACOSX)
#   ENDIF()
# ELSEIF(UNIX)
#   IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#     ADD_DEFINITIONS(-DLUA_USE_LINUX)
#   ENDIF()
# ENDIF()

#==============================================================================
################################################## Build Dependencies #########
#==============================================================================
# Build Lua Static library
# SET(LUA_LIBRARY lua)
# SET(LUA_SOURCE_DIRECTORY src/lua/src)
# SET(LUA_SOURCES
#   # Base
#   ${LUA_SOURCE_DIRECTORY}/lapi.c
#   ${LUA_SOURCE_DIRECTORY}/lcode.c
#   ${LUA_SOURCE_DIRECTORY}/lctype.c
#   ${LUA_SOURCE_DIRECTORY}/ldebug.c
#   ${LUA_SOURCE_DIRECTORY}/ldo.c
#   ${LUA_SOURCE_DIRECTORY}/ldump.c
#   ${LUA_SOURCE_DIRECTORY}/lfunc.c
#   ${LUA_SOURCE_DIRECTORY}/lgc.c
#   ${LUA_SOURCE_DIRECTORY}/llex.c
#   ${LUA_SOURCE_DIRECTORY}/lmem.c
#   ${LUA_SOURCE_DIRECTORY}/lobject.c
#   ${LUA_SOURCE_DIRECTORY}/lopcodes.c
#   ${LUA_SOURCE_DIRECTORY}/lparser.c
#   ${LUA_SOURCE_DIRECTORY}/lstate.c
#   ${LUA_SOURCE_DIRECTORY}/lstring.c
#   ${LUA_SOURCE_DIRECTORY}/ltable.c
#   ${LUA_SOURCE_DIRECTORY}/ltm.c
#   ${LUA_SOURCE_DIRECTORY}/lundump.c
#   ${LUA_SOURCE_DIRECTORY}/lvm.c
#   ${LUA_SOURCE_DIRECTORY}/lzio.c
#   # Lib [AM] We might not need all this...
#   ${LUA_SOURCE_DIRECTORY}/lauxlib.c
#   ${LUA_SOURCE_DIRECTORY}/lbaselib.c
#   ${LUA_SOURCE_DIRECTORY}/lbitlib.c
#   ${LUA_SOURCE_DIRECTORY}/lcorolib.c
#   ${LUA_SOURCE_DIRECTORY}/ldblib.c
#   ${LUA_SOURCE_DIRECTORY}/liolib.c
#   ${LUA_SOURCE_DIRECTORY}/lmathlib.c
#   ${LUA_SOURCE_DIRECTORY}/loslib.c
#   ${LUA_SOURCE_DIRECTORY}/lstrlib.c
#   ${LUA_SOURCE_DIRECTORY}/ltablib.c
#   ${LUA_SOURCE_DIRECTORY}/loadlib.c
#   ${LUA_SOURCE_DIRECTORY}/linit.c)
# 
# INCLUDE_DIRECTORIES(${LUA_SOURCE_DIRECTORY})
# ADD_LIBRARY(${LUA_LIBRARY} STATIC ${LUA_SOURCES})

#==============================================================================
################################################### Find Dependencies #########
#==============================================================================
IF(ICONV_LIBRARIES)
  SET(SAVED_ICONV_LIBRARIES ${ICONV_LIBRARIES})
ENDIF()
FIND_PACKAGE(Iconv REQUIRED)
INCLUDE_DIRECTORIES(${ICONV_INCLUDE_DIR})
IF(SAVED_ICONV_LIBRARIES)
  SET(ICONV_LIBRARIES ${SAVED_ICONV_LIBRARIES})
ENDIF()

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

IF(PNG_LIBRARIES)
  SET(SAVED_PNG_LIBRARIES ${PNG_LIBRARIES})
ENDIF()
FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})
IF(SAVED_PNG_LIBRARIES)
  SET(PNG_LIBRARIES ${SAVED_PNG_LIBRARIES})
ENDIF()
SET(HAVE_LIBPNG TRUE)

FIND_PACKAGE(FFI REQUIRED)
INCLUDE_DIRECTORIES(${FFI_INCLUDE_DIR})

# IF(WIN32)
#   FIND_PACKAGE(INTL REQUIRED)
#   INCLUDE_DIRECTORIES(${INTL_INCLUDE_DIR})
# ENDIF()

FIND_PACKAGE(PCREPOSIX REQUIRED)
INCLUDE_DIRECTORIES(${PCRE_INCLUDE_DIR})
SET(HAVE_LIBPCREPOSIX TRUE)

IF(WIN32)
  FIND_PACKAGE(GLib REQUIRED COMPONENTS
    gio
    gmodule
    gobject
    gthread
  )
  INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})
ELSE()
  FIND_PACKAGE(GLib REQUIRED COMPONENTS
    gio
    gmodule
    gobject
    gthread
    gio-unix
  )
  INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(${GIO_UNIX_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(Fontconfig REQUIRED)
INCLUDE_DIRECTORIES(${FONTCONFIG_INCLUDE_DIR})

IF(FREETYPE_LIBRARIES)
  SET(SAVED_FREETYPE_LIBRARIES ${FREETYPE_LIBRARIES})
ENDIF()
FIND_PACKAGE(FreeType REQUIRED)
INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})
IF(SAVED_FREETYPE_LIBRARIES)
  SET(FREETYPE_LIBRARIES ${SAVED_FREETYPE_LIBRARIES})
ENDIF()

FIND_PACKAGE(HarfBuzz REQUIRED)
INCLUDE_DIRECTORIES(${HARFBUZZ_INCLUDE_DIR})

IF(EXPAT_LIBRARIES)
  SET(SAVED_EXPAT_LIBRARIES ${EXPAT_LIBRARIES})
ENDIF()
FIND_PACKAGE(EXPAT REQUIRED)
INCLUDE_DIRECTORIES(${EXPAT_INCLUDE_DIR})
IF(SAVED_EXPAT_LIBRARIES)
  SET(EXPAT_LIBRARIES ${SAVED_EXPAT_LIBRARIES})
ENDIF()

FIND_PACKAGE(Pixman REQUIRED)
INCLUDE_DIRECTORIES(${PIXMAN_INCLUDE_DIR})

FIND_PACKAGE(Cairo REQUIRED)
INCLUDE_DIRECTORIES(${CAIRO_INCLUDE_DIR})

IF(MINGW OR WIN32)
  FIND_PACKAGE(Pango REQUIRED COMPONENTS
    pangoft2
    pangowin32
    pangocairo
  )
ELSEIF(APPLE)
  FIND_PACKAGE(Pango REQUIRED COMPONENTS
    pangoft2
    pangocairo
  )
ELSE()
  FIND_PACKAGE(Pango REQUIRED COMPONENTS
    pangoft2
    pangoxft
    pangocairo
  )
ENDIF()
INCLUDE_DIRECTORIES(${PANGO_INCLUDE_DIR})

FIND_PACKAGE(GObjectIntrospection REQUIRED)
INCLUDE_DIRECTORIES(${GOBJECT_INTROSPECTION_INCLUDE_DIR})

# FIND_PACKAGE(Lua52 REQUIRED)
# INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})

FIND_PACKAGE(Lua REQUIRED)
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})

FIND_PACKAGE(LGI REQUIRED)

FIND_PACKAGE(XDiff REQUIRED)
INCLUDE_DIRECTORIES(${XDIFF_INCLUDE_DIR})

FIND_PACKAGE(ENet REQUIRED)
INCLUDE_DIRECTORIES(${ENET_INCLUDE_DIR})

FIND_PACKAGE(SDL REQUIRED)
SET(HAVE_LIBSDL TRUE)
INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR})

FIND_PACKAGE(SDL_mixer REQUIRED)
SET(HAVE_LIBSDL_MIXER TRUE)
SET(HAVE_MIXER TRUE)
INCLUDE_DIRECTORIES(${SDLMIXER_INCLUDE_DIR})

FIND_PACKAGE(OpenGL)
IF(OPENGL_FOUND)
  INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
ELSE()
  IF(GL_DOOM)
    MESSAGE(FATAL_ERROR "OpenGL not found")
  ELSE()
    MESSAGE(WARNING "OpenGL not found")
  ENDIF()
ENDIF()

FIND_PACKAGE(LZMA)
IF(LZMA_FOUND)
  INCLUDE_DIRECTORIES(${LZMA_INCLUDE_DIRS})
ELSE()
  MESSAGE(FATAL_ERROR "LZMA (XZ) not found")
ENDIF()

IF(JPEG_LIBRARIES)
  SET(SAVED_JPEG_LIBRARIES ${JPEG_LIBRARIES})
ENDIF()

FIND_PACKAGE(JPEG)
IF(JPEG_FOUND)
  INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
  IF(SAVED_JPEG_LIBRARIES)
    SET(JPEG_LIBRARIES ${SAVED_JPEG_LIBRARIES})
  ENDIF()
  SET(HAVE_LIBJPG TRUE)
ELSE()
  MESSAGE(WARNING "JPEG library not found")
ENDIF()

IF(TIFF_LIBRARIES)
  SET(SAVED_TIFF_LIBRARIES ${TIFF_LIBRARIES})
ENDIF()

FIND_PACKAGE(TIFF)
IF(TIFF_FOUND)
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
  IF(SAVED_TIFF_LIBRARIES)
    SET(TIFF_LIBRARIES ${SAVED_TIFF_LIBRARIES})
  ENDIF()
  SET(HAVE_LIBTIFF TRUE)
ELSE()
  MESSAGE(WARNING "LibTIFF not found")
ENDIF()

FIND_PACKAGE(WebP)
IF(WEBP_FOUND)
  INCLUDE_DIRECTORIES(${WEBP_INCLUDE_DIR})
  SET(HAVE_WEBP TRUE)
ELSE()
  MESSAGE(WARNING "WebP not found")
ENDIF()

FIND_PACKAGE(SDL_image)
IF(SDLIMAGE_FOUND)
  INCLUDE_DIRECTORIES(${SDLIMAGE_INCLUDE_DIR})
  SET(HAVE_LIBSDL_IMAGE TRUE)
ELSE()
  MESSAGE(WARNING "SDL_image not found")
  SET(HAVE_LIBSDL_IMAGE FALSE)
ENDIF()

FIND_PACKAGE(Dumb)
IF(DUMB_FOUND)
  INCLUDE_DIRECTORIES(${DUMB_INCLUDE_DIR})
  SET(HAVE_LIBDUMB TRUE)
ELSE()
  MESSAGE(WARNING "Dumb not found")
ENDIF()

FIND_PACKAGE(PortMidi)
IF(PORTMIDI_FOUND)
  INCLUDE_DIRECTORIES(${PORTMIDI_INCLUDE_DIR})
  SET(HAVE_LIBPORTMIDI TRUE)
ELSE()
  MESSAGE(WARNING "PortMidi not found")
ENDIF()

FIND_PACKAGE(FluidSynth)
IF(FLUIDSYNTH_FOUND)
  INCLUDE_DIRECTORIES(${FLUIDSYNTH_INCLUDE_DIR})
  SET(HAVE_LIBFLUIDSYNTH TRUE)
ELSE()
  MESSAGE(WARNING "FluidSynth not found")
ENDIF()

FIND_PACKAGE(MikMod)
IF(MIKMOD_FOUND)
  INCLUDE_DIRECTORIES(${MIKMOD_INCLUDE_DIR})
ELSE()
  MESSAGE(WARNING "MikMod not found")
ENDIF()

FIND_PACKAGE(Mad)
IF(MAD_FOUND)
  INCLUDE_DIRECTORIES(${MAD_INCLUDE_DIR})
  SET(HAVE_LIBMAD TRUE)
ELSE()
  MESSAGE(WARNING "Mad not found")
ENDIF()

FIND_PACKAGE(Vorbis)
IF(VORBIS_FOUND)
  INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR})
  SET(HAVE_LIBVORBISFILE TRUE)
ELSE()
  MESSAGE(WARNING "Vorbis not found")
ENDIF()

FIND_PACKAGE(Ogg)
IF(OGG_FOUND)
  INCLUDE_DIRECTORIES(${OGG_INCLUDE_DIR})
ELSE()
  MESSAGE(WARNING "Ogg not found")
ENDIF()

FIND_PACKAGE(FLAC)
IF(FLAC_FOUND)
  INCLUDE_DIRECTORIES(${FLAC_INCLUDE_DIR})
ELSE()
  MESSAGE(WARNING "FLAC not found")
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
  FIND_PACKAGE(GPerf)
  # MESSAGE(STATUS "Looking for Google Performance Tools")
  # FIND_LIBRARY(GPERF_LIBRARIES profiler HINTS $ENV{GPERF_DIR})
  IF(${GPERF_LIBRARIES} STREQUAL "GPERF_LIBRARIES-NOTFOUND")
    IF(PROFILE)
      MESSAGE(
        WARNING
        "Google Performance Tools not found, profiling will not work"
      )
    ENDIF()
  ENDIF()
ENDIF()

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

#==============================================================================
################################################## Set Project Source #########
#==============================================================================

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ICONS)

IF(GCC_OR_CLANG)
  ADD_DEFINITIONS(-D_GNU_SOURCE)
ENDIF()

IF(MINGW)
  ADD_DEFINITIONS(-D_GNU_SOURCE)
  ADD_DEFINITIONS(-D__USE_MINGW_ANSI_STDIO=1)
  # ADD_DEFINITIONS(-DPCRE_STATIC)
  # ADD_DEFINITIONS(-DMIKMOD_STATIC)
  # ADD_DEFINITIONS(-DFLUIDSYNTH_NOT_A_DLL)
  TRY_C_COMPILER_FLAG(-mno-ms-bitfields FLAG_C_nomsbitfields)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/SDL)
SET(SDL_SOURCES
  ${CMAKE_SOURCE_DIR}/src/SDL/i_font.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_input.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_joy.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_emusic.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_mouse.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_music.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_network.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_sound.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_sshot.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_video.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/PCSOUND)
SET(PCSOUND_SOURCES
  ${CMAKE_SOURCE_DIR}/src/PCSOUND/pcsound.c
  ${CMAKE_SOURCE_DIR}/src/PCSOUND/pcsound_sdl.c
)

IF(APPLE)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/MAC)
  FILE(GLOB ARCH_SPECIFIC_SOURCES ${CMAKE_SOURCE_DIR}/src/MAC/*.m)
ELSE()
  IF(UNIX)
    SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/PCSOUND/pcsound_linux.c
      ${CMAKE_SOURCE_DIR}/src/n_uds.c
    )
  ELSE()
    # SET(ARCH_SPECIFIC_SOURCES ${CMAKE_SOURCE_DIR}/src/SDL/SDL_win32_main.c)
    SET(ARCH_SPECIFIC_SOURCES ${ARCH_SPECIFIC_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/PCSOUND/pcsound_win32.c
    )
  ENDIF()
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/textscreen)
FILE(GLOB TEXTSCREEN_SOURCES ${CMAKE_SOURCE_DIR}/src/TEXTSCREEN/*.c)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/MUSIC)
FILE(GLOB MUSIC_SOURCES ${CMAKE_SOURCE_DIR}/src/MUSIC/*.c)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
SET(D2K_SOURCES
  ${CMAKE_SOURCE_DIR}/src/am_map.c
  ${CMAKE_SOURCE_DIR}/src/c_eci.c
  ${CMAKE_SOURCE_DIR}/src/c_main.c
  ${CMAKE_SOURCE_DIR}/src/cl_cmd.c
  ${CMAKE_SOURCE_DIR}/src/cl_main.c
  ${CMAKE_SOURCE_DIR}/src/cmp.c
  ${CMAKE_SOURCE_DIR}/src/d_deh.c
  ${CMAKE_SOURCE_DIR}/src/d_items.c
  ${CMAKE_SOURCE_DIR}/src/d_main.c
  ${CMAKE_SOURCE_DIR}/src/d_msg.c
  ${CMAKE_SOURCE_DIR}/src/doomdef.c
  ${CMAKE_SOURCE_DIR}/src/doomstat.c
  ${CMAKE_SOURCE_DIR}/src/dstrings.c
  ${CMAKE_SOURCE_DIR}/src/e6y.c
  ${CMAKE_SOURCE_DIR}/src/e6y_launcher.c
  ${CMAKE_SOURCE_DIR}/src/f_finale.c
  ${CMAKE_SOURCE_DIR}/src/f_wipe.c
  ${CMAKE_SOURCE_DIR}/src/g_game.c
  ${CMAKE_SOURCE_DIR}/src/g_keys.c
  ${CMAKE_SOURCE_DIR}/src/g_overflow.c
  ${CMAKE_SOURCE_DIR}/src/g_save.c
  ${CMAKE_SOURCE_DIR}/src/hu_lib.c
  ${CMAKE_SOURCE_DIR}/src/hu_stuff.c
  ${CMAKE_SOURCE_DIR}/src/hu_tracers.c
  ${CMAKE_SOURCE_DIR}/src/i_capture.c
  ${CMAKE_SOURCE_DIR}/src/i_pcsound.c
  ${CMAKE_SOURCE_DIR}/src/i_smp.c
  ${CMAKE_SOURCE_DIR}/src/info.c
  ${CMAKE_SOURCE_DIR}/src/m_avg.c
  ${CMAKE_SOURCE_DIR}/src/m_bbox.c
  ${CMAKE_SOURCE_DIR}/src/m_buf.c
  ${CMAKE_SOURCE_DIR}/src/m_cheat.c
  ${CMAKE_SOURCE_DIR}/src/m_delta.c
  ${CMAKE_SOURCE_DIR}/src/m_file.c
  ${CMAKE_SOURCE_DIR}/src/m_menu.c
  ${CMAKE_SOURCE_DIR}/src/m_misc.c
  ${CMAKE_SOURCE_DIR}/src/m_pbuf.c
  ${CMAKE_SOURCE_DIR}/src/m_random.c
  ${CMAKE_SOURCE_DIR}/src/md5.c
  ${CMAKE_SOURCE_DIR}/src/n_main.c
  ${CMAKE_SOURCE_DIR}/src/n_net.c
  ${CMAKE_SOURCE_DIR}/src/n_pack.c
  ${CMAKE_SOURCE_DIR}/src/n_peer.c
  ${CMAKE_SOURCE_DIR}/src/n_proto.c
  ${CMAKE_SOURCE_DIR}/src/n_state.c
  ${CMAKE_SOURCE_DIR}/src/p_ceilng.c
  ${CMAKE_SOURCE_DIR}/src/p_checksum.c
  ${CMAKE_SOURCE_DIR}/src/p_cmd.c
  ${CMAKE_SOURCE_DIR}/src/p_doors.c
  ${CMAKE_SOURCE_DIR}/src/p_enemy.c
  ${CMAKE_SOURCE_DIR}/src/p_floor.c
  ${CMAKE_SOURCE_DIR}/src/p_genlin.c
  ${CMAKE_SOURCE_DIR}/src/p_ident.c
  ${CMAKE_SOURCE_DIR}/src/p_inter.c
  ${CMAKE_SOURCE_DIR}/src/p_lights.c
  ${CMAKE_SOURCE_DIR}/src/p_map.c
  ${CMAKE_SOURCE_DIR}/src/p_maputl.c
  ${CMAKE_SOURCE_DIR}/src/p_mobj.c
  ${CMAKE_SOURCE_DIR}/src/p_plats.c
  ${CMAKE_SOURCE_DIR}/src/p_pspr.c
  ${CMAKE_SOURCE_DIR}/src/p_saveg.c
  ${CMAKE_SOURCE_DIR}/src/p_setup.c
  ${CMAKE_SOURCE_DIR}/src/p_sight.c
  ${CMAKE_SOURCE_DIR}/src/p_spec.c
  ${CMAKE_SOURCE_DIR}/src/p_switch.c
  ${CMAKE_SOURCE_DIR}/src/p_telept.c
  ${CMAKE_SOURCE_DIR}/src/p_tick.c
  ${CMAKE_SOURCE_DIR}/src/p_user.c
  ${CMAKE_SOURCE_DIR}/src/r_bsp.c
  ${CMAKE_SOURCE_DIR}/src/r_data.c
  ${CMAKE_SOURCE_DIR}/src/r_demo.c
  ${CMAKE_SOURCE_DIR}/src/r_draw.c
  ${CMAKE_SOURCE_DIR}/src/r_filter.c
  ${CMAKE_SOURCE_DIR}/src/r_fps.c
  ${CMAKE_SOURCE_DIR}/src/r_main.c
  ${CMAKE_SOURCE_DIR}/src/r_patch.c
  ${CMAKE_SOURCE_DIR}/src/r_plane.c
  ${CMAKE_SOURCE_DIR}/src/r_screenmultiply.c
  ${CMAKE_SOURCE_DIR}/src/r_segs.c
  ${CMAKE_SOURCE_DIR}/src/r_sky.c
  ${CMAKE_SOURCE_DIR}/src/r_things.c
  ${CMAKE_SOURCE_DIR}/src/s_advsound.c
  ${CMAKE_SOURCE_DIR}/src/s_sndnew.c
  ${CMAKE_SOURCE_DIR}/src/s_sndold.c
  ${CMAKE_SOURCE_DIR}/src/s_sound.c
  ${CMAKE_SOURCE_DIR}/src/sc_man.c
  ${CMAKE_SOURCE_DIR}/src/sounds.c
  ${CMAKE_SOURCE_DIR}/src/st_lib.c
  ${CMAKE_SOURCE_DIR}/src/st_stuff.c
  ${CMAKE_SOURCE_DIR}/src/sv_main.c
  ${CMAKE_SOURCE_DIR}/src/tables.c
  ${CMAKE_SOURCE_DIR}/src/v_overlay.c
  ${CMAKE_SOURCE_DIR}/src/v_video.c
  ${CMAKE_SOURCE_DIR}/src/version.c
  ${CMAKE_SOURCE_DIR}/src/w_wad.c
  ${CMAKE_SOURCE_DIR}/src/wi_stuff.c
  ${CMAKE_SOURCE_DIR}/src/x_main.c
  ${CMAKE_SOURCE_DIR}/src/z_bmalloc.c
  ${CMAKE_SOURCE_DIR}/src/z_zone.c
  ${CMAKE_SOURCE_DIR}/src/xam_main.c
  ${CMAKE_SOURCE_DIR}/src/xc_main.c
  ${CMAKE_SOURCE_DIR}/src/xcl_main.c
  ${CMAKE_SOURCE_DIR}/src/xd_msg.c
  ${CMAKE_SOURCE_DIR}/src/xd_main.c
  ${CMAKE_SOURCE_DIR}/src/xg_game.c
  ${CMAKE_SOURCE_DIR}/src/xg_keys.c
  ${CMAKE_SOURCE_DIR}/src/xi_input.c
  ${CMAKE_SOURCE_DIR}/src/xi_main.c
  ${CMAKE_SOURCE_DIR}/src/xm_menu.c
  ${CMAKE_SOURCE_DIR}/src/xm_misc.c
  ${CMAKE_SOURCE_DIR}/src/xn_main.c
  ${CMAKE_SOURCE_DIR}/src/xp_user.c
  ${CMAKE_SOURCE_DIR}/src/xst_main.c
  ${CMAKE_SOURCE_DIR}/src/xv_main.c
)

SET(CLIENT_SOURCES
  ${CMAKE_SOURCE_DIR}/src/SDL/i_main.c
  ${CMAKE_SOURCE_DIR}/src/SDL/i_system.c
  ${CMAKE_SOURCE_DIR}/src/m_argv.c
)

SET(GL_SOURCES
  ${CMAKE_SOURCE_DIR}/src/gl_clipper.c
  ${CMAKE_SOURCE_DIR}/src/gl_detail.c
  ${CMAKE_SOURCE_DIR}/src/gl_drawinfo.c
  ${CMAKE_SOURCE_DIR}/src/gl_fbo.c
  ${CMAKE_SOURCE_DIR}/src/gl_gamma.c
  ${CMAKE_SOURCE_DIR}/src/gl_hires.c
  ${CMAKE_SOURCE_DIR}/src/gl_hqresize.c
  ${CMAKE_SOURCE_DIR}/src/gl_light.c
  ${CMAKE_SOURCE_DIR}/src/gl_main.c
  ${CMAKE_SOURCE_DIR}/src/gl_map.c
  ${CMAKE_SOURCE_DIR}/src/gl_missingtexture.c
  ${CMAKE_SOURCE_DIR}/src/gl_opengl.c
  ${CMAKE_SOURCE_DIR}/src/gl_preprocess.c
  ${CMAKE_SOURCE_DIR}/src/gl_shader.c
  ${CMAKE_SOURCE_DIR}/src/gl_shadow.c
  ${CMAKE_SOURCE_DIR}/src/gl_sky.c
  ${CMAKE_SOURCE_DIR}/src/gl_soft2gl.c
  ${CMAKE_SOURCE_DIR}/src/gl_texture.c
  ${CMAKE_SOURCE_DIR}/src/gl_vertex.c
  ${CMAKE_SOURCE_DIR}/src/gl_wipe.c
)

IF(HAVE_MMAP)
  SET(WAD_SOURCES ${CMAKE_SOURCE_DIR}/src/w_mmap.c)
ELSE()
  SET(WAD_SOURCES ${CMAKE_SOURCE_DIR}/src/w_memcache.c)
ENDIF()

SET(MUS2MID_SOURCES
  ${CMAKE_SOURCE_DIR}/src/memio.c
  ${CMAKE_SOURCE_DIR}/src/mus2mid.c
)

SET(D2K_CLIENT_SOURCES
  ${ARCH_SPECIFIC_SOURCES}
  ${SDL_SOURCES}
  ${PCSOUND_SOURCES}
  ${TEXTSCREEN_SOURCES}
  ${MUSIC_SOURCES}
  ${D2K_SOURCES}
  ${CLIENT_SOURCES}
  ${MUS2MID_SOURCES}
  ${WAD_SOURCES}
)

IF(GL_DOOM)
  SET(D2K_CLIENT_SOURCES ${D2K_CLIENT_SOURCES} ${GL_SOURCES})
ENDIF()

FILE(GLOB TESTING_SOURCES ${CMAKE_SOURCE_DIR}/unittests/*.c)

SET(TESTING_SOURCES ${TESTING_SOURCES} ${CMAKE_SOURCE_DIR}/cutest/CuTest.c)

SET(D2K_TESTING_SOURCES ${D2K_CLIENT_SOURCES} ${TESTING_SOURCES})

SET(D2K_LIBRARIES "-lm")

IF(GPERF_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${GPERF_LIBRARIES})
ENDIF()

IF(FLUIDSYNTH_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${FLUIDSYNTH_LIBRARIES})
ENDIF()

IF(PORTMIDI_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${PORTMIDI_LIBRARIES})
ENDIF()

IF(DUMB_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${DUMB_LIBRARIES})
ENDIF()

SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${SDLMIXER_LIBRARY})

IF(FLAC_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${FLAC_LIBRARIES})
ENDIF()

IF(VORBIS_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${VORBIS_LIBRARIES})
ENDIF()

IF(OGG_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${OGG_LIBRARIES})
ENDIF()

IF(MAD_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${MAD_LIBRARIES})
ENDIF()

IF(MIKMOD_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${MIKMOD_LIBRARIES})
ENDIF()

IF(SDLIMAGE_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${SDLIMAGE_LIBRARY})
ENDIF()

IF(WEBP_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${WEBP_LIBRARIES})
ENDIF()

IF(TIFF_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${TIFF_LIBRARIES})
ENDIF()

IF(JPEG_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${JPEG_LIBRARIES})
ENDIF()

IF(LZMA_FOUND)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${LZMA_LIBRARIES})
ENDIF()

SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${SDL_LIBRARY})

IF(GL_DOOM)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${OPENGL_gl_LIBRARY})
  IF(USE_GLU_IMAGESCALE OR USE_GLU_MIPMAP OR USE_GLU_TESS)
    SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${OPENGL_glu_LIBRARY})
  ENDIF()
ENDIF()

SET(D2K_LIBRARIES ${D2K_LIBRARIES}
  ${ENET_LIBRARIES}
  ${XDIFF_LIBRARIES}
  ${PANGOCAIRO_LIBRARIES}
  ${PANGOFT2_LIBRARIES}
)

IF(WIN32)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${PANGOWIN32_LIBRARIES})
ELSEIF(NOT APPLE)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${PANGOXFT_LIBRARIES})
ENDIF()

SET(D2K_LIBRARIES ${D2K_LIBRARIES}
  ${PANGO_LIBRARIES}
  ${CAIRO_LIBRARIES}
  ${PIXMAN_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${HARFBUZZ_LIBRARIES}
  ${FONTCONFIG_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${GLIB_GTHREAD_LIBRARIES}
  ${GLIB_GOBJECT_LIBRARIES}
  ${GLIB_GMODULE_LIBRARIES}
  ${GLIB_GIO_LIBRARIES}
  ${GLIB_LIBRARIES}
  ${GOBJECT_INTROSPECTION_LIBRARIES}
  ${PCRE_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${ARCH_SPECIFIC_LIBRARIES}
)

IF(WIN32)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES}
    ${INTL_LIBRARIES}
  )
ENDIF()

SET(D2K_LIBRARIES ${D2K_LIBRARIES}
  ${FFI_LIBRARIES}
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARIES}
)

IF(ICONV_STANDALONE)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${ICONV_LIBRARIES})
ENDIF()

SET(D2K_LIBRARIES ${D2K_LIBRARIES} ${LUA_LIBRARIES})

IF(LINUX)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} dl)
ENDIF()

IF(WIN32)
  SET(D2K_LIBRARIES ${D2K_LIBRARIES} 
    gdi32
    shlwapi
    ole32
    ws2_32
    winmm
    ddraw
    dsound
    dxguid
    comctl32
    usp10
  )
ENDIF()

ADD_EXECUTABLE(d2k ${D2K_CLIENT_SOURCES})
TARGET_LINK_LIBRARIES(d2k ${D2K_LIBRARIES})

# INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/unittests)
# INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cutest)
# ADD_DEFINITIONS(-DRUNNING_UNIT_TESTS)
# ADD_EXECUTABLE(test-d2k ${D2K_TESTING_SOURCES})
# TARGET_LINK_LIBRARIES(test-d2k ${D2K_LIBRARIES})

SET(BIN_DIR "${PREFIX}/bin")
SET(LIB_DIR "${PREFIX}/lib")
SET(INCLUDE_DIR "${PREFIX}/include")
SET(SHARE_DIR "${PREFIX}/share")

# TODO: Need to run test-d2k and use deutex to build the WAD here

INSTALL(
  TARGETS d2k
  RUNTIME DESTINATION ${BIN_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  ARCHIVE DESTINATION ${LIB_DIR}
)

# vi: ai:ts=2:sw=2:et


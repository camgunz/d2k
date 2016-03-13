/*****************************************************************************/
/* D2K: A Doom Source Port for the 21st Century                              */
/*                                                                           */
/* Copyright (C) 2014: See COPYRIGHT file                                    */
/*                                                                           */
/* This file is part of D2K.                                                 */
/*                                                                           */
/* D2K is free software: you can redistribute it and/or modify it under the  */
/* terms of the GNU General Public License as published by the Free Software */
/* Foundation, either version 2 of the License, or (at your option) any      */
/* later version.                                                            */
/*                                                                           */
/* D2K is distributed in the hope that it will be useful, but WITHOUT ANY    */
/* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS */
/* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    */
/* details.                                                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with D2K.  If not, see <http://www.gnu.org/licenses/>.                    */
/*                                                                           */
/*****************************************************************************/


/**/
/* Define to the full name of this package. */
#define PACKAGE_NAME "PrBoom-Plus"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "prboom-plus"

/* Define to the version of this package. */
#define PACKAGE_VERSION "2.5.1.4"

#ifdef DEBUG

/* Define to enable internal range checking */
#define RANGECHECK 1

/* Define this to see real-time memory allocation
 * statistics, and enable extra debugging features
 */
#define INSTRUMENTED 1

/* Uncomment this to exhaustively run memory checks
 * while the game is running (this is EXTREMELY slow).
 * Only useful if INSTRUMENTED is also defined.
 */
#define CHECKHEAP 1

/* Uncomment this to cause heap dumps to be generated.
 * Only useful if INSTRUMENTED is also defined.
 */
#define HEAPDUMP 1

/* Uncomment this to perform id checks on zone blocks,
 * to detect corrupted and illegally freed blocks
 */
#define ZONEIDCHECK 1

/* CPhipps - some debugging macros for the new wad lump handling code */
/* Defining this causes quick checks which only impose an overhead if a
 *  posible error is detected. */
#define SIMPLECHECKS 1

/* Defining this causes time stamps to be created each time a lump is locked, and
 *  lumps locked for long periods of time are reported */
#define TIMEDIAG 1

#endif // DEBUG

#define DOGS 1
#define MONITOR_VISIBILITY 1
/*#define DISABLE_LUMP_CACHING*/

/**/
/*#define HAVE_MIXER 1*/
#define HAVE_NET 1
#define USE_SDL_NET 1

/* Define if you have the SDL image library -lSDL_image */
#define HAVE_LIBSDL_IMAGE

/* Define if you want to use FBO for some tricks in OpenGL */
#define USE_FBO_TECHNIQUE 1

/* Define if you want to use hardware shaders in OpenGL */
#define USE_SHADERS 1

/**/
#define HIGHRES 1
#define GL_DOOM 1
#define USE_GLU_TESS 1
#define USE_GLU_IMAGESCALE 1
#define USE_GLU_MIPMAP 1
#define DISABLE_DOUBLEBUFFER

/**/
#define STDC_HEADERS 1

#define stricmp strcasecmp
#define strnicmp strncasecmp

#define HAVE_INET_ATON 1
#define HAVE_INET_NTOP 1
#define HAVE_INET_PTON 1
#define HAVE_SETSOCKOPT 1

#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1

#define HAVE_MKSTEMPS 1

#define HAVE_IPv6 1

#define HAVE_UNISTD_H
#define HAVE_SYS_WAIT_H
#define HAVE_GETOPT
/* causes a duplicate define warning
#define HAVE_NETINET_IN_H
*/
#define HAVE_DECL_SYS_SIGLIST 1

/**/
#ifdef __BIG_ENDIAN__
#define WORDS_BIGENDIAN
#endif

#ifdef __i386__
#define I386_ASM 1
#endif

#define PACKEDATTR __attribute__((packed))

#define MACOSX
#define HAVE_LIBKERN_OSBYTEORDER_H
#define HAVE_OWN_MUSIC
#define UPDATE_MUSIC
#define SCREENSHOT_DIR I_DoomExeDir()
#define HEAPDUMP_DIR I_DoomExeDir()

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
typedef unsigned char byte;
#endif

/* vi: set et ts=2 sw=2: */


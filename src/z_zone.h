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


#ifndef Z_ZONE_H__
#define Z_ZONE_H__

#ifdef __GNUC__

#ifdef __MINGW32__
#define PRINTF_DECL(x, y) __attribute__((format(gnu_printf, x, y)))
#else
#define PRINTF_DECL(x, y) __attribute__((format(printf, x, y)))
#endif
#else
#define __attribute__(x)
#define PRINTF_DECL(x, y)
#endif

// Include system definitions so that prototypes become
// active before macro replacements below are in effect.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define __STDC_FORMAT_MACROS

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <direct.h>
#include <commctrl.h>
#include <io.h>
#include <process.h>
#include <stddef.h>
#include <winreg.h>
#include <delayimp.h>
#define    F_OK    0    /* Check for file existence */
#define    W_OK    2    /* Check for write permission */
#define    R_OK    4    /* Check for read permission */
#else
#include <dirent.h>
#include <libgen.h>
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <iconv.h>
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#ifdef HAVE_ASM_BYTEORDER_H
#include <asm/byteorder.h>
#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#if defined(HAVE_MMAP) && !defined(_WIN32)
#include <sys/mman.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

// ZONE MEMORY
// PU - purge tags.

enum {PU_FREE, PU_STATIC, PU_SOUND, PU_MUSIC, PU_LEVEL, PU_LEVSPEC, PU_CACHE,
      /* Must always be last -- killough */ PU_MAX};

#define PU_PURGELEVEL PU_CACHE        /* First purgable tag's level */

#ifdef INSTRUMENTED
#define DA(x,y) ,x,y
#define DAC(x,y) x,y
#else
#define DA(x,y)
#define DAC(x,y)
#endif

void *(Z_Malloc)(size_t size, int tag, void **ptr DA(const char *, int));
void (Z_Free)(void *ptr DA(const char *, int));
void (Z_FreeTags)(int lowtag, int hightag DA(const char *, int));
void (Z_ChangeTag)(void *ptr, int tag DA(const char *, int));
void (Z_Init)(void);
void Z_Close(void);
void *(Z_Calloc)(size_t n, size_t n2, int tag, void **user DA(const char *, int));
void *(Z_Realloc)(void *p, size_t n, int tag, void **user DA(const char *, int));
char *(Z_Strdup)(const char *s, int tag, void **user DA(const char *, int));
void (Z_CheckHeap)(DAC(const char *,int));   // killough 3/22/98: add file/line info
void Z_DumpHistory(char *);

#ifdef INSTRUMENTED
/* cph - save space if not debugging, don't require file
 * and line to memory calls */
#define Z_Free(a)          (Z_Free)     (a,      __FILE__,__LINE__)
#define Z_FreeTags(a,b)    (Z_FreeTags) (a,b,    __FILE__,__LINE__)
#define Z_ChangeTag(a,b)   (Z_ChangeTag)(a,b,    __FILE__,__LINE__)
#define Z_Malloc(a,b,c)    (Z_Malloc)   (a,b,c,  __FILE__,__LINE__)
#define Z_Strdup(a,b,c)    (Z_Strdup)   (a,b,c,  __FILE__,__LINE__)
#define Z_Calloc(a,b,c,d)  (Z_Calloc)   (a,b,c,d,__FILE__,__LINE__)
#define Z_Realloc(a,b,c,d) (Z_Realloc)  (a,b,c,d,__FILE__,__LINE__)
#define Z_CheckHeap()      (Z_CheckHeap)(__FILE__,__LINE__)
#endif

/* cphipps 2001/11/18 -
 * If we're using memory mapped file access to WADs, we won't need to maintain
 * our own heap. So we *could* let "normal" malloc users use the libc malloc
 * directly, for efficiency. Except we do need a wrapper to handle out of memory
 * errors... damn, ok, we'll leave it for now.
 */
#ifndef HAVE_LIBDMALLOC
// Remove all definitions before including system definitions

#undef malloc
#undef free
#undef realloc
#undef calloc
#undef strdup

#define malloc(n)          Z_Malloc(n,PU_STATIC,0)
#define free(p)            Z_Free(p)
#define realloc(p,n)       Z_Realloc(p,n,PU_STATIC,0)
#define calloc(n1,n2)      Z_Calloc(n1,n2,PU_STATIC,0)
#define strdup(s)          Z_Strdup(s,PU_STATIC,0)

#else

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#endif

void Z_ZoneHistory(char *);

/* [CG] Some GCC-specific defines */
#ifdef __GNUC__
#define CONSTFUNC __attribute__((const))
#define PUREFUNC  __attribute__((pure))
#define NORETURN  __attribute__((noreturn))
#else
#define CONSTFUNC
#define PUREFUNC
#define NORETURN
#endif

/* [CG]: Some generic compiler defines */

#ifdef WIN32
#define C_DECL __cdecl
#else
#define C_DECL
#endif

#ifdef _MSC_VER
  #define INLINE __forceinline /* use __forceinline (VC++ specific) */
#else
  #define INLINE inline        /* use standard inline */
#endif

//e6y
#ifndef BETWEEN
#define BETWEEN(l,u,x) ((l)>(x)?(l):(x)>(u)?(u):(x))
#endif

/* [CG]: Include the basic data structures so other stuff can use them */

#include <glib.h>

#include "cmp.h"

#include "d_msg.h"
#include "m_buf.h"
#include "m_fixed.h"
#include "m_pbuf.h"
#include "tables.h"
#include "info.h"

void I_Error(const char *error, ...) PRINTF_DECL(1, 2);

#endif

/* vi: set et ts=2 sw=2: */


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


#ifndef DOOMTYPE_H__
#define DOOMTYPE_H__

#ifndef BYTEBOOL__
#define BYTEBOOL__

/* Fixed to use builtin bool type with C++. */
#ifdef __cplusplus
typedef bool dboolean;
#elif !defined(HAVE_STDBOOL_H)
typedef enum {false, true} dboolean;
#else
typedef int dboolean;
#endif

#ifdef uint8_t // CG
typedef uint8_t byte;
#else
typedef unsigned char byte;
#endif

#endif // BYTEBOOL__

//e6y
#ifndef BETWEEN
#define BETWEEN(l,u,x) ((l)>(x)?(l):(x)>(u)?(u):(x))
#endif

/* cph - Wrapper for the long long type, as Win32 used a different name.
 * Except I don't know what to test as it's compiler specific
 * Proff - I fixed it */
#ifndef _MSC_VER
#ifdef HAVE_STDINT_H
typedef int64_t int_64_t;
typedef uint64_t uint_64_t;
#else
typedef signed long long int_64_t;
typedef unsigned long long uint_64_t;
#endif
// define compiled-specific long-long contstant notation here
#define LONGLONG(num)   (uint_64_t)num ## ll
#else
typedef __int64 int_64_t;
typedef unsigned __int64 uint_64_t;
// define compiled-specific long-long contstant notation here
#define LONGLONG(num) (uint_64_t)num
#undef PATH_MAX
#define PATH_MAX 1024
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define S_ISDIR(x) (((sbuf.st_mode & S_IFDIR)==S_IFDIR)?1:0)
#endif

#ifdef __GNUC__
#define CONSTFUNC __attribute__((const))
#define PUREFUNC  __attribute__((pure))
#define NORETURN  __attribute__((noreturn))
#else
#define CONSTFUNC
#define PUREFUNC
#define NORETURN
#endif

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

/* cph - move compatibility levels here so we can use them in d_server.c */
typedef enum {
  doom_12_compatibility,   /* Doom v1.2 */
  doom_1666_compatibility, /* Doom v1.666 */
  doom2_19_compatibility,  /* Doom & Doom 2 v1.9 */
  ultdoom_compatibility,   /* Ultimate Doom and Doom95 */
  finaldoom_compatibility,     /* Final Doom */
  dosdoom_compatibility,     /* DosDoom 0.47 */
  tasdoom_compatibility,     /* TASDoom */
  boom_compatibility_compatibility,      /* Boom's compatibility mode */
  boom_201_compatibility,                /* Boom v2.01 */
  boom_202_compatibility,                /* Boom v2.02 */
  lxdoom_1_compatibility,                /* LxDoom v1.3.2+ */
  mbf_compatibility,                     /* MBF */
  prboom_1_compatibility,                /* PrBoom 2.03beta? */
  prboom_2_compatibility,                /* PrBoom 2.1.0-2.1.1 */
  prboom_3_compatibility,                /* PrBoom 2.2.x */
  prboom_4_compatibility,                /* PrBoom 2.3.x */
  prboom_5_compatibility,                /* PrBoom 2.4.0 */
  prboom_6_compatibility,                /* Latest PrBoom */
  MAX_COMPATIBILITY_LEVEL,               /* Must be last entry */
  /* Aliases follow */
  boom_compatibility = boom_201_compatibility, /* Alias used by G_Compatibility */
  best_compatibility = prboom_6_compatibility,
} complevel_t_e;
typedef int complevel_t;

/* cph - from v_video.h, needed by gl_struct.h */
#define VPT_ALIGN_MASK 0xf
#define VPT_STRETCH_MASK 0x1f
enum patch_translation_e {
  // e6y: wide-res
  VPT_ALIGN_LEFT         = 1,
  VPT_ALIGN_RIGHT        = 2,
  VPT_ALIGN_TOP          = 3,
  VPT_ALIGN_LEFT_TOP     = 4,
  VPT_ALIGN_RIGHT_TOP    = 5,
  VPT_ALIGN_BOTTOM       = 6,
  VPT_ALIGN_WIDE         = 7,
  VPT_ALIGN_LEFT_BOTTOM  = 8,
  VPT_ALIGN_RIGHT_BOTTOM = 9,
  VPT_ALIGN_MAX          = 10,
  VPT_STRETCH            = 16, // Stretch to compensate for high-res

  VPT_NONE    = 128, // Normal
  VPT_FLIP    = 256, // Flip image horizontally
  VPT_TRANS   = 512, // Translate image via a translation table
  VPT_NOOFFSET = 1024,
};

#endif

/* vi: set et ts=2 sw=2: */


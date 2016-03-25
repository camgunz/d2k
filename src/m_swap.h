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


#ifndef M_SWAP_H__
#define M_SWAP_H__

#ifdef __GNUG__
#pragma interface
#endif

/* CPhipps - now the endianness handling, converting input or output to/from
 * the machine's endianness to that wanted for this type of I/O
 *
 * To find our own endianness, use config.h
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Endianess handling. */

/* cph - First the macros to do the actual byte swapping */

/* leban
 * rather than continue the confusing tradition of redefining the
 * stardard macro, we now present the doom_ntoh and doom_hton macros....
 * might as well use the xdoom macros.
 */

/* Try to use superfast macros on systems that support them */
#ifdef HAVE_ASM_BYTEORDER_H
#include <asm/byteorder.h>
#ifdef __arch__swab16
#define doom_swap_s  (int16_t)__arch__swab16
#endif
#ifdef __arch__swab32
#define doom_swap_l  (int32_t)__arch__swab32
#endif
#ifdef __arch__swab64
#define doom_swap_ll (int64_t)__arch__swab64
#endif
#endif /* HAVE_ASM_BYTEORDER_H */

#ifdef HAVE_LIBKERN_OSBYTEORDER_H
#include <libkern/OSByteOrder.h>

#define doom_swap_s (short)OSSwapInt16
#define doom_swap_l (long)OSSwapInt32
#ifdef OSSwapInt64
#define doom_swap_ll (long long)OSSwapInt64
#endif
#endif

#ifndef doom_swap_ll
#define doom_swap_ll(x) \
        ((int64_t)((((uint64_t)(x) & 0x00000000000000ffLL) << 56) | \
                   (((uint64_t)(x) & 0x000000000000ff00LL) << 40) | \
                   (((uint64_t)(x) & 0x0000000000ff00ffLL) << 24) | \
                   (((uint64_t)(x) & 0x00000000ff0000ffLL) <<  8) | \
                   (((uint64_t)(x) & 0x000000ff000000ffLL) >>  8) | \
                   (((uint64_t)(x) & 0x0000ff00000000ffLL) >> 24) | \
                   (((uint64_t)(x) & 0x00ff0000000000ffLL) >> 40) | \
                   (((uint64_t)(x) & 0xff000000000000ffLL) >> 56)))
#endif

#ifndef doom_swap_l
#define doom_swap_l(x) \
        ((int32_t)((((unsigned long int)(x) & 0x000000ffU) << 24) | \
                   (((unsigned long int)(x) & 0x0000ff00U) <<  8) | \
                   (((unsigned long int)(x) & 0x00ff0000U) >>  8) | \
                   (((unsigned long int)(x) & 0xff000000U) >> 24)))
#endif

#ifndef doom_swap_s
#define doom_swap_s(x) \
        ((int16_t)((((unsigned short int)(x) & 0x00ff) << 8) | \
                   (((unsigned short int)(x) & 0xff00) >> 8)))
#endif

/* Macros are named doom_XtoYT, where
 * X is thing to convert from, Y is thing to convert to, chosen from
 * n for network, h for host (i.e our machine's), w for WAD (Doom data files)
 * and T is the type, l or s for long or short
 *
 * CPhipps - all WADs and network packets will be little endian for now
 * Use separate macros so network could be converted to big-endian later.
 */

#ifdef WORDS_BIGENDIAN

#define doom_b16(x) (int16_t)(x)
#define doom_b32(x) (int32_t)(x)
#define doom_b64(x) (int64_t)(x)
#define doom_l16(x) doom_swap_s(x)
#define doom_l32(x) doom_swap_l(x)
#define doom_l64(x) doom_swap_ll(x)

#else

#define doom_b16(x) doom_swap_s(x)
#define doom_b32(x) doom_swap_l(x)
#define doom_b64(x) doom_swap_ll(x)
#define doom_l16(x) (int16_t)(x)
#define doom_l32(x) (int32_t)(x)
#define doom_l64(x) (int64_t)(x)

#endif

#define doom_wtohll(x) doom_l64(x)
#define doom_htowll(x) doom_l64(x)
#define doom_wtohl(x)  doom_l32(x) /* CG: Actually used */
#define doom_htowl(x)  doom_l32(x)
#define doom_wtohs(x)  doom_l16(x)
#define doom_htows(x)  doom_l16(x) /* CG: Actually used */

#define doom_ntohll(x) doom_b64(x)
#define doom_htonll(x) doom_b64(x)
#define doom_ntohl(x)  doom_b32(x)
#define doom_htonl(x)  doom_b32(x)
#define doom_ntohs(x)  doom_b16(x)
#define doom_htons(x)  doom_b16(x)

/* CPhipps - Boom's old LONG and SHORT endianness macros are for WAD stuff */

#define LittleLong(x) doom_wtohl(x)
#define LittleShort(x) doom_htows(x)

#endif

/* vi: set et ts=2 sw=2: */


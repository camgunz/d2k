/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Low level UDP network interface. This is shared between the server
 *  and client, with SERVER defined for the former to select some extra
 *  functions. Handles socket creation, and packet send and receive.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef M_UTF_H__
#define M_UTF_H__

typedef char rune;

const char* M_GetUTFError(void);

dboolean    M_IsControlChar(wchar_t sc);

size_t M_DecodeASCII(rune **out, char *in, size_t in_size);

size_t M_EncodeWCHAR(wchar_t **out, rune *in, size_t in_size);
size_t M_DecodeWCHAR(rune **out, wchar_t *in, size_t in_size);
size_t M_DecodeWCHARNoAlloc(rune *out, uint16_t *in,
                            size_t out_size, size_t in_size);

size_t M_EncodeLocal(char **out, rune *in, size_t in_size);
size_t M_DecodeLocal(rune **out, char *in);

#endif

/* vi: set et sw=2 ts=2: */


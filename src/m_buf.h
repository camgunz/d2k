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
 *  A simple buffer
 *
 *-----------------------------------------------------------------------------
 */

#ifndef BUF_H__
#define BUF_H__

typedef struct buf_s {
  size_t size;
  size_t capacity;
  size_t cursor;
  byte *data;
} buf_t;

void     M_BufferInit(buf_t **buf);
void     M_BufferInitWithCapacity(buf_t **buf, size_t capacity);
void     M_BufferCopy(buf_t *dst, buf_t *src);
void     M_BufferSetData(buf_t *buf, byte *data, size_t size);
void     M_BufferSetString(buf_t *buf, byte *data, size_t length);
dboolean M_BufferSetFile(buf_t *buf, const char *filename);
void     M_BufferAppend(buf_t *buf, byte *data, size_t size);
dboolean M_BufferEqualsString(buf_t *buf, const rune *s);
void     M_BufferEnsureCapacity(buf_t *buf, size_t capacity);
void     M_BufferEnsureTotalCapacity(buf_t *buf, size_t capacity);
void     M_BufferCompact(buf_t *buf);
void     M_BufferClear(buf_t *buf);
void     M_BufferFree(buf_t *buf);

#endif

/* vi: set et ts=2 sw=2: */


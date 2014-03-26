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
 *  A buffer of commands
 *
 *-----------------------------------------------------------------------------
 */

#ifndef N_CMDBUF_H__
#define N_CMDBUF_H__

typedef cobjbuf_t cmdbuf_t;

void     N_CmdBufferInit(cmdbuf_t **cmds);
void     N_CmdBufferInitWithCapacity(cmdbuf_t **cmds, size_t capacity);
void     N_CmdBufferAppend(cmdbuf_t *cmds, netticcmd_t *cmd);
void     N_CmdBufferRemoveOld(cmdbuf_t *cmds, int tic);
void     N_CmdBufferClear(cmdbuf_t *cmds);
void     N_CmdBufferFree(cmdbuf_t *cmds);

#endif

/* vi: set et ts=2 sw=2: */


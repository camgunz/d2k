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
 *      Savegame I/O, archiving, persistence.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_SAVEG__
#define __P_SAVEG__

#ifdef __GNUG__
#pragma interface
#endif

/* Persistent storage/archiving.
 * These are the load / save game routines. */
void P_ArchivePlayers(buf_t *savebuffer);
void P_UnArchivePlayers(buf_t *savebuffer);
void P_ArchiveWorld(buf_t *savebuffer);
void P_UnArchiveWorld(buf_t *savebuffer);
void P_ArchiveThinkers(buf_t *savebuffer);
void P_UnArchiveThinkers(buf_t *savebuffer);
void P_ArchiveSpecials(buf_t *savebuffer);
void P_UnArchiveSpecials(buf_t *savebuffer);

void P_ThinkerToIndex(void); /* phares 9/13/98: save soundtarget in savegame */
void P_IndexToThinker(void); /* phares 9/13/98: save soundtarget in savegame */

/* 1/18/98 killough: add RNG info to savegame */
void P_ArchiveRNG(buf_t *savebuffer);
void P_UnArchiveRNG(buf_t *savebuffer);

/* 2/21/98 killough: add automap info to savegame */
void P_ArchiveMap(buf_t *savebuffer);
void P_UnArchiveMap(buf_t *savebuffer);

#if 0
void CheckSaveGame(size_t,const char*, int);              /* killough */
#define CheckSaveGame(a) (CheckSaveGame)(a, __FILE__, __LINE__)
#endif

#endif

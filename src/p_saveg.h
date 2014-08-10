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


#ifndef P_SAVEG_H__
#define P_SAVEG_H__

#ifdef __GNUG__
#pragma interface
#endif

/* Persistent storage/archiving.
 * These are the load / save game routines. */
void P_ArchivePlayers(pbuf_t *savebuffer);
void P_UnArchivePlayers(pbuf_t *savebuffer);
void P_ArchiveWorld(pbuf_t *savebuffer);
void P_UnArchiveWorld(pbuf_t *savebuffer);
void P_ArchiveThinkers(pbuf_t *savebuffer);
void P_UnArchiveThinkers(pbuf_t *savebuffer);
void P_ArchiveSpecials(pbuf_t *savebuffer);
void P_UnArchiveSpecials(pbuf_t *savebuffer);
/* 1/18/98 killough: add RNG info to savegame */
void P_ArchiveRNG(pbuf_t *savebuffer);
void P_UnArchiveRNG(pbuf_t *savebuffer);
/* 2/21/98 killough: add automap info to savegame */
void P_ArchiveMap(pbuf_t *savebuffer);
void P_UnArchiveMap(pbuf_t *savebuffer);

#endif

/* vi: set et ts=2 sw=2: */


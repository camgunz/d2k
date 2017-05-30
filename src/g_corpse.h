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


#ifndef G_CORPSE_H__
#define G_CORPSE_H__

struct mobj_s;
typedef struct mobj_s mobj_t;

bool   G_CorpseQueueInitialized(void);
void   G_CorpseQueueInit(void);
void   G_CorpseQueueClear(void);
void   G_CorpseQueuePush(mobj_t *mobj);
void   G_CorpseQueuePop(void);
size_t G_CorpseQueueGetLen(void);
int    G_CorpseQueueGetMaxLen(void);
void   G_CorpseQueueSetMaxLen(int new_max_len);

#endif

/* vi: set et ts=2 sw=2: */
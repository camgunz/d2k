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


#include "z_zone.h"

#include "g_corpse.h"
#include "p_mobj.h"

// killough 2/8/98: make corpse queue variable in size
/*
 * CG 09/25/2014: Use a GQueue for this instead
 *
 * int    bodyqueslot, bodyquesize;        // killough 2/8/98
 * mobj_t **bodyque = 0;                   // phares 8/10/98
 */
static int corpse_queue_initialized;
static int corpse_queue_size; // killough 2/8/98
static GQueue *corpse_queue = NULL;

bool G_CorpseQueueInitialized(void) {
  return corpse_queue_initialized;
}

void G_CorpseQueueInit(void) {
  if (corpse_queue_initialized) {
    I_Error("G_CorpseQueueInit: Corpse queue already initialized");
  }

  corpse_queue = g_queue_new();
}

void G_CorpseQueueClear(void) {
  /*
   * CG: This is only called when loading a new state or map, therefore the
   *     queued corpses don't leak.  This is why the call to P_RemoveMobj or
   *     free is omitted.
   */
  if (corpse_queue_size < 0) {
    I_Error("clear_corpses: corpse_queue_size < 0 (%d)", corpse_queue_size);
  }

  if (corpse_queue) {
    g_queue_free(corpse_queue);
    corpse_queue = NULL;
  }

  if (corpse_queue_size > 0) {
    corpse_queue = g_queue_new();
  }
}

void G_CorpseQueuePush(mobj_t *mobj) {
  g_queue_push_tail(corpse_queue, mobj);
}

void G_CorpseQueuePop(void) {
  P_RemoveMobj(g_queue_pop_head(corpse_queue));
}

int G_CorpseQueueGetMaxLen(void) {
  return corpse_queue_size;
}

void G_CorpseQueueSetMaxLen(int new_max_len) {
  if (new_max_len < 0) {
    I_Error("G_CorpseQueueSetMaxLen: Cannot set corpse queue len < 0\n");
  }

  corpse_queue_size = new_max_len;
}

/* vi: set et ts=2 sw=2: */

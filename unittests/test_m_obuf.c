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
 *  Tests
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include "m_buf.h"
#include "m_cbuf.h"
#include "m_obuf.h"
#include "m_list.h"

#include "g_game.h"
#include "lprintf.h"
#include "n_net.h"

static void set_ncmd(netticcmd_t *ncmd, char n) {
  ncmd->tic             = n;
  ncmd->cmd.forwardmove = n;
  ncmd->cmd.sidemove    = n;
  ncmd->cmd.angleturn   = n;
  ncmd->cmd.consistancy = n;
  ncmd->cmd.chatchar    = n;
  ncmd->cmd.buttons     = n;
}

static void print_ncmd(netticcmd_t *ncmd) {
  printf("%1d, %1d, %1d, %1d, %1d, %1d, %1d\n",
    ncmd->tic,
    ncmd->cmd.forwardmove,
    ncmd->cmd.sidemove,
    ncmd->cmd.angleturn,
    ncmd->cmd.consistancy,
    ncmd->cmd.chatchar,
    ncmd->cmd.buttons
  );
}

static void print_obuf(const char *s, obuf_t *obuf) {
  int index = -1;
  netticcmd_t *ncmd = NULL;

  printf("%s:\n", s);

  printf("[");
  for (int i = 0; i < obuf->capacity; i++) {
    if (obuf->objects[i] == NULL)
      printf(" unused");
    else
      printf(" used");
  }
  printf(" ]\n");

  printf("%d, %d.\n", obuf->capacity, M_OBufGetObjectCount(obuf));

  while (M_OBufIter(obuf, &index, (void **)&ncmd))
    print_ncmd(ncmd);
}

static void test_obuf(void) {
  obuf_t obuf;
  netticcmd_t ncmd1, ncmd2, ncmd3, ncmd4, ncmd5, ncmd6;

  set_ncmd(&ncmd1, 1);
  set_ncmd(&ncmd2, 2);
  set_ncmd(&ncmd3, 3);
  set_ncmd(&ncmd4, 4);
  set_ncmd(&ncmd5, 5);
  set_ncmd(&ncmd6, 6);

  M_OBufInit(&obuf);
  M_OBufAppend(&obuf, &ncmd1);
  M_OBufAppend(&obuf, &ncmd2);
  M_OBufAppend(&obuf, &ncmd3);

  print_obuf("3 commands", &obuf);

  M_OBufRemove(&obuf, 0);
  print_obuf("Removed 1st command", &obuf);

  M_OBufInsert(&obuf, 0, &ncmd1);
  print_obuf("Re-inserted 1st command", &obuf);

  M_OBufAppend(&obuf, &ncmd4);
  M_OBufAppend(&obuf, &ncmd5);
  M_OBufAppend(&obuf, &ncmd6);
  print_obuf("Appended 3 more commands", &obuf);

  M_OBufRemove(&obuf, 0);
  M_OBufRemove(&obuf, 2);
  M_OBufRemove(&obuf, 4);
  print_obuf("Removed 1st, 3rd and 5th command", &obuf);

  M_OBufConsolidate(&obuf);
  print_obuf("Consolidated commands", &obuf);

  M_OBufClear(&obuf);
  print_obuf("Cleared commands", &obuf);

  M_OBufAppend(&obuf, &ncmd1);
  M_OBufAppend(&obuf, &ncmd2);
  M_OBufAppend(&obuf, &ncmd3);
  print_obuf("Appended 3 more commands", &obuf);

  M_OBufFree(&obuf);
  print_obuf("Free'd buffer", &obuf);
}


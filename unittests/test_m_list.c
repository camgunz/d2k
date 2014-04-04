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
#include "m_queue.h"

#include "d_ticcmd.h"
#include "g_game.h"
#include "lprintf.h"

static void set_cmd(ticcmd_t *cmd, int val) {
  cmd->forwardmove = val;
  cmd->sidemove    = val;
  cmd->angleturn   = val;
  cmd->consistancy = val;
  cmd->chatchar    = val;
  cmd->buttons     = val;
}

static void print_cmd(ticcmd_t *cmd) {
  printf("\t[ %d %d %d %d %u %u ]\n",
    cmd->forwardmove,
    cmd->sidemove,
    cmd->angleturn,
    cmd->consistancy,
    cmd->chatchar,
    cmd->buttons
  );
}

static void print_list(const char *message, list_t *ls) {
  printf("%s:\n", message);

  LIST_FOR_EACH(ls, node)
    print_cmd((ticcmd_t *)node->obj);

  printf("%d entries\n", ls->size);
}

void test_m_list(void) {
  ticcmd_t *cmd;
  list_t ls, ls_copy;
  ticcmd_t cmd1, cmd2, cmd3, cmd4, cmd5, cmd6;

  set_cmd(&cmd1, 1);
  set_cmd(&cmd2, 2);
  set_cmd(&cmd3, 3);
  set_cmd(&cmd4, 4);
  set_cmd(&cmd5, 5);
  set_cmd(&cmd6, 6);

  M_ListInit(&ls);
  M_ListInit(&ls_copy);
  print_list("Initialized", &ls);

  M_ListPrepend(&ls, &cmd3);
  print_list("Prepended command 3", &ls);

  M_ListPrepend(&ls, &cmd2);
  print_list("Prepended command 2", &ls);

  M_ListPrepend(&ls, &cmd1);
  print_list("Prepended command 1", &ls);

  M_ListAppend(&ls, &cmd5);
  print_list("Appended command 5", &ls);

  LIST_FOR_EACH(&ls, node) {
    cmd = node->obj;

    if (cmd == &cmd5) {
      M_ListInsertBefore(&ls, node, &cmd4);
      print_list("Inserted command 4 before command 5", &ls);
      M_ListInsertAfter(&ls, node, &cmd6);
      print_list("Inserted command 6 after command 5", &ls);
    }
  }

  printf("3rd node == command 3: %d.\n", M_ListGetIndex(&ls, &cmd3) == 2);
  printf(
    "2nd node == command 2: %d.\n", M_ListGetNode(&ls, &cmd3)->obj == &cmd3
  );

  M_ListCopy(&ls_copy, &ls);
  print_list("Copied", &ls_copy);

  LIST_FOR_EACH(&ls, node) {
    cmd = node->obj;

    if (cmd == &cmd3)
      M_ListRemove(&ls, node);
  }

  print_list("Removed 3rd command", &ls);
  print_list("Copy should stay the same", &ls_copy);

  cmd = M_ListPop(&ls);
  printf("Should be command 1:\n");
  print_cmd(cmd);
  print_list("After pop", &ls);

  cmd = M_ListPopBack(&ls);
  printf("Should be command 6:\n");
  print_cmd(cmd);
  print_list("After pop-back", &ls);

  M_ListPushFront(&ls, &cmd6);
  print_list("Pushed command 6 to the front", &ls);

  M_ListPush(&ls, &cmd1);
  print_list("Pushed command 1 to the back", &ls);

  M_ListFree(&ls);
  print_list("Freed list", &ls);

  M_ListFree(&ls_copy);
  print_list("Freed copy", &ls_copy);
}


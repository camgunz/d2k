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

#include "g_game.h"
#include "lprintf.h"
#include "n_net.h"

static void test_buf(void) {
  buf_t buf1;
  buf_t buf2;
  char d[14] = {
    'B', 'u', 'f', 'f', 'e', 'r', 'S', 'e', 't', 'D', 'a', 't', 'a', 0
  };

  M_BufferInit(&buf1);
  M_BufferInit(&buf2);

  M_BufferSetString(&buf1, "Hello from M_Buffer!", 20);
  printf("%s\n", buf1.data);

  M_BufferCopy(&buf2, &buf1);
  printf("%s\n", buf2.data);

  M_BufferSetData(&buf1, d, 14);
  printf("%s\n", buf1.data);

  printf(
    "M_BufferEqualsString: %d\n", M_BufferEqualsString(&buf1, "BufferSetData")
  );

  printf("M_BufferEnsureCapacity: %lu/%lu/%lu => ",
    buf1.size, buf1.capacity, buf1.cursor
  );
  M_BufferEnsureCapacity(&buf1, 10);
  printf("%lu/%lu/%lu\n", buf1.size, buf1.capacity, buf1.cursor);

  printf("M_BufferEnsureTotalCapacity: %lu/%lu/%lu => ",
    buf1.size, buf1.capacity, buf1.cursor
  );
  M_BufferEnsureTotalCapacity(&buf1, 500);
  printf("%lu/%lu/%lu\n", buf1.size, buf1.capacity, buf1.cursor);

  printf("M_BufferCompact: %lu/%lu/%lu => ",
    buf1.size, buf1.capacity, buf1.cursor
  );
  M_BufferCompact(&buf1);
  printf("%lu/%lu/%lu\n", buf1.size, buf1.capacity, buf1.cursor);

  M_BufferSetFile(&buf1, "doom.h.txt");
  printf("%s\n", buf1.data);

  M_BufferFree(&buf1);
  M_BufferFree(&buf2);
}


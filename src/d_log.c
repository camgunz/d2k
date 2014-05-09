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
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include "d_log.h"
#include "lprintf.h"

static cbuf_t log_files;

static void close_logs(void) {
  CBUF_FOR_EACH(&log_files, entry) {
    FILE *fh = (FILE *)entry.obj;

    fclose(fh);
  }
}

void D_InitLogging(void) {
  M_CBufInitWithCapacity(&log_files, sizeof(FILE *), LOG_MAX);
  atexit(close_logs);
}

void D_EnableLogChannel(log_channel_e channel, const char *filename) {
  FILE *fh = fopen(filename, "w");

  if (fh == NULL)
    I_Error("Error opening log file %s: %s.\n", filename, strerror(errno));

  M_CBufInsert(&log_files, channel, fh);
}

void D_Log(log_channel_e channel, const char *fmt, ...) {
  FILE *fh = M_CBufGet(&log_files, channel);
  va_list args;

  if (fh != NULL) {
    va_start(args, fmt);
    vfprintf(fh, fmt, args);
    va_end(args);
  }
}

/* vi: set et ts=2 sw=2: */


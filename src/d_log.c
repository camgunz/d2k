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

#include "lprintf.h"

static GPtrArray *log_files;

static void close_log(gpointer data) {
  FILE *fh = data;

  if (fh != NULL) {
    fflush(fh);
    fclose(fh);
  }
}

static void close_logs(void) {
  g_ptr_array_free(log_files, true);
}

void D_InitLogging(void) {
  log_files = g_ptr_array_new_full(LOG_MAX, close_log);
  g_ptr_array_set_size(log_files, LOG_MAX);
  atexit(close_logs);
}

void D_EnableLogChannel(log_channel_e channel, const char *filename) {
  FILE *fh = fopen(filename, "w");

  if (fh == NULL)
    I_Error("Error opening log file %s: %s.\n", filename, strerror(errno));

  printf("Inserting %p at %u (%u, %u)\n", fh, channel, log_files->len, LOG_MAX);
  g_ptr_array_insert(log_files, channel, fh);
}

void D_Log(log_channel_e channel, const char *fmt, ...) {
  FILE *fh;
  va_list args;

  if (channel >= LOG_MAX)
    I_Error("D_Log: Invalid channel %d (valid: 0 - %d)", channel, LOG_MAX);
  
  fh = (FILE *)g_ptr_array_index(log_files, channel);

  if (fh == NULL)
    return;

  va_start(args, fmt);
  vfprintf(fh, fmt, args);
  va_end(args);
}

/* vi: set et ts=2 sw=2: */


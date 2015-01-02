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

#include "doomtype.h"
#include "d_event.h"
#include "c_main.h"
#include "lprintf.h"
#include "i_main.h"
#include "i_system.h"
#include "m_argv.h"
#include "e6y.h"//e6y

int cons_error_mask  = -1 - LO_INFO; /* all but LO_INFO when redir'd */
int cons_output_mask = -1;           /* all output enabled */

void lprintf(OutputLevels pri, const char *s, ...) {
  va_list args;

  va_start(args, s);

  if (!C_Initialized())
    vprintf(s, args);
  else if (pri & cons_output_mask)
    C_VPrintf(s, args);

  /* CG: TODO: Removed error mask handling, re-add it? */

  va_end(args);
}

/*
 * I_Error
 *
 * cphipps - moved out of i_* headers, to minimise source files that depend on
 * the low-level headers. All this does is print the error, then call the
 * low-level safe exit function.
 * killough 3/20/98: add const
 */

void I_Error(const char *error, ...) {
  char errmsg[MAX_MESSAGE_LENGTH];
  va_list argptr;

  va_start(argptr,error);
  doom_vsnprintf(errmsg, sizeof(errmsg), error, argptr);
  va_end(argptr);

  lprintf(LO_ERROR, "%s\n", errmsg);
  I_SafeExit(-1);
}

// Wrapper to handle non-standard stdio implementations

int doom_vsnprintf(char *buf, size_t max, const char *fmt, va_list va)
{
  int rv;
  va_list vc;

  assert((max == 0 && buf == NULL) || (max != 0 && buf != NULL));
  assert(fmt != NULL);

  va_copy(vc, va);
  rv = vsnprintf(buf, max, fmt, vc);
  va_end(vc);

  if (rv < 0) // Handle an unhelpful return value.
  {
    // write into a scratch buffer that keeps growing until the output fits
    static char *backbuffer;
    static size_t backsize = 1024;

    for (; rv < 0; backsize *= 2)
    {
      if (backsize <= max) continue;

      backbuffer = (realloc)(backbuffer, backsize);
      assert(backbuffer != NULL);

      va_copy(vc, va);
      rv = vsnprintf(backbuffer, backsize, fmt, vc);
      va_end(vc);
    }

    if (buf)
    {
      size_t end = (size_t) rv >= max ? max-1 : rv;
      memmove(buf, backbuffer, end);
      buf[end] = '\0';
    }
  }

  if (buf && (size_t) rv >= max && buf[max-1]) // ensure null-termination
    buf[max-1] = '\0';

  return rv;
}

int doom_snprintf(char *buf, size_t max, const char *fmt, ...)
{
  int rv;
  va_list va;

  va_start(va, fmt);
  rv = doom_vsnprintf(buf, max, fmt, va);
  va_end(va);

  return rv;
}

/* vi: set et ts=2 sw=2: */


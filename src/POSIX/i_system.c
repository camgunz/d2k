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

#include "m_fixed.h"
#include "i_system.h"
#include "doomdef.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif
#include "i_system.h"

void I_uSleep(unsigned long usecs) {
#ifdef HAVE_USLEEP
  usleep(usecs);
#else
  /* Fall back on select(2) */
  struct timeval tv = { usecs / 1000000, usecs % 1000000 };
  select(0, NULL, NULL, NULL, &tv);
#endif
}

/*
 * CPhipps - believe it or not, it is possible with consecutive calls to
 * gettimeofday to receive times out of order, e.g you query the time twice and
 * the second time is earlier than the first. Cheap'n'cheerful fix here.
 * NOTE: only occurs with bad kernel drivers loaded, e.g. pc speaker drv
 */

static unsigned long lasttimereply;
static unsigned long basetime;

int I_GetTime_RealTime(void) {
  struct timeval tv;
  struct timezone tz;
  unsigned long thistimereply;

  gettimeofday(&tv, &tz);

  thistimereply = (tv.tv_sec * TICRATE + (tv.tv_usec * TICRATE) / 1000000);

  /* Fix for time problem */
  if (!basetime) {
    basetime = thistimereply;
    thistimereply = 0;
  }
  else {
    thistimereply -= basetime;
  }

  if (thistimereply < lasttimereply)
    thistimereply = lasttimereply;

  lasttimereply = thistimereply;

  return lasttimereply;
}

/*
 * I_GetRandomTimeSeed
 *
 * CPhipps - extracted from G_ReloadDefaults because it is O/S based
 */
unsigned long I_GetRandomTimeSeed(void) {
  /* killough 3/26/98: shuffle random seed, use the clock */
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return (tv.tv_sec * 1000ul + tv.tv_usec / 1000ul);
}

/* cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char* I_GetVersionString(char *buf, size_t sz) {
  snprintf(buf, sz, "%s v%s (%s)", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_URL);
  return buf;
}

/* cphipps - I_SigString
 * Returns a string describing a signal number
 */
const char* I_SigString(char* buf, size_t sz, int signum) {
#if HAVE_DECL_SYS_SIGLIST // NSM: automake defines this symbol as 0 or 1
  if (strlen(sys_siglist[signum]) < sz)
    strcpy(buf, sys_siglist[signum]);
  else
#endif
    sprintf(buf, "signal %d", signum);

  return buf;
}

/* vi: set et ts=2 sw=2: */


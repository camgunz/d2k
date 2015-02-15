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


#ifndef LPRINTF_H__
#define LPRINTF_H__

/* cphipps - enlarged message buffer and made non-static
 * We still have to be careful here, this function can be called after exit
 */
#define MAX_MESSAGE_LENGTH 2048

typedef enum                /* Logical output levels */
{
  LO_INFO=1,                /* One of these is used in each physical output    */
  LO_CONFIRM=2,             /* call. Which are output, or echoed to console    */
  LO_WARN=4,                /* if output redirected is determined by the       */
  LO_ERROR=8,               /* global masks: cons_output_mask,cons_error_mask. */
  LO_FATAL=16,
  LO_DEBUG=32,
  LO_ALWAYS=64,
} OutputLevels;

extern int cons_output_mask;
extern int cons_error_mask;

void D_LoadStartupMessagesIntoConsole(void);
void lprintf(OutputLevels pri, const char *fmt, ...) PRINTF_DECL(2, 3);

/* killough 3/20/98: add const
 * killough 4/25/98: add gcc attributes
 * cphipps 01/11- moved from i_system.h */
void I_Error(const char *error, ...) PRINTF_DECL(1, 2);

#ifdef _WIN32
void I_ConTextAttr(unsigned char a);
void I_UpdateConsole(void);
int Init_ConsoleWin(void);
void Done_ConsoleWin(void);
#endif

int doom_vsnprintf(char *buf, size_t max, const char *fmt, va_list va);
int doom_snprintf(char *buf, size_t max, const char *fmt, ...) PRINTF_DECL(3, 4);
#endif

/* vi: set et ts=2 sw=2: */


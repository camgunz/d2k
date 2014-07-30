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


#ifndef D_LOG_H__
#define D_LOG_H__

typedef enum {
  LOG_NET,
  LOG_SYNC,
  LOG_STATE,
  LOG_MEM,
  LOG_MAX
} log_channel_e;

void D_InitLogging(void);
void D_EnableLogChannel(log_channel_e channel, const char *filename);
void D_Log(log_channel_e channel, const char *fmt, ...) PRINTF_DECL(2, 3);

#endif

/* vi: set et ts=2 sw=2: */


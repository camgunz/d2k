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


#ifndef E6Y_LAUNCHER_H__
#define E6Y_LAUNCHER_H__

#ifdef _WIN32

typedef enum
{
  launcher_enable_never,
  launcher_enable_smart,
  launcher_enable_always,

  launcher_enable_count
} launcher_enable_t;
extern launcher_enable_t launcher_enable;
extern const char *launcher_enable_states[];
extern char *launcher_history[10];

void LauncherShow(unsigned int params);

#endif

#endif

/* vi: set et ts=2 sw=2: */


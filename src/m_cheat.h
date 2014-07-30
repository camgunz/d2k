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


#ifndef M_CHEAT_H__
#define M_CHEAT_H__

#define CHEAT(cheat, deh_cheat, when, func, arg) \
  { cheat, deh_cheat, when, func, arg, 0, 0, \
    sizeof(cheat) - 1, 0, 0, 0, "" }

#define CHEAT_ARGS_MAX 8  /* Maximum number of args at end of cheats */

/* killough 4/16/98: Cheat table structure */

typedef struct cheatseq_s {
  const char *	cheat;
  const char *const deh_cheat;
  enum {
    always   = 0,
    not_dm   = 1,
    not_coop = 2,
    not_demo = 4,
    not_menu = 8,
    not_deh = 16,
    not_net = not_dm | not_coop,
    cht_never = not_net | not_demo
  } const when;
  void (*const func)();
  const int arg;
  uint_64_t code, mask;

  // settings for this cheat
  size_t sequence_len;
  size_t deh_sequence_len;

  // state used during the game
  size_t chars_read;
  int param_chars_read;
  char parameter_buf[CHEAT_ARGS_MAX];
} cheatseq_t;

extern cheatseq_t cheat[];

dboolean M_FindCheats(int key);

#endif

/* vi: set et ts=2 sw=2: */


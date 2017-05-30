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


#ifndef I_INPUT_H__
#define I_INPUT_H__

struct event_s;
typedef struct event_s event_t;

typedef struct key_states_s {
  bool shiftdown;
  bool ctrldown;
  bool altdown;
  bool metadown;
} key_states_t;

extern key_states_t key_states;

void        I_InputInit(void);
void        I_InputUpdateKeyStates(void);
void        I_InputHandle(void);
bool        I_Responder(event_t *ev);
const char* I_GetKeyString(int keycode);

#endif

/* vi: set et ts=2 sw=2: */


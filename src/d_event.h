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


#ifndef D_EVENT_H__
#define D_EVENT_H__

//
// Event handling.
//

// Input event types.
typedef enum {
  ev_none,
  ev_key,
  ev_mouse,
  ev_joystick,
  ev_mouse_movement,
  ev_joystick_movement
} evtype_t;

typedef enum {
  ev_joystick_none,
  ev_joystick_axis,
  ev_joystick_ball,
  ev_joystick_hat
} jsevtype_e;

// Event structure.
// CG 01/10/15: Updated to be a little more explanatory
typedef struct event_s {
  int        device_id; // generating device ID
  evtype_t   type;      // event type
  jsevtype_e jstype;    // type of joystick event, axis, ball or hat
  bool       pressed;   // whether or not button/key was pressed
  int        key;       // keys/buttons (kb, mouse, joystick)
  int        value;     // generic value, used for joystick for example
  int        xmove;     // mouse/joystick x move
  int        ymove;     // mouse/joystick y move
  uint16_t   wchar;     // CG 07/15/14: SDL relays Unicode input as UTF-16
} event_t;

//
// Button/action code definitions.
//
typedef enum buttoncode_e {
  // Press "Fire".
  BT_ATTACK       = 1,

  // Use button, to open doors, activate switches.
  BT_USE          = 2,

  // Flag: game events, not really buttons.
  BT_SPECIAL      = 128,
  BT_SPECIALMASK  = 3,

  // Flag, weapon change pending.
  // If true, the next 4 bits hold weapon num.
  BT_CHANGE       = 4,

  // The 4bit weapon mask and shift, convenience.
  BT_WEAPONMASK_OLD   = (8+16+32),//e6y
  BT_WEAPONMASK   = (8+16+32+64), // extended to pick up SSG        // phares
  BT_WEAPONSHIFT  = 3,

  // Special events
  BTS_LOADGAME    = 0, // Loads a game
  // Pause the game.
  BTS_PAUSE       = 1,
  // Save the game at each console.
  BTS_SAVEGAME    = 2,
  BTS_RESTARTLEVEL= 3, // Restarts the current level

  // Savegame slot numbers occupy the second byte of buttons.
  BTS_SAVEMASK    = (4+8+16),
  BTS_SAVESHIFT   = 2,

} buttoncode_t;

#endif

/* vi: set et ts=2 sw=2: */


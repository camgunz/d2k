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


#ifndef ST_STUFF_H__
#define ST_STUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "r_defs.h"

// Size of statusbar.
// Now sensitive for scaling.

// proff 08/18/98: Changed for high-res
#define ST_HEIGHT 32
#define ST_WIDTH  320
#define ST_Y      (200 - ST_HEIGHT)

// e6y: wide-res
extern int ST_SCALED_HEIGHT;
extern int ST_SCALED_WIDTH;
extern int ST_SCALED_Y;

//
// STATUS BAR
//

void ST_SetAutomapEntered(void);
void ST_SetAutomapExited(void);

// Called by main loop.
dboolean ST_Responder(event_t* ev);

// Called by main loop.
void ST_Ticker(void);

// Called by main loop.
void ST_Drawer(dboolean st_statusbaron, dboolean refresh, dboolean fullmenu);

// Called when the console player is spawned on each level.
void ST_Start(void);

// Called by startup code.
void ST_Init(void);

// After changing videomode;
void ST_SetResolution(void);

// States for status bar code.
typedef enum
{
  AutomapState,
  FirstPersonState
} st_stateenum_t;

// States for the chat code.
typedef enum
{
  StartChatState,
  WaitDestState,
  GetChatState
} st_chatstateenum_t;

// killough 5/2/98: moved from m_misc.c:

extern int health_red;    // health amount less than which status is red
extern int health_yellow; // health amount less than which status is yellow
extern int health_green;  // health amount above is blue, below is green
extern int armor_red;     // armor amount less than which status is red
extern int armor_yellow;  // armor amount less than which status is yellow
extern int armor_green;   // armor amount above is blue, below is green
extern int ammo_red;      // ammo percent less than which status is red
extern int ammo_yellow;   // ammo percent less is yellow more green
extern int sts_always_red;// status numbers do not change colors
extern int sts_pct_always_gray;// status percents do not change colors
extern int sts_traditional_keys;  // display keys the traditional way

extern int st_palette;    // cph 2006/04/06 - make palette visible

typedef enum {
  ammo_colour_behaviour_no,
  ammo_colour_behaviour_full_only,
  ammo_colour_behaviour_yes,
  ammo_colour_behaviour_max
} ammo_colour_behaviour_t;
extern ammo_colour_behaviour_t ammo_colour_behaviour;
extern const char *ammo_colour_behaviour_list[];

// e6y: makes sense for wide resolutions
extern patchnum_t grnrock;
extern patchnum_t brdr_t, brdr_b, brdr_l, brdr_r;
extern patchnum_t brdr_tl, brdr_tr, brdr_bl, brdr_br;

#endif

/* vi: set et ts=2 sw=2: */


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


#ifndef DOOMDEF_H__
#define DOOMDEF_H__

/* use config.h if autoconf made one -- josh */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// This must come first, since it redefines malloc(), free(), etc. -- killough:
#include "z_zone.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

// this should go here, not in makefile/configure.ac -- josh
#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "m_swap.h"
#include "version.h"
#include "doomtype.h"

extern dboolean bfgedition;

// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
typedef enum {
  shareware,    // DOOM 1 shareware, E1, M9
  registered,   // DOOM 1 registered, E3, M27
  commercial,   // DOOM 2 retail, E1 M34  (DOOM 2 german edition not handled)
  retail,       // DOOM 1 retail, E4, M36
  indetermined  // Well, no IWAD found.
} GameMode_t;

// Mission packs - might be useful for TC stuff?
typedef enum {
  doom,         // DOOM 1
  doom2,        // DOOM 2
  pack_tnt,     // TNT mission pack
  pack_plut,    // Plutonia pack
  pack_nerve,   // No Rest For The Living
  hacx,         // HACX - Twitch 'n Kill
  chex,         // Chex Quest
  none
} GameMission_t;

// Identify language to use, software localization.
typedef enum {
  english,
  french,
  german,
  unknown
} Language_t;

//
// For resize of screen, at start of game.
//

#define BASE_WIDTH 320

// It is educational but futile to change this
//  scaling e.g. to 2. Drawing of status bar,
//  menues etc. is tied to the scale implied
//  by the graphics.

#define INV_ASPECT_RATIO   0.625 /* 0.75, ideally */

// killough 2/8/98: MAX versions for maximum screen sizes
// allows us to avoid the overhead of dynamic allocation
// when multiple screen sizes are supported

// SCREENWIDTH and SCREENHEIGHT define the visible size
extern int SCREENWIDTH;
extern int SCREENHEIGHT;
// SCREENPITCH is the size of one line in the buffer and
// can be bigger than the SCREENWIDTH depending on the size
// of one pixel (8, 16 or 32 bit) and the padding at the
// end of the line caused by hardware considerations
extern int SCREENPITCH;

extern int REAL_SCREENWIDTH;
extern int REAL_SCREENHEIGHT;
extern int REAL_SCREENPITCH;

// e6y: wide-res
extern int WIDE_SCREENWIDTH;
extern int WIDE_SCREENHEIGHT;
extern int SCREEN_320x200;

// The maximum number of players, multiplayer/networking.
#define MAXPLAYERS       4

// phares 5/14/98:
// DOOM Editor Numbers (aka doomednum in mobj_t)

#define DEN_PLAYER5 4001
#define DEN_PLAYER6 4002
#define DEN_PLAYER7 4003
#define DEN_PLAYER8 4004

// State updates, number of tics / second.
#define TICRATE          35

// The current state of the game: whether we are playing, gazing
// at the intermission screen, the game final animation, or a demo.

typedef enum {
  GS_BAD = -1,
  GS_LEVEL = 0,
  GS_INTERMISSION,
  GS_FINALE,
  GS_DEMOSCREEN
} gamestate_t;

//
// Difficulty/skill settings/filters.
//
// These are Thing flags

// Skill flags.
#define MTF_EASY                1
#define MTF_NORMAL              2
#define MTF_HARD                4
// Deaf monsters/do not react to sound.
#define MTF_AMBUSH              8

/* killough 11/98 */
#define MTF_NOTSINGLE          16
#define MTF_NOTDM              32
#define MTF_NOTCOOP            64
#define MTF_FRIEND            128
#define MTF_RESERVED          256

typedef enum {
  sk_none=-1, //jff 3/24/98 create unpicked skill setting
  sk_baby=0,
  sk_easy,
  sk_medium,
  sk_hard,
  sk_nightmare
} skill_t;

//
// Key cards.
//

typedef enum {
  it_bluecard,
  it_yellowcard,
  it_redcard,
  it_blueskull,
  it_yellowskull,
  it_redskull,
  NUMCARDS
} card_t;

// The defined weapons, including a marker
// indicating user has not changed weapon.
typedef enum {
  wp_fist,
  wp_pistol,
  wp_shotgun,
  wp_chaingun,
  wp_missile,
  wp_plasma,
  wp_bfg,
  wp_chainsaw,
  wp_supershotgun,

  NUMWEAPONS,
  wp_nochange              // No pending weapon change.
} weapontype_t;

// Ammunition types defined.
typedef enum {
  am_clip,    // Pistol / chaingun ammo.
  am_shell,   // Shotgun / double barreled shotgun.
  am_cell,    // Plasma rifle, BFG.
  am_misl,    // Missile launcher.
  NUMAMMO,
  am_noammo   // Unlimited for chainsaw / fist.
} ammotype_t;

// Power up artifacts.
typedef enum {
  pw_invulnerability,
  pw_strength,
  pw_invisibility,
  pw_ironfeet,
  pw_allmap,
  pw_infrared,
  NUMPOWERS
} powertype_t;

// Power up durations (how many seconds till expiration).
typedef enum {
  INVULNTICS  = (30*TICRATE),
  INVISTICS   = (60*TICRATE),
  INFRATICS   = (120*TICRATE),
  IRONTICS    = (60*TICRATE)
} powerduration_t;

// DOOM keyboard definition.
// This is the stuff configured by Setup.Exe.
// Most key data are simple ascii (uppercased).

enum {
  KEYD_LEFTARROW = 1,
  KEYD_UPARROW,
  KEYD_RIGHTARROW,
  KEYD_DOWNARROW,
  KEYD_ESCAPE,
  KEYD_ENTER,
  KEYD_TAB,
  KEYD_F1,
  KEYD_F2,
  KEYD_F3,
  KEYD_F4,
  KEYD_F5,
  KEYD_F6,
  KEYD_F7,
  KEYD_F8,
  KEYD_F9,
  KEYD_F10,
  KEYD_F11,
  KEYD_F12,
  KEYD_BACKSPACE,
  KEYD_PAUSE,
  KEYD_EQUALS,
  KEYD_MINUS,
  KEYD_RSHIFT,
  KEYD_RCTRL,
  KEYD_RALT,
  KEYD_LALT = KEYD_RALT,
  KEYD_RMETA,
  KEYD_RSUPER,
  KEYD_CAPSLOCK,
  KEYD_BACKQUOTE,

// phares 3/2/98:
  KEYD_INSERT,
  KEYD_HOME,
  KEYD_PAGEUP,
  KEYD_PAGEDOWN,
  KEYD_DEL,
  KEYD_END,
  KEYD_SCROLLLOCK,
  KEYD_SPACEBAR,
// phares 3/2/98

  KEYD_NUMLOCK,

// cph - Add the numeric keypad keys, as suggested by krose 4/22/99:
// The way numbers are assigned to keys is a mess, but it's too late to
// change that easily. At least these additions are don neatly.
// Codes 0x100-0x200 are reserved for number pad

  KEYD_KEYPAD0,
  KEYD_KEYPAD1,
  KEYD_KEYPAD2,
  KEYD_KEYPAD3,
  KEYD_KEYPAD4,
  KEYD_KEYPAD5,
  KEYD_KEYPAD6,
  KEYD_KEYPAD7,
  KEYD_KEYPAD8,
  KEYD_KEYPAD9,
  KEYD_KEYPADENTER,
  KEYD_KEYPADDIVIDE,
  KEYD_KEYPADMULTIPLY,
  KEYD_KEYPADMINUS,
  KEYD_KEYPADPLUS,
  KEYD_KEYPADPERIOD,

// haleyjd: virtual keys
  KEYD_MOUSE1,
  KEYD_MOUSE2,
  KEYD_MOUSE3,
  KEYD_MWHEELUP,
  KEYD_MWHEELDOWN
};

// phares 4/19/98:
// Defines Setup Screen groups that config variables appear in.
// Used when resetting the defaults for every item in a Setup group.

typedef enum {
  ss_none,
  ss_keys,
  ss_weap,
  ss_stat,
  ss_auto,
  ss_enem,
  ss_mess,
  ss_chat,
  ss_gen,       /* killough 10/98 */
  ss_comp,      /* killough 10/98 */
  ss_max
} ss_types;

// phares 3/20/98:
//
// Player friction is variable, based on controlling
// linedefs. More friction can create mud, sludge,
// magnetized floors, etc. Less friction can create ice.

#define MORE_FRICTION_MOMENTUM 15000       // mud factor based on momentum
#define ORIG_FRICTION          0xE800      // original value
#define ORIG_FRICTION_FACTOR   2048        // original value
#define FRICTION_FLY           0xeb00

#endif          // __DOOMDEF__

/* vi: set et ts=2 sw=2: */


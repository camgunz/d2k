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

#include "i_main.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_mouse.h"
#include "e6y.h"
#include "g_comp.h"
#include "g_demo.h"
#include "g_input.h"
#include "g_keys.h"
#include "g_game.h"
#include "mn_main.h"
#include "p_camera.h"
#include "pl_main.h"
#include "pl_weap.h"
#include "v_video.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#define MAXPLMOVE   (forwardmove[1])
#define SLOWTURNTICS  6
#define QUICKREVERSE (short)32768 // 180 degree reverse // phares

static int turnheld; // for accelerative turning

// Set to -1 or +1 to switch to the previous or next weapon.
static int next_weapon = 0;

// Used for prev/next weapon keys.
static const struct {
  weapontype_t weapon;
  weapontype_t weapon_num;
} weapon_order_table[] = {
  { wp_fist,         wp_fist     },
  { wp_chainsaw,     wp_fist     },
  { wp_pistol,       wp_pistol   },
  { wp_shotgun,      wp_shotgun  },
  { wp_supershotgun, wp_shotgun  },
  { wp_chaingun,     wp_chaingun },
  { wp_missile,      wp_missile  },
  { wp_plasma,       wp_plasma   },
  { wp_bfg,          wp_bfg      }
};

static int mousearray[6];
static int *mousebuttons = &mousearray[1];    // allow [-1]

// mouse values are used once
static int   mousex;
static int   mousey;
static int   dclicktime;
static int   dclickstate;
static int   dclicks;
static int   dclicktime2;
static int   dclickstate2;
static int   dclicks2;

// joystick values are repeated
static int   joyxmove;
static int   joyymove;
bool  joyarray[9];
bool *joybuttons = &joyarray[1];    // allow [-1]

int upmove;
int autorun = false;      // always running?          // phares
// e6y
// There is a new command-line switch "-shorttics".
// This makes it possible to practice routes and tricks
// (e.g. glides, where this makes a significant difference)
// with the same mouse behaviour as when recording,
// but without having to be recording every time.
int shorttics;

fixed_t forwardmove[2] = {0x19, 0x32};
fixed_t sidemove[2]    = {0x18, 0x28};
fixed_t angleturn[3]   = {640, 1280, 320};  // + slow turn
fixed_t flyspeed[2]    = {1*256, 3*256};

fixed_t forwardmove_normal[2] = {0x19, 0x32};
fixed_t sidemove_normal[2]    = {0x18, 0x28};
fixed_t sidemove_strafe50[2]  = {0x19, 0x32};

// Game events info
buttoncode_t special_event; // Event triggered by local player, to send

static inline signed char fudgef(signed char b) {
/*e6y
  static int c;
  if (!b || !demo_compatibility || longtics) return b;
  if (++c & 0x1f) return b;
  b |= 1; if (b>2) b-=2;*/
  return b;
}

static inline signed short fudgea(signed short b) {
    // e6y
    // There is a new command-line switch "-shorttics".
    // This makes it possible to practice routes and tricks
    // (e.g. glides, where this makes a significant difference)
    // with the same mouse behaviour as when recording,
    // but without having to be recording every time.
/*e6y
  if (!b || !demo_compatibility || !longtics) return b;
  b |= 1; if (b>2) b-=2;*/

  if (shorttics && !demorecording && !demoplayback) {
    return (((b + 128) >> 8) << 8);
  }
  else {
    return b;
  }
}

static bool weapon_selectable(weapontype_t weapon) {
  if (gamemode == shareware) {
    if (weapon == wp_plasma || weapon == wp_bfg) {
      return false;
    }
  }

  // Can't select the super shotgun in Doom 1.
  if (weapon == wp_supershotgun && gamemission == doom) {
    return false;
  }

  // Can't select a weapon if we don't own it.
  if (!P_GetConsolePlayer()->weaponowned[weapon]) {
    return false;
  }

  return true;
}

static int G_NextWeapon(int direction) {
  weapontype_t weapon;
  int i;
  int arrlen;

  // Find index in the table.
  if (P_GetConsolePlayer()->pendingweapon == wp_nochange) {
    weapon = P_GetConsolePlayer()->readyweapon;
  }
  else {
    weapon = P_GetConsolePlayer()->pendingweapon;
  }

  arrlen = sizeof(weapon_order_table) / sizeof(*weapon_order_table);
  for (i = 0; i < arrlen; i++) {
    if (weapon_order_table[i].weapon == weapon) {
      break;
    }
  }

  // Switch weapon.
  do {
    i += direction;
    i = (i + arrlen) % arrlen;
  } while (!weapon_selectable(weapon_order_table[i].weapon));

  return weapon_order_table[i].weapon_num;
}

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
void G_BuildTiccmd(ticcmd_t *cmd) {
  int strafe = false;
  int bstrafe;
  int speed = autorun;
  int tspeed;
  int forward;
  int side;
  int newweapon; // phares
  player_t *player = P_GetConsolePlayer();

  I_StartTic();
  
  if (gamekeydown[key_strafe]) {
    strafe = true;
  }
  else if (mousebuttons[mousebstrafe]) {
    strafe = true;
  }
  else if (joybuttons[joybstrafe]) {
    strafe = true;
  }

  //e6y: the "RUN" key inverts the autorun state

  if (gamekeydown[key_speed]) {
    speed = !autorun;
  }

  if (joybuttons[joybspeed]) {
    speed = !autorun;
  }

  forward = side = 0;

  G_SkipDemoCheck(); //e6y

  if (democontinue) {
    mousex = mousey = 0;
    return;
  }

  // use two stage accelerative turning on the keyboard and joystick
  if (joyxmove < 0 || joyxmove > 0 || gamekeydown[key_right] ||
                                      gamekeydown[key_left]) {
    turnheld++;
  }
  else {
    turnheld = 0;
  }

  if (turnheld < SLOWTURNTICS) {
    tspeed = 2;             // slow turn
  }
  else {
    tspeed = speed;
  }

  // turn 180 degrees in one keystroke?                           // phares
                                                                  //    |
  if (gamekeydown[key_reverse]) {                                 //    V
    cmd->angleturn += QUICKREVERSE;                               //    ^
    gamekeydown[key_reverse] = false;                             //    |
  }                                                               // phares

  // let movement keys cancel each other out

  if (strafe) {
    if (gamekeydown[key_right]) {
      side += sidemove[speed];
    }
    if (gamekeydown[key_left]) {
      side -= sidemove[speed];
    }
    if (joyxmove > 0) {
      side += sidemove[speed];
    }
    if (joyxmove < 0) {
      side -= sidemove[speed];
    }
  }
  else {
    if (gamekeydown[key_right]) {
      cmd->angleturn -= angleturn[tspeed];
    }
    if (gamekeydown[key_left]) {
      cmd->angleturn += angleturn[tspeed];
    }
    if (joyxmove > 0) {
      cmd->angleturn -= angleturn[tspeed];
    }
    if (joyxmove < 0) {
      cmd->angleturn += angleturn[tspeed];
    }
  }

  if (gamekeydown[key_up]) {
    forward += forwardmove[speed];
  }
  if (gamekeydown[key_down]) {
    forward -= forwardmove[speed];
  }
  if (joyymove < 0) {
    forward += forwardmove[speed];
  }
  if (joyymove > 0) {
    forward -= forwardmove[speed];
  }
  if (gamekeydown[key_straferight] || joybuttons[joybstraferight]) {
    side += sidemove[speed];
  }
  if (gamekeydown[key_strafeleft] || joybuttons[joybstrafeleft]) {
    side -= sidemove[speed];
  }

  // buttons
  // cmd->chatchar = HU_dequeueChatChar();

  if (gamekeydown[key_fire]) {
    cmd->buttons |= BT_ATTACK;
  }

  if (mousebuttons[mousebfire]) {
    cmd->buttons |= BT_ATTACK;
  }

  if (gamekeydown[key_use] || mousebuttons[mousebuse] || joybuttons[joybuse]) {
    cmd->buttons |= BT_USE;
    dclicks = 0; // clear double clicks if hit use button
  }

  // Toggle between the top 2 favorite weapons.                   // phares
  // If not currently aiming one of these, switch to              // phares
  // the favorite. Only switch if you possess the weapon.         // phares

  // CG: The above comment is by phares (rofl)

  // killough 3/22/98:
  //
  // Perform automatic weapons switch here rather than in p_pspr.c,
  // except in demo_compatibility mode.
  //
  // killough 3/26/98, 4/2/98: fix autoswitch when no weapons are left

  if ((!demo_compatibility && player->attackdown && // killough
       !PL_CheckAmmo(player)) ||
      gamekeydown[key_weapontoggle]) {
    newweapon = PL_SwitchWeapon(player);           // phares
  }
  else { // phares 02/26/98: Added gamemode checks
    if (next_weapon) {
      newweapon = G_NextWeapon(next_weapon);
      next_weapon = 0;
    }
    else {
      // killough 5/2/98: reformatted
      // CG: 04/15/2014: re-reformatted
      bool can_check_wp9 = (!demo_compatibility) && gamemode == commercial;

      newweapon = wp_nochange;

      if (gamekeydown[key_weapon1]) {
        newweapon = wp_fist;
      }
      else if (gamekeydown[key_weapon2]) {
        newweapon = wp_pistol;
      }
      else if (gamekeydown[key_weapon3]) {
        newweapon = wp_shotgun;
      }
      else if (gamekeydown[key_weapon4]) {
        newweapon = wp_chaingun;
      }
      else if (gamekeydown[key_weapon5]) {
        newweapon = wp_missile;
      }
      else if (gamekeydown[key_weapon6] && gamemode != shareware) {
        newweapon = wp_plasma;
      }
      else if (gamekeydown[key_weapon7] && gamemode != shareware) {
        newweapon = wp_bfg;
      }
      else if (gamekeydown[key_weapon8]) {
        newweapon = wp_chainsaw;
      }
      else if (can_check_wp9 && gamekeydown[key_weapon9]) {
        newweapon = wp_supershotgun;
      }
    }

    // killough 3/22/98: For network and demo consistency with the
    // new weapons preferences, we must do the weapons switches here
    // instead of in p_user.c. But for old demos we must do it in
    // p_user.c according to the old rules. Therefore demo_compatibility
    // determines where the weapons switch is made.

    // killough 2/8/98:
    // Allow user to switch to fist even if they have chainsaw.
    // Switch to fist or chainsaw based on preferences.
    // Switch to shotgun or SSG based on preferences.

    if (!demo_compatibility) {
      // only select chainsaw from '1' if it's owned, it's
      // not already in use, and the player prefers it or
      // the fist is already in use, or the player does not
      // have the berserker strength.

      if (newweapon == wp_fist &&
          player->weaponowned[wp_chainsaw] &&
          player->readyweapon != wp_chainsaw && (
            player->readyweapon == wp_fist ||
            !player->powers[pw_strength] ||
            PL_WeaponPreferred(wp_chainsaw, wp_fist))) {
        newweapon = wp_chainsaw;
      }

      // Select SSG from '3' only if it's owned and the player
      // does not have a shotgun, or if the shotgun is already
      // in use, or if the SSG is not already in use and the
      // player prefers it.

      if (newweapon == wp_shotgun &&
          gamemode == commercial &&
          player->weaponowned[wp_supershotgun] && (
            !player->weaponowned[wp_shotgun] ||
            player->readyweapon == wp_shotgun || (
              player->readyweapon != wp_supershotgun &&
              PL_WeaponPreferred(wp_supershotgun, wp_shotgun)))) {
        newweapon = wp_supershotgun;
      }
    }
    // killough 2/8/98, 3/22/98 -- end of weapon selection changes
  }

  if (newweapon != wp_nochange) {
    cmd->buttons |= BT_CHANGE;
    cmd->buttons |= newweapon << BT_WEAPONSHIFT;
  }

  // mouse
  if (mousebuttons[mousebforward]) {
    forward += forwardmove[speed];
  }
  if (mousebuttons[mousebbackward]) {
    forward -= forwardmove[speed];
  }

  if (mouse_doubleclick_as_use) { //e6y
    // forward double click
    if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1) {
      dclickstate = mousebuttons[mousebforward];

      if (dclickstate) {
        dclicks++;
      }

      if (dclicks == 2) {
        cmd->buttons |= BT_USE;
        dclicks = 0;
      }
      else {
        dclicktime = 0;
      }

    }
    else if ((dclicktime++) > 20) {
      dclicks = 0;
      dclickstate = 0;
    }

    // strafe double click

    bstrafe = mousebuttons[mousebstrafe] || joybuttons[joybstrafe];

    if (bstrafe != dclickstate2 && dclicktime2 > 1) {
      dclickstate2 = bstrafe;

      if (dclickstate2) {
        dclicks2++;
      }

      if (dclicks2 == 2) {
        cmd->buttons |= BT_USE;
        dclicks2 = 0;
      }
      else {
        dclicktime2 = 0;
      }
    }
    else if ((dclicktime2++) > 20) {
      dclicks2 = 0;
      dclickstate2 = 0;
    }
  } //e6y: end if (mouse_doubleclick_as_use)

  forward += mousey;

  if (strafe) {
    side += mousex / 4;       /* mead  Don't want to strafe as fast as turns.*/
  }
  else {
    cmd->angleturn -= mousex; /* mead now have enough dynamic range 2-10-00 */
  }

  if ((walkcamera.mode == camera_mode_disabled) || menuactive) { //e6y
    mousex = 0;
    mousey = 0;
    joyxmove = 0;
    joyymove = 0;
  }

#ifdef GL_DOOM
  motion_blur.curr_speed_pow2 = 0;
#endif

  if (forward > MAXPLMOVE) {
    forward = MAXPLMOVE;
  }
  else if (forward < -MAXPLMOVE) {
    forward = -MAXPLMOVE;
  }

  if (side > MAXPLMOVE) {
    side = MAXPLMOVE;
  }
  else if (side < -MAXPLMOVE) {
    side = -MAXPLMOVE;
  }

  //e6y
  if (movement_strafe50) {
    if (!speed) {
      if (side > sidemove_strafe50[0]) {
        side = sidemove_strafe50[0];
      }
      else if (side < -sidemove_strafe50[0]) {
        side = -sidemove_strafe50[0];
      }
    }
    else if (!movement_strafe50onturns && !strafe && cmd->angleturn) {
      if (side > sidemove_normal[1]) {
        side = sidemove_normal[1];
      }
      else if (side < -sidemove_normal[1]) {
        side = -sidemove_normal[1];
      }
    }
  }

  cmd->forwardmove += fudgef((signed char)forward);
  cmd->sidemove += side;
  cmd->angleturn = fudgea(cmd->angleturn);

  upmove = 0;
  if (gamekeydown[key_flyup]) {
    upmove += flyspeed[speed];
  }
  if (gamekeydown[key_flydown]) {
    upmove -= flyspeed[speed];
  }

  // CPhipps - special events (game new/load/save/pause)
  if (special_event & BT_SPECIAL) {
    cmd->buttons = special_event;
    special_event = 0;
  }
}

void G_SetSpeed(void) {
  int p;

  if (movement_strafe50) {
    sidemove[0] = sidemove_strafe50[0];
    sidemove[1] = sidemove_strafe50[1];
  }
  else {
    movement_strafe50onturns = false;
    sidemove[0] = sidemove_normal[0];
    sidemove[1] = sidemove_normal[1];
  }

  if ((p = M_CheckParm("-turbo"))) {
    int scale = ((p < myargc - 1) ? atoi(myargv[p + 1]) : 200);

    scale = BETWEEN(10, 400, scale);

    D_MsgLocalInfo("turbo scale: %i%%\n", scale);

    forwardmove[0] = forwardmove_normal[0] * scale / 100;
    forwardmove[1] = forwardmove_normal[1] * scale / 100;
    sidemove[0] = sidemove[0] * scale / 100;
    sidemove[1] = sidemove[1] * scale / 100;
  }
}

void G_InputClear(void) {
  memset(gamekeydown, 0, sizeof(gamekeydown));
  joyxmove = joyymove = 0;
  mousex = mousey = 0;
  mlooky = 0;                     // e6y
  special_event = 0;
  paused = false;
  memset(&mousearray, 0, sizeof(mousearray));
  memset(&joyarray, 0, sizeof(joyarray));
}

bool G_InputHandleEvent(event_t *ev) {
  // If the next/previous weapon keys are pressed, set the next_weapon
  // variable to change weapons when the next ticcmd is generated.
  if (ev->type == ev_key && ev->pressed) {
    if (ev->key == key_prevweapon) {
      next_weapon = -1;
    }
    else if (ev->key == key_nextweapon) {
      next_weapon = 1;
    }
  }

  switch (ev->type) {
    case ev_key:
      if (ev->pressed) {
        if (ev->key == key_pause) {// phares
          special_event = BT_SPECIAL | (BTS_PAUSE & BT_SPECIALMASK);
        }
        else if (ev->key < NUMKEYS) {
          gamekeydown[ev->key] = true;
        }

        /*
         * CG: Don't do this anymore
         *
         * return true;    // eat key down events
         *
         */
      }
      else {
        if (ev->key < NUMKEYS) {
          gamekeydown[ev->key] = false;
        }

        return false;              // always let key up events filter down
      }
      break;
    case ev_mouse:
      if (ev->key == SDL_BUTTON_LEFT) {
        mousebuttons[0] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_MIDDLE) {
        mousebuttons[1] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_RIGHT) {
        mousebuttons[2] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_X1) {
        mousebuttons[3] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_X2) {
        mousebuttons[4] = ev->pressed;
      }

      return true;
      break;
    case ev_mouse_movement:

      /*
       * bmead@surfree.com
       * Modified by Barry Mead after adding vastly more resolution
       * to the Mouse Sensitivity Slider in the options menu 1-9-2000
       * Removed the mouseSensitivity "*4" to allow more low end
       * sensitivity resolution especially for lsdoom users.
       */

      // e6y mousex += (ev->data2*(mouseSensitivity_horiz))/10;  /* killough */
      // e6y mousey += (ev->data3*(mouseSensitivity_vert))/10;  /*Mead rm *4 */

      /* killough */

      // e6y
      mousex += (D_MouseAccelerate(ev->xmove) * (mouse_sensitivity_horiz)) / 10;
      if (D_GetMouseLook()) {
        if (movement_mouseinvert) {
          mlooky += (
            D_MouseAccelerate(ev->ymove) * (mouse_sensitivity_mlook)
          ) / 10;
        }
        else {
          mlooky -= (
            D_MouseAccelerate(ev->ymove) * (mouse_sensitivity_mlook)
          ) / 10;
        }
      }
      else {
        mousey += (
          D_MouseAccelerate(ev->ymove) * (mouse_sensitivity_vert)
        ) / 40;
      }

      return true;    // eat events
      break;
    case ev_joystick:
      if (ev->key >= 0 && ev->key <= 7) {
        joybuttons[ev->key] = ev->pressed;
      }

      return true;
      break;
    case ev_joystick_movement:
      if (ev->jstype == ev_joystick_axis) {
        if (ev->key == 0) {
          joyxmove = ev->value;
        }
        else if (ev->key == 1) {
          joyymove = ev->value;
        }
      }
      else if (ev->jstype == ev_joystick_ball) {
        joyxmove = ev->xmove;
        joyymove = ev->ymove;
      }
      else if (ev->jstype == ev_joystick_hat) {
        if (ev->value == SDL_HAT_CENTERED) {
          joyxmove = 0;
          joyymove = 0;
        }
        else if (ev->value == SDL_HAT_UP) {
          joyymove = 1;
        }
        else if (ev->value == SDL_HAT_RIGHT) {
          joyxmove = 1;
        }
        else if (ev->value == SDL_HAT_DOWN) {
          joyymove = -1;
        }
        else if (ev->value == SDL_HAT_LEFT) {
          joyxmove = -1;
        }
        else if (ev->value == SDL_HAT_RIGHTUP) {
          joyxmove = 1;
          joyymove = 1;
        }
        else if (ev->value == SDL_HAT_RIGHTDOWN) {
          joyxmove = 1;
          joyymove = -1;
        }
        else if (ev->value == SDL_HAT_LEFTUP) {
          joyxmove = -1;
          joyymove = 1;
        }
        else if (ev->value == SDL_HAT_LEFTDOWN) {
          joyxmove = -1;
          joyymove = -1;
        }
      }
      return true;    // eat events
      break;
    default:
      break;
  }

  return false;
}

/* vi: set et ts=2 sw=2: */

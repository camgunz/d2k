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

#include "doomstat.h"
#include "d_event.h"
#include "c_main.h"
#include "e6y.h"//e6y
#include "g_game.h"
#include "hu_stuff.h"
#include "i_main.h"
#include "i_system.h"
#include "lprintf.h"
#include "n_net.h"
#include "n_main.h"
#include "n_proto.h"
#include "cl_main.h"
#include "p_cmd.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_user.h"
#include "r_demo.h"
#include "r_fps.h"
#include "r_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "x_main.h"

static void add_player_vmessage(int pn, bool is_markup, bool centered, int sfx,
                                const char *fmt, va_list args) {
  gchar *gcontent;
  player_message_t *msg;

  if (CL_Synchronizing() || CL_RePredicting())
    return;

  if (strlen(fmt) <= 0)
    return;

  msg = malloc(sizeof(player_message_t));

  if (msg == NULL)
    I_Error("P_Printf: malloc failed");

  gcontent = g_strdup_vprintf(fmt, args);

  if (pn == consoleplayer)
    C_Write(gcontent);

  if (!is_markup) {
    gchar *escaped_gcontent = g_markup_escape_text(gcontent, -1);

    g_free(gcontent);
    gcontent = escaped_gcontent;
  }

  msg->content = strdup(gcontent);
  msg->centered = centered;
  msg->processed = false;
  msg->sfx = sfx;

  g_free(gcontent);

  P_AddMessage(pn, msg);
}

static void add_player_message(int pn, bool is_markup, bool centered, int sfx,
                               const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(pn, is_markup, centered, sfx, fmt, args);
  va_end(args);
}

//
// Movement.
//

// 16 pixels of bob

#define MAXBOB  0x100000

dboolean onground; // whether player is on ground or in air

//
// P_Thrust
// Moves the given origin along a given angle.
//

void P_SideThrust(player_t *player, angle_t angle, fixed_t move) {
  angle >>= ANGLETOFINESHIFT;

  player->mo->momx += FixedMul(move, finecosine[angle]);
  player->mo->momy += FixedMul(move, finesine[angle]);
}

void P_Thrust(player_t* player,angle_t angle,fixed_t move) {
  angle >>= ANGLETOFINESHIFT;

  if ((player->mo->flags & MF_FLY) && player->mo->pitch != 0) {
    angle_t pitch = player->mo->pitch >> ANGLETOFINESHIFT;
    fixed_t zpush = FixedMul(move, finesine[pitch]);
    player->mo->momz -= zpush;
    move = FixedMul(move, finecosine[pitch]);
  }

  player->mo->momx += FixedMul(move, finecosine[angle]);
  player->mo->momy += FixedMul(move, finesine[angle]);
}


/*
 * P_Bob
 * Same as P_Thrust, but only affects bobbing.
 *
 * killough 10/98: We apply thrust separately between the real physical player
 * and the part which affects bobbing. This way, bobbing only comes from player
 * motion, nothing external, avoiding many problems, e.g. bobbing should not
 * occur on conveyors, unless the player walks on one, and bobbing should be
 * reduced at a regular rate, even on ice (where the player coasts).
 */

static void P_Bob(player_t *player, angle_t angle, fixed_t move) {
  if (!mbf_features && !prboom_comp[PC_PRBOOM_FRICTION].state) //e6y
    return;

  player->momx += FixedMul(move, finecosine[angle >>= ANGLETOFINESHIFT]);
  player->momy += FixedMul(move, finesine[angle]);
}

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//

void P_CalcHeight(player_t *player) {
  int angle;
  fixed_t bob;

  // Regular movement bobbing
  // (needs to be calculated for gun swing
  // even if not on ground)
  // OPTIMIZE: tablify angle
  // Note: a LUT allows for effects
  //  like a ramp with low health.


  /* killough 10/98: Make bobbing depend only on player-applied motion.
   *
   * Note: don't reduce bobbing here if on ice: if you reduce bobbing here,
   * it causes bobbing jerkiness when the player moves from ice to non-ice,
   * and vice-versa.
   */

  player->bob = 0;

  if ((player->mo->flags & MF_FLY) && !onground)
    player->bob = FRACUNIT / 2;

  if (mbf_features) {
    if (player_bobbing) {
      player->bob = (
        FixedMul(player->momx, player->momx) +
        FixedMul(player->momy, player->momy)
      ) >> 2;
    }
  }
  else if (demo_compatibility || player_bobbing ||
           prboom_comp[PC_FORCE_INCORRECT_BOBBING_IN_BOOM].state) {
    player->bob = (
      FixedMul(player->mo->momx, player->mo->momx) +
      FixedMul(player->mo->momy, player->mo->momy)
    ) >> 2;
  }

  //e6y
  if (!prboom_comp[PC_PRBOOM_FRICTION].state &&
      compatibility_level >= boom_202_compatibility && 
      compatibility_level <= lxdoom_1_compatibility &&
      player->mo->friction > ORIG_FRICTION) { // ice?
    if (player->bob > (MAXBOB >> 2))
      player->bob = MAXBOB >> 2;
  }
  else if (player->bob > MAXBOB) {
    player->bob = MAXBOB;
  }

  if (!onground || player->cheats & CF_NOMOMENTUM) {
    player->viewz = player->mo->z + VIEWHEIGHT;

    if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
      player->viewz = player->mo->ceilingz - 4 * FRACUNIT;

    // The following line was in the Id source and appears    // phares 2/25/98
    // to be a bug. player->viewz is checked in a similar
    // manner at a different exit below.

    // player->viewz = player->mo->z + player->viewheight;
    return;
  }

  angle = (FINEANGLES / 20 * leveltime) & FINEMASK;
  bob = FixedMul(player->bob / 2, finesine[angle]);

  // move viewheight

  if (player->playerstate == PST_LIVE) {
    player->viewheight += player->deltaviewheight;

    if (player->viewheight > VIEWHEIGHT) {
      player->viewheight = VIEWHEIGHT;
      player->deltaviewheight = 0;
    }

    if (player->viewheight < VIEWHEIGHT / 2) {
      player->viewheight = VIEWHEIGHT / 2;
      if (player->deltaviewheight <= 0)
        player->deltaviewheight = 1;
    }

    if (player->deltaviewheight) {
      player->deltaviewheight += FRACUNIT / 4;

      if (!player->deltaviewheight)
        player->deltaviewheight = 1;
    }
  }

  player->viewz = player->mo->z + player->viewheight + bob;

  if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
    player->viewz = player->mo->ceilingz - 4 * FRACUNIT;
}


//
// P_SetPitch
// Mouse Look Stuff
//
void P_SetPitch(player_t *player) {
  mobj_t *mo = player->mo;

  if (player == &players[consoleplayer]) {
    if (!(demoplayback || democontinue)) {
      if (GetMouseLook()) {
        if (!mo->reactiontime &&
            (!(automapmode & am_active) || (automapmode & am_overlay))) {
          mo->pitch += (mlooky << 16);
          CheckPitch((signed int *)&mo->pitch);
        }
      }
      else {
        mo->pitch = 0;
      }

      R_DemoEx_WriteMLook(mo->pitch);
    }
    else {
      mo->pitch = R_DemoEx_ReadMLook();
      CheckPitch((signed int *)&mo->pitch);
    }
  }
  else {
    mo->pitch = 0;
  }
}

//
// P_MovePlayer
//
// Adds momentum if the player is not in the air
//
// killough 10/98: simplified

void P_MovePlayer(player_t *player) {
  ticcmd_t *cmd = &player->cmd;
  mobj_t *mo = player->mo;

  mo->angle += cmd->angleturn << 16;

  if (demo_smoothturns && player == &players[displayplayer])
    R_SmoothPlaying_Add(cmd->angleturn << 16);

  onground = mo->z <= mo->floorz;

  if ((player->mo->flags & MF_FLY) && player == &players[consoleplayer] && upmove != 0)
    mo->momz = upmove << 8;

  if (comperr(comperr_allowjump)) {
    if (upmove > 0 &&
        onground &&
        player == &players[consoleplayer] &&
        !(player->mo->flags & MF_FLY)) {
      if (!player->jumpTics) {
        mo->momz = 9 * FRACUNIT;
        player->jumpTics = 18;
      }
    }
  }

  // killough 10/98:
  //
  // We must apply thrust to the player and bobbing separately, to avoid
  // anomalies. The thrust applied to bobbing is always the same strength on
  // ice, because the player still "works just as hard" to move, while the
  // thrust applied to the movement varies with 'movefactor'.

  //e6y
  if ((!demo_compatibility &&
       !mbf_features &&
       !prboom_comp[PC_PRBOOM_FRICTION].state) || 
      (cmd->forwardmove | cmd->sidemove)) { // killough 10/98
    if (onground || mo->flags & MF_BOUNCES || (mo->flags & MF_FLY)) {
      // killough 8/9/98
      int friction, movefactor = P_GetMoveFactor(mo, &friction);

      // killough 11/98:
      // On sludge, make bobbing depend on efficiency.
      // On ice, make it depend on effort.

      int bobfactor =
        friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR;

      if (cmd->forwardmove) {
        P_Bob(player,mo->angle,cmd->forwardmove*bobfactor);
        P_Thrust(player,mo->angle,cmd->forwardmove*movefactor);
      }

      if (cmd->sidemove) {
        P_Bob(player,mo->angle - ANG90,cmd->sidemove * bobfactor);
        P_SideThrust(player,mo->angle - ANG90, cmd->sidemove * movefactor);
      }
    }
    else if (comperr(comperr_allowjump)) {
      if (!onground) {
        if (cmd->forwardmove)
          P_Thrust(player, mo->angle, FRACUNIT >> 8);
        if (cmd->sidemove)
          P_Thrust(player, mo->angle, FRACUNIT >> 8);
      }
    }

    if (mo->state == states + S_PLAY)
      P_SetMobjState(mo, S_PLAY_RUN1);
  }
}

#define ANG5 (ANG90/18)

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//

void P_DeathThink(player_t *player) {
  angle_t angle;
  angle_t delta;

  P_MovePsprites(player);

  // fall to the ground

  onground = (player->mo->z <= player->mo->floorz);
  if (player->mo->type == MT_GIBDTH) {
    // Flying bloody skull
    player->viewheight = 6 * FRACUNIT;
    player->deltaviewheight = 0;
    if (onground && (int)player->mo->pitch > -(int)ANG1 * 19)
      player->mo->pitch -= ((int)ANG1 * 19 - player->mo->pitch) / 8;
  }
  else {
    if (player->viewheight > 6 * FRACUNIT)
      player->viewheight -= FRACUNIT;

    if (player->viewheight < 6 * FRACUNIT)
      player->viewheight = 6 * FRACUNIT;

    player->deltaviewheight = 0;
  }

  P_CalcHeight(player);

  if (player->attacker && player->attacker != player->mo) {
    angle = R_PointToAngle2(
      player->mo->x, player->mo->y, player->attacker->x, player->attacker->y
    );

    delta = angle - player->mo->angle;

    if (delta < ANG5 || delta > (unsigned)-ANG5) {
      // Looking at killer,
      //  so fade damage flash down.

      player->mo->angle = angle;

      if (player->damagecount)
        player->damagecount--;
    }
    else if (delta < ANG180) {
      player->mo->angle += ANG5;
    }
    else {
      player->mo->angle -= ANG5;
    }
  }
  else if (player->damagecount) {
    player->damagecount--;
  }

  if (player->cmd.buttons & BT_USE)
    player->playerstate = PST_REBORN;

  R_SmoothPlaying_Reset(player); // e6y
}


//
// P_PlayerThink
//

void P_PlayerThink(int playernum) {
  player_t *player = &players[playernum];

  if (movement_smooth) {
    player->prev_viewz = player->viewz;
    player->prev_viewangle = R_SmoothPlaying_Get(player) + viewangleoffset;
    player->prev_viewpitch = player->mo->pitch;

    if (&players[displayplayer] == player)
      P_ResetWalkcam();
  }

  // killough 2/8/98, 3/21/98:
  if (player->cheats & CF_NOCLIP)
    player->mo->flags |= MF_NOCLIP;
  else
    player->mo->flags &= ~MF_NOCLIP;

  P_RunPlayerCommands(playernum);

  if (player->playerstate == PST_DEAD) {
    P_DeathThink(player);
    return;
  }

  // Determine if there's anything about the sector you're in that's
  // going to affect you, like painful floors.
  if (player->mo->subsector->sector->special)
    P_PlayerInSpecialSector(player);

  // Counters, time dependent power ups.

  // Strength counts up to diminish fade.

  if (player->powers[pw_strength])
    player->powers[pw_strength]++;

  // killough 1/98: Make idbeholdx toggle:

  if (player->powers[pw_invulnerability] > 0) // killough
    player->powers[pw_invulnerability]--;

  if (player->powers[pw_invisibility] > 0) {  // killough
    player->powers[pw_invisibility]--;

    if (!player->powers[pw_invisibility])
      player->mo->flags &= ~MF_SHADOW;
  }

  if (player->powers[pw_infrared] > 0)        // killough
    player->powers[pw_infrared]--;

  if (player->powers[pw_ironfeet] > 0)        // killough
    player->powers[pw_ironfeet]--;

  if (player->damagecount)
    player->damagecount--;

  if (player->bonuscount)
    player->bonuscount--;

  // Handling colormaps.
  // killough 3/20/98: reformat to terse C syntax
  player->fixedcolormap = palette_onpowers &&
    (player->powers[pw_invulnerability] > 4 * 32 ||
    player->powers[pw_invulnerability] & 8) ? INVERSECOLORMAP :
    player->powers[pw_infrared] > 4 * 32 || player->powers[pw_infrared] & 8;

}

void P_SetName(int playernum, char *name) {
  player_t *player = &players[playernum];

  if (player->name)
    free(player->name);

  player->name = name;
}

void P_InitPlayerMessages(int playernum) {
  players[playernum].messages.messages = g_ptr_array_new_with_free_func(
    P_DestroyMessage
  );
  players[playernum].messages.updated = false;
}

void P_AddMessage(int playernum, player_message_t *message) {
  g_ptr_array_add(players[playernum].messages.messages, message);
  players[playernum].messages.updated = true;
}

void P_ClearMessagesUpdated(int playernum) {
  players[playernum].messages.updated = false;
}

void P_Printf(int playernum, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, false, false, 0, fmt, args);
  va_end(args);
}

void P_VPrintf(int playernum, const char *fmt, va_list args) {
  add_player_vmessage(playernum, false, false, 0, fmt, args);
}

void P_Echo(int playernum, const char *message) {
  add_player_message(playernum, false, false, 0, "%s\n", message);
}

void P_Write(int playernum, const char *message) {
  add_player_message(playernum, false, false, 0, "%s", message);
}

void P_MPrintf(int playernum, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, true, false, 0, fmt, args);
  va_end(args);
}

void P_MVPrintf(int playernum, const char *fmt, va_list args) {
  add_player_vmessage(playernum, true, false, 0, fmt, args);
}

void P_MEcho(int playernum, const char *message) {
  add_player_message(playernum, true, false, 0, "%s\n", message);
}

void P_MWrite(int playernum, const char *message) {
  add_player_message(playernum, true, false, 0, "%s", message);
}

void P_SPrintf(int playernum, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, false, false, sfx, fmt, args);
  va_end(args);
}

void P_SVPrintf(int playernum, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(playernum, false, false, sfx, fmt, args);
}

void P_SEcho(int playernum, int sfx, const char *message) {
  add_player_message(playernum, false, false, sfx, "%s\n", message);
}

void P_SWrite(int playernum, int sfx, const char *message) {
  add_player_message(playernum, false, false, sfx, "%s", message);
}

void P_MSPrintf(int playernum, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, true, false, sfx, fmt, args);
  va_end(args);
}

void P_MSVPrintf(int playernum, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(playernum, true, false, sfx, fmt, args);
}

void P_MSEcho(int playernum, int sfx, const char *message) {
  add_player_message(playernum, true, false, sfx, "%s\n", message);
}

void P_MSWrite(int playernum, int sfx, const char *message) {
  add_player_message(playernum, true, false, sfx, "%s", message);
}

void P_CenterPrintf(int playernum, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, false, true, 0, fmt, args);
  va_end(args);
}

void P_CenterVPrintf(int playernum, const char *fmt, va_list args) {
  add_player_vmessage(playernum, false, true, 0, fmt, args);
}

void P_CenterEcho(int playernum, const char *message) {
  add_player_message(playernum, false, true, 0, "%s\n", message);
}

void P_CenterWrite(int playernum, const char *message) {
  add_player_message(playernum, false, true, 0, "%s", message);
}

void P_CenterMPrintf(int playernum, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, true, true, 0, fmt, args);
  va_end(args);
}

void P_CenterMVPrintf(int playernum, const char *fmt, va_list args) {
  add_player_vmessage(playernum, true, true, 0, fmt, args);
}

void P_CenterMEcho(int playernum, const char *message) {
  add_player_message(playernum, true, true, 0, "%s\n", message);
}

void P_CenterMWrite(int playernum, const char *message) {
  add_player_message(playernum, true, true, 0, "%s", message);
}

void P_CenterSPrintf(int playernum, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, false, true, sfx, fmt, args);
  va_end(args);
}

void P_CenterSVPrintf(int playernum, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(playernum, false, true, sfx, fmt, args);
}

void P_CenterSEcho(int playernum, int sfx, const char *message) {
  add_player_message(playernum, false, true, sfx, "%s\n", message);
}

void P_CenterSWrite(int playernum, int sfx, const char *message) {
  add_player_message(playernum, false, true, sfx, "%s", message);
}

void P_CenterMSPrintf(int playernum, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(playernum, true, true, sfx, fmt, args);
  va_end(args);
}

void P_CenterMSVPrintf(int playernum, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(playernum, true, true, sfx, fmt, args);
}

void P_CenterMSEcho(int playernum, int sfx, const char *message) {
  add_player_message(playernum, true, true, sfx, "%s\n", message);
}

void P_CenterMSWrite(int playernum, int sfx, const char *message) {
  add_player_message(playernum, true, true, sfx, "%s", message);
}

void P_SendMessage(const char *message) {
  int sfx;

  if (gamemode == commercial)
    sfx = sfx_radio;
  else
    sfx = sfx_tink;

  if (CLIENT) {
    CL_SendMessage(message);
  }
  else if (SERVER) {
    SV_BroadcastMessage(message);
  }

  if (players[consoleplayer].name != NULL)
    P_Printf(consoleplayer, "<%s>: %s\n", players[consoleplayer].name, message);
  else
    P_Printf(consoleplayer, "<Player %d>: %s\n", consoleplayer, message);

  S_StartSound(NULL, sfx);
}

void P_DestroyMessage(gpointer data) {
  player_message_t *msg = data;

  free(msg->content);
  free(msg);
}

void P_ClearMessages(int playernum) {
  if (players[playernum].messages.messages->len == 0)
    return;

  g_ptr_array_remove_range(
    players[playernum].messages.messages,
    0,
    players[playernum].messages.messages->len
  );
}

void P_SetPlayerName(int playernum, const char *name) {
  if (!name)
    return;

  if (!strlen(name))
    return;

  P_Printf(consoleplayer, "%s is now %s.\n", players[playernum].name, name);

  if (SERVER)
    SV_BroadcastPrintf("%s is now %s.\n", players[playernum].name, name);

  if (players[playernum].name)
    free(players[playernum].name);

  players[playernum].name = strdup(name);

  if (playernum != consoleplayer)
    return;

  if (CLIENT)
    CL_SendNameChange(players[playernum].name);

  if (SERVER)
    SV_BroadcastPlayerNameChanged(playernum, players[playernum].name);
}

/* vi: set et ts=2 sw=2: */


/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Player related stuff.
 *      Bobbing POV/weapon, movement.
 *      Pending weapon.
 *
 *-----------------------------------------------------------------------------*/

#ifndef P_USER__
#define P_USER__

#include "d_player.h"

typedef struct player_message_s {
  uint64_t timestamp;
  char *content;
  bool centered;
} player_message_t;

void P_PlayerThink(player_t *player);
void P_CalcHeight(player_t *player);
void P_DeathThink(player_t *player);
void P_MovePlayer(player_t *player);
void P_Thrust(player_t *player, angle_t angle, fixed_t move);
void P_SetPitch(player_t *player);
void P_RunPlayerCommands(player_t *player);
void P_Printf(int playernum, const char *fmt, ...) PRINTF_DECL(2, 3);
void P_Echo(int playernum, const char *message);
void P_Write(int playernum, const char *message);
void P_CenterPrintf(int playernum, const char *fmt, ...) PRINTF_DECL(2, 3);
void P_CenterEcho(int playernum, const char *message);
void P_CenterWrite(int playernum, const char *message);
void P_ClearMessages(int playernum);

#endif  /* P_USER__ */

/* vi: set et ts=2 sw=2: */


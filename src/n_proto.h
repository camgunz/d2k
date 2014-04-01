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
 *
 *
 *-----------------------------------------------------------------------------
 */

#ifndef N_PROTO_H__
#ifndef N_PROTO_H__

#ifndef GAME_OPTIONS_SIZE
#define GAME_OPTIONS_SIZE 64
#endif

const byte nm_resources              = 1;  /* S => C | BOTH    |   reliable */
const byte nm_fullstate              = 2;  /* S => C | BOTH    |   reliable */
const byte nm_statedelta             = 3;  /* S => C | DELTA   | unreliable */
const byte nm_authresponse           = 4;  /* S => C | BOTH    |   reliable */
const byte nm_servermessage          = 5;  /* S => C | BOTH    |   reliable */
const byte nm_playermessage          = 6;  /* BOTH   | BOTH    |   reliable */
const byte nm_playercommandreceived  = 7;  /* BOTH   | BOTH    |   reliable */
const byte nm_playercommands         = 8;  /* NOT DELTA CLIENT | unreliable */
const byte nm_savegamenamechange     = 9;  /* NOT DELTA CLIENT |   reliable */
const byte nm_playerpreferencechange = 10; /* NOT DELTA CLIENT |   reliable */
const byte nm_statereceived          = 11; /* C => S | DELTA   |   reliable */
const byte nm_authrequest            = 12; /* C => S | BOTH    |   reliable */
const byte nm_rconcommand            = 13; /* C => S | BOTH    |   reliable */
const byte nm_voterequest            = 14; /* C => S | BOTH    |   reliable */

void   N_InitProtocol(void);
void   N_HandlePacket(int peernum, void *data, size_t data_size);
buf_t* N_GetMessageRecipientBuffer(void);

void SV_SendSetup(short playernum);
void SV_SendStateDelta(short playernum);
void SV_SendFullState(short playernum);
void SV_SendAuthResponse(short playernum, auth_level_e auth_level);
void SV_SendPlayerCommandReceived(short playernum, int tic);
void SV_SendMessage(short playernum, rune *message);
void SV_BroadcastMessage(rune *message);

void CL_SendMessageToServer(rune *message);
void CL_SendMessageToPlayer(short recipient, rune *message);
void CL_SendMessageToTeam(byte team, rune *message);
void CL_SendMessageToCurrentTeam(rune *message);
void CL_SendPlayerCommandReceived(int tic);
void CL_SendCommands(void);
void CL_SendSaveGameNameChange(rune *new_save_game_name);
void CL_SendNameChange(rune *new_name);
void CL_SendTeamChange(byte new_team);
void CL_SendPWOChange(void); /* CG: TODO */
void CL_SendWSOPChange(byte new_wsop_flags);
void CL_SendBobbingChange(double new_bobbing_amount);
void CL_SendAutoaimChange(dboolean new_autoaim_enabled);
void CL_SendWeaponSpeedChange(byte new_weapon_speed);
void CL_SendColorChange(byte new_red, byte new_green, byte new_blue);
void CL_SendColormapChange(int new_color);
void CL_SendSkinChange(void); /* CG: TODO */
void CL_SendStateReceived(int tic);
void CL_SendAuthRequest(rune *password);
void CL_SendRCONCommand(rune *command);
void CL_SendVoteRequest(rune *command);

#endif


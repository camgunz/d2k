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

/*
################################################################################
# CG: General Interface
################################################################################
*/

void N_InitProtocol(void);
void N_HandlePacket(int peernum, void *data, size_t data_size);

/*
################################################################################
# CG: new netcode
################################################################################
*/
const byte nm_statedelta        = 1;
const byte nm_fullstate         = 2;
const byte nm_authresponse      = 3;
const byte nm_servermessage     = 4;
const byte nm_playermessage     = 5;
const byte nm_playercommand     = 6;
const byte nm_authrequest       = 7;
const byte nm_namechange        = 8;
const byte nm_teamchange        = 9;
const byte nm_pwochange         = 10;
const byte nm_wsopchange        = 11;
const byte nm_bobbingchange     = 12;
const byte nm_autoaimchange     = 13;
const byte nm_weaponspeedchange = 14;
const byte nm_colorchange       = 15;
const byte nm_skinchange        = 16;
const byte nm_rconcommand       = 17;
const byte nm_voterequest       = 18;

void SV_SendStateDelta(short playernum);
void SV_SendFullState(short playernum);
void SV_SendAuthResponse(short playernum, auth_level_e auth_level);
void SV_SendMessage(short playernum, rune *message);
void SV_BroadcastMessage(rune *message);

void CL_SendMessage(short recipient, rune *message);
void CL_SendCommand(
  unsigned int   index,
  unsigned int   world_index,
  signed   char  forward,
  signed   char  side,
  signed   short angle,
  byte           buttons,
);
void CL_SendAuthRequest(rune *password);
void CL_SendNameChange(rune *new_name);
void CL_SendTeamChange(byte new_team);
void CL_SendPWOChange(void); /* CG: TODO */
void CL_SendWSOPChange(byte new_wsop_flags);
void CL_SendBobbingChange(double new_bobbing_amount);
void CL_SendAutoaimChange(dboolean new_autoaim_enabled);
void CL_SendWeaponSpeedChange(byte new_weapon_speed);
void CL_SendColorChange(byte new_red, byte new_green, byte new_blue);
void CL_SendSkinChange(void); /* CG: TODO */
void CL_SendRCONCommand(rune *command);
void CL_SendVoteRequest(rune *command);

/*
################################################################################
# CG: old netcode
################################################################################
*/

const byte PKT_INIT    = 0;  /* initial packet to server    */
const byte PKT_SETUP   = 1;  /* game information packet     */
const byte PKT_GO      = 2;  /* game has started            */
const byte PKT_TICC    = 3;  /* tics from client            */
const byte PKT_TICS    = 4;  /* tics from server            */
const byte PKT_RETRANS = 5;  /* Request for retransmission  */
const byte PKT_COLOR   = 6;  /* Player changed color        */
const byte PKT_SAVEG   = 7;  /* Save game name changed      */
const byte PKT_QUIT    = 8;  /* Player quit game            */
const byte PKT_DOWN    = 9;  /* Server downed               */
const byte PKT_WAD     = 10; /* Wad file request            */
const byte PKT_BACKOFF = 11; /* Request for client back-off */

typedef struct {
  // byte checksum;    /* Simple checksum of the entire packet */
  byte type;        /* Type of packet                       */
  // byte reserved[2];	/* Was random in prboom <=2.2.4, now 0  */
  unsigned tic;     /* Timestamp                            */
} PACKEDATTR packet_header_t;

typedef struct setup_packet_s {
  short players;
  short yourplayer;
  byte skill;
  byte episode;
  byte level;
  byte deathmatch;
  byte complevel;
  byte ticdup;
  byte extratic;
  byte game_options[GAME_OPTIONS_SIZE];
  byte numwads;
  byte wadnames[1]; /* Actually longer */
} setup_packet_t;

void CL_SendInitPacket(short wanted_player_number);
void CL_SendGoPacket(void);
void CL_SendTicPacket(int tic_count, objbuf_t *commands);
void CL_SendWadPacket(rune *wad_name);
void CL_SendRetransPacket(int tic);
void CL_SendColorPacket(mapcolor_me new_color);
void CL_SendSaveGameNamePacket(rune *new_save_game_name);
void CL_SendQuit(void);

void SV_BroadcastDownPacket(void);
void SV_BroadcastGoPacket(void);
void SV_SendSetupPacket(short clientnum, setup_packet_t *setupinfo);
void SV_SendRetransPacket(short clientnum, int tic);
void SV_SendWadPacket(short clientnum, rune *wad_url);
void SV_SendTicPacket(short clientnum, int tic_count, short player_count,
                      ticcmd_t *commands);
void SV_SendBackoffPacket(short clientnum, int tic);

#endif


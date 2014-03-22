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

#ifndef N_PACK_H__
#define N_PACK_H__

void     N_InitPacker(void);
dboolean N_LoadNewMessage(netpeer_t *np, byte *message_type);

/* CG: C/S message packing/unpacking here */

void     N_PackStateDelta(netpeer_t *np, int from_tic, int to_tic,
                                         buf_t *buf);
dboolean N_UnpackStateDelta(netpeer_t *np, int *from_tic, int *to_tic,
                                           buf_t *buf);

void     N_PackFullState(netpeer_t *np, buf_t *buf);
dboolean N_UnpackFullState(netpeer_t *np, int *tic, buf_t *buf);

void     N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level);
dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level);

void     N_PackServerMessage(netpeer_t *np, rune *message);
dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf);

void     N_PackPlayerMessage(netpeer_t *np, short recipient, rune *message);
dboolean N_UnpackPlayerMessage(netpeer_t *np, short *recipient, buf_t *buf);

void     N_PackPlayerCommands(netpeer_t *np);
dboolean N_UnpackPlayerCommands(netpeer_t *np);

void     N_PackAuthRequest(netpeer_t *np, rune *password);
dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf);

void     N_PackNameChange(netpeer_t *np, rune *new_name);
dboolean N_UnpackNameChange(netpeer_t *np, buf_t *buf);

void     N_PackTeamChange(netpeer_t *np, byte new_team);
dboolean N_UnpackTeamChange(netpeer_t *np, byte *new_team);

void     N_PackPWOChange(netpeer_t *np);
dboolean N_UnpackPWOChange(netpeer_t *np);

void     N_PackWSOPChange(netpeer_t *np, byte new_wsop_flags);
dboolean N_UnpackWSOPChange(netpeer_t *np, byte *new_wsop_flags);

void     N_PackBobbingChange(netpeer_t *np, double new_bobbing_amount);
dboolean N_UnpackBobbingchange(netpeer_t *np, double *new_bobbing_amount);

void     N_PackAutoaimChange(netpeer_t *np, dboolean new_autoaim_enabled);
dboolean N_UnpackAutoaimChange(netpeer_t *np, dboolean *new_autoaim_enabled);

void     N_PackWeaponSpeedChange(netpeer_t *np, byte new_weapon_speed);
dboolean N_UnpackWeaponSpeedChange(netpeer_t *np, byte *new_weapon_speed);

void     N_PackColorChange(netpeer_t *np, byte new_red, byte new_green,
                                          byte new_blue);
dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red, byte *new_green,
                                            byte *new_blue);

void     N_PackSkinChange(netpeer_t *np);
dboolean N_UnpackSkinChange(netpeer_t *np);

void     N_PackRCONCommand(netpeer_t *np, rune *command);
dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf);

void     N_PackVoteRequest(netpeer_t *np, rune *command);
dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf);

/* CG: P2P message packing/unpacking here */
void     N_PackInit(netpeer_t *np, short wanted_player_number);
dboolean N_UnpackInit(netpeer_t *np, short *wanted_player_number);

void     N_PackSetup(netpeer_t *np, setup_packet_t *sinfo, buf_t *wad_names);
dboolean N_UnpackSetup(netpeer_t *np, setup_packet_t *sinfo,
                                      objbuf_t *wad_names);

void     N_PackGo(netpeer_t *np);

void     N_PackClientTic(netpeer_t *np, int tic, objbuf_t *commands);
dboolean N_UnpackClientTic(netpeer_t *np, int *tic, objbuf_t *commands);

void     N_PackServerTic(netpeer_t *np, int tic, objbuf_t *commands);
dboolean N_UnpackServerTic(netpeer_t *np, int *tic, objbuf_t *commands);

void     N_PackRetransmissionRequest(netpeer_t *np, int tic);
dboolean N_UnpackRetransmissionRequest(netpeer_t *np, int *tic);

void     N_PackColor(netpeer_t *np, mapcolor_me new_color);
dboolean N_UnpackColor(netpeer_t *np, int *tic, short *playernum,
                                      mapcolor_me *new_color);

void     N_PackSaveGameName(netpeer_t *np, rune *new_save_game_name);
dboolean N_UnpackSaveGameName(netpeer_t *np, int *tic, buf_t *buf);

void     N_PackQuit(netpeer_t *np);
dboolean N_UnpackQuit(netpeer_t *np, int *tic, short *playernum);

void     N_PackDown(netpeer_t *np);

void     N_PackWad(netpeer_t *np, rune *wad_name_or_url);
dboolean N_UnpackWad(netpeer_t *np, buf_t *wad_name_or_url);

void     N_PackBackoff(netpeer_t *np, int tic);
dboolean N_UnpackBackoff(netpeer_t *np, int *tic);

#endif

/* vi: set cindent et ts=2 sw=2: */


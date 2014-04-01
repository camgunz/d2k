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
 * *  You should have received a copy of the GNU General Public License
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

void     N_PackSetup(netpeer_t *np);
dboolean N_UnpackSetup(netpeer_t *np, unsigned short *player_count,
                                      unsigned short *playernum);

void     N_PackFullState(netpeer_t *np, buf_t *buf);
dboolean N_UnpackFullState(netpeer_t *np, int *tic, buf_t *buf);

void     N_PackStateDelta(netpeer_t *np, int from_tic, int to_tic,
                                         buf_t *buf);
dboolean N_UnpackStateDelta(netpeer_t *np, int *from_tic, int *to_tic,
                                           buf_t *buf);

void     N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level);
dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level);

void     N_PackServerMessage(netpeer_t *np, rune *message);
dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf);

void     N_PackPlayerMessage(netpeer_t *np, unsigned short sender,
                                            size_t recipient_count,
                                            buf_t *recipients,
                                            rune *message);
dboolean N_UnpackPlayerMessage(netpeer_t *np, unsigned short *sender,
                                              size_t *recipient_count,
                                              buf_t *recipients,
                                              buf_t *buf);

void     N_PackPlayerCommandReceived(netpeer_t *np, int tic);
void     N_UnpackPlayerCommandReceived(netpeer_t *np, int *tic);

void     N_PackPlayerCommands(netpeer_t *np);
dboolean N_UnpackPlayerCommands(netpeer_t *np);

void     N_PackSaveGameNameChange(netpeer_t *np, rune *new_save_game_name);
dboolean N_UnpackSaveGameNameChange(netpeer_t *np, buf_t *buf);

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

void     N_PackColormapChange(netpeer_t *np, int new_color);
dboolean N_UnpackColormapChange(netpeer_t *np, int *new_color);

void     N_PackColorChange(netpeer_t *np, byte new_red, byte new_green,
                                          byte new_blue);
dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red, byte *new_green,
                                            byte *new_blue);

void     N_PackSkinChange(netpeer_t *np);
dboolean N_UnpackSkinChange(netpeer_t *np);

void     N_PackStateReceived(netpeer_t *np, int tic);
void     N_UnpackStateReceived(netpeer_t *np, int *tic);

void     N_PackAuthRequest(netpeer_t *np, rune *password);
dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf);

void     N_PackRCONCommand(netpeer_t *np, rune *command);
dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf);

void     N_PackVoteRequest(netpeer_t *np, rune *command);
dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf);

#endif

/* vi: set et ts=2 sw=2: */


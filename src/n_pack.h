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
void     N_LoadPacketData(void *data, size_t data_size);
dboolean N_LoadNewMessage(netpeer_t *np, byte *message_type);

void     N_PackSetup(netpeer_t *np);
dboolean N_UnpackSetup(netpeer_t *np, net_sync_type_e *sync_type,
                                      unsigned short *player_count,
                                      unsigned short *playernum);

void     N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level);
dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level);

void     N_PackServerMessage(netpeer_t *np, char *message);
dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf);

void     N_PackCommandClientSync(netpeer_t *np);
dboolean N_UnpackCommandClientSync(netpeer_t *np);

void     N_PackCommandServerSync(netpeer_t *np);
dboolean N_UnpackCommandServerSync(netpeer_t *np);

void     N_PackDeltaClientSync(netpeer_t *np);
dboolean N_UnpackDeltaClientSync(netpeer_t *np);

void     N_PackDeltaServerSync(netpeer_t *np);
dboolean N_UnpackDeltaServerSync(netpeer_t *np);

void     N_PackPlayerMessage(netpeer_t *np, unsigned short sender,
                                            size_t recipient_count,
                                            buf_t *recipients,
                                            char *message);
dboolean N_UnpackPlayerMessage(netpeer_t *np, unsigned short *sender,
                                              size_t *recipient_count,
                                              buf_t *recipients,
                                              buf_t *buf);

dboolean N_UnpackPlayerPreferenceChange(netpeer_t *np, short *playernum,
                                                       int *tic,
                                                       size_t *count);
dboolean N_UnpackPlayerPreferenceName(netpeer_t *np, size_t pref_index,
                                                     buf_t *buf);

void     N_PackNameChange(netpeer_t *np, short playernum, char *new_name);
dboolean N_UnpackNameChange(netpeer_t *np, buf_t *buf);

void     N_PackTeamChange(netpeer_t *np, short playernum, byte new_team);
dboolean N_UnpackTeamChange(netpeer_t *np, byte *new_team);

void     N_PackPWOChange(netpeer_t *np, short playernum);
dboolean N_UnpackPWOChange(netpeer_t *np);

void     N_PackWSOPChange(netpeer_t *np, short playernum,
                                         byte new_wsop_flags);
dboolean N_UnpackWSOPChange(netpeer_t *np, byte *new_wsop_flags);

void     N_PackBobbingChange(netpeer_t *np, short playernum,
                                            double new_bobbing_amount);
dboolean N_UnpackBobbingchange(netpeer_t *np, double *new_bobbing_amount);

void     N_PackAutoaimChange(netpeer_t *np, short playernum,
                                            dboolean new_autoaim_enabled);
dboolean N_UnpackAutoaimChange(netpeer_t *np, dboolean *new_autoaim_enabled);

void     N_PackWeaponSpeedChange(netpeer_t *np, short playernum,
                                                byte new_weapon_speed);
dboolean N_UnpackWeaponSpeedChange(netpeer_t *np, byte *new_weapon_speed);

void     N_PackColorChange(netpeer_t *np, short playernum, byte new_red,
                                                           byte new_green,
                                                           byte new_blue);
dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red, byte *new_green,
                                            byte *new_blue);

void     N_PackColormapChange(netpeer_t *np, short playernum, int new_color);
dboolean N_UnpackColormapChange(netpeer_t *np, int *new_color);

void     N_PackSkinChange(netpeer_t *np, short playernum);
dboolean N_UnpackSkinChange(netpeer_t *np);

void     N_PackAuthRequest(netpeer_t *np, char *password);
dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf);

void     N_PackRCONCommand(netpeer_t *np, char *command);
dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf);

void     N_PackVoteRequest(netpeer_t *np, char *command);
dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf);

#endif

/* vi: set et ts=2 sw=2: */


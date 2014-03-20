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
dboolean N_UnpackMessageType(netpeer_t *np, byte *message_type);
void     N_PackGameState(netpeer_t *np, void *state_data, size_t state_size);
dboolean N_UnpackGameState(netpeer_t *np, buf_t *buf);
void     N_PackServerMessage(netpeer_t *np, rune *message);
dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf);
void     N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level);
dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level);
void     N_PackPlayerCommand(netpeer_t *np, netticcmd_t *cmd);
dboolean N_UnpackPlayerCommand(netpeer_t *np, netticcmd_t *cmd);
void     N_PackPlayerMessage(netpeer_t *np, short recipient, rune *message);
dboolean N_UnpackPlayerMessage(netpeer_t *np, short *recipient, buf_t *buf);
void     N_PackAuthRequest(netpeer_t *np, rune *password);
dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf);
void     N_PackNameChanged(netpeer_t *np, rune *new_name);
dboolean N_UnpackNameChanged(netpeer_t *np, buf_t *buf);
void     N_PackTeamChanged(netpeer_t *np, byte new_team);
dboolean N_UnpackTeamChanged(netpeer_t *np, byte *new_team);
void     N_PackPWOChanged(netpeer_t *np);
dboolean N_UnpackPWOChanged(netpeer_t *np);
void     N_PackWSOPChanged(netpeer_t *np, byte new_wsop_flags);
dboolean N_UnpackWSOPChanged(netpeer_t *np, byte *new_wsop_flags);
void     N_PackBobbingChanged(netpeer_t *np, double new_bobbing_amount);
dboolean N_UnpackBobbingchanged(netpeer_t *np, double *new_bobbing_amount);
void     N_PackAutoaimChanged(netpeer_t *np, dboolean new_autoaim_enabled);
dboolean N_UnpackAutoaimChanged(netpeer_t *np, dboolean *new_autoaim_enabled);
void     N_PackWeaponSpeedChanged(netpeer_t *np, byte new_weapon_speed);
dboolean N_UnpackWeaponSpeedChanged(netpeer_t *np, byte *new_weapon_speed);
void     N_PackColorChanged(netpeer_t *np, byte new_red, byte new_green,
                                       byte new_blue);
dboolean N_UnpackColorChanged(netpeer_t *np, byte *new_red, byte *new_green,
                                             byte *new_blue);
void     N_PackSkinChanged(netpeer_t *np);
dboolean N_UnpackSkinChanged(netpeer_t *np);
void     N_PackRCONCommand(netpeer_t *np, rune *command);
dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf);
void     N_PackVoteRequest(netpeer_t *np, rune *command);
dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf);

#endif

/* vi: set cindent et ts=2 sw=2: */


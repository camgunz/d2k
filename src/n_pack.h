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


#ifndef N_PACK_H__
#define N_PACK_H__

void     N_PackSetup(netpeer_t *np);
dboolean N_UnpackSetup(netpeer_t *np, net_sync_type_e *sync_type,
                                      unsigned short *player_count,
                                      unsigned short *playernum);

void     N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level);
dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level);

void     N_PackServerMessage(netpeer_t *np, char *message);
dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf);

void     N_PackSync(netpeer_t *np);
dboolean N_UnpackSync(netpeer_t *np, dboolean *update_sync);

void     N_PackDeltaSync(netpeer_t *np);
dboolean N_UnpackDeltaSync(netpeer_t *np);

void     N_PackPlayerMessage(netpeer_t *np, short sender, buf_t *recipients,
                                            char *message);
dboolean N_UnpackPlayerMessage(netpeer_t *np, short *sender, buf_t *recipients,
                                              buf_t *buf);

dboolean N_UnpackPlayerPreferenceChange(netpeer_t *np, int *tic,
                                                       short *playernum,
                                                       unsigned int *count);
dboolean N_UnpackPlayerPreferenceName(netpeer_t *np, buf_t *buf);

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
dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red,
                                            byte *new_green,
                                            byte *new_blue);

void     N_PackColorIndexChange(netpeer_t *np, short playernum, int new_color);
dboolean N_UnpackColorIndexChange(netpeer_t *np, int *new_color);

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


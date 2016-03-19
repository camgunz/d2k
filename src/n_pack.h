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

void N_PackSetup(netpeer_t *np);
bool N_UnpackSetup(netpeer_t *np, unsigned short *player_count,
                                  unsigned short *playernum);

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level);
bool N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level);

void N_PackPing(netpeer_t *np, double server_time);
bool N_UnpackPing(netpeer_t *np, double *server_time);

void N_PackChatMessage(netpeer_t *np, const char *message);
void N_PackRelayedChatMessage(netpeer_t *np, unsigned short sender,
                                                 const char *message);

void N_PackTeamChatMessage(netpeer_t *np, const char *message);
void N_PackRelayedTeamChatMessage(netpeer_t *np, unsigned short sender,
                                                     const char *message);

void N_PackPlayerChatMessage(netpeer_t *np, unsigned short recipient,
                                                const char *message);
void N_PackRelayedPlayerChatMessage(netpeer_t *np,
                                    unsigned short sender,
                                    unsigned short recipient,
                                    const char *message);

void N_PackServerChatMessage(netpeer_t *np, const char *message);

bool N_UnpackChatMessage(netpeer_t *np,
                         chat_channel_e *chat_channel,
                         unsigned short *message_sender,
                         unsigned short *message_recipient,
                         buf_t *message_contents);

void N_PackSync(netpeer_t *np);
bool N_UnpackSync(netpeer_t *np);

bool N_UnpackPlayerPreferenceChange(netpeer_t *np,
                                    int *tic,
                                    unsigned short *playernum,
                                    unsigned int *count);
bool N_UnpackPlayerPreferenceName(netpeer_t *np, buf_t *buf);

void N_PackNameChange(netpeer_t *np, unsigned short playernum,
                                         const char *new_name);
bool N_UnpackNameChange(netpeer_t *np, buf_t *buf);

void N_PackTeamChange(netpeer_t *np, unsigned short playernum,
                                         unsigned char new_team);
bool N_UnpackTeamChange(netpeer_t *np, unsigned char *new_team);

void N_PackPWOChange(netpeer_t *np, unsigned short playernum);
bool N_UnpackPWOChange(netpeer_t *np);

void N_PackWSOPChange(netpeer_t *np, unsigned short playernum,
                                         unsigned char new_wsop_flags);
bool N_UnpackWSOPChange(netpeer_t *np, unsigned char *new_wsop_flags);

void N_PackBobbingChange(netpeer_t *np, unsigned short playernum,
                                            double new_bobbing_amount);
bool N_UnpackBobbingchange(netpeer_t *np, double *new_bobbing_amount);

void N_PackAutoaimChange(netpeer_t *np, unsigned short playernum,
                                            bool new_autoaim_enabled);
bool N_UnpackAutoaimChange(netpeer_t *np, bool *new_autoaim_enabled);

void N_PackWeaponSpeedChange(netpeer_t *np, unsigned short playernum,
                                                unsigned char new_weapon_speed);
bool N_UnpackWeaponSpeedChange(netpeer_t *np,
                                   unsigned char *new_weapon_speed);

void N_PackColorChange(netpeer_t *np, unsigned short playernum,
                                      unsigned char new_red,
                                      unsigned char new_green,
                                      unsigned char new_blue);
bool N_UnpackColorChange(netpeer_t *np, unsigned char *new_red,
                                        unsigned char *new_green,
                                        unsigned char *new_blue);

void N_PackColorIndexChange(netpeer_t *np, unsigned short playernum,
                                           int new_color);
bool N_UnpackColorIndexChange(netpeer_t *np, int *new_color);

void N_PackSkinChange(netpeer_t *np, unsigned short playernum);
bool N_UnpackSkinChange(netpeer_t *np);

void N_PackAuthRequest(netpeer_t *np, const char *password);
bool N_UnpackAuthRequest(netpeer_t *np, buf_t *buf);

void N_PackRCONCommand(netpeer_t *np, const char *command);
bool N_UnpackRCONCommand(netpeer_t *np, buf_t *buf);

void N_PackVoteRequest(netpeer_t *np, const char *command);
bool N_UnpackVoteRequest(netpeer_t *np, buf_t *buf);

#endif

/* vi: set et ts=2 sw=2: */


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
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef N_PROTO_H__
#define N_PROTO_H__

#ifndef GAME_OPTIONS_SIZE
#define GAME_OPTIONS_SIZE 64
#endif

#define nm_setup                  1
#define nm_authresponse           2
#define nm_servermessage          3
#define nm_sync                   4
#define nm_playermessage          5
#define nm_playerpreferencechange 6
#define nm_authrequest            7
#define nm_rconcommand            8
#define nm_voterequest            9

void   N_InitProtocol(void);
buf_t* N_GetMessageRecipientBuffer(void);
void   N_HandlePacket(int peernum, void *data, size_t data_size);
void   N_UpdateSync(void);

void SV_SetupNewPeer(int peernum);
void SV_SendSetup(short playernum);
void SV_SendStateDelta(short playernum);
void SV_SendFullState(short playernum);
void SV_SendAuthResponse(short playernum, auth_level_e auth_level);
void SV_SendMessage(short playernum, char *message);
void SV_BroadcastMessage(char *message);
void SV_BroadcastPlayerNameChanged(short playernum, char *new_name);
void SV_BroadcastPlayerTeamChanged(short playernum, byte new_team);
void SV_BroadcastPlayerPWOChanged(short playernum);
void SV_BroadcastPlayerWSOPChanged(short playernum, byte new_wsop_flags);
void SV_BroadcastPlayerBobbingChanged(short playernum,
                                      double new_bobbing_amount);
void SV_BroadcastPlayerAutoaimChanged(short playernum,
                                      dboolean new_autoaim_enabled);
void SV_BroadcastPlayerWeaponSpeedChanged(short playernum,
                                          byte new_weapon_speed);
void SV_BroadcastPlayerColorChanged(short playernum, byte new_red,
                                                     byte new_green,
                                                     byte new_blue);
void SV_BroadcastPlayerColorIndexChanged(short playernum, int new_color);
void SV_BroadcastPlayerSkinChanged(short playernum);
void SV_BroadcastStateUpdates(void);
void SV_ResyncPeers(void);

void CL_SendMessageToServer(char *message);
void CL_SendMessageToPlayer(short recipient, char *message);
void CL_SendMessageToTeam(byte team, char *message);
void CL_SendMessageToCurrentTeam(char *message);
void CL_SendMessage(char *message);
void CL_SendCommands(void);
void CL_SendSaveGameNameChange(char *new_save_game_name);
void CL_SendNameChange(char *new_name);
void CL_SendTeamChange(byte new_team);
void CL_SendPWOChange(void); /* CG: TODO */
void CL_SendWSOPChange(byte new_wsop_flags);
void CL_SendBobbingChange(double new_bobbing_amount);
void CL_SendAutoaimChange(dboolean new_autoaim_enabled);
void CL_SendWeaponSpeedChange(byte new_weapon_speed);
void CL_SendColorChange(byte new_red, byte new_green, byte new_blue);
void CL_SendColorIndexChange(int new_color);
void CL_SendSkinChange(void); /* CG: TODO */
void CL_SendStateReceived(void);
void CL_SendAuthRequest(char *password);
void CL_SendRCONCommand(char *command);
void CL_SendVoteRequest(char *command);

#endif


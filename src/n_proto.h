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


#ifndef N_PROTO_H__
#define N_PROTO_H__

struct netpeer_s;
typedef struct netpeer_s netpeer_t;

typedef enum {
  NM_NONE,
  NM_SETUP,
  NM_FULL_STATE,
  NM_AUTH,
  NM_CHAT_MESSAGE,
  NM_SYNC,
  NM_PLAYER_PREFERENCE_CHANGE,
  NM_GAME_ACTION,
  NM_RCON_COMMAND,
  NM_VOTE_REQUEST,
  NM_PING,
  NM_MAX,
} net_message_e;

typedef enum {
  NET_CHANNEL_RELIABLE,
  NET_CHANNEL_UNRELIABLE,
  NET_CHANNEL_MAX
} net_channel_e;

typedef struct {
  bool reliable;
  bool throttle; /* only flushes once per TIC if true */
} net_channel_info_t;

typedef struct {
  const char *name;
  net_channel_e channel;
} net_message_info_t;

extern net_channel_info_t net_channel_info[NET_CHANNEL_MAX];

#define CHAT_CHANNEL_MIN CHAT_CHANNEL_SERVER
#define CHAT_CHANNEL_MAX CHAT_CHANNEL_ALL

typedef enum {
  CHAT_CHANNEL_SERVER = 1,
  CHAT_CHANNEL_PLAYER,
  CHAT_CHANNEL_TEAM,
  CHAT_CHANNEL_ALL
} chat_channel_e;

typedef enum {
  NM_PEER_STATUS_CHANGE_NAME = 1,
  NM_PEER_STATUS_CHANGE_AUTH_LEVEL,
  NM_PEER_STATUS_CHANGE_TYPE
} netpeer_status_change_e;

void N_InitProtocol(void);
void N_HandlePacket(netpeer_t *np, void *data, size_t data_size);
void N_UpdateSync(void);

void SV_SendAuthResponse(netpeer_t *np, auth_level_e auth_level);
void SV_SendPing(netpeer_t *np);
void SV_SendMessage(netpeer_t *np, const char *message);
void SV_BroadcastMessage(const char *message);
void SV_BroadcastPrintf(const char *fmt, ...) PRINTF_DECL(1, 2);
void SV_BroadcastPlayerNameChanged(netpeer_t *np, const char *new_name);
void SV_BroadcastPlayerTeamChanged(netpeer_t *np, uint32_t new_team_id);
void SV_BroadcastPlayerPWOChanged(netpeer_t *np);
void SV_BroadcastPlayerWSOPChanged(netpeer_t *np,
                                   unsigned char new_wsop_flags);
void SV_BroadcastPlayerBobbingChanged(netpeer_t *np,
                                      double new_bobbing_amount);
void SV_BroadcastPlayerAutoaimChanged(netpeer_t *np,
                                      bool new_autoaim_enabled);
void SV_BroadcastPlayerWeaponSpeedChanged(netpeer_t *np,
                                          unsigned char new_weapon_speed);
void SV_BroadcastPlayerColorChanged(netpeer_t *np, unsigned char new_red,
                                                   unsigned char new_green,
                                                   unsigned char new_blue);
void SV_BroadcastPlayerColorIndexChanged(netpeer_t *np, int new_color);
void SV_BroadcastPlayerSkinChanged(netpeer_t *np);
void SV_BroadcastStateUpdates(void);
void SV_ResyncPeers(void);
void SV_BroadcastGameActionChange(void);

void CL_SendSetupRequest(void);
void CL_SendPing(double server_time);
void CL_SendMessageToServer(const char *message);
void CL_SendMessageToPeer(uint32_t peer_id, const char *message);
void CL_SendMessageToTeam(const char *message);
void CL_SendMessage(const char *message);
void CL_SendCommands(void);
void CL_SendSaveGameNameChange(const char *new_save_game_name);
void CL_SendNameChange(const char *new_name);
void CL_SendTeamChange(unsigned char new_team);
void CL_SendPWOChange(void); /* CG: TODO */
void CL_SendWSOPChange(unsigned char new_wsop_flags);
void CL_SendBobbingChange(double new_bobbing_amount);
void CL_SendAutoaimChange(bool new_autoaim_enabled);
void CL_SendWeaponSpeedChange(unsigned char new_weapon_speed);
void CL_SendColorChange(unsigned char new_red, unsigned char new_green,
                                               unsigned char new_blue);
void CL_SendColorIndexChange(int new_color);
void CL_SendSkinChange(void); /* CG: TODO */
void CL_SendStateReceived(void);
void CL_SendAuthRequest(const char *password);
void CL_SendRCONCommand(const char *command);
void CL_SendVoteRequest(const char *command);

#endif

/* vi: set et ts=2 sw=2: */

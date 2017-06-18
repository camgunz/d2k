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


#ifndef N_MAIN_H__
#define N_MAIN_H__

struct player_s;
typedef struct player_s player_t;

struct net_peer_s;
typedef struct net_peer_s net_peer_t;

typedef enum {
  NM_NONE,
  NM_SETUP,
  NM_FULL_STATE,
  NM_MESSAGE,
  NM_SYNC,
  NM_CLIENT_ATTRIBUTE_CHANGE,
  NM_GAME_ACTION,
  NM_RCON_COMMAND,
  NM_VOTE_REQUEST,
  NM_PING,
  NM_MAX,
} net_message_e;

typedef enum {
  AUTH_LEVEL_NONE,
  AUTH_LEVEL_SPECTATOR,
  AUTH_LEVEL_PLAYER,
  AUTH_LEVEL_MODERATOR,
  AUTH_LEVEL_ADMINISTRATOR,
  AUTH_LEVEL_MAX
} auth_level_e;

typedef enum {
  CLIENT_ATTRIBUTE_AUTH = 1,
  CLIENT_ATTRIBUTE_NAME,
  CLIENT_ATTRIBUTE_TEAM,
  CLIENT_ATTRIBUTE_COLORMAP_INDEX,
  CLIENT_ATTRIBUTE_PWO,
  CLIENT_ATTRIBUTE_WSOP,
  CLIENT_ATTRIBUTE_BOBBING,
  CLIENT_ATTRIBUTE_AUTOAIM,
  CLIENT_ATTRIBUTE_WEAPON_SPEED,
  CLIENT_ATTRIBUTE_COLOR,
  CLIENT_ATTRIBUTE_SKIN,
} client_attribute_e;

void            N_Disconnect(disconnection_reason_e reason);
void            N_Shutdown(void);
bool            N_Listen(const char *host, uint16_t port);
base_netpeer_t* N_Connect(const char *host, uint16_t port);
bool            N_Connected(void);
bool            N_Reconnect(void);
bool            N_ConnectToServer(const char *address);
void            N_DisconnectPeer(net_peer_t *np,
                                 disconnection_reason_e reason);
void            N_DisconnectPlayerID(uint32_t player_id,
                                     disconnection_reason_e reason);
void            N_DisconnectPlayer(player_t *player,
                                   disconnection_reason_e reason);
void            N_ServiceNetworkTimeout(int timeout_ms);
void            N_ServiceNetwork(void);
uint32_t        N_GetUploadBandwidth(void);
uint32_t        N_GetDownloadBandwidth(void);
void            N_InitNetGame(void);
void            N_RunTic(void);
bool            N_TryRunTics(void);

#endif

/* vi: set et ts=2 sw=2: */

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


#ifndef NET_H__
#define NET_H__

#define DEFAULT_PORT 10666
#define NET_SETUP_TIMEOUT 1
#define NET_PEER_MINIMUM_TIMEOUT 1
#define NET_PEER_MAXIMUM_TIMEOUT 10
#define NET_CONNECT_TIMEOUT 1
#define NET_DISCONNECT_TIMEOUT 1

#define MULTINET     (netgame && (!solonet) && (!netdemo))
#define CLIENT       (MULTINET && (!netserver))
#define SERVER       (MULTINET && netserver)
#define SINGLEPLAYER (!demorecording && !demoplayback && !democontinue && !netgame)
#define comperr(i)   (default_comperr[i] && \
                      !demorecording && \
                      !demoplayback && \
                      !democontinue && \
                      !netgame)

typedef enum {
  NET_CHANNEL_RELIABLE,
  NET_CHANNEL_UNRELIABLE,
  NET_CHANNEL_MAX
} net_channel_e;

#define CHAT_CHANNEL_MIN CHAT_CHANNEL_SERVER
#define CHAT_CHANNEL_MAX CHAT_CHANNEL_ALL

typedef enum {
  CHAT_CHANNEL_SERVER,
  CHAT_CHANNEL_PLAYER,
  CHAT_CHANNEL_TEAM,
  CHAT_CHANNEL_ALL
} chat_channel_e;

typedef enum {
  AUTH_LEVEL_NONE,
  AUTH_LEVEL_SPECTATOR,
  AUTH_LEVEL_PLAYER,
  AUTH_LEVEL_MODERATOR,
  AUTH_LEVEL_ADMINISTRATOR,
  AUTH_LEVEL_MAX
} auth_level_e;

typedef struct netticcmd_s {
  int   index;
  int   tic;
  int   server_tic;
  char  forward;
  char  side;
  short angle;
  byte  buttons;
} netticcmd_t;

extern bool netgame;
extern bool netdemo;
extern bool solonet;
extern bool netserver;

size_t      N_IPToString(uint32_t address, char *buffer);
const char* N_IPToConstString(uint32_t address);
bool        N_IPToInt(const char *address_string, uint32_t *address_int);
size_t      N_GetHostFromAddressString(const char *address, char **host);
bool        N_GetPortFromAddressString(const char *address, uint16_t *port);
size_t      N_ParseAddressString(const char *address, char **host,
                                                      uint16_t *port);
void        N_Init(void);
void        N_Disconnect(void);
void        N_Shutdown(void);
bool        N_Listen(const char *host, uint16_t port);
bool        N_Connect(const char *host, uint16_t port);
bool        N_Reconnect(void);
bool        N_ConnectToServer(const char *address);
void        N_PrintAddress(FILE *fp, int peernum);
void        N_DisconnectPeer(int peernum);
void        N_DisconnectPlayer(short playernum);
void        N_ServiceNetworkTimeout(int timeout_ms);
void        N_ServiceNetwork(void);
uint32_t    N_GetUploadBandwidth(void);
uint32_t    N_GetDownloadBandwidth(void);

#endif

/* vi: set et ts=2 sw=2: */


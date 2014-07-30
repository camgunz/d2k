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

#ifndef NET_H__
#define NET_H__

#define DEFAULT_PORT 10666
#define NET_TIMEOUT 6
#define CONNECT_TIMEOUT NET_TIMEOUT
#define DISCONNECT_TIMEOUT NET_TIMEOUT

#define MULTINET    (netgame && (!solonet))
#define CMDSYNC     (netsync == NET_SYNC_TYPE_COMMAND)
#define DELTASYNC   (netsync == NET_SYNC_TYPE_DELTA)
#define CLIENT      (MULTINET && (!netserver))
#define SERVER      (MULTINET && netserver)
#define CMDCLIENT   (CLIENT && CMDSYNC)
#define CMDSERVER   (SERVER && CMDSYNC)
#define DELTACLIENT (CLIENT && DELTASYNC)
#define DELTASERVER (SERVER && DELTASYNC)

typedef enum {
  AUTH_LEVEL_NONE,
  AUTH_LEVEL_SPECTATOR,
  AUTH_LEVEL_PLAYER,
  AUTH_LEVEL_MODERATOR,
  AUTH_LEVEL_ADMINISTRATOR,
  AUTH_LEVEL_MAX
} auth_level_e;

typedef enum {
  NET_CHANNEL_RELIABLE,
  NET_CHANNEL_UNRELIABLE,
  NET_CHANNEL_MAX
} net_channel_e;

typedef enum {
  NET_SYNC_TYPE_NONE,
  NET_SYNC_TYPE_COMMAND,
  NET_SYNC_TYPE_DELTA,
} net_sync_type_e;

typedef struct netticcmd_s {
  int index;
  int tic;
  ticcmd_t cmd;
} netticcmd_t;

extern dboolean        netgame;
extern dboolean        solonet;
extern dboolean        netserver;
extern net_sync_type_e netsync;

size_t      N_IPToString(int address, char *buffer);
const char* N_IPToConstString(int address);
int         N_IPToInt(const char *address);
size_t      N_GetHostFromAddressString(const char *address, char **host);
dboolean    N_GetPortFromAddressString(const char *address,
                                       unsigned short *port);
size_t      N_ParseAddressString(const char *address, char **host,
                                 unsigned short *port);

void        N_Init(void);
void        N_Disconnect(void);
void        N_Shutdown(void);
dboolean    N_Listen(const char *host, unsigned short port);
dboolean    N_Connect(const char *host, unsigned short port);
dboolean    N_Reconnect(void);
dboolean    N_ConnectToServer(const char *address);
void        N_PrintAddress(FILE *fp, int peernum);
void        N_DisconnectPeer(int peernum);
void        N_DisconnectPlayer(short playernum);
void        N_ServiceNetworkTimeout(int timeout_ms);
void        N_ServiceNetwork(void);
uint32_t    N_GetUploadBandwidth(void);
uint32_t    N_GetDownloadBandwidth(void);

#endif


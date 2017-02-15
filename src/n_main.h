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

#define DEFAULT_PORT 10666
#define NET_PEER_MINIMUM_TIMEOUT 1
#define NET_PEER_MAXIMUM_TIMEOUT 10
#define NET_CONNECT_TIMEOUT 1
#define NET_DISCONNECT_TIMEOUT 1

#define MULTINET     (netgame && (!solonet) && (!netdemo))
#define CLIENT       (MULTINET && (!netserver))
#define SERVER       (MULTINET && netserver)
#define SINGLEPLAYER (!demorecording && !demoplayback && !democontinue && !netgame)

struct game_state_delta_s;
typedef struct game_state_delta_s game_state_delta_t;

typedef enum {
  NET_CHANNEL_RELIABLE,
  NET_CHANNEL_UNRELIABLE,
  NET_CHANNEL_MAX
} net_channel_e;

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

typedef struct {
  const char *name;
  net_channel_e channel;
} net_message_info_t;

struct netpeer_s;
typedef struct netpeer_s netpeer_t;

typedef void netpeer_iter_t;

#define NETPEER_FOR_EACH(pin) \
  for (netpeer_iterator_t (pin) = {NULL, NULL}; N_PeerIter(&pin.it, &pin.np);)

typedef struct netpeer_iterator_s {
  netpeer_iter_t *it;
  netpeer_t *np;
} netpeer_iterator_t;

extern bool netgame;
extern bool netdemo;
extern bool solonet;
extern bool netserver;

extern net_message_info_t network_message_info[NM_MAX];

/* Net Addresses (n_addr.c) */

size_t      N_IPToString(uint32_t address, char *buffer);
const char* N_IPToConstString(uint32_t address);
bool        N_IPToInt(const char *address_string, uint32_t *address_int);
size_t      N_GetHostFromAddressString(const char *address, char **host);
bool        N_GetPortFromAddressString(const char *address, uint16_t *port);
size_t      N_ParseAddressString(const char *address, char **host,
                                                      uint16_t *port);

/* General Networking (n_net.c) */

void     N_Init(void);
void     N_Disconnect(void);
void     N_Shutdown(void);
bool     N_Listen(const char *host, uint16_t port);
bool     N_Connect(const char *host, uint16_t port);
bool     N_Connected(void);
bool     N_Reconnect(void);
bool     N_ConnectToServer(const char *address);
void     N_DisconnectPeer(netpeer_t *np);
void     N_DisconnectPlayer(unsigned short playernum);
void     N_ServiceNetworkTimeout(int timeout_ms);
void     N_ServiceNetwork(void);
uint32_t N_GetUploadBandwidth(void);
uint32_t N_GetDownloadBandwidth(void);

/* Miscellaneous Networking (n_misc.c) */

bool        N_GetWad(const char *name);
const char* N_RunningStateName(void);

/* Main Networking (n_main.c) */

void N_InitNetGame(void);
void N_RunTic(void);
bool N_TryRunTics(void);

/* Net Protocol (n_proto.c) */

void N_InitProtocol(void);
void N_HandlePacket(netpeer_t *np, void *data, size_t data_size);
void N_UpdateSync(void);

void SV_SendAuthResponse(unsigned short playernum, auth_level_e auth_level);
void SV_SendPing(unsigned short playernum);
void SV_SendMessage(unsigned short playernum, const char *message);
void SV_BroadcastMessage(const char *message);
void SV_BroadcastPrintf(const char *fmt, ...) PRINTF_DECL(1, 2);
void SV_BroadcastPlayerNameChanged(unsigned short playernum,
                                   const char *new_name);
void SV_BroadcastPlayerTeamChanged(unsigned short playernum,
                                   unsigned char new_team);
void SV_BroadcastPlayerPWOChanged(unsigned short playernum);
void SV_BroadcastPlayerWSOPChanged(unsigned short playernum,
                                   unsigned char new_wsop_flags);
void SV_BroadcastPlayerBobbingChanged(unsigned short playernum,
                                      double new_bobbing_amount);
void SV_BroadcastPlayerAutoaimChanged(unsigned short playernum,
                                      bool new_autoaim_enabled);
void SV_BroadcastPlayerWeaponSpeedChanged(unsigned short playernum,
                                          unsigned char new_weapon_speed);
void SV_BroadcastPlayerColorChanged(unsigned short playernum,
                                    unsigned char new_red,
                                    unsigned char new_green,
                                    unsigned char new_blue);
void SV_BroadcastPlayerColorIndexChanged(unsigned short playernum,
                                         int new_color);
void SV_BroadcastPlayerSkinChanged(unsigned short playernum);
void SV_BroadcastStateUpdates(void);
void SV_ResyncPeers(void);
void SV_BroadcastGameActionChange(void);

void CL_SendSetupRequest(void);
void CL_SendPing(double server_time);
void CL_SendMessageToServer(const char *message);
void CL_SendMessageToPlayer(unsigned short recipient, const char *message);
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
void CL_SendColorChange(unsigned char new_red,
                        unsigned char new_green,
                        unsigned char new_blue);
void CL_SendColorIndexChange(int new_color);
void CL_SendSkinChange(void); /* CG: TODO */
void CL_SendStateReceived(void);
void CL_SendAuthRequest(const char *password);
void CL_SendRCONCommand(const char *command);
void CL_SendVoteRequest(const char *command);

/* Net Message Packing (n_pack.c) */

void N_PackSetupRequest(netpeer_t *np);

void N_PackSetup(netpeer_t *np);
bool N_UnpackSetup(netpeer_t *np, unsigned short *playernum);

void N_PackFullState(netpeer_t *np);
bool N_UnpackFullState(netpeer_t *np);

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
bool N_UnpackSkinChange(netpeer_t *np, unsigned short *playernum);

void N_PackAuthRequest(netpeer_t *np, const char *password);
bool N_UnpackAuthRequest(netpeer_t *np, buf_t *buf);

void N_PackRCONCommand(netpeer_t *np, const char *command);
bool N_UnpackRCONCommand(netpeer_t *np, buf_t *buf);

void N_PackVoteRequest(netpeer_t *np, const char *command);
bool N_UnpackVoteRequest(netpeer_t *np, buf_t *buf);

void N_PackGameActionChange(netpeer_t *np);
bool N_UnpackGameActionChange(netpeer_t *np, gameaction_t *new_gameaction,
                                             int *new_gametic);

/* Net Peers (n_peer.c) */

unsigned int N_PeerGetPlayernum(netpeer_t *np);
void         N_PeerSetPlayernum(netpeer_t *np, unsigned int playernum);
double       N_PeerGetConnectionWaitTime(netpeer_t *np);
double       N_PeerGetDisconnectionWaitTime(netpeer_t *np);
double       N_PeerGetLastSetupRequestTime(netpeer_t *np);
void         N_PeerUpdateLastSetupRequestTime(netpeer_t *np);
auth_level_e N_PeerGetAuthLevel(netpeer_t *np);
void         N_PeerSetAuthLevel(netpeer_t *np, auth_level_e auth_level);
bool         N_PeerCheckTimeout(netpeer_t *np);
bool         N_PeerCanRequestSetup(netpeer_t *np);

uint32_t       N_PeerGetIPAddress(netpeer_t *np);
const char*    N_PeerGetIPAddressConstString(netpeer_t *np);
unsigned short N_PeerGetPort(netpeer_t *np);

void         N_InitPeers(void);
unsigned int N_PeerGetCount(void);
netpeer_t*   N_PeerAdd(void *enet_peer);
netpeer_t*   N_PeerGet(int peernum);
netpeer_t*   N_PeerForPeer(void *enet_peer);
netpeer_t*   N_PeerForPlayer(unsigned int playernum);
unsigned int N_PeerGetNumForPlayer(unsigned int playernum);
void         N_PeerRemove(netpeer_t *np);

void N_PeerSetConnected(netpeer_t *np);
void N_PeerDisconnect(netpeer_t *np);

bool N_PeerIter(netpeer_iter_t **it, netpeer_t **np);
void N_PeerIterRemove(netpeer_iter_t *it, netpeer_t *np);

void    N_PeerFlushChannel(netpeer_t *np, net_channel_e channel);
void    N_PeerFlushChannels(netpeer_t *np);
pbuf_t* N_PeerBeginMessage(netpeer_t *np, net_message_e type);
bool    N_PeerSetIncoming(netpeer_t *np, unsigned char *data, size_t size);
bool    N_PeerLoadNextMessage(netpeer_t *np, net_message_e *message_type);
pbuf_t* N_PeerGetIncomingMessageData(netpeer_t *np);
void    N_PeerClearChannel(netpeer_t *np, net_channel_e channel);
void    N_PeerSendReset(netpeer_t *np);
size_t  N_PeerGetBytesUploaded(netpeer_t *np);
size_t  N_PeerGetBytesDownloaded(netpeer_t *np);
void*   N_PeerGetENetPeer(netpeer_t *np);

bool N_PeerSyncNeedsGameInfo(netpeer_t *np);
void N_PeerSyncSetNeedsGameInfo(netpeer_t *np);
void N_PeerSyncSetHasGameInfo(netpeer_t *np);
bool N_PeerSyncNeedsGameState(netpeer_t *np);
void N_PeerSyncSetNeedsGameState(netpeer_t *np);
void N_PeerSyncSetHasGameState(netpeer_t *np);
bool N_PeerSynchronized(netpeer_t *np);
bool N_PeerSyncOutdated(netpeer_t *np);
void N_PeerSyncSetOutdated(netpeer_t *np);
void N_PeerSyncSetNotOutdated(netpeer_t *np);
bool N_PeerSyncUpdated(netpeer_t *np);
void N_PeerSyncSetUpdated(netpeer_t *np);
void N_PeerSyncSetNotUpdated(netpeer_t *np);

bool N_PeerTooLagged(netpeer_t *np);

int  N_PeerGetSyncTIC(netpeer_t *np);
void N_PeerSetSyncTIC(netpeer_t *np, int sync_tic);
void N_PeerUpdateSyncTIC(netpeer_t *np, int sync_tic);

unsigned int N_PeerGetSyncCommandIndex(netpeer_t *np);
void         N_PeerSetSyncCommandIndex(netpeer_t *np,
                                       unsigned int command_index);
void         N_PeerUpdateSyncCommandIndex(netpeer_t *np,
                                          unsigned int command_index);

game_state_delta_t* N_PeerGetSyncStateDelta(netpeer_t *np);
void                N_PeerUpdateSyncStateDelta(netpeer_t *np,
                                               int from_tic,
                                               int to_tic,
                                               pbuf_t *delta_data);
void                N_PeerBuildNewSyncStateDelta(netpeer_t *np);
bool                N_PeerSyncStateDeltaUpdated(netpeer_t *np);
void                N_PeerResetSyncStateDelta(netpeer_t *np, int sync_tic,
                                                             int from_tic,
                                                             int to_tic);

unsigned int N_PeerGetSyncCommandIndexForPlayer(netpeer_t *np,
                                                unsigned int playernum);
void         N_PeerSetSyncCommandIndexForPlayer(netpeer_t *np,
                                                unsigned int playernum,
                                                unsigned int command_index);
void         N_PeerUpdateSyncCommandIndexForPlayer(netpeer_t *np,
                                                   unsigned int playernum,
                                                   unsigned int command_index);

void N_PeerResetSync(netpeer_t *np);

#endif

/* vi: set et ts=2 sw=2: */

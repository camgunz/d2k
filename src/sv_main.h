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


#ifndef SV_MAIN_H__
#define SV_MAIN_H__

typedef struct remote_server_s {
  netcom_t  com;
  netsync_t sync;
} remote_server_t;

extern int   sv_limit_player_commands;
extern char *sv_spectate_password;
extern char *sv_join_password;
extern char *sv_moderate_password;
extern char *sv_administrate_password;

void           SV_CleanupOldCommandsAndStates(void);
unsigned int   SV_GetPlayerCommandLimit(int playernum);
void           SV_UnlagSetTIC(int tic);
bool           SV_UnlagStart(void);
void           SV_UnlagEnd(void);
const char*    SV_GetServerName(void);
const char*    SV_GetDirSrvGroup(const char *address, unsigned short port);
const char*    SV_GetHost(void);
unsigned short SV_GetPort(void);
void           SV_DisconnectPeer(netpeer_t *np, disconnection_reason_e reason);
void           SV_DisconnectPlayerID(uint32_t player_id,
                                     disconnection_reason_e reason);
void           SV_DisconnectPlayer(player_t *player,
                                   disconnection_reason_e reason);

#endif

/* vi: set et ts=2 sw=2: */


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

#ifndef I_NETWORK_H__
#define I_NETWORK_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_SDL_NET
 #include "SDL_net.h"
 #define UDP_SOCKET UDPsocket
 #define UDP_PACKET UDPpacket
 #define AF_INET
 #define UDP_CHANNEL int
 extern UDP_SOCKET udp_socket;
#else
 #define UDP_CHANNEL struct sockaddr
#endif

#ifndef IPPORT_RESERVED
        #define IPPORT_RESERVED 1024
#endif

void I_InitNetwork(void);
size_t I_GetPacket(packet_header_t* buffer, size_t buflen);
void I_SendPacket(packet_header_t* packet, size_t len);
void I_WaitForPacket(int ms);

#ifdef USE_SDL_NET
UDP_SOCKET I_Socket(Uint16 port);
int I_ConnectToServer(const char *serv);
UDP_CHANNEL I_RegisterPlayer(IPaddress *ipaddr);
void I_UnRegisterPlayer(UDP_CHANNEL channel);
extern IPaddress sentfrom_addr;
#endif

#ifdef AF_INET
void I_SendPacketTo(packet_header_t* packet, size_t len, UDP_CHANNEL *to);
void I_SetupSocket(int sock, int port, int family);
void I_PrintAddress(FILE* fp, UDP_CHANNEL *addr);

extern UDP_CHANNEL sentfrom;
extern int v4socket, v6socket;
#endif

extern size_t sentbytes, recvdbytes;

#endif


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


#ifndef I_NET_H__
#define I_NET_H__

typedef void base_netpeer_t;
typedef void netpacket_t;
typedef void netevent_t;

typedef (void)(net_connection_handler_f)(base_net_peer_t *peer);
typedef (void)(net_disconnection_handler_f)(base_net_peer_t *peer,
                                            disconnection_reason_e reason);
typedef (void)(net_data_handler_f)(base_net_peer_t *peer, unsigned char *data,
                                                          size_t size);

void     I_NetPeerInit(base_netpeer_t *peer);
void     I_NetPeerSendPacket(base_netpeer_t *peer, netpacket_t *packet);
void     I_NetPeerSendReset(base_netpeer_t *peer);
uint32_t I_NetPeerGetIPAddress(base_netpeer_t *peer);
float    I_NetPeerGetPacketLoss(base_netpeer_t *peer);
float    I_NetPeerGetPacketLossJitter(base_netpeer_t *peer);

netpacket_t*   I_NetPacketNewReliable(size_t size);
netpacket_t*   I_NetPacketNewUnreliable(size_t size);
unsigned char* I_NetPacketGetData(netpacket_t *packet);

base_net_peer_t* I_NetEventGetPeer(netevent_t *event);
uint32_t         I_NetEventGetData(netevent_t *event);
netpacket_t*     I_NetEventGetData(netevent_t *event);
bool             I_NetEventIsConnection(netevent_t *event);
bool             I_NetEventIsDisconnection(netevent_t *event);
bool             I_NetEventIsData(netevent_t *event);

void I_NetInit(void);
bool I_NetListen(const char *host,
                 uint16_t port,
                 net_connection_handler_f net_connection_handler,
                 net_disconnection_handler_f net_disconnection_handler,
                 net_data_handler_f net_data_handler);
bool I_NetConnect(const char *host,
                  uint16_t port,
                  net_connection_handler_f net_connection_handler,
                  net_disconnection_handler_f net_disconnection_handler,
                  net_data_handler_f net_data_handler);
bool I_NetConnected(void);
bool I_NetReconnect(void);
void I_NetSetConnectionHandler(net_connection_handler_f handler);
void I_NetSetDisconnectionHandler(net_disconnection_handler_f handler);
void I_NetSetDataHandler(net_data_handler_f handler);
void I_NetDisconnect(disconnection_reason_e reason);
void I_NetShutdown(void);

/* vi: set et ts=2 sw=2: */

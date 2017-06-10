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


#ifndef N_COM_H__
#define N_COM_H__

typedef struct netcom_s {
  base_netpeer_t *base_netpeer;
  netchan_t       incoming;
  netchan_t       outgoing_reliable;
  netchan_t       outgoing_unreliable;
  size_t          bytes_uploaded;
  size_t          bytes_downloaded;
} netcom_t;

void        N_ComInit(netcom_t *nc, void *base_netpeer);
void        N_ComFree(netcom_t *nc);
void        N_ComReset(netcom_t *nc);
uint32_t    N_ComGetIPAddress(netcom_t *nc);
const char* N_ComGetIPAddressConstString(netcom_t *nc);
uint16_t    N_ComGetPort(netcom_t *nc);
float       N_ComGetPacketLoss(netcom_t *nc);
float       N_ComGetPacketLossJitter(netcom_t *nc);
size_t      N_ComGetBytesUploaded(netcom_t *nc);
size_t      N_ComGetBytesDownloaded(netcom_t *nc);
pbuf_t*     N_ComBeginMessage(netcom_t *nc, net_message_e type);
bool        N_ComSetIncoming(netcom_t *nc, unsigned char *data, size_t size);
void        N_ComFlushChannels(netcom_t *nc);
void        N_ComFlushReliableChannel(netcom_t *nc);
void        N_ComFlushUnreliableChannel(netcom_t *nc);
void        N_ComClearReliableChannel(netcom_t *nc);
void        N_ComClearUnreliableChannel(netcom_t *nc);
void        N_ComSendReset(netcom_t *nc);
pbuf_t*     N_ComGetIncomingMessageData(netcom_t *nc);
bool        N_ComLoadNextMessage(netcom_t *nc, net_message_e *message_type);

#endif

/* vi: set et ts=2 sw=2: */

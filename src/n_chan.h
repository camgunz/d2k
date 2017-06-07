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


#ifndef N_CHAN_H__
#define N_CHAN_H__

typedef struct netchan_s {
  bool    reliable;
  bool    throttled;
  GArray *toc;
  pbuf_t  messages;
  pbuf_t  packet_data;
  pbuf_t  packed_toc;
  int     last_flush_tic;
} netchan_t;

void    N_ChannelInit(netchan_t *nc, bool reliable, bool throttled);
void    N_ChannelClear(netchan_t *nc);
void    N_ChannelFree(netchan_t *nc);
size_t  N_ChannelFlush(netchan_t *nc);
pbuf_t* N_ChannelBeginMessage(netchan_t *nc, unsigned char type);
pbuf_t* N_ChannelGetMessage(netchan_t *nc);
bool    N_ChannelLoadFromData(netchan_t *nc, unsigned char *data, size_t size);
bool    N_ChannelLoadNextMessage(netchan_t *nc, net_message_e *message_type);

#endif

/* vi: set et ts=2 sw=2: */

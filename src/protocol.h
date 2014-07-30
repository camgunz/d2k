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

#ifndef __PROTOCOL__
#define __PROTOCOL__

#include "doomtype.h"
#include "d_ticcmd.h"
#include "m_swap.h"

enum packet_type_e {
  PKT_INIT,    // initial packet to server
  PKT_SETUP,   // game information packet
  PKT_GO,      // game has started
  PKT_TICC,    // tics from client
  PKT_TICS,    // tics from server
  PKT_RETRANS, // Request for retransmission
  PKT_EXTRA,   // Extra info packet
  PKT_QUIT,    // Player quit game
  PKT_DOWN,    // Server downed
  PKT_WAD,     // Wad file request
  PKT_BACKOFF, // Request for client back-off
};

typedef struct {
  byte checksum;       // Simple checksum of the entire packet
  byte type;           /* Type of packet */
  byte reserved[2];	/* Was random in prboom <=2.2.4, now 0 */
  unsigned tic;        // Timestamp
} PACKEDATTR packet_header_t;

static inline void packet_set(packet_header_t* p, enum packet_type_e t, unsigned long tic)
{ p->tic = doom_htonl(tic); p->type = t; p->reserved[0] = 0; p->reserved[1] = 0; }

#ifndef GAME_OPTIONS_SIZE
// From g_game.h
#define GAME_OPTIONS_SIZE 64
#endif

struct setup_packet_s {
  byte players, yourplayer, skill, episode, level, deathmatch, complevel, ticdup, extratic;
  byte game_options[GAME_OPTIONS_SIZE];
  byte numwads;
  byte wadnames[1]; // Actually longer
};

/* cph - convert network byte stream to usable ticcmd_t and visa-versa
 *     - the functions are functionally identical apart from parameters
 *     - the void* param can be unaligned. By using void* as the parameter
 *       it means gcc won't assume alignment so won't make false assumptions
 *       when optimising. So I'm told.
 */
inline static void RawToTic(ticcmd_t* dst, const void* src)
{
  memcpy(dst,src,sizeof *dst);
  dst->angleturn = doom_ntohs(dst->angleturn);
  dst->consistancy = doom_ntohs(dst->consistancy);
}

inline static void TicToRaw(void* dst, const ticcmd_t* src)
{
  /* We have to make a copy of the source struct, then do byte swaps,
   * and fnially copy to the destination (can't do the swaps in the
   * destination, because it might not be aligned).
   */
  ticcmd_t tmp = *src;
  tmp.angleturn = doom_ntohs(tmp.angleturn);
  tmp.consistancy = doom_ntohs(tmp.consistancy);
  memcpy(dst,&tmp,sizeof tmp);
}

#endif // __PROTOCOL__


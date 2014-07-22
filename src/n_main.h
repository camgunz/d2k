/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *
 *-----------------------------------------------------------------------------*/

#ifndef N_MAIN__
#define N_MAIN__

void N_LogPlayerPosition(player_t *player);
void N_PrintPlayerCommands(cbuf_t *commands);
void N_InitNetGame(void);
bool N_GetWad(const char *name);

bool CL_LoadingState(void);
bool CL_Predicting(void);
bool CL_ReceivedSetup(void);
void CL_SetReceivedSetup(dboolean new_received_setup);
void CL_SetAuthorizationLevel(auth_level_e level);
bool CL_LoadState(void);

void SV_RemoveOldCommands(void);
void SV_RemoveOldStates(void);

cbuf_t* N_GetLocalCommands(void);
void    N_ResetLocalCommandIndex(void);
void    N_TryRunTics(void);

#endif

/* vi: set et ts=2 sw=2: */

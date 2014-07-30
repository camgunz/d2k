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


#ifndef P_MAP_H__
#define P_MAP_H__

#include "r_defs.h"
#include "d_player.h"

#define USERANGE        (64*FRACUNIT)
#define MELEERANGE      (64*FRACUNIT)
#define MISSILERANGE    (32*64*FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes the spider demon
// is larger, but we do not have any moving sectors nearby
#define MAXRADIUS       (32*FRACUNIT)

// killough 3/15/98: add fourth argument to P_TryMove
dboolean P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, dboolean dropoff);

// killough 8/9/98: extra argument for telefragging
dboolean P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y,dboolean boss);
void    P_SlideMove(mobj_t *mo);
dboolean P_CheckSight(mobj_t *t1, mobj_t *t2);
void    P_UseLines(player_t *player);

typedef dboolean (*CrossSubsectorFunc)(int num);
extern CrossSubsectorFunc P_CrossSubsector;
dboolean P_CrossSubsector_Doom(int num);
dboolean P_CrossSubsector_Boom(int num);
dboolean P_CrossSubsector_PrBoom(int num);

// killough 8/2/98: add 'mask' argument to prevent friends autoaiming at others
fixed_t P_AimLineAttack(mobj_t *t1,angle_t angle,fixed_t distance, uint_64_t mask);

void    P_LineAttack(mobj_t *t1, angle_t angle, fixed_t distance,
                     fixed_t slope, int damage );
void    P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage);
dboolean P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);

//jff 3/19/98 P_CheckSector(): new routine to replace P_ChangeSector()
dboolean P_ChangeSector(sector_t* sector,dboolean crunch);
dboolean P_CheckSector(sector_t *sector, dboolean crunch);
void    P_DelSeclist(msecnode_t*);                          // phares 3/16/98
void    P_FreeSecNodeList(void);                            // sf
void    P_CreateSecNodeList(mobj_t*,fixed_t,fixed_t);       // phares 3/14/98
dboolean Check_Sides(mobj_t *, int, int);                    // phares

int     P_GetMoveFactor(mobj_t *mo, int *friction);   // killough 8/28/98
int     P_GetFriction(const mobj_t *mo, int *factor);       // killough 8/28/98
void    P_ApplyTorque(mobj_t *mo);                          // killough 9/12/98

/* cphipps 2004/08/30 */
void	P_MapStart(void);
void	P_MapEnd(void);

// If "floatok" true, move would be ok if within "tmfloorz - tmceilingz".
extern dboolean floatok;
extern dboolean felldown;   // killough 11/98: indicates object pushed off ledge
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;
extern line_t *ceilingline;
extern line_t *floorline;      // killough 8/23/98
extern mobj_t *linetarget;     // who got hit (or NULL)
extern mobj_t *crosshair_target;
extern msecnode_t *sector_list;                             // phares 3/16/98
extern fixed_t tmbbox[4];         // phares 3/20/98
extern line_t *blockline;   // killough 8/11/98

#endif // __P_MAP__

/* vi: set et ts=2 sw=2: */


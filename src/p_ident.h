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


#ifndef P_IDENT_H__
#define P_IDENT_H__

void     P_IdentInit(void);
void     P_IdentGetID(void *obj, uint32_t *obj_id);
void     P_IdentAssignID(void *obj, uint32_t obj_id);
void     P_IdentReleaseID(uint32_t *obj_id);
void*    P_IdentLookup(uint32_t id);
void     P_IdentReset(void);
uint32_t P_IdentGetMaxID(void);
void     P_IdentSetMaxID(uint32_t new_max_id);

#endif

/* vi: set et ts=2 sw=2: */


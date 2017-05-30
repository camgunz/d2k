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


#ifndef D_MOUSE_H__
#define D_MOUSE_H__

extern int mouse_sensitivity_horiz;
extern int mouse_sensitivity_vert;
extern int mouse_sensitivity_mlook;
extern int movement_mouselook;
extern int movement_mouseinvert;
extern int movement_maxviewpitch;
extern int mouse_handler;
extern int mouse_doubleclick_as_use;
extern int mouse_acceleration;
extern bool mlook_or_fov;
extern int mlooky;

bool D_MouseShouldBeGrabbed(void);
void D_Mouse(bool increase, int *value); /* killough */
void D_MouseChangeHoriz(bool increase);
void D_MouseChangeVert(bool increase);
void D_MouseChangeMouseLook(bool increase);
void D_MouseChangeAccel(bool increase);
void D_MouseScaleAccel(void);
int  D_MouseAccelerate(int val);
void D_ChangeMouseLook(void);
bool D_GetMouseLook(void);
bool D_HaveMouseLook(void);

void MN_MouseChangeHoriz(int choice);
void MN_MouseChangeVert(int choice);
void MN_MouseChangeMouseLook(int choice);
void MN_MouseChangeAccel(int choice);

#endif

/* vi: set et ts=2 sw=2: */

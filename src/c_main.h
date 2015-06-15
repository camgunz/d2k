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


#ifndef C_MAIN_H__
#define C_MAIN_H__

void C_Init(void);
bool C_ECIAvailable(void);
void C_Reset(void);
void C_ScrollDown(void);
void C_ScrollUp(void);
void C_ToggleScroll(void);
void C_Banish(void);
void C_SetFullscreen(void);
bool C_Active(void);
bool C_HandleInput(char *input_text);
void C_Printf(const char *fmt, ...) PRINTF_DECL(1, 2);
void C_VPrintf(const char *fmt, va_list args);
void C_MPrintf(const char *fmt, ...) PRINTF_DECL(1, 2);
void C_MVPrintf(const char *fmt, va_list args);
void C_Echo(const char *message);
void C_MEcho(const char *message);
void C_Write(const char *message);
void C_MWrite(const char *message);

#endif

/* vi: set et ts=2 sw=2: */


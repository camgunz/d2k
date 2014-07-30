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

#ifndef SC_MAN_H__
#define SC_MAN_H__

extern char *sc_String;
extern int sc_Number;
extern int sc_Line;
extern dboolean sc_End;
extern dboolean sc_Crossed;
extern dboolean sc_FileScripts;

void     SC_OpenLump(const char *name);
void     SC_OpenLumpByNum(int lump);
void     SC_Close(void);
dboolean SC_GetString(void);
void     SC_MustGetString(void);
void     SC_MustGetStringName(const char *name);
dboolean SC_GetNumber(void);
void     SC_MustGetNumber(void);
void     SC_UnGet(void);
dboolean SC_Check(void);
dboolean SC_Compare(const char *text);
int      SC_MatchString(const char **strings);
int      SC_MustMatchString(const char **strings);
void     SC_ScriptError(const char *message);

#endif // __SC_MAN__


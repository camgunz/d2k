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


#ifndef C_CVAR_H__
#define C_CVAR_H__

#define X_CVAR_NAMESPACE "cvars"

/*
 * [CG] The CVar API is a read-only API.  CVars should only be modified from
 *      scripting.
 */

bool        C_CVarGetBool(const char *section_path, const char *value);
int32_t     C_CVarGetInt(const char *section_path, const char *value);
uint32_t    C_CVarGetUInt(const char *section_path, const char *value);
double      C_CVarGetFloat(const char *section_path, const char *value);
const char* C_CVarGetString(const char *section_path, const char *value);

#endif

/* vi: set et ts=2 sw=2: */


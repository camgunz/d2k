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


#ifndef D_CFG_H__
#define D_CFG_H__

void    D_ConfigInit(void);
bool    D_ConfigSectionExists(const char *section_name);
bool    D_ConfigCreateSection(const char *section_name);

bool    D_ConfigSafeGetBool(const char *section_name, const char *value_name,
                                                      bool *stored_value);
bool    D_ConfigGetBool(const char *section_name, const char *value_name);
bool    D_ConfigSetBool(const char *section_name, const char *value_name,
                                                  bool value);

bool    D_ConfigSafeGetStr(const char *section_name, const char *value_name,
                                                     char **stored_value);
char*   D_ConfigGetStr(const char *section_name, const char *value_name);
bool    D_ConfigSetStr(const char *section_name, const char *value_name,
                                                 const char *new_value);

bool    D_ConfigSafeGetInt(const char *section_name, const char *value_name,
                                                     int64_t *stored_value);
int64_t D_ConfigGetInt(const char *section_name, const char *value_name);
bool    D_ConfigSetInt(const char *section_name, const char *value_name,
                                                 int64_t new_value);

bool    D_ConfigSafeGetDec(const char *section_name, const char *value_name,
                                                     double *stored_value);
double  D_ConfigGetDec(const char *section_name, const char *value_name);
bool    D_ConfigSetDec(const char *section_name, const char *value_name,
                                                 double new_value);

bool    D_ConfigWrite(const char *contents);

#endif

/* vi: set et ts=2 sw=2: */


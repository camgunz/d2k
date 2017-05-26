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


#ifndef M_MISC_H__
#define M_MISC_H__

double M_GetCurrentTime(void);
int    M_StrToInt(const char *s, int *l);
int    M_StrToFloat(const char *s, float *f);
int    M_DoubleToInt(double x);
char*  M_Strlwr(char* str);
char*  M_Strupr(char* str);
char*  M_StrRTrim(char* str);
char*  M_StrNormalizeSlashes(char *str);

#endif

/* vi: set et ts=2 sw=2: */

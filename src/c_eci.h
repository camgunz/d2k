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


#ifndef C_ECI_H__
#define C_ECI_H__

#ifdef G_OS_UNIX
#define C_ECIInit C_ECIInitUNIX
#define C_ECIService C_ECIServiceUNIX
#else
#define C_ECIInit C_ECIInitNonUNIX
#define C_ECIService C_ECIServiceNonUNIX
#endif

void C_ECIInitUNIX(void);
void C_ECIServiceUNIX(void);

void C_ECIInitNonUNIX(void);
void C_ECIServiceNonUNIX(void);

#endif

/* vi: set et ts=2 sw=2: */


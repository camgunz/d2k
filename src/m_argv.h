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


#ifndef M_ARGV_H__
#define M_ARGV_H__

/*
 * MISC
 */
extern int  myargc;
extern char **myargv;

/* Returns the position of the given parameter in the arg list (0 if not found). */
int M_CheckParm(const char *check);

/* Returns the position of the given parameter in the params list (-1 if not found). */
int M_CheckParmEx(const char *check, char **params, int paramscount);

/* Add one parameter to myargv list */
void M_AddParam(const char *param);

/* Parses the command line and sets up the argv[] array */
void M_ParseCmdLine(char *cmdstart, char **argv, char *args, int *numargs, int *numchars);

#endif

/* vi: set et ts=2 sw=2: */


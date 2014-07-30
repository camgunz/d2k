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


#ifndef I_SYSTEM_H__
#define I_SYSTEM_H__

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif

#ifdef _MSC_VER
#define    F_OK    0    /* Check for file existence */
#define    W_OK    2    /* Check for write permission */
#define    R_OK    4    /* Check for read permission */
#endif

extern int interpolation_method;
extern int ms_to_next_tick;

uint32_t      I_GetTicks(void);
dboolean      I_StartDisplay(void);
void          I_EndDisplay(void);
int           I_GetTime_RealTime(void);     /* killough */
#ifndef PRBOOM_SERVER
fixed_t       I_GetTimeFrac (void);
#endif
void          I_GetTime_SaveMS(void);
unsigned long I_GetRandomTimeSeed(void); /* cphipps */
void          I_uSleep(unsigned long usecs);
/*
 * cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char*   I_GetVersionString(char* buf, size_t sz); 
/*
 * cphipps - I_SigString
 * Returns a string describing a signal number
 */
const char*   I_SigString(char* buf, size_t sz, int signum);
#ifdef _WIN32
void          I_SwitchToWindow(HWND hwnd);
#endif
const char*   I_GetTempDir(void); // e6y
const char*   I_DoomExeDir(void); // killough 2/16/98: path to executable's dir
dboolean      HasTrailingSlash(const char* dn);
char*         I_FindFile(const char* wfname, const char* ext);
char*         I_FindFileEx(const char* wfname, const char* ext);
const char*   I_FindFile2(const char* wfname, const char* ext);
dboolean      I_FileToBuffer(const char *filename, byte **data, int *size);
/* cph 2001/11/18 - wrapper for read(2) which deals with partial reads */
void          I_Read(int fd, void* buf, size_t sz);
/* cph 2001/11/18 - Move W_Filelength to i_system.c */
int           I_Filelength(int handle);

#endif

/* vi: set et ts=2 sw=2: */


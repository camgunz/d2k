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


#ifndef D_RES_H__
#define D_RES_H__

/* cph - MBF-like wad/deh/bex autoload code */
/* proff 2001/7/1 - added prboom.wad as last entry so it's always loaded and
   doesn't overlap with the cfg settings */
#define MAXLOADFILES 3

// CPhipps - defined enum in wider scope
// Ty 08/29/98 - add source field to identify where this lump came from
typedef enum {
  // CPhipps - define elements in order of 'how new/unusual'
  source_iwad = 0,  // iwad file load 
  source_pre,       // predefined lump
  source_auto_load, // lump auto-loaded by config file
  source_pwad,      // pwad file load
  source_lmp,       // lmp file load
  source_net        // CPhipps

  /* e6y
  ,source_deh_auto_load
  ,source_deh
  ,source_err
  */
} wad_source_e;

extern const char *wad_file_names[MAXLOADFILES];
extern const char *deh_file_names[MAXLOADFILES];

GPtrArray*  D_ResNew(void);
void        D_ResAdd(GPtrArray *resources, const char *file,
                                           wad_source_e source,
                                           int handle);
void        D_ResClear(GPtrArray *resources);
void        D_ResFree(GPtrArray *resources);

void        D_ResourcesInit(void);
void        D_ResourcesSet(GPtrArray *wad_files);
void        D_AutoLoadResources(void);
void        D_AutoLoadPatches(void);
void        D_AddIWAD(const char *iwad);
void        D_SetIWAD(const char *iwad);
const char* D_GetIWAD(void);
const char* D_GetIWADPath(void);
void        D_ClearIWAD(void);
void        D_CheckIWAD(const char *iwadname, GameMode_e *gmode, bool *hassec);
char*       D_FindIWADFile(void);
void        D_IdentifyVersion(void);
void        D_AddDEH(const char *filename, int lumpnum);
void        D_AddResource(const char *file, wad_source_e source);
void        D_ClearResourceFiles(void);
void        D_ClearDEHFiles(void);

#endif

/* vi: set et ts=2 sw=2: */

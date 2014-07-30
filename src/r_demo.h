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

#ifndef R_DEMO_H__
#define R_DEMO_H__

#include "doomdef.h"
#include "doomtype.h"
#include "tables.h"
#include "d_player.h"
#include "w_wad.h"

//
// Smooth playing stuff
//

#define SMOOTH_PLAYING_MAXFACTOR 16 

extern int demo_smoothturns;
extern int demo_smoothturnsfactor;

void    R_SmoothPlaying_Reset(player_t *player);
void    R_SmoothPlaying_Add(int delta);
angle_t R_SmoothPlaying_Get(player_t *player);
void    R_ResetAfterTeleport(player_t *player);

//
// DemoEx stuff
//

typedef struct
{
  wadinfo_t header;
  filelump_t *lumps;
  char* data;
  int datasize;
} wadtbl_t;

typedef struct
{
  wadfile_info_t *wadfiles;
  size_t numwadfiles;
} waddata_t;

typedef struct
{
  int pattern_num;
  char pattern_name[80];
  char *missed;
} patterndata_t;

extern int demo_extendedformat;
extern int demo_extendedformat_default;
extern const char *demo_demoex_filename;

extern int demo_patterns_count;
extern const char *demo_patterns_mask;
extern char **demo_patterns_list;
extern const char *demo_patterns_list_def[];

extern const char *getwad_cmdline;

int WadDataInit(waddata_t *waddata);
int WadDataAddItem(waddata_t *waddata, const char *filename, wad_source_t source, int handle);
void WadDataFree(waddata_t *wadfiles);

int CheckDemoExDemo(void);
int CheckAutoDemo(void);
int ParseDemoPattern(const char *str, waddata_t* waddata, char **missed, dboolean trytodownload);
int DemoNameToWadData(const char * demoname, waddata_t *waddata, patterndata_t *patterndata);
void WadDataToWadFiles(waddata_t *waddata);
void WadFilesToWadData(waddata_t *waddata);

void M_ChangeDemoExtendedFormat(void);

byte* G_GetDemoFooter(const char *filename, const byte **footer, size_t *size);
void G_SetDemoFooter(const char *filename, wadtbl_t *wadtbl);
void G_WriteDemoFooter(FILE *file);
void I_DemoExShutdown(void);

void W_InitPWADTable(wadtbl_t *wadtbl);
void W_FreePWADTable(wadtbl_t *wadtbl);
void W_AddLump(wadtbl_t *wadtbl, const char *name, const byte* data, size_t size);

extern dboolean use_demoex_info;
void R_DemoEx_WriteMLook(angle_t pitch);
angle_t R_DemoEx_ReadMLook(void);

int IsDemoPlayback(void);
int IsDemoContinue(void);

int LoadDemo(const char *name, const byte **buffer, int *length, int *lump);

#endif // __R_DEMO__


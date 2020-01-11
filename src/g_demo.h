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


#ifndef G_DEMO_H__
#define G_DEMO_H__

struct player_s;
typedef struct player_s player_t;

struct wadtbl_s;
typedef struct wadtbl_s wadtbl_t;

#define DEMOMARKER 0x80
#define SMOOTH_PLAYING_MAXFACTOR 16 

//e6y
#define RDH_SAFE        0x00000001
#define RDH_SKIP_HEADER 0x00000002

typedef struct {
  int pattern_num;
  char pattern_name[80];
  char *missed;
} patterndata_t;

//e6y
extern bool  democontinue;
extern char *demo_continue_name;
extern int   demover;

extern int  demo_insurance;
extern int  default_demo_insurance; // killough 4/5/98

extern bool timingdemo; // Print timing information after quitting.  killough
extern bool singledemo; // Quit after playing a demo from cmdline.
extern bool fastdemo;   // Run tick clock at fastest speed possible while
                        // playing demo.  killough

extern int  demo_skiptics;
extern int  demo_tics_count;
extern int  demo_curr_tic;
extern int  demo_playerscount;
extern char demo_len_st[80];

extern bool demo_stoponnext;
extern bool demo_stoponend;
extern bool demo_warp;
extern int  demo_overwriteexisting;

extern int demo_smoothturns;
extern int demo_smoothturnsfactor;

extern bool use_demoex_info;

//e6y: for r_demo.c
extern int longtics;
extern int bytes_per_tic;

extern int          demo_extendedformat;
extern int          demo_extendedformat_default;
extern const char  *demo_demoex_filename;
extern int          demo_patterns_count;
extern const char  *demo_patterns_mask;
extern char       **demo_patterns_list;
extern const char  *demo_patterns_list_def[];
extern const char  *getwad_cmdline;

void G_BeginRecording(void);
void G_DeferedPlayDemo(const char *demo); // CPhipps - const
const unsigned char* G_ReadDemoHeaderEx(const unsigned char *demo_p,
                                        size_t size,
                                        unsigned int params);
const unsigned char* G_ReadDemoHeader(const unsigned char *demo_p,
                                      size_t size);
void G_CalculateDemoParams(const unsigned char *demo_p);
bool G_CheckDemoStatus(void);
void G_CheckDemoContinue(void);
void G_RecordDemo(const char *name); // Only called by startup code.

void    G_DemoSmoothPlayingReset(player_t *player);
void    G_DemoSmoothPlayingAdd(int delta);
angle_t G_DemoSmoothPlayingGet(player_t *player);
void    G_ResetAfterTeleport(player_t *player);

bool G_DemoCheckExDemo(void);
bool G_DemoCheckAutoDemo(void);
int  G_DemoParseDemoPattern(const char *str, GPtrArray *resources,
                                             char **missed,
                                             bool trytodownload);
int  G_DemoNameToWadData(const char *demoname, GPtrArray *resources,
                                               patterndata_t *patterndata);
void G_DemoChangeExtendedFormat(void);
void G_DemoSetFooter(const char *filename, wadtbl_t *wadtbl);
void G_DemoWriteFooter(FILE *file);
void G_DemoShutdown(void);

unsigned char* G_DemoGetFooter(const char *filename,
                               const unsigned char **footer,
                               size_t *size);


void    G_DemoWriteMouseLook(angle_t pitch);
angle_t G_DemoReadMouseLook(void);

const char* G_DemoGetDemoPlaybackArg(void);
const char* G_DemoGetDemoContinueArg(void);
const char* G_DemoGetDemoArg(void);

bool G_DemoIsPlayback(void);
bool G_DemoIsContinue(void);

bool G_DemoLoad(const char *name, unsigned char **buffer, int *length,
                                                          int *lump);
bool G_DemoCheckAutoDemo(void);
bool G_DemoCheckExDemo(void);

#endif

/* vi: set et ts=2 sw=2: */

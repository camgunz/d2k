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


#ifndef E6Y_H__
#define E6Y_H__

#define HU_HUDADDX (HU_HUDX)
#define HU_HUDADDY (HU_HUDY+(-1)*HU_GAPY)
#define HU_CENTERMSGX (320/2)
#define HU_CENTERMSGY ((200-ST_HEIGHT)/2 - 1 - LittleShort(hu_font[0].height))

#define HU_HUDADDX_D (HU_HUDX_LL)
#define HU_HUDADDY_D (HU_HUDY_LL+(-1)*HU_GAPY)

#define STSTR_SECRETFOUND   "A secret is revealed!"

#define S_CANT_GL_ARB_MULTITEXTURE 0x10000000
#define S_CANT_GL_ARB_MULTISAMPLEFACTOR  0x20000000

#define GL_COMBINE_ARB                    0x8570
#define GL_RGB_SCALE_ARB                  0x8573

#define FOV_CORRECTION_FACTOR (1.13776f)
#define FOV90 (90)

extern bool wasWiped;

extern int totalleveltimes;

extern int secretfound;

extern int avi_shot_time;
extern int avi_shot_num;
extern const char *avi_shot_fname;

extern int speed_step;

extern int hudadd_gamespeed;
extern int hudadd_leveltime;
extern int hudadd_demotime;
extern int hudadd_secretarea;
extern int hudadd_smarttotals;
extern int hudadd_demoprogressbar;
extern int hudadd_crosshair;
extern int hudadd_crosshair_scale;
extern int hudadd_crosshair_color;
extern int hudadd_crosshair_health;
extern int hudadd_crosshair_target;
extern int hudadd_crosshair_target_color;
extern int hudadd_crosshair_lock_target;
extern int movement_strafe50;
extern int movement_shorttics;
extern int movement_strafe50onturns;
extern int render_multisampling;
extern int render_paperitems;
extern int render_wipescreen;

extern int render_fov;
extern int render_aspect;
extern float render_ratio;
extern float render_fovratio;
extern float render_fovy;
extern float render_multiplier;
void M_ChangeAspectRatio(void);
void M_ChangeStretch(void);

extern int showendoom;

extern int palette_ondamage;
extern int palette_onbonus;
extern int palette_onpowers;

extern int PitchSign;
extern angle_t viewpitch;
extern float skyscale;
extern float screen_skybox_zplane;
extern float maxNoPitch[];
extern float tan_pitch;
extern float skyUpAngle;
extern float skyUpShift;
extern float skyXShift;
extern float skyYShift;

void e6y_assert(const char *format, ...);

void ParamsMatchingCheck();
void e6y_InitCommandLine(void);
#ifdef GL_DOOM
void M_ChangeFOV(void);
void M_ChangeUseDetail(void);
void M_ChangeMultiSample(void);
void M_ChangeSpriteClip(void);
void M_ChangeAllowBoomColormaps(void);
void M_ChangeTextureUseHires(void);
void M_ChangeAllowFog(void);
void M_ChangeTextureHQResize(void);
#endif
void M_ChangeRenderPrecise(void);
void M_ChangeSpeed(void);
void M_ChangeScreenMultipleFactor(void);
void M_ChangeInterlacedScanning(void);
void I_Init2(void);

extern float viewPitch;
extern bool transparentpresent;

void R_ClearClipSegs (void);
void R_RenderBSPNode(int bspnum);

typedef struct prboom_comp_s
{
  unsigned int minver;
  unsigned int maxver;
  bool state;
  const char *cmd;
} prboom_comp_t;

enum
{
  PC_MONSTER_AVOID_HAZARDS,
  PC_REMOVE_SLIME_TRAILS,
  PC_NO_DROPOFF,
  PC_TRUNCATED_SECTOR_SPECIALS,
  PC_BOOM_BRAINAWAKE,
  PC_PRBOOM_FRICTION,
  PC_REJECT_PAD_WITH_FF,
  PC_FORCE_LXDOOM_DEMO_COMPATIBILITY,
  PC_ALLOW_SSG_DIRECT,
  PC_TREAT_NO_CLIPPING_THINGS_AS_NOT_BLOCKING,
  PC_FORCE_INCORRECT_PROCESSING_OF_RESPAWN_FRAME_ENTRY,
  PC_FORCE_CORRECT_CODE_FOR_3_KEYS_DOORS_IN_MBF,
  PC_UNINITIALIZE_CRUSH_FIELD_FOR_STAIRS,
  PC_FORCE_BOOM_FINDNEXTHIGHESTFLOOR,
  PC_ALLOW_SKY_TRANSFER_IN_BOOM,
  PC_APPLY_GREEN_ARMOR_CLASS_TO_ARMOR_BONUSES,
  PC_APPLY_BLUE_ARMOR_CLASS_TO_MEGASPHERE,
  PC_FORCE_INCORRECT_BOBBING_IN_BOOM,
  PC_BOOM_DEH_PARSER,
  PC_MBF_REMOVE_THINKER_IN_KILLMOBJ,
  PC_DO_NOT_INHERIT_FRIENDLYNESS_FLAG_ON_SPAWN,
  PC_DO_NOT_USE_MISC12_FRAME_PARAMETERS_IN_A_MUSHROOM,
  PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL,
  PC_RESET_MONSTERSPAWNER_PARAMS_AFTER_LOADING,
  PC_MAX
};

extern prboom_comp_t prboom_comp[];

int StepwiseSum(int value, int direction, int step, int minval, int maxval, int defval);

enum
{
  TT_ALLKILL,
  TT_ALLITEM,
  TT_ALLSECRET,

  TT_TIME,
  TT_TOTALTIME,
  TT_TOTALKILL,
  TT_TOTALITEM,
  TT_TOTALSECRET,

  TT_MAX
};

typedef struct timetable_s
{
  char map[16];

  int kill[VANILLA_MAXPLAYERS];
  int item[VANILLA_MAXPLAYERS];
  int secret[VANILLA_MAXPLAYERS];
  
  int stat[TT_MAX];
} timetable_t;

#ifdef _WIN32
const char* WINError(void);
#endif

void e6y_G_DoCompleted(void);

void e6y_G_DoWorldDone(void);

extern int realtic_clock_rate;

void e6y_G_Compatibility(void);

#define I_FindName(a)	((a)->Name)
#define I_FindAttr(a)	((a)->Attribs)

typedef struct
{
	unsigned int Attribs;
	unsigned int Times[3*2];
	unsigned int Size[2];
	unsigned int Reserved[2];
	char Name[PATH_MAX];
	char AltName[14];
} findstate_t;

char* PathFindFileName(const char* pPath);
void NormalizeSlashes2(char *str);
unsigned int AfxGetFileName(const char* lpszPathName, char* lpszTitle, unsigned int nMax);

//extern int viewMaxY;

extern bool isskytexture;

extern int levelstarttic;

extern int force_singletics_to;

int HU_DrawDemoProgress(int force);

#ifdef _MSC_VER
int GetFullPath(const char* FileName, const char* ext, char *Buffer, size_t BufferLength);
#endif

#define PRB_MB_OK                       0x00000000
#define PRB_MB_OKCANCEL                 0x00000001
#define PRB_MB_ABORTRETRYIGNORE         0x00000002
#define PRB_MB_YESNOCANCEL              0x00000003
#define PRB_MB_YESNO                    0x00000004
#define PRB_MB_RETRYCANCEL              0x00000005
#define PRB_MB_DEFBUTTON1               0x00000000
#define PRB_MB_DEFBUTTON2               0x00000100
#define PRB_MB_DEFBUTTON3               0x00000200
#define PRB_IDOK                1
#define PRB_IDCANCEL            2
#define PRB_IDABORT             3
#define PRB_IDRETRY             4
#define PRB_IDIGNORE            5
#define PRB_IDYES               6
#define PRB_IDNO                7

bool SmoothEdges(unsigned char * buffer,int w, int h);

#ifdef _WIN32
extern int mus_extend_volume;
void I_midiOutSetVolumes(int volume);
#endif

#endif

/* vi: set et ts=2 sw=2: */

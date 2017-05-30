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


#include "z_zone.h"

#include "m_argv.h"
#include "d_deh.h"
#include "e6y.h"
#include "g_comp.h"
#include "g_demo.h"
#include "g_game.h"
#include "hu_tracers.h"
#include "p_map.h"
#include "r_defs.h"
#include "r_main.h"
#include "r_screenmultiply.h"
#include "v_video.h"

#include "hu_stuff.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#if 0
#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
#endif

#include "d_main.h"
#include "s_sound.h"
#include "i_system.h"
#include "i_main.h"
#include "sounds.h"
#include "i_sound.h"
#include "mn_main.h"
#include "m_argv.h"
#include "m_misc.h"
#include "pl_main.h"
#include "g_game.h"
#include "n_main.h"
#include "i_system.h"
#include "r_defs.h"
#include "r_draw.h"
#include "r_screenmultiply.h"
#include "r_main.h"
#include "r_patch.h"
#include "r_things.h"
#include "r_sky.h"
#include "v_video.h"

#include "p_maputl.h"
#include "p_map.h"
#include "i_video.h"
#include "info.h"
#include "am_map.h"
#include "hu_lib.h"
#include "gl_opengl.h"
#include "gl_struct.h"
#include "pl_main.h"
#include "w_wad.h"
#include "r_demo.h"
#include "d_deh.h"

#endif

bool wasWiped = false;

int secretfound;
int avi_shot_time;
int avi_shot_num;
const char *avi_shot_fname;

bool doSkip;
int speed_step;

int movement_strafe50;
int movement_shorttics;
int movement_strafe50onturns;
int render_fov;
int render_multisampling;
int render_paperitems;
int render_wipescreen;
int showendoom;

int palette_ondamage;
int palette_onbonus;
int palette_onpowers;

float skyscale;
float screen_skybox_zplane;
float tan_pitch;
float skyUpAngle;
float skyUpShift;
float skyXShift;
float skyYShift;

#ifdef _WIN32
const char* WINError(void)
{
  static char *WinEBuff = NULL;
  DWORD err = GetLastError();
  char *ch;

  if (WinEBuff)
  {
    LocalFree(WinEBuff);
  }

  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &WinEBuff, 0, NULL) == 0)
  {
    return "Unknown error";
  }

  if ((ch = strchr(WinEBuff, '\n')) != 0)
    *ch = 0;
  if ((ch = strchr(WinEBuff, '\r')) != 0)
    *ch = 0;

  return WinEBuff;
}
#endif

//--------------------------------------------------
#ifdef _WIN32
HWND WIN32_GetHWND(void);
#endif
//--------------------------------------------------

void e6y_assert(const char *format, ...) 
{
  static FILE *f = NULL;
  va_list argptr;
  va_start(argptr,format);
  //if (!f)
    f = fopen("d:\\a.txt", "ab+");
  vfprintf(f, format, argptr);
  fclose(f);
  va_end(argptr);
}

/* ParamsMatchingCheck
 * Conflicting command-line parameters could cause the engine to be confused 
 * in some cases. Added checks to prevent this.
 * Example: glboom.exe -record mydemo -playdemo demoname
 */
void ParamsMatchingCheck()
{
  bool recording_attempt = 
    M_CheckParm("-record") || 
    M_CheckParm("-recordfrom") ||
    M_CheckParm("-recordfromto");
  
  bool playbacking_attempt = 
    M_CheckParm("-playdemo") || 
    M_CheckParm("-timedemo") ||
    M_CheckParm("-fastdemo");

  if (recording_attempt && playbacking_attempt)
    I_Error("Params are not matching: Can not being played back and recorded at the same time.");
}

prboom_comp_t prboom_comp[PC_MAX] = {
  {0xffffffff, 0x02020615, 0, "-force_monster_avoid_hazards"},
  {0x00000000, 0x02040601, 0, "-force_remove_slime_trails"},
  {0x02020200, 0x02040801, 0, "-force_no_dropoff"},
  {0x00000000, 0x02040801, 0, "-force_truncated_sector_specials"},
  {0x00000000, 0x02040802, 0, "-force_boom_brainawake"},
  {0x00000000, 0x02040802, 0, "-force_prboom_friction"},
  {0x02020500, 0x02040000, 0, "-reject_pad_with_ff"},
  {0xffffffff, 0x02040802, 0, "-force_lxdoom_demo_compatibility"},
  {0x00000000, 0x0202061b, 0, "-allow_ssg_direct"},
  {0x00000000, 0x02040601, 0, "-treat_no_clipping_things_as_not_blocking"},
  {0x00000000, 0x02040803, 0, "-force_incorrect_processing_of_respawn_frame_entry"},
  {0x00000000, 0x02040601, 0, "-force_correct_code_for_3_keys_doors_in_mbf"},
  {0x00000000, 0x02040601, 0, "-uninitialize_crush_field_for_stairs"},
  {0x00000000, 0x02040802, 0, "-force_boom_findnexthighestfloor"},
  {0x00000000, 0x02040802, 0, "-allow_sky_transfer_in_boom"},
  {0x00000000, 0x02040803, 0, "-apply_green_armor_class_to_armor_bonuses"},
  {0x00000000, 0x02040803, 0, "-apply_blue_armor_class_to_megasphere"},
  {0x02020200, 0x02050003, 0, "-force_incorrect_bobbing_in_boom"},
  {0xffffffff, 0x00000000, 0, "-boom_deh_parser"},
  {0x00000000, 0x02050007, 0, "-mbf_remove_thinker_in_killmobj"},
  {0x00000000, 0x02050007, 0, "-do_not_inherit_friendlyness_flag_on_spawn"},
  {0x00000000, 0x02050007, 0, "-do_not_use_misc12_frame_parameters_in_a_mushroom"},
  {0x00000000, 0x02050102, 0, "-apply_mbf_codepointers_to_any_complevel"},
  {0x00000000, 0x02050104, 0, "-reset_monsterspawner_params_after_loading"},
};

void e6y_InitCommandLine(void) {
  int p;

  if ((p = M_CheckParm("-skipsec")) && (p < myargc - 1)) {
    demo_skiptics = (int)(atof(myargv[p + 1]) * 35);
  }

  if ((G_DemoIsPlayback() || G_DemoIsContinue()) &&
      (startmap > 1 || demo_skiptics)) {
    G_SkipDemoStart();
  }

  if ((p = M_CheckParm("-avidemo")) && (p < myargc - 1)) {
    avi_shot_fname = myargv[p + 1];
  }

  // TAS-tracers
  InitTracers();

  shorttics = movement_shorttics || M_CheckParm("-shorttics");
}

void MN_ChangeRenderPrecise(void) {
#ifdef GL_DOOM
  if (V_GetMode() != VID_MODEGL) {
    gl_seamless = false;
    return;
  }
  else {
    if (render_precise) {
      gl_seamless = true;
      gld_InitVertexData();
    }
    else {
      gl_seamless = false;
      gld_CleanVertexData();
    }
  }
#endif // GL_DOOM
}

void MN_ChangeScreenMultipleFactor(void) {
  V_ChangeScreenResolution();
}

void MN_ChangeInterlacedScanning(void) {
  if (render_interlaced_scanning) {
    interlaced_scanning_requires_clearing = 1;
  }
}

int render_aspect;
float render_ratio;
float render_fovratio;
float render_fovy = FOV90;
float render_multiplier;

void MN_ChangeAspectRatio(void) {
  extern int screenblocks;

#ifdef GL_DOOM
  MN_ChangeFOV();
#endif

  R_SetViewSize(screenblocks);
}

void MN_ChangeStretch(void) {
  extern int screenblocks;

  render_stretch_hud = render_stretch_hud_default;

  R_SetViewSize(screenblocks);
}

#ifdef GL_DOOM
void MN_ChangeFOV(void) {
  float f1, f2;
  int p;
  int render_aspect_width, render_aspect_height;

  if ((p = M_CheckParm("-aspect")) && (p+1 < myargc) && (strlen(myargv[p+1]) <= 21) &&
     (2 == sscanf(myargv[p+1], "%dx%d", &render_aspect_width, &render_aspect_height)))
  {
    SetRatio(SCREENWIDTH, SCREENHEIGHT);
    render_fovratio = (float)render_aspect_width / (float)render_aspect_height;
    render_ratio = RMUL * render_fovratio;
    render_multiplier = 64.0f / render_fovratio / RMUL;
  }
  else
  {
    SetRatio(SCREENWIDTH, SCREENHEIGHT);
    render_ratio = gl_ratio;
    render_multiplier = (float)ratio_multiplier;
    if (!tallscreen)
    {
      render_fovratio = 1.6f;
    }
    else
    {
      render_fovratio = render_ratio;
    }
  }

  render_fovy = (float)(2 * RAD2DEG(atan(tan(DEG2RAD(render_fov) / 2) / render_fovratio)));

  screen_skybox_zplane = 320.0f/2.0f/(float)tan(DEG2RAD(render_fov/2));

  f1 = (float)(320.0f / 200.0f * (float)render_fov / (float)FOV90 - 0.2f);
  f2 = (float)tan(DEG2RAD(render_fovy)/2.0f);
  if (f1-f2<1)
    skyUpAngle = (float)-RAD2DEG(asin(f1-f2));
  else
    skyUpAngle = -90.0f;

  skyUpShift = (float)tan(DEG2RAD(render_fovy)/2.0f);

  skyscale = 1.0f / (float)tan(DEG2RAD(render_fov / 2));
}

void MN_ChangeSpriteClip(void)
{
  gl_sprite_offset = (gl_spriteclip != spriteclip_const ? 0 : (.01f * (float)gl_sprite_offset_default));
  gl_spriteclip_threshold_f = (float)gl_spriteclip_threshold / MAP_COEFF;
}

void ResolveColormapsHiresConflict(bool prefer_colormap)
{
  gl_boom_colormaps = !r_have_internal_hires && !gl_texture_external_hires;
#if 0
  if (prefer_colormap)
  {
    if (gl_boom_colormaps_default)
    {
      gl_texture_external_hires = false;
    }
    else if (gl_texture_external_hires)
    {
      gl_boom_colormaps = false;
      gl_boom_colormaps_default = false;
    }
  }
  else
  {
    if (gl_texture_external_hires)
    {
      gl_boom_colormaps = false;
      gl_boom_colormaps_default = false;
    }
    else if (gl_boom_colormaps_default)
    {
      gl_texture_external_hires = false;
    }
  }
#endif
}

void MN_ChangeAllowBoomColormaps(void)
{
  if (gl_boom_colormaps == -1)
  {
    gl_boom_colormaps = gl_boom_colormaps_default;
    ResolveColormapsHiresConflict(true);
  }
  else
  {
    gl_boom_colormaps = gl_boom_colormaps_default;
    ResolveColormapsHiresConflict(true);
    gld_FlushTextures();
    gld_Precache();
  }
}

void MN_ChangeTextureUseHires(void)
{
  ResolveColormapsHiresConflict(false);

  gld_FlushTextures();
  gld_Precache();
}

void MN_ChangeTextureHQResize(void)
{
  gld_FlushTextures();
}
#endif //GL_DOOM

bool transparentpresent;

int StepwiseSum(int value, int direction, int step, int minval, int maxval, int defval)
{
  static int prev_value = 0;
  static int prev_direction = 0;

  int newvalue;
  int val = (direction > 0 ? value : value - 1);
  
  if (direction == 0)
    return defval;

  direction = (direction > 0 ? 1 : -1);
  
  if (step != 0)
    newvalue = (prev_direction * direction < 0 ? prev_value : value + direction * step);
  else
  {
    int exp = 1;
    while (exp * 10 <= val)
      exp *= 10;
    newvalue = direction * (val < exp * 5 && exp > 1 ? exp / 2 : exp);
    newvalue = (value + newvalue) / newvalue * newvalue;
  }

  if (newvalue > maxval) newvalue = maxval;
  if (newvalue < minval) newvalue = minval;

  if ((value < defval && newvalue > defval) || (value > defval && newvalue < defval))
    newvalue = defval;

  if (newvalue != value)
  {
    prev_value = value;
    prev_direction = direction;
  }

  return newvalue;
}

int numlevels = 0;
int levels_max = 0;
timetable_t *stats = NULL;

void e6y_G_DoCompleted(void) {
  if (doSkip && (demo_stoponend || demo_warp)) {
    G_SkipDemoStop();
  }
}

void e6y_G_DoWorldDone(void)
{
  if (doSkip)
  {
    static int firstmap = 1;
    int episode = 0;
    int map = 0;
    int p;

    if ((p = M_CheckParm ("-warp")))
    {
      if (gamemode == commercial)
      {
        if (p < myargc - 1)
          map = atoi(myargv[p + 1]);
      }
      else
      {
        if (p < myargc - 2)
        {
          episode = atoi(myargv[++p]);
          map = atoi(myargv[p + 1]);
        }
      }
    }

    demo_warp = demo_stoponnext ||
      (gamemode == commercial ? (map == gamemap) : (episode == gameepisode && map == gamemap));
    
    if (demo_warp && demo_skiptics == 0 && !firstmap)
      G_SkipDemoStop();

    firstmap = 0;
  }
}

//--------------------------------------------------

#ifdef _WIN32
HWND WIN32_GetHWND(void)
{
  static HWND Window = NULL; 
  if(!Window)
  {
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWMInfo(&wminfo);
#if SDL_VERSION_ATLEAST(1, 3, 0)
    Window = wminfo.info.win.window;
#else
    Window = wminfo.window;
#endif
  }
  return Window;
}
#endif

void e6y_G_Compatibility(void)
{
  deh_applyCompatibility();

  if (G_DemoIsPlayback())
  {
    int i, p;

    //"2.4.8.2" -> 0x02040802
    if ((p = M_CheckParm("-emulate")) && (p < myargc - 1))
    {
      unsigned int emulated_version = 0;
      int b[4], k = 1;
      memset(b, 0, sizeof(b));
      sscanf(myargv[p + 1], "%d.%d.%d.%d", &b[0], &b[1], &b[2], &b[3]);
      for (i = 3; i >= 0; i--, k *= 256)
      {
#ifdef RANGECHECK
        if (b[i] >= 256)
          I_Error("Wrong version number of package: %s", PACKAGE_VERSION);
#endif
        emulated_version += b[i] * k;
      }
      
      for (i = 0; i < PC_MAX; i++)
      {
        prboom_comp[i].state = 
          (emulated_version >= prboom_comp[i].minver && 
           emulated_version <  prboom_comp[i].maxver);
      }
    }

    for (i = 0; i < PC_MAX; i++)
    {
      if (M_CheckParm(prboom_comp[i].cmd))
        prboom_comp[i].state = true;
    }
  }

  P_CrossSubsector = P_CrossSubsector_PrBoom;
  if (!prboom_comp[PC_FORCE_LXDOOM_DEMO_COMPATIBILITY].state)
  {
    if (demo_compatibility)
      P_CrossSubsector = P_CrossSubsector_Doom;

    switch (compatibility_level)
    {
    case boom_compatibility_compatibility:
    case boom_201_compatibility:
    case boom_202_compatibility:
    case mbf_compatibility:
      P_CrossSubsector = P_CrossSubsector_Boom;
    break;
    }
  }
}

char* PathFindFileName(const char* pPath)
{
  const char* pT = pPath;
  
  if (pPath)
  {
    for ( ; *pPath; pPath++)
    {
      if ((pPath[0] == '\\' || pPath[0] == ':' || pPath[0] == '/')
        && pPath[1] &&  pPath[1] != '\\'  &&   pPath[1] != '/')
        pT = pPath + 1;
    }
  }
  
  return (char*)pT;
}

void NormalizeSlashes2(char *str)
{
  size_t l;

  if (!str || !(l = strlen(str)))
    return;
  if (str[--l]=='\\' || str[l]=='/')
    str[l]=0;
  while (l--)
    if (str[l]=='/')
      str[l]='\\';
}

unsigned int AfxGetFileName(const char* lpszPathName, char* lpszTitle, unsigned int nMax)
{
  char* lpszTemp = PathFindFileName(lpszPathName);
  
  if (lpszTitle == NULL)
    return strlen(lpszTemp)+1;
  
  strncpy(lpszTitle, lpszTemp, nMax-1);
  return 0;
}

int levelstarttic;

int force_singletics_to = 0;

#ifdef _WIN32
int GetFullPath(const char* FileName, const char* ext, char *Buffer, size_t BufferLength)
{
  int i, Result;
  char *p;
  char dir[PATH_MAX];
  
  for (i=0; i<3; i++)
  {
    switch(i)
    {
    case 0:
      getcwd(dir, sizeof(dir));
      break;
    case 1:
      if (!getenv("DOOMWADDIR"))
        continue;
      strcpy(dir, getenv("DOOMWADDIR"));
      break;
    case 2:
      strcpy(dir, I_DoomExeDir());
      break;
    }

    Result = SearchPath(
      (LPCWSTR)dir,
      (LPCWSTR)FileName,
      (LPCWSTR)ext,
      BufferLength,
      (LPWSTR)Buffer,
      (WCHAR **)&p
    );

    if (Result)
      return Result;
  }

  return false;
}
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include <mmsystem.h>
#pragma comment( lib, "winmm.lib" )
int mus_extend_volume;
void I_midiOutSetVolumes(int volume)
{
  // NSM changed to work on the 0-15 volume scale,
  // and to check mus_extend_volume itself.
  
  MMRESULT result;
  int calcVolume;
  MIDIOUTCAPS capabilities;
  unsigned int i;

  if (volume > 15)
    volume = 15;
  if (volume < 0)
    volume = 0;
  calcVolume = (65535 * volume / 15);

  //SDL_LockAudio(); // this function doesn't touch anything the audio callback touches

  //Device loop
  for (i = 0; i < midiOutGetNumDevs(); i++)
  {
    //Get device capabilities
    result = midiOutGetDevCaps(i, &capabilities, sizeof(capabilities));
    if (result == MMSYSERR_NOERROR)
    {
      //Adjust volume on this candidate
      if ((capabilities.dwSupport & MIDICAPS_VOLUME))
      {
        midiOutSetVolume((HMIDIOUT)i, MAKELONG(calcVolume, calcVolume));
      }
    }
  }

  //SDL_UnlockAudio();
}
#endif

//Begin of GZDoom code
/*
**---------------------------------------------------------------------------
** Copyright 2004-2005 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 4. When not used as part of GZDoom or a GZDoom derivative, this code will be
**    covered by the terms of the GNU Lesser General Public License as published
**    by the Free Software Foundation; either version 2.1 of the License, or (at
**    your option) any later version.
*/

//===========================================================================
// 
// smooth the edges of transparent fields in the texture
// returns false when nothing is manipulated to save the work on further
// levels

// 28/10/2003: major optimization: This function was far too pedantic.
// taking the value of one of the neighboring pixels is fully sufficient
//
//===========================================================================

#ifdef WORDS_BIGENDIAN
#define MSB 0
#define SOME_MASK 0xffffff00
#else
#define MSB 3
#define SOME_MASK 0x00ffffff
#endif

#define CHKPIX(ofs) (l1[(ofs)*4+MSB]==255 ? (( ((unsigned int*)l1)[0] = ((unsigned int*)l1)[ofs]&SOME_MASK), trans=true ) : false)

bool SmoothEdges(unsigned char * buffer,int w, int h)
{
  int x,y;
  int trans=buffer[MSB]==0; // If I set this to false here the code won't detect textures 
                            // that only contain transparent pixels.
  unsigned char * l1;

  // makes (a) no sense and (b) doesn't work with this code!
  // if (h<=1 || w<=1)
  // e6y: Do not smooth small patches.
  // Makes sense for HUD small digits
  // 2 and 7 still look ugly
  if (h<=8 || w<=8)
    return false;

  l1=buffer;

  if (l1[MSB]==0 && !CHKPIX(1)) CHKPIX(w);
  l1+=4;
  for(x=1;x<w-1;x++, l1+=4)
  {
    if (l1[MSB]==0 &&  !CHKPIX(-1) && !CHKPIX(1)) CHKPIX(w);
  }
  if (l1[MSB]==0 && !CHKPIX(-1)) CHKPIX(w);
  l1+=4;

  for(y=1;y<h-1;y++)
  {
    if (l1[MSB]==0 && !CHKPIX(-w) && !CHKPIX(1)) CHKPIX(w);
    l1+=4;
    for(x=1;x<w-1;x++, l1+=4)
    {
      if (l1[MSB]==0 &&  !CHKPIX(-w) && !CHKPIX(-1) && !CHKPIX(1)) CHKPIX(w);
    }
    if (l1[MSB]==0 && !CHKPIX(-w) && !CHKPIX(-1)) CHKPIX(w);
    l1+=4;
  }

  if (l1[MSB]==0 && !CHKPIX(-w)) CHKPIX(1);
  l1+=4;
  for(x=1;x<w-1;x++, l1+=4)
  {
    if (l1[MSB]==0 &&  !CHKPIX(-w) && !CHKPIX(-1)) CHKPIX(1);
  }
  if (l1[MSB]==0 && !CHKPIX(-w)) CHKPIX(-1);

  return trans;
}

#undef MSB
#undef SOME_MASK
//End of GZDoom code

/* vi: set et ts=2 sw=2: */


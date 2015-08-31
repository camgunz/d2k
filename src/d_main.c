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

#include "doomdef.h"
#include "doomstat.h"
#include "am_map.h"
#include "c_main.h"
#include "d_deh.h"  // Ty 04/08/98 - Externalizations
#include "d_event.h"
#include "d_main.h"
#include "d_net.h"
#include "dstrings.h"
#include "f_finale.h"
#include "f_wipe.h"
#include "g_game.h"
#include "g_keys.h"
#include "hu_stuff.h"
#include "i_main.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_file.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_checksum.h"
#include "p_ident.h"
#include "p_setup.h"
#include "p_user.h"
#include "r_draw.h"
#include "r_fps.h"
#include "r_main.h"
#include "sounds.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "xam_main.h"
#include "xc_main.h"
#include "xd_msg.h"
#include "xd_main.h"
#include "xg_game.h"
#include "xg_keys.h"
#include "xi_input.h"
#include "xi_main.h"
#include "xm_menu.h"
#include "xm_misc.h"
#include "xp_user.h"
#include "xst_main.h"
#include "xv_main.h"

//e6y
#include "r_demo.h"
#include "e6y.h"
#ifdef USE_WINDOWS_LAUNCHER
#include "e6y_launcher.h"
#endif

// NSM
#include "i_capture.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "x_main.h"

static char *iwad_base = NULL;
static char *iwad_path = NULL;
static int  demosequence;         // killough 5/2/98: made static
static int  pagetic;
static const char *pagename; // CPhipps - const


void GetFirstMap(int *ep, int *map); // Ty 08/29/98 - add "-warp x" functionality
static void D_PageDrawer(void);

// CPhipps - removed wadfiles[] stuff

dboolean devparm;        // started game with -devparm

// jff 1/24/98 add new versions of these variables to remember command line
dboolean clnomonsters;   // checkparm of -nomonsters
dboolean clrespawnparm;  // checkparm of -respawn
dboolean clfastparm;     // checkparm of -fast
// jff 1/24/98 end definition of command line version of play mode switches

dboolean nomonsters;     // working -nomonsters
dboolean respawnparm;    // working -respawn
dboolean fastparm;       // working -fast

dboolean singletics = false; // debug flag to cancel adaptiveness

//jff 1/22/98 parms for disabling music and sound
dboolean nosfxparm;
dboolean nomusicparm;

skill_t startskill;
int     startepisode;
int     startmap;
dboolean autostart;
FILE    *debugfile;
int ffmap;

dboolean advancedemo;

char    *basesavegame;             // killough 2/16/98: savegame directory

/*
 * CG: Keep track of the specified resource and DEH/BEX files so we can
 *     potentially send them to a client on request, or save them in a
 *     savegame, or whatever.
 */
GPtrArray *resource_files = NULL;
GPtrArray *deh_files = NULL;

//jff 4/19/98 list of standard IWAD names
const char *const standard_iwads[]=
{
  "doom2f.wad",
  "doom2.wad",
  "plutonia.wad",
  "tnt.wad",

  "doom.wad",
  "doom1.wad",
  "doomu.wad", /* CPhipps - alow doomu.wad */

  "freedoom2.wad", /* wart@kobold.org:  added freedoom for Fedora Extras */
  "freedoom1.wad",
  "freedm.wad",

  "hacx.wad",
  "chex.wad",

  "bfgdoom2.wad",
  "bfgdoom.wad",
};

//e6y static
const int nstandard_iwads = sizeof(standard_iwads) / sizeof(*standard_iwads);

/*
 * D_Responder - Main event handler
 */

bool D_Responder(event_t *ev) {
  if (menuactive)
    return false;

  if (!menuactive) {                                           // phares
    if (ev->key == key_autorun) {                              //  |
      autorun = !autorun;                                      //  V
      return true;
    }

    if (ev->key == key_help) {
      M_StartControlPanel();

      M_SetCurrentMenu(&HelpDef); // killough 10/98: new help screen

      itemOn = 0;
      S_StartSound(NULL, sfx_swtchn);

      return true;
    }

    if (ev->key == key_savegame) {
      M_StartControlPanel();
      S_StartSound(NULL, sfx_swtchn);
      M_SaveGame(0);
      return true;
    }

    if (ev->key == key_loadgame) {
      M_StartControlPanel();
      S_StartSound(NULL, sfx_swtchn);
      M_LoadGame(0);
      return true;
    }

    if (ev->key == key_soundvolume) {
      M_StartControlPanel();
      M_SetCurrentMenu(&SoundDef);
      itemOn = sfx_vol;
      S_StartSound(NULL, sfx_swtchn);
      return true;
    }

    if (ev->key == key_quicksave) {
      S_StartSound(NULL, sfx_swtchn);
      M_QuickSave();
      return true;
    }

    if (ev->key == key_endgame) {
      S_StartSound(NULL, sfx_swtchn);
      M_EndGame(0);
      return true;
    }

    if (ev->key == key_messages) {
      M_ChangeMessages(0);
      S_StartSound(NULL, sfx_swtchn);
      return true;
    }

    if (ev->key == key_quickload) {
      S_StartSound(NULL, sfx_swtchn);
      M_QuickLoad();
      return true;
    }

    if (ev->key == key_quit) {
      S_StartSound(NULL, sfx_swtchn);
      M_QuitDOOM(0);
      return true;
    }

    if (ev->key == key_gamma) {
//e6y
#ifdef GL_DOOM
      if (V_GetMode() == VID_MODEGL && gl_hardware_gamma) {
        useglgamma++;

        if (useglgamma > MAX_GLGAMMA)
          useglgamma = 0;

        P_Printf(consoleplayer, "Gamma correction level %d", useglgamma);

        gld_SetGammaRamp(useglgamma);
      }
      else
#endif
      {
        usegamma++;

        if (usegamma > 4)
          usegamma = 0;

        if (usegamma == 0)
          P_Echo(consoleplayer, s_GAMMALVL0);
        else if (usegamma == 1)
          P_Echo(consoleplayer, s_GAMMALVL1);
        else if (usegamma == 2)
          P_Echo(consoleplayer, s_GAMMALVL2);
        else if (usegamma == 3)
          P_Echo(consoleplayer, s_GAMMALVL3);

        V_SetPalette(0);

        return true;
      }
    }


    if ((ev->type == ev_key && ev->pressed) && ev->key == key_zoomout) {
      if (automapmode & am_active)
        return false;

      M_SizeDisplay(0);
      S_StartSound(NULL, sfx_stnmov);
      return true;
    }

    // jff 2/23/98 allow key_hud == key_zoomin
    if ((ev->type == ev_key && ev->pressed) && ev->key == key_zoomin) {
      if (automapmode & am_active)
        return false;

      M_SizeDisplay(1);                                        //  ^
      S_StartSound(NULL, sfx_stnmov);                          //  |
      return true;                                             // phares
    }

    //e6y
    if (ev->key == key_speed_default && (!netgame || demoplayback)) {
      realtic_clock_rate = StepwiseSum(
        realtic_clock_rate, 0, speed_step, 3, 10000, 100
      );
      I_Init2();
      return true;
    }

    if (ev->key == key_speed_up && (!netgame || demoplayback)) {
      realtic_clock_rate = StepwiseSum(
        realtic_clock_rate, 1, speed_step, 3, 10000, 100
      );
      I_Init2();
      return true;
    }

    if (ev->key == key_speed_down && (!netgame || demoplayback)) {
      realtic_clock_rate = StepwiseSum(
        realtic_clock_rate, -1, speed_step, 3, 10000, 100
      );
      I_Init2();
      return true;
    }

    if (ev->key == key_nextlevel) {
      if (demoplayback && !doSkip && singledemo) {
        demo_stoponnext = true;
        G_SkipDemoStart();
        return true;
      }

      if (G_GotoNextLevel())
        return true;
    }

    if (ev->key == key_level_restart && G_ReloadLevel())
      return true;

    if (ev->key == key_demo_endlevel) {
      if (demoplayback && !doSkip && singledemo) {
        demo_stoponend = true;
        G_SkipDemoStart();
        return true;
      }
    }

    if (ev->key == key_demo_skip) {
      if (demoplayback && singledemo) {
        if (doSkip)
          G_SkipDemoStop();
        else
          G_SkipDemoStart();

        return true;
      }
    }

    if (ev->key == key_walkcamera) {
      if (demoplayback && gamestate == GS_LEVEL) {
        walkcamera.type = (walkcamera.type + 1) % 3;

        P_SyncWalkcam(true, (walkcamera.type != 2));
        R_ResetViewInterpolation();

        if (walkcamera.type == 0)
          R_SmoothPlaying_Reset(NULL);

        return true;
      }
    }

#ifdef GL_DOOM
    if (V_GetMode() == VID_MODEGL) {
      if (ev->key == key_showalive) {
        show_alive = (show_alive + 1) % 3;
        if (show_alive == 0)
          P_Echo(consoleplayer, "Show Alive Monsters off");
        if (show_alive == 1)
          P_Echo(consoleplayer, "Show Alive Monsters (mode 1) on");
        if (show_alive == 2)
          P_Echo(consoleplayer, "Show Alive Monsters (mode 2) on");
      }
    }
#endif

    if (ev->key == key_mlook) {
      movement_mouselook = !movement_mouselook;
      M_ChangeMouseLook();
      return true;
    }

    if (ev->key == key_hud) {
      // jff 2/22/98
      if (automapmode & am_active)
        return false; // HUD mode control

      if (screenSize < 8) {                      // function on default F5
        while (screenSize < 8 || !hud_displayed) // make hud visible
          M_SizeDisplay(1);                      // when configuring it
      }
      else {
        hud_displayed = 1; //jff 3/3/98 turn hud on
        HU_NextHud();
        HU_MoveHud(true);  //jff 3/9/98 move it now to avoid glitch
      }
      return true;
    }

    /* killough 10/98: allow key shortcut into Setup menu */
    if ((ev->type == ev_key && ev->pressed) && ev->key == key_setup) {
      M_StartControlPanel();
      S_StartSound(NULL, sfx_swtchn);
      M_SetupNextMenu(&SetupDef);
      return true;
    }
  }

  return false;
}

//
// D_Wipe
//
// CPhipps - moved the screen wipe code from D_Display to here
// The screens to wipe between are already stored, this just does the timing
// and screen updating
//
static void D_Wipe(void) {
  dboolean done;
  int wipestart;

  if (!render_wipescreen)
    return; //e6y

  wipestart = I_GetTime() - 1;

  do {
    int nowtime;
    int tics;

    do {
      I_Sleep(5); // CPhipps - don't thrash cpu in this loop
      nowtime = I_GetTime();
      tics = nowtime - wipestart;
    } while (!tics);

    wipestart = nowtime;

    done = wipe_ScreenWipe(tics);

    I_UpdateNoBlit();

    if (MULTINET)
      N_TryRunTics();
    else
      M_Drawer(); // menu is drawn even on top of wipes

    I_FinishUpdate(); // page flip or blit buffer
  } while (!done);
}

//
// D_Display
//  draw current display, possibly wiping it from the previous
//

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t     wipegamestate = GS_DEMOSCREEN;
gamestate_t     oldgamestate = GS_BAD;
extern dboolean setsizeneeded;
extern int      showMessages;

void D_Display(void) {
  dboolean wipe;

  if (doSkip) {
    if (HU_DrawDemoProgress(false))
      I_FinishUpdate();

    if (!gamekeydown[key_use])
      return;

#ifdef GL_DOOM
    if (V_GetMode() == VID_MODEGL)
      gld_PreprocessLevel();
#endif
  }

  /* CG: What is this? */
  // if (!doSkip || !gamekeydown[key_use])

  if (nodrawers)                    // for comparative timing / profiling
    return;

  if (!I_StartDisplay())
    return;

  HU_Erase();

  if (setsizeneeded) {               // change the view size if needed
    R_ExecuteSetViewSize();
    oldgamestate = GS_BAD;        // force background redraw
  }

  // save the current screen if about to wipe
  if ((wipe = (gamestate != wipegamestate))) {
    wipe_StartScreen();
    R_ResetViewInterpolation();
  }

  /*
   * CG [TODO]: Eventually this will be all done in scripting.  The input
   *            handler (the renaming of which seems imminent) will tick and
   *            draw its contained interfaces according to its own
   *            configuration and state.  Buuuuut for now, it remains a hacky
   *            shitheap.
   */


  if (gamestate == GS_LEVEL) {
    if (gametic != basetic)
      if (!X_Call(X_GetState(), "game_interface", "draw", 0, 0))
        I_Error("Error drawing game interface: %s", X_GetError(X_GetState()));

      // G_Drawer();
  }
  else { // Not a level
    if (oldgamestate == GS_BAD || oldgamestate == GS_LEVEL)
      V_SetPalette(0); // cph - use default (basic) palette

    if (gamestate == GS_INTERMISSION)
      WI_Drawer();
    else if (gamestate == GS_FINALE)
      F_Drawer();
    else if (gamestate == GS_DEMOSCREEN)
      D_PageDrawer();
  }

  HU_Drawer();
  HU_DrawDemoProgress(true); //e6y

  oldgamestate = wipegamestate = gamestate;

  // draw pause pic
  if (paused && (menuactive != mnact_full)) {
    // Simplified the "logic" here and no need for x-coord caching - POPE
    V_DrawNamePatch((320 - V_NamePatchWidth("M_PAUSE"))/2, 4,
                    0, "M_PAUSE", CR_DEFAULT, VPT_STRETCH);
  }

  // menus go directly to the screen
  if (!X_Call(X_GetState(), "menu", "draw", 0, 0))
    I_Error("Error drawing menu interface: %s", X_GetError(X_GetState()));

#if 0
  M_Drawer();          // menu is drawn even on top of everything
#endif

  if (!X_Call(X_GetState(), "console", "draw", 0, 0))
    I_Error("Error drawing console: %s", X_GetError(X_GetState()));

  // normal update
  if (!wipe) {
    I_FinishUpdate();              // page flip or blit buffer
  }
  else {
    // wipe update
    wipe_EndScreen();
    D_Wipe();
  }

  // e6y
  // Don't thrash cpu during pausing or if the window doesnt have focus
  if ((paused && !walkcamera.type) || (!window_focused))
    I_Sleep(5);

  I_EndDisplay();
}

// CPhipps - Auto screenshot Variables

static int auto_shot_count, auto_shot_time;
static const char *auto_shot_fname;

//
//  D_DoomLoop()
//
// Not a globally visible function,
//  just included for source reference,
//  called by D_DoomMain, never exits.
// Manages timing and IO,
//  calls all ?_Responder, ?_Ticker, and ?_Drawer,
//  calls I_GetTime and I_StartTic
//

static void D_DoomLoop(void) {
  bool tic_elapsed;

  for (;;) {
    // frame syncronous IO operations

    if (ffmap == gamemap)
      ffmap = 0;

    // process one or more tics
    if (singletics) {
      G_BuildTiccmd(&players[consoleplayer].cmd);
      if (advancedemo)
        D_DoAdvanceDemo();
      M_Ticker();
      G_Ticker();
      P_Checksum(gametic);
      gametic++;
      tic_elapsed = true;
    }
    else {
      tic_elapsed = N_TryRunTics(); // Returns true if a TIC was run
    }

    if (nodrawers)
      continue;

    if (!tic_elapsed)
      continue;

    // killough 3/16/98: change consoleplayer to displayplayer
    if (players[displayplayer].mo) // cph 2002/08/10
      S_UpdateSounds(players[displayplayer].mo);// move positional sounds

    // CPhipps - auto screenshot
    if (auto_shot_fname && !--auto_shot_count) {
      auto_shot_count = auto_shot_time;
      M_DoScreenShot(auto_shot_fname);
    }

    //e6y
    if (avi_shot_fname && !doSkip) {
      int len;
      char *avi_shot_curr_fname;

      avi_shot_num++;
      len = snprintf(NULL, 0, "%s%06d.tga", avi_shot_fname, avi_shot_num);
      avi_shot_curr_fname = malloc(len + 1);
      sprintf(avi_shot_curr_fname, "%s%06d.tga", avi_shot_fname, avi_shot_num);
      M_DoScreenShot(avi_shot_curr_fname);
      free(avi_shot_curr_fname);
    }

    // NSM
    if (capturing_video && !doSkip)
      I_CaptureFrame();
  }
}

//
//  DEMO LOOP
//

dboolean bfgedition = 0;

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker(void) {
  if (--pagetic < 0)
    D_AdvanceDemo();
}

//
// D_PageDrawer
//
static void D_PageDrawer(void) {
  // proff/nicolas 09/14/98 -- now stretchs bitmaps to fullscreen!
  // CPhipps - updated for new patch drawing
  // proff - added M_DrawCredits
  if (pagename) {
    V_DrawNamePatch(0, 0, 0, pagename, CR_DEFAULT, VPT_STRETCH);
    V_FillBorder(-1, 0); // e6y: wide-res
  }
  else {
    M_DrawCredits();
  }
}

//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void) {
  advancedemo = true;
}

/*
 * killough 11/98: functions to perform demo sequences
 * cphipps 10/99: constness fixes
 */

static void D_SetPageName(const char *name) {
  if ((bfgedition) && name && !strncmp(name, "TITLEPIC", 8))
    pagename = "DMENUPIC";
  else
    pagename = name;
}

static void D_DrawTitle1(const char *name) {
  S_StartMusic(mus_intro);
  pagetic = (TICRATE*170)/35;
  D_SetPageName(name);
}

static void D_DrawTitle2(const char *name) {
  S_StartMusic(mus_dm2ttl);
  D_SetPageName(name);
}

/*
 * killough 11/98: tabulate demo sequences
 */

static struct
{
  void (*func)(const char *);
  const char *name;
} const demostates[][4] =
  {
    {
      {D_DrawTitle1, "TITLEPIC"},
      {D_DrawTitle1, "TITLEPIC"},
      {D_DrawTitle2, "TITLEPIC"},
      {D_DrawTitle1, "TITLEPIC"},
    },

    {
      {G_DeferedPlayDemo, "demo1"},
      {G_DeferedPlayDemo, "demo1"},
      {G_DeferedPlayDemo, "demo1"},
      {G_DeferedPlayDemo, "demo1"},
    },
    {
      {D_SetPageName, NULL},
      {D_SetPageName, NULL},
      {D_SetPageName, NULL},
      {D_SetPageName, NULL},
    },

    {
      {G_DeferedPlayDemo, "demo2"},
      {G_DeferedPlayDemo, "demo2"},
      {G_DeferedPlayDemo, "demo2"},
      {G_DeferedPlayDemo, "demo2"},
    },

    {
      {D_SetPageName, "HELP2"},
      {D_SetPageName, "HELP2"},
      {D_SetPageName, "CREDIT"},
      {D_DrawTitle1,  "TITLEPIC"},
    },

    {
      {G_DeferedPlayDemo, "demo3"},
      {G_DeferedPlayDemo, "demo3"},
      {G_DeferedPlayDemo, "demo3"},
      {G_DeferedPlayDemo, "demo3"},
    },

    {
      {NULL},
      {NULL},
      // e6y
      // Both Plutonia and TNT are commercial like Doom2,
      // but in difference from  Doom2, they have demo4 in demo cycle.
      {G_DeferedPlayDemo, "demo4"},
      {D_SetPageName, "CREDIT"},
    },

    {
      {NULL},
      {NULL},
      {NULL},
      {G_DeferedPlayDemo, "demo4"},
    },

    {
      {NULL},
      {NULL},
      {NULL},
      {NULL},
    }
  };

/*
 * This cycles through the demo sequences.
 * killough 11/98: made table-driven
 */

void D_DoAdvanceDemo(void) {
  players[consoleplayer].playerstate = PST_LIVE;  /* not reborn */
  advancedemo = usergame = paused = false;
  gameaction = ga_nothing;

  pagetic = TICRATE * 11;         /* killough 11/98: default behavior */
  gamestate = GS_DEMOSCREEN;

  if (netgame && !demoplayback)
    demosequence = 0;
  else if (!demostates[++demosequence][gamemode].func)
    demosequence = 0;

  demostates[demosequence][gamemode].func(
    demostates[demosequence][gamemode].name
  );
}

//
// D_StartTitle
//
void D_StartTitle (void) {
  gameaction = ga_nothing;
  demosequence = -1;
  D_AdvanceDemo();
}

// killough 10/98: support -dehout filename
// cph - made const, don't cache results
//e6y static
static const char* D_dehout(void) {
  int p = M_CheckParm("-dehout");

  if (!p)
    p = M_CheckParm("-bexout");

  if (p && ++p < myargc)
    return myargv[p];

  return NULL;
}

//
// D_AddFile
//
// Rewritten by Lee Killough
//
// Ty 08/29/98 - add source parm to indicate where this came from
// CPhipps - static, const char* parameter
//         - source is an enum
//         - modified to allocate & use new wadfiles array
void D_AddFile(const char *path, wad_source_t source) {
  char *wad_ext_path;
  char *wad_path;
  char *gwa_ext_path;
  char *gwa_filepath;
  int len;
  wadfile_info_t *wadfile;
  wadfile_info_t *gwafile;

  wad_ext_path = M_AddDefaultExtension(path, "wad");

  for (unsigned int i = 0; i < resource_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(resource_files, i);

    if (strcmp(wad_ext_path, wf->name) == 0) {
      D_Msg(MSG_INFO, "D_AddFile: Skipping %s (already added).\n", path);
      free(wad_ext_path);
      return;
    }
  }

  D_Msg(MSG_INFO, "D_AddFile: Searching for %s...\n", wad_ext_path);

  wad_path = I_FindFile(wad_ext_path, NULL);

  if (wad_path == NULL)
    I_Error(" %s missing (original path: %s)\n", wad_ext_path, path);

  wadfile = malloc(sizeof(wadfile_info_t));

  if (wadfile == NULL)
    I_Error("D_AddFile: Allocating WAD file info failed");

  wadfile->name = wad_path;
  wadfile->src = source; // Ty 08/29/98
  wadfile->handle = 0;

  g_ptr_array_add(resource_files, wadfile);

  // No Rest For The Living
  len = strlen(wadfile->name);
  if (len >= 9 && !strnicmp(wadfile->name + len - 9, "nerve.wad", 9)) {
    if (bfgedition)
      gamemission = pack_nerve;
  }

  // proff: automatically try to add the gwa files
  // proff - moved from w_wad.c
  gwa_ext_path = M_SetFileExtension(wad_ext_path, "gwa");

  free(wad_ext_path);

  gwa_filepath = I_FindFile(gwa_ext_path, NULL);

  free(gwa_ext_path);

  if (gwa_filepath != NULL) {
    gwafile = malloc(sizeof(wadfile_info_t));

    if (gwafile == NULL)
      I_Error("D_AddFile: Allocating GWA file info failed");

    wadfile->name = gwa_filepath;
    wadfile->src = source; // Ty 08/29/98
    wadfile->handle = 0;

    g_ptr_array_add(resource_files, wadfile);
  }

  free(gwa_filepath);
}

//
// D_AddDEH
//
void D_AddDEH(const char *filename, int lumpnum) {
  char *deh_path;
  deh_file_t *dehfile;
  const char *deh_out = D_dehout();

  if (filename == NULL && lumpnum == 0)
    I_Error("D_AddDEH: No filename or lumpnum given\n");

  if (!filename) {
    /*
    if (lumpnum > numlumps)
      I_Error("D_AddDEH: lumpnum out of range (%d/%d)\n", lumpnum, numlumps);

    deh_path = strdup(lumpinfo[lumpnum].name);
    */
    deh_path = NULL;
  }
  else {
    deh_path = I_FindFile(filename, NULL);

    if (deh_path == NULL)
      I_Error("D_AddDEH: Couldn't find %s\n", filename);
  }

  for (unsigned int i = 0; i < deh_files->len; i++) {
    deh_file_t *stored_deh_file = g_ptr_array_index(deh_files, i);

    if (!filename) {
      if (lumpnum == stored_deh_file->lumpnum) {
        D_Msg(MSG_INFO,
          "D_AddDEH: Skipping duplicate DeH/BEX (%d)\n", lumpnum
        );
        return;
      }
      continue;
    }

    if (strcmp(deh_path, stored_deh_file->filename) == 0) {
      D_Msg(MSG_INFO, "D_AddDEH: Skipping %s (already added).\n", deh_path);
      return;
    }
  }

  if (deh_path)
    D_Msg(MSG_INFO, "D_AddDEH: Adding %s.\n", deh_path);

  dehfile = malloc(sizeof(deh_file_t));

  if (dehfile == NULL)
    I_Error("D_AddDEH: Error allocating DEH file info");

  dehfile->filename = deh_path;
  if (deh_out != NULL)
    dehfile->outfilename = strdup(deh_out);
  else
    dehfile->outfilename = NULL;
  dehfile->lumpnum = lumpnum;

  g_ptr_array_add(deh_files, dehfile);
}

//
// AddIWAD
//
void AddIWAD(const char *iwad) {
  size_t i;

  if (!(iwad && *iwad))
    return;

  D_Msg(MSG_INFO, "IWAD found: %s\n", iwad); //jff 4/20/98 print only if found
  CheckIWAD(iwad, &gamemode, &haswolflevels);

  /*
   * jff 8/23/98 set gamemission global appropriately in all cases
   * cphipps 12/1999 - no version output here, leave that to the caller
   */
  i = strlen(iwad);

  switch(gamemode) {
  case retail:
  case registered:
  case shareware:
    gamemission = doom;
    if (i >= 8 && !strnicmp(iwad + i - 8,"chex.wad", 8))
      gamemission = chex;
    break;
  case commercial:
    gamemission = doom2;
    if (i >= 10 && !strnicmp(iwad + i - 10,"doom2f.wad", 10))
      language=french;
    else if (i >= 7 && !strnicmp(iwad + i - 7,"tnt.wad", 7))
      gamemission = pack_tnt;
    else if (i >= 12 && !strnicmp(iwad + i - 12,"plutonia.wad", 12))
      gamemission = pack_plut;
    else if (i >= 8 && !strnicmp(iwad + i - 8,"hacx.wad", 8))
      gamemission = hacx;
    break;
  default:
    gamemission = none;
    break;
  }

  if (gamemode == indetermined)
    D_Msg(MSG_INFO, "Unknown Game Version, may not work\n");
}

void D_SetIWAD(const char *iwad) {
  if (iwad_base)
    free(iwad_base);

  iwad_base = M_Basename(iwad);

  if (iwad_base == NULL) {
    I_Error(
      "D_SetIWAD: Error getting basename of %s: %s\n", iwad, M_GetFileError()
    );
  }

  if (iwad_path)
    free(iwad_path);

  iwad_path = strdup(iwad);
}

const char* D_GetIWAD(void) {
  return iwad_base;
}

const char* D_GetIWADPath(void) {
  return iwad_path;
}

//
// D_ClearIWAD
//
void D_ClearIWAD(void) {
  if (iwad_base)
    free(iwad_base);

  if (iwad_path)
    free(iwad_path);

  iwad_base = NULL;
  iwad_path = NULL;
}

//
// D_ClearResourceFiles
//
void D_ClearResourceFiles(void) {
  if (!resource_files)
    return;

  for (unsigned int i = resource_files->len; i > 0; i--)
    g_ptr_array_remove_index_fast(resource_files, i - 1);
}

//
// D_ClearDEHFiles
//
void D_ClearDEHFiles(void) {
  if (!deh_files)
    return;

  for (unsigned int i = deh_files->len; i > 0; i--)
    g_ptr_array_remove_index_fast(deh_files, i - 1);
}

//
// CheckIWAD
//
// Verify a file is indeed tagged as an IWAD
// Scan its lumps for levelnames and return gamemode as indicated
// Detect missing wolf levels in DOOM II
//
// The filename to check is passed in iwadname, the gamemode detected is
// returned in gmode, hassec returns the presence of secret levels
//
// jff 4/19/98 Add routine to test IWAD for validity and determine
// the gamemode from it. Also note if DOOM II, whether secret levels exist
// CPhipps - const char* for iwadname, made static
//e6y static
void CheckIWAD(const char *iwadname, GameMode_t *gmode, dboolean *hassec) {
  int ud = 0;
  int rg = 0;
  int sw = 0;
  int cm = 0;
  int sc = 0;
  int hx = 0;
  int cq = 0;
  dboolean noiwad = false;
  FILE *fp;
  wadinfo_t header;
  size_t length;
  filelump_t *fileinfo;

  if (access(iwadname, R_OK)) // error from access call
    I_Error("CheckIWAD: IWAD %s not readable", iwadname);

  if (!(fp = fopen(iwadname, "rb"))) // error from open call
    I_Error("CheckIWAD: Can't open IWAD %s", iwadname);

  *gmode = indetermined;
  *hassec = false;

  if (fread(&header, sizeof(header), 1, fp) != 1) {
    fclose(fp);
    return;
  }

  // Identify IWAD correctly

  // read IWAD header

  // check for missing IWAD tag in header
  if (strncmp(header.identification, "IWAD", 4))
    noiwad = true;

  // read IWAD directory
  header.numlumps = LittleLong(header.numlumps);
  header.infotableofs = LittleLong(header.infotableofs);
  length = header.numlumps;
  fileinfo = malloc(length * sizeof(filelump_t));

  if (fileinfo == NULL)
    I_Error("CheckIWAD: malloc returned NULL");

  if (fseek(fp, header.infotableofs, SEEK_SET) ||
      fread(fileinfo, sizeof(filelump_t), length, fp) != length ||
      fclose(fp)) {
    I_Error("CheckIWAD: failed to read directory %s", iwadname);
  }

  // scan directory for levelname lumps
  while (length--) {
    if (fileinfo[length].name[0] == 'E' &&
        fileinfo[length].name[2] == 'M' &&
        fileinfo[length].name[4] == 0) {
      if (fileinfo[length].name[1] == '4')
        ud++;
      else if (fileinfo[length].name[1] == '3')
        rg++;
      else if (fileinfo[length].name[1] == '2')
        rg++;
      else if (fileinfo[length].name[1] == '1')
        sw++;
    }
    else if (fileinfo[length].name[0] == 'M' &&
             fileinfo[length].name[1] == 'A' &&
             fileinfo[length].name[2] == 'P' &&
             fileinfo[length].name[5] == 0) {
      cm++;
      if (fileinfo[length].name[3] == '3') {
        if (fileinfo[length].name[4] == '1' ||
            fileinfo[length].name[4] == '2') {
          sc++;
        }
      }
    }

    if (!strncmp(fileinfo[length].name, "DMENUPIC", 8))
      bfgedition++;
    if (!strncmp(fileinfo[length].name, "HACX", 4))
      hx++;
    if (!strncmp(fileinfo[length].name, "W94_1", 5) ||
        !strncmp(fileinfo[length].name, "POSSH0M0", 8)) {
      cq++;
    }
  }

  free(fileinfo);

  if (noiwad && !bfgedition && cq < 2)
    I_Error("CheckIWAD: IWAD tag %s not present", iwadname);

  // Determine game mode from levels present
  // Must be a full set for whichever mode is present
  // Lack of wolf-3d levels also detected here

  if (cm >= 30 || (cm >= 20 && hx)) {
    *gmode = commercial;
    *hassec = sc >= 2;
  }
  else if (ud >= 9) {
    *gmode = retail;
  }
  else if (rg >= 18) {
    *gmode = registered;
  }
  else if (sw >= 9) {
    *gmode = shareware;
  }
}

// NormalizeSlashes
//
// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// jff 4/19/98 Make killoughs slash fixer a subroutine
//
static void NormalizeSlashes(char *str)
{
  size_t l;

  // killough 1/18/98: Neater / \ handling.
  // Remove trailing / or \ to prevent // /\ \/ \\, and change \ to /

  if (!str || !(l = strlen(str)))
    return;
  if (str[--l]=='/' || str[l]=='\\')     // killough 1/18/98
    str[l]=0;
  while (l--)
    if (str[l]=='\\')
      str[l]='/';
}

/*
 * FindIWADFIle
 *
 * Search for one of the standard IWADs
 * CPhipps  - static, proper prototype
 *    - 12/1999 - rewritten to use I_FindFile
 */
static char* FindIWADFile(void) {
  int pi;
  char *iwad;

  if (CLIENT) {
    D_Msg(MSG_INFO, "FindIWADFile: Looking for IWAD %s\n", D_GetIWAD());
    return I_FindFile(D_GetIWAD(), NULL);
  }

  pi = M_CheckParm("-iwad");

  if (pi && (++pi < myargc)) {
    D_Msg(MSG_INFO, "FindIWADFile: Searching for IWAD %s\n", myargv[pi]);
    return I_FindFile(myargv[pi], ".wad");
  }

  for (int i = 0; i < nstandard_iwads; i++) {
    D_Msg(MSG_INFO, "FindIWADFile: Searching for IWAD %s\n",
      standard_iwads[i]
    );
    iwad = I_FindFile(standard_iwads[i], ".wad");

    if (iwad)
      return iwad;
  }

  return NULL;
}

//
// IdentifyVersion
//
// Set the location of the defaults file and the savegame root
// Locate and validate an IWAD file
// Determine gamemode from the IWAD
//
// supports IWADs with custom names. Also allows the -iwad parameter to
// specify which iwad is being searched for if several exist in one dir.
// The -iwad parm may specify:
//
// 1) a specific pathname, which must exist (.wad optional)
// 2) or a directory, which must contain a standard IWAD,
// 3) or a filename, which must be found in one of the standard places:
//   a) current dir,
//   b) exe dir
//   c) $DOOMWADDIR
//   d) or $HOME
//
// jff 4/19/98 rewritten to use a more advanced search algorithm

void IdentifyVersion(void) {
  int i;            //jff 3/24/98 index of args on commandline
  struct stat sbuf; //jff 3/24/98 used to test save path for existence
  char *iwad;

  // set save path to -save parm or current dir

  //jff 3/27/98 default to current dir
  //V.Aguilar (5/30/99): In LiNUX, default to $HOME/.lxdoom
  // CPhipps - use DOOMSAVEDIR if defined
  const char *p = getenv("DOOMSAVEDIR");

  if (p == NULL)
    p = I_DoomExeDir();

  free(basesavegame);
  basesavegame = strdup(p);

  //jff 3/24/98 if -save present
  if ((i = M_CheckParm("-save")) && i < myargc - 1) {
    if (!stat(myargv[i + 1], &sbuf) && S_ISDIR(sbuf.st_mode)) { // and is a dir
      free(basesavegame);
      basesavegame = strdup(myargv[i + 1]); //jff 3/24/98 use that for savegame
      NormalizeSlashes(basesavegame);       //jff 9/22/98 fix c:\ not working
    }
    else {
      D_Msg(MSG_INFO, "Error: -save path does not exist, using %s\n",
        basesavegame
      );
    }
  }

  // locate the IWAD and determine game mode from it
  iwad = FindIWADFile();

#if (defined(GL_DOOM) && defined(LEVELINFO_DEBUG))
  // proff 11/99: used for debugging
  FILE *f = fopen("levelinfo.txt", "w");

  if (f) {
    fprintf(f, "%s\n", iwad);
    fclose(f);
  }
#endif

  if (!iwad || !(*iwad))
    I_Error("IdentifyVersion: IWAD not found\n");

  AddIWAD(iwad);
  D_SetIWAD(iwad);
  D_AddFile(iwad_path, source_iwad);
  free(iwad);
}

// killough 5/3/98: old code removed
//
// Find a Response File
//

static void FindResponseFile (void)
{
  for (int i = 1; i < myargc; i++) {
    if (myargv[i][0] == '@') {
      size_t size;
      int index;
      int indexinfile;
      byte *file = NULL;
      const char **moreargs = malloc(myargc * sizeof(const char*));
      char **newargv;
      // proff 04/05/2000: Added for searching responsefile
      char *fname;

      fname = M_AddDefaultExtension(&myargv[i][i], "rsp");

      // READ THE RESPONSE FILE INTO MEMORY
      // proff 04/05/2000: changed for searching responsefile
      // cph 2002/08/09 - use M_ReadFile for simplicity
      // proff 04/05/2000: Added for searching responsefile
      if (!M_ReadFile(fname, (char **)&file, &size)) {
        char *exe_fname = M_PathJoin(I_DoomExeDir(), fname);

        if (!exe_fname) {
          I_Error("FindResponseFile: Error joining %s and %s: %s",
            I_DoomExeDir(), fname, M_GetFileError()
          );
        }

        /*
         * proff 04/05/2000: Changed from LO_FATAL
         * proff 04/05/2000: Simply removed the exit(1);
         * cph - made fatal, don't drop through and SEGV
         */
        if (!M_ReadFile(fname, (char **)&file, &size))
          I_Error("No such response file: %s", fname);
      }


      D_Msg(MSG_INFO, "Found response file %s\n", fname);
      free(fname);

      // proff 04/05/2000: Added check for empty rsp file
      if (size <= 0) {
        int k;
        D_Msg(MSG_ERROR, "\nResponse file empty!\n");

        newargv = calloc(sizeof(newargv[0]), myargc);
        newargv[0] = myargv[0];
        for (k = 1,index = 1; k < myargc; k++) {
          if (i != k)
            newargv[index++] = myargv[k];
        }
        myargc = index;
        myargv = newargv;
        return;
      }

      // KEEP ALL CMDLINE ARGS FOLLOWING @RESPONSEFILE ARG
      memcpy(
        (void *)moreargs,
        &myargv[i + 1],
        (index = myargc - i - 1) * sizeof(myargv[0])
      );

      {
        char *firstargv = myargv[0];
        newargv = calloc(sizeof(newargv[0]), 1);
        newargv[0] = firstargv;
      }

      {
        byte *infile = file;
        indexinfile = 0;
        indexinfile++;  // SKIP PAST ARGV[0] (KEEP IT)
        do {
          while (size > 0 && isspace(*infile)) {
            infile++;
            size--;
          }

          if (size > 0) {
            char *s = malloc(size + 1);
            char *p = s;
            int quoted = 0;

            while (size > 0) {
              // Whitespace terminates the token unless quoted
              if (!quoted && isspace(*infile))
                break;

              if (*infile == '\"') {
                // Quotes are removed but remembered
                infile++;
                size--;
                quoted ^= 1;
              }
              else {
                *p++ = *infile++;
                size--;
              }
            }
            if (quoted)
              I_Error("Runaway quoted string in response file");

            // Terminate string, realloc and add to argv
            *p = 0;
            newargv = realloc(newargv, sizeof(newargv[0]) * (indexinfile + 1));
            newargv[indexinfile++] = realloc(s, strlen(s) + 1);
          }
        } while(size > 0);
      }
      free(file);

      newargv = realloc(newargv, sizeof(newargv[0]) * (indexinfile + index));
      memcpy(
        (void *)&newargv[indexinfile], moreargs, index * sizeof(moreargs[0])
      );
      free((void *)moreargs);

      myargc = indexinfile + index;
      myargv = newargv;

      // DISPLAY ARGS
      D_Msg(MSG_INFO, "%d command-line args:\n", myargc);
      for (index = 1; index < myargc; index++)
        D_Msg(MSG_INFO, "%s\n", myargv[index]);

      break;
    }
  }
}

//
// DoLooseFiles
//
// Take any file names on the command line before the first switch parm
// and insert the appropriate -file, -deh or -playdemo switch in front
// of them.
//
// Note that more than one -file, etc. entry on the command line won't
// work, so we have to go get all the valid ones if any that show up
// after the loose ones.  This means that boom fred.wad -file wilma
// will still load fred.wad and wilma.wad, in that order.
// The response file code kludges up its own version of myargv[] and
// unfortunately we have to do the same here because that kludge only
// happens if there _is_ a response file.  Truth is, it's more likely
// that there will be a need to do one or the other so it probably
// isn't important.  We'll point off to the original argv[], or the
// area allocated in FindResponseFile, or our own areas from strdups.
//
// CPhipps - OUCH! Writing into *myargv is too dodgy, damn
//
// e6y
// Fixed crash if numbers of wads/lmps/dehs is greater than 100
// Fixed bug when length of argname is smaller than 3
// Refactoring of the code to avoid use the static arrays
// The logic of DoLooseFiles has been rewritten in more optimized style
// MAXARGVS has been removed.

static void DoLooseFiles(void)
{
  char **wads;  // store the respective loose filenames
  char **lmps;
  char **dehs;
  int wadcount = 0;      // count the loose filenames
  int lmpcount = 0;
  int dehcount = 0;
  int i,k,n,p;
  char **tmyargv;  // use these to recreate the argv array
  int tmyargc;
  dboolean *skip; // CPhipps - should these be skipped at the end

  struct {
    const char *ext;
    char ***list;
    int *count;
  } looses[] = {
    {".wad", &wads, &wadcount},
    {".lmp", &lmps, &lmpcount},
    {".deh", &dehs, &dehcount},
    {".bex", &dehs, &dehcount},
    // assume wad if no extension or length of the extention is not equal to 3
    // must be last entrie
    {"",     &wads, &wadcount},
    {0}
  };

  struct {
    const char *cmdparam;
    char ***list;
    int *count;
  } params[] = {
    {"-file"    , &wads, &wadcount},
    {"-deh"     , &dehs, &dehcount},
    {"-playdemo", &lmps, &lmpcount},
    {0}
  };

  wads = malloc(myargc * sizeof(*wads));
  lmps = malloc(myargc * sizeof(*lmps));
  dehs = malloc(myargc * sizeof(*dehs));
  skip = malloc(myargc * sizeof(dboolean));

  for (i = 0; i < myargc; i++)
    skip[i] = false;

  for (i = 1; i < myargc; i++)
  {
    size_t arglen, extlen;

    if (*myargv[i] == '-') break;  // quit at first switch

    // so now we must have a loose file.  Find out what kind and store it.
    arglen = strlen(myargv[i]);

    k = 0;
    while (looses[k].ext)
    {
      extlen = strlen(looses[k].ext);
      if (arglen >= extlen && !stricmp(&myargv[i][arglen - extlen], looses[k].ext))
      {
        (*(looses[k].list))[(*looses[k].count)++] = strdup(myargv[i]);
        break;
      }
      k++;
    }
    /*if (myargv[i][j-4] != '.')  // assume wad if no extension
      wads[wadcount++] = strdup(myargv[i]);*/
    skip[i] = true; // nuke that entry so it won't repeat later
  }

  // Now, if we didn't find any loose files, we can just leave.
  if (wadcount+lmpcount+dehcount != 0)
  {
    n = 0;
    k = 0;
    while (params[k].cmdparam)
    {
      if ((p = M_CheckParm (params[k].cmdparam)))
      {
        skip[p] = true;    // nuke the entry
        while (++p != myargc && *myargv[p] != '-')
        {
          (*(params[k].list))[(*params[k].count)++] = strdup(myargv[p]);
          skip[p] = true;  // null any we find and save
        }
      }
      else
      {
        if (*(params[k].count) > 0)
        {
          n++;
        }
      }
      k++;
    }

    // Now go back and redo the whole myargv array with our stuff in it.
    // First, create a new myargv array to copy into
    tmyargv = calloc(sizeof(tmyargv[0]), myargc + n);
    tmyargv[0] = myargv[0]; // invocation
    tmyargc = 1;

    k = 0;
    while (params[k].cmdparam)
    {
      // put our stuff into it
      if (*(params[k].count) > 0)
      {
        tmyargv[tmyargc++] = strdup(params[k].cmdparam); // put the switch in
        for (i=0;i<*(params[k].count);)
          tmyargv[tmyargc++] = (*(params[k].list))[i++]; // allocated by strdup above
      }
      k++;
    }

    // then copy everything that's there now
    for (i = 1; i < myargc; i++)
    {
      if (!skip[i])  // skip any zapped entries
        tmyargv[tmyargc++] = myargv[i];  // pointers are still valid
    }
    // now make the global variables point to our array
    myargv = tmyargv;
    myargc = tmyargc;
  }

  free(wads);
  free(lmps);
  free(dehs);
  free(skip);
}

/* cph - MBF-like wad/deh/bex autoload code */
const char *wad_file_names[MAXLOADFILES], *deh_file_names[MAXLOADFILES];

// CPhipps - misc screen stuff
unsigned int desired_screenwidth, desired_screenheight;

static void D_destroyDEHFile(gpointer data) {
  deh_file_t *df = (deh_file_t *)data;

  if (df->filename != NULL)
    free(df->filename);
  if (df->outfilename != NULL)
    free(df->outfilename);
  free(df);
}

static void D_destroyWADFile(gpointer data) {
  wadfile_info_t *wf = (wadfile_info_t *)data;

  if (wf->handle > 0) {
    M_Close(wf->handle);
    wf->handle = 0;
  }

  free(wf->name);
  free(wf);
}

//
// D_DoomMainSetup
//
// CPhipps - the old contents of D_DoomMain, but moved out of the main
//  line of execution so its stack space can be freed
const char* doomverstr = NULL;

static void D_DoomMainSetup(void) {
  int p;
  int slot;
  dboolean rsp_found;

  X_Init(); /* CG 07/22/2014: Scripting */

  // proff 04/05/2000: Added support for include response files
  /* proff 2001/7/1 - Moved up, so -config can be in response files */

  do {
    rsp_found = false;
    for (int i = 0; i < myargc; i++)
      if (myargv[i][0] == '@')
        rsp_found = true;

    FindResponseFile();
  } while (rsp_found == true);

  resource_files = g_ptr_array_new_with_free_func(D_destroyWADFile);
  deh_files = g_ptr_array_new_with_free_func(D_destroyDEHFile);

  // e6y: moved to main()
  /*
  D_Msg(MSG_INFO,"M_LoadDefaults: Load system defaults.\n");
  M_LoadDefaults();              // load before initing other systems
  */

  // figgi 09/18/00-- added switch to force classic bsp nodes
  if (M_CheckParm("-forceoldbsp")) {
    extern dboolean forceOldBsp;
    forceOldBsp = true;
  }

  D_BuildBEXTables(); // haleyjd

  // 1/18/98 killough: Z_Init() call moved to i_main.c

  // CPhipps - move up netgame init
  D_Msg(MSG_INFO, "N_InitNetGame: Checking for network game.\n");
  N_InitNetGame();

  if (!CLIENT) {
    DoLooseFiles();  // Ty 08/29/98 - handle "loose" files on command line

    IdentifyVersion();

    // e6y: DEH files preloaded in wrong order
    // http://sourceforge.net/p/prboom-plus/bugs/18/
    // The dachaked stuff has been moved below an autoload

    // jff 1/24/98 set both working and command line value of play parms
    nomonsters  = clnomonsters  = M_CheckParm("-nomonsters");
    respawnparm = clrespawnparm = M_CheckParm("-respawn");
    fastparm    = clfastparm    = M_CheckParm("-fast");
    // jff 1/24/98 end of set to both working and command line value

    devparm = M_CheckParm ("-devparm");

    if (M_CheckParm ("-altdeath"))
      deathmatch = 2;
    else if (M_CheckParm ("-deathmatch"))
      deathmatch = 1;
  }

  switch (gamemode) {
    case retail:
      switch (gamemission) {
        case chex:
          doomverstr = "Chex(R) Quest";
          break;
        default:
          doomverstr = "The Ultimate DOOM";
          break;
      }
      break;
    case shareware:
      doomverstr = "DOOM Shareware";
      break;
    case registered:
      doomverstr = "DOOM Registered";
      break;
    case commercial:  // Ty 08/27/98 - fixed gamemode vs gamemission
      switch (gamemission) {
        case pack_plut:
          doomverstr = "Final DOOM - The Plutonia Experiment";
          break;
        case pack_tnt:
          doomverstr = "Final DOOM - TNT: Evilution";
          break;
        case hacx:
          doomverstr = "HACX - Twitch 'n Kill";
          break;
        default:
          doomverstr = "DOOM 2: Hell on Earth";
          break;
      }
      break;
    default:
      doomverstr = "Public DOOM";
      break;
  }

  if (bfgedition) {
    char *tempverstr;
    const char bfgverstr[] = " (BFG Edition)";
    size_t tempverstrsize = strlen(doomverstr) + strlen(bfgverstr) + 1;

    tempverstr = malloc(tempverstrsize);

    if (tempverstr == NULL)
      I_Error("D_DoomMainSetup: Allocating temporary version string failed");

    strncpy(tempverstr, doomverstr, tempverstrsize);
    strncat(tempverstr, bfgverstr, tempverstrsize);
    doomverstr = strdup(tempverstr);
    free(tempverstr);
  }

  /* cphipps - the main display. This shows the build date, copyright, and game type */
  D_Msg(MSG_INFO, PACKAGE_NAME" (built %s), playing: %s\n"
    PACKAGE_NAME" is released under the GNU General Public license v2.0.\n"
    "You are welcome to redistribute it under certain conditions.\n"
    "It comes with ABSOLUTELY NO WARRANTY. See the file COPYING for details.\n",
    version_date, doomverstr);

  if (!CLIENT) {
    if (devparm)
      D_Msg(MSG_INFO, "%s", D_DEVSTR);

    // turbo option
    if ((p = M_CheckParm("-turbo"))) {
      int scale = 200;
      extern int forwardmove[2];
      extern int sidemove[2];

      if (p < myargc - 1)
        scale = atoi(myargv[p + 1]);
      if (scale < 10)
        scale = 10;
      if (scale > 400)
        scale = 400;

      D_Msg(MSG_INFO, "turbo scale: %i%%\n", scale);
      forwardmove[0] = forwardmove[0] * scale / 100;
      forwardmove[1] = forwardmove[1] * scale / 100;
      sidemove[0] = sidemove[0] * scale / 100;
      sidemove[1] = sidemove[1] * scale / 100;
    }
  }

  modifiedgame = false;

  // get skill / episode / map from parms

  if (!CLIENT) {
    startskill = sk_none; // jff 3/24/98 was sk_medium, just note not picked
    startepisode = 1;
    startmap = 1;
    autostart = false;

    if ((p = M_CheckParm ("-skill")) && p < myargc - 1) {
      startskill = myargv[p + 1][0]-'1';
      autostart = true;
    }

    if ((p = M_CheckParm("-episode")) && p < myargc - 1) {
      startepisode = myargv[p + 1][0] - '0';
      startmap = 1;
      autostart = true;
    }

    if ((p = M_CheckParm("-timer")) && p < myargc - 1 && deathmatch) {
      int time = atoi(myargv[p + 1]);

      D_Msg(MSG_INFO,
        "Levels will end after %d minute%s.\n", time, time > 1 ? "s" : ""
      );
    }

    if ((p = M_CheckParm ("-avg")) && p < myargc - 1 && deathmatch) {

      D_Msg(MSG_INFO,
        "Austin Virtual Gaming: Levels will end after 20 minutes\n"
      );
    }

    // killough 5/2/98
    // CG (killough added "warp")
    // Ty 08/29/98 - moved this check later so we can have
    //               -warp alone: && p < myargc - 1)
    if ((p = M_CheckParm ("-warp")) || (p = M_CheckParm ("-wart"))) {
      startmap = 0; // Ty 08/29/98 - allow "-warp x" to go to first map in wad(s)
      autostart = true; // Ty 08/29/98 - move outside the decision tree
      if (gamemode == commercial) {
        if (p < myargc - 1)
          startmap = atoi(myargv[p + 1]);   // Ty 08/29/98 - add test if last parm
      }
      else {  // 1/25/98 killough: fix -warp xxx from crashing Doom 1 / UD
        if (p < myargc - 1) {
          int episode, map;

          if (sscanf(myargv[p + 1], "%d", &episode) == 1) {
            startepisode = episode;
            startmap = 1;
            if (p < myargc-2 && sscanf(myargv[p + 2], "%d", &map) == 1)
              startmap = map;
          }
        }
      }
    }
  }

  // Ty 08/29/98 - later we'll check for startmap=0 and autostart=true
  // as a special case that -warp * was used.  Actually -warp with any
  // non-numeric will do that but we'll only document "*"

  //jff 1/22/98 add command line parms to disable sound and music
  /* CG: The server does this itself */
  if (!SERVER) {
    int nosound = M_CheckParm("-nosound");

    nomusicparm = nosound || M_CheckParm("-nomusic");
    nosfxparm   = nosound || M_CheckParm("-nosfx");
    //jff end of sound/music command line parms

    // killough 3/2/98: allow -nodraw -noblit generally
    nodrawers = M_CheckParm ("-nodraw");
    noblit = M_CheckParm ("-noblit");
  }

  //proff 11/22/98: Added setting of viewangleoffset
  p = M_CheckParm("-viewangle");
  if (p) {
    viewangleoffset = atoi(myargv[p + 1]);
    viewangleoffset =
      viewangleoffset < 0 ? 0 : (viewangleoffset > 7 ? 7 : viewangleoffset);
    viewangleoffset = (8 - viewangleoffset) * ANG45;
  }

  // init subsystems

  if (!CLIENT)
    G_ReloadDefaults();    // killough 3/4/98: set defaults just loaded.
  // jff 3/24/98 this sets startskill if it was -1

#ifdef GL_DOOM
  // proff 04/05/2000: for GL-specific switches
  gld_InitCommandLine();
#endif

  D_Msg(MSG_INFO, "V_Init: allocate screens.\n");
  V_Init();

  //e6y: Calculate the screen resolution and init all buffers
  I_InitScreenResolution();

  //e6y: some stuff from command-line should be initialised before ProcessDehFile()
  e6y_InitCommandLine();

  // CPhipps - autoloading of wads
  // Designed to be general, instead of specific to boomlump.wad
  // Some people might find this useful
  // cph - support MBF -noload parameter
  //
  if ((!MULTINET) && (!M_CheckParm("-noload"))) {
    // only autoloaded wads here - autoloaded patches moved down below W_Init
    int i;

    for (i = 0; i < MAXLOADFILES; i++) {
      const char *fname = wad_file_names[i];
      char *fpath;

      if (!(fname && *fname))
        continue;

      // Filename is now stored as a zero terminated string
      fpath = I_FindFile(fname, ".wad");
      if (!fpath) {
        D_Msg(MSG_INFO, "Failed to autoload %s\n", fname);
      }
      else {
        D_AddFile(fpath, source_auto_load);
        free(fpath);
      }
    }
  }

  /*
   * proff 2001/7/1 - added prboom.wad as last entry so it's always loaded and
   * doesn't overlap with the cfg settings
   */
  //e6y: Check on existence of prboom.wad
  /*
   * CG: Just append the BOOM resource WAD for now, we'll see if it's missing later
   *
   * if (!(wad_files[MAXLOADFILES-1] = I_FindFile(PACKAGE_TARNAME ".wad", "")))
   *   I_Error(PACKAGE_TARNAME ".wad not found. Can't continue.");
   */

  if (!CLIENT)
    D_AddFile(PACKAGE_TARNAME ".wad", source_auto_load);

  // add any files specified on the command line with -file wadfile
  // to the wad list

  // killough 1/31/98, 5/2/98: reload hack removed, -wart same as -warp now.

  if ((!CLIENT) && (p = M_CheckParm("-file"))) {
    // the parms after p are wadfile/lump names,
    // until end of parms or another - preceded parm
    modifiedgame = true;            // homebrew levels
    while (++p != myargc && *myargv[p] != '-') {
      // e6y
      // reorganization of the code for looking for wads
      // in all standard dirs (%DOOMWADDIR%, etc)
      char *file = I_FindFile(myargv[p], ".wad");

      if (!file && N_GetWad(myargv[p]))
        file = I_FindFile(myargv[p], ".wad");

      if (file) {
        D_AddFile(file, source_pwad);
        free(file);
      }
    }
  }

  if (!(p = M_CheckParm("-playdemo")) || p >= myargc - 1) {  /* killough */
    if ((p = M_CheckParm("-fastdemo")) && p < myargc - 1) {  /* killough */
      fastdemo = true;             // run at fastest speed possible
    }
    else {
      if ((p = IsDemoContinue())) {
        if (demo_continue_name)
          free(demo_continue_name);

        demo_continue_name = M_AddDefaultExtension(myargv[p + 2], "lmp");
        democontinue = true;
      }
      else {
        p = M_CheckParm("-timedemo");
      }
    }
  }

  if (p && p < myargc - 1) {
    char *file = M_AddDefaultExtension(myargv[p + 1], "lmp");     // killough
    char *file_path = I_FindFile(file, NULL);

    if (file_path) {
      D_Msg(MSG_INFO, "Playing demo %s\n", file);
      D_AddFile(file, source_lmp);
    }
    else {
      D_Msg(MSG_INFO,
        "Demo file %s not found, assuming %s is a lump name\n",
        file, myargv[p + 1]
      );
    }

    if ((p = M_CheckParm("-ffmap")) && p < myargc - 1)
      ffmap = atoi(myargv[p + 1]);

    free(file);
  }

  // internal translucency set to config file value               // phares
  general_translucency = default_translucency;                    // phares

  //e6y
  {
    int demo_footer = CheckDemoExDemo();
    if (!demo_footer)
      demo_footer = CheckAutoDemo();
#ifdef USE_WINDOWS_LAUNCHER
    LauncherShow(demo_footer);
#endif
  }

  if (!DELTACLIENT)
    W_Init(); // CPhipps - handling of wadfiles init changed

  // e6y
  // option to disable automatic loading of dehacked-in-wad lump
  if (!M_CheckParm ("-nodeh")) {
    // MBF-style DeHackEd in wad support: load all lumps, not just the last one
    for (p = -1; (p = W_ListNumFromName("DEHACKED", p)) >= 0; ) {
      // Split loading DEHACKED lumps into IWAD/autoload and PWADs/others
      if (lumpinfo[p].source == source_iwad ||
          lumpinfo[p].source == source_pre  ||
          lumpinfo[p].source == source_auto_load) {
        D_AddDEH(NULL, p); // cph - add dehacked-in-a-wad support
      }
    }

    if (bfgedition) {
      int lump = (W_CheckNumForName)("BFGBEX", ns_prboom);

      if (lump != -1)
        D_AddDEH(NULL, lump);

      if (gamemission == pack_nerve) {
        lump = (W_CheckNumForName)("NERVEBEX", ns_prboom);

        if (lump != -1)
          D_AddDEH(NULL, lump);
      }
    }

    if (gamemission == chex) {
      int lump = (W_CheckNumForName)("CHEXDEH", ns_prboom);

      if (lump != -1)
        D_AddDEH(NULL, lump);
    }
  }

  // now do autoloaded dehacked patches, after IWAD patches but before PWAD
  if ((!CLIENT) && !M_CheckParm("-noload")) {
    for (int i = 0; i < MAXLOADFILES; i++) {
      const char *fname = deh_file_names[i];
      char *fpath;

      if (!(fname && *fname))
        continue;

      // Filename is now stored as a zero terminated string
      fpath = I_FindFile(fname, ".bex");

      if (!fpath) {
        D_Msg(MSG_INFO, "Failed to autoload %s\n", fname);
      }
      else {
        D_AddDEH(fpath, 0);
        // this used to set modifiedgame here, but patches shouldn't
        free(fpath);
      }
    }
  }

  if (!M_CheckParm("-nodeh")) {
    for (p = -1; (p = W_ListNumFromName("DEHACKED", p)) >= 0; ) {
      if (!(lumpinfo[p].source == source_iwad ||
            lumpinfo[p].source == source_pre  ||
            lumpinfo[p].source == source_auto_load)) {
        D_AddDEH(NULL, p);
      }
    }
  }

  // Load command line dehacked patches after WAD dehacked patches

  // e6y: DEH files preloaded in wrong order
  // http://sourceforge.net/p/prboom-plus/bugs/18/

  // ty 03/09/98 do dehacked stuff
  // Using -deh in BOOM, others use -dehacked.
  // Ty 03/18/98 also allow .bex extension.  .bex overrides if both exist.

  p = M_CheckParm("-deh");
  if ((!CLIENT) && p) {
    // the parms after p are deh/bex file names,
    // until end of parms or another - preceded parm
    // Ty 04/11/98 - Allow multiple -deh files in a row
    //
    // e6y
    // reorganization of the code for looking for bex/deh patches
    // in all standard dirs (%DOOMWADDIR%, etc)
    while (++p != myargc && *myargv[p] != '-') {
      char *file = NULL;

      if ((file = I_FindFile(myargv[p], ".bex")) ||
          (file = I_FindFile(myargv[p], ".deh"))) {
        // during the beta we have debug output to dehout.txt
        D_AddDEH(file, 0);
        free(file);
      }
      else {
        I_Error("D_DoomMainSetup: Cannot find .deh or .bex file named %s",myargv[p]);
      }
    }
  }

  V_InitColorTranslation(); //jff 4/24/98 load color translation lumps

  // killough 2/22/98: copyright / "modified game" / SPA banners removed

  // Ty 04/08/98 - Add 5 lines of misc. data, only if nonblank
  // The expectation is that these will be set in a .bex file
  if (*startup1)
    D_Msg(MSG_INFO, "%s", startup1);
  if (*startup2)
    D_Msg(MSG_INFO, "%s", startup2);
  if (*startup3)
    D_Msg(MSG_INFO, "%s", startup3);
  if (*startup4)
    D_Msg(MSG_INFO, "%s", startup4);
  if (*startup5)
    D_Msg(MSG_INFO, "%s", startup5);
  // End new startup strings

  // CG 6/13/2015 Return of C_Init
  C_Init();

  D_Msg(MSG_INFO, "M_Init: Init miscellaneous info.\n");
  M_Init();

  D_Msg(MSG_INFO, "R_Init: Init DOOM refresh daemon - ");
  R_Init();

  D_Msg(MSG_INFO, "\nP_IdentInit: Init actor IDs.\n");
  P_IdentInit();

  D_Msg(MSG_INFO,"P_Init: Init Playloop state.\n");
  P_Init();

  D_Msg(MSG_INFO, "I_Init: Setting up machine state.\n");
  I_Init();

  D_Msg(MSG_INFO, "S_Init: Setting up sound.\n");
  S_Init(snd_SfxVolume /* *8 */, snd_MusicVolume /* *8*/ );

  if ((!SERVER) && !(M_CheckParm("-nodraw") && M_CheckParm("-nosound")))
    I_InitGraphics();

  graphics_initialized = true;

  D_Msg(MSG_INFO, "HU_Init: Setting up heads up display.\n");
  HU_Init();

  // NSM
  if ((p = M_CheckParm("-viddump")) && (p < myargc - 1))
    I_CapturePrep(myargv[p + 1]);

  D_Msg(MSG_INFO, "ST_Init: Init status bar.\n");
  ST_Init();

  XAM_RegisterInterface();
  XC_RegisterInterface();
  XD_RegisterInterface();
  XD_MsgRegisterInterface();
  XG_GameRegisterInterface();
  XG_KeysRegisterInterface();
  XI_RegisterInterface();
  XI_InputRegisterInterface();
  XM_MenuRegisterInterface();
  XM_MiscRegisterInterface();
  XP_UserRegisterInterface();
  XST_RegisterInterface();
  XV_RegisterInterface();

  X_ExposeInterfaces(NULL);

  X_Start();

  // CPhipps - auto screenshots
  if ((p = M_CheckParm("-autoshot")) && (p < myargc - 2))
    if ((auto_shot_count = auto_shot_time = atoi(myargv[p + 1])))
      auto_shot_fname = myargv[p + 2];

  // start the apropriate game based on parms

  // killough 12/98:
  // Support -loadgame with -record and reimplement -recordfrom.

  if ((slot = M_CheckParm("-recordfrom")) && (p = slot + 2) < myargc) {
    G_RecordDemo(myargv[p]);
  }
  else {
    slot = M_CheckParm("-loadgame");
    if ((p = M_CheckParm("-record")) && ++p < myargc) {
      autostart = true;
      G_RecordDemo(myargv[p]);
    }
  }

  if ((p = M_CheckParm ("-checksum")) && ++p < myargc)
    P_RecordChecksum(myargv[p]);

  if ((p = M_CheckParm ("-fastdemo")) && ++p < myargc) { // killough
    fastdemo = true;                // run at fastest speed possible
    timingdemo = true;              // show stats after quit
    G_DeferedPlayDemo(myargv[p]);
    singledemo = true;              // quit after one demo
  }
  else if ((p = M_CheckParm("-timedemo")) && ++p < myargc) {
    singletics = true;
    timingdemo = true;            // show stats after quit
    G_DeferedPlayDemo(myargv[p]);
    singledemo = true;            // quit after one demo
  }
  else if ((p = M_CheckParm("-playdemo")) && ++p < myargc) {
    G_DeferedPlayDemo(myargv[p]);
    singledemo = true;          // quit after one demo
  }
  else if ((p = IsDemoContinue())) { //e6y
    G_DeferedPlayDemo(myargv[p + 1]);
    G_CheckDemoContinue();
  }

  if (slot && ++slot < myargc) {
    slot = atoi(myargv[slot]); // killough 3/16/98: add slot info
    // killough 5/15/98: add command flag
    // cph - no filename
    G_LoadGame(slot, true);
  }
  else if (!singledemo) { /* killough 12/98 */
    if (autostart || netgame) {
      if (autostart) // sets first map and first episode if unknown
        GetFirstMap(&startepisode, &startmap);

      if (CLIENT)
        N_LoadLatestState(true);
      else
        G_InitNew(startskill, startepisode, startmap);

      if (demorecording)
        G_BeginRecording();
    }
    else {
      D_StartTitle();                 // start up intro loop
    }
  }

  // do not try to interpolate during timedemo
  M_ChangeUncappedFrameRate();
}

//
// D_DoomMain
//

void D_DoomMain(void) {
  // CPhipps - setup out of main execution stack
  D_DoomMainSetup();

  // never returns
  D_DoomLoop();
}

//
// GetFirstMap
//
// Ty 08/29/98 - determine first available map from the loaded wads and run it
//

void GetFirstMap(int *ep, int *map)
{
  int i,j; // used to generate map name
  dboolean done = false;  // Ty 09/13/98 - to exit inner loops
  char test[6];  // MAPxx or ExMx plus terminator for testing
  char name[6];  // MAPxx or ExMx plus terminator for display
  dboolean newlevel = false;  // Ty 10/04/98 - to test for new level
  int ix;  // index for lookup

  strcpy(name,""); // initialize
  if (*map == 0) // unknown so go search for first changed one
  {
    *ep = 1;
    *map = 1; // default E1M1 or MAP01
    if (gamemode == commercial)
    {
      for (i=1;!done && i<33;i++)  // Ty 09/13/98 - add use of !done
      {
        sprintf(test,"MAP%02d",i);
        ix = W_CheckNumForName(test);
        if (ix != -1)  // Ty 10/04/98 avoid -1 subscript
        {
          if (lumpinfo[ix].source == source_pwad)
          {
            *map = i;
            strcpy(name,test);  // Ty 10/04/98
            done = true;  // Ty 09/13/98
            newlevel = true; // Ty 10/04/98
          }
          else
          {
            if (!*name)  // found one, not pwad.  First default.
               strcpy(name,test);
          }
        }
      }
    }
    else // one of the others
    {
      strcpy(name,"E1M1");  // Ty 10/04/98 - default for display
      for (i=1;!done && i<5;i++)  // Ty 09/13/98 - add use of !done
      {
        for (j=1;!done && j<10;j++)  // Ty 09/13/98 - add use of !done
        {
          sprintf(test,"E%dM%d",i,j);
          ix = W_CheckNumForName(test);
          if (ix != -1)  // Ty 10/04/98 avoid -1 subscript
          {
            if (lumpinfo[ix].source == source_pwad)
            {
              *ep = i;
              *map = j;
              strcpy(name,test); // Ty 10/04/98
              done = true;  // Ty 09/13/98
              newlevel = true; // Ty 10/04/98
            }
            else
            {
              if (!*name)  // found one, not pwad.  First default.
                 strcpy(name,test);
            }
          }
        }
      }
    }
    D_Msg(MSG_INFO, "Auto-warping to first %slevel: %s\n",
      newlevel ? "new " : "", name
    );  // Ty 10/04/98 - new level test
  }
}

/* vi: set et ts=2 sw=2: */


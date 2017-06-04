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

#include "i_sound.h"
#include "i_system.h"
#include "m_argv.h"
#include "m_file.h"
#include "m_random.h"
#include "am_map.h"
#include "c_main.h"
#include "d_deh.h"
#include "d_dump.h"
#include "d_event.h"
#include "d_main.h"
#include "d_mouse.h"
#include "d_res.h"
#include "e6y.h"
#include "f_finale.h"
#include "g_comp.h"
#include "g_demo.h"
#include "g_game.h"
#include "g_input.h"
#include "g_keys.h"
#include "g_player.h"
#include "g_save.h"
#include "hu_stuff.h"
#include "mn_main.h"
#include "p_camera.h"
#include "p_ident.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_tick.h"
#include "pl_cmd.h"
#include "pl_main.h"
#include "r_data.h"
#include "r_defs.h"
#include "r_draw.h"
#include "r_fps.h"
#include "r_main.h"
#include "r_sky.h"
#include "s_sound.h"
#include "sounds.h"
#include "st_stuff.h"
#include "v_video.h"
#include "w_wad.h"
#include "wi_stuff.h"
#include "x_main.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#include "n_main.h"
#include "n_proto.h"
#include "cl_main.h"
#include "cl_net.h"

extern bool setsizeneeded;
extern bool doSkip;

static bool secretexit;
static skill_t d_skill;
static int d_episode;
static int d_map;
static bool saved_fastdemo;
static bool saved_nodrawers;
static bool saved_nosfxparm;
static bool saved_nomusicparm;
static int saved_render_precise;

// DOOM Par Times
int pars[5][10] = {
  { 0 },
  { 0, 30, 75, 120, 90, 165, 180, 180, 30, 165 },
  { 0, 90, 90, 90, 120, 90, 360, 240, 30, 170 },
  { 0, 90, 45, 90, 150, 90, 90, 165, 30, 135 },

  // from Doom 3 BFG Edition
  { 0, 165, 255, 135, 150, 180, 390, 135, 360, 180 }
};

// DOOM II Par Times
int cpars[34] = {
  30, 90, 120, 120, 90, 150, 120, 120, 270, 90,     //  1-10
  210, 150, 150, 150, 210, 150, 420, 150, 210, 150, // 11-20
  240, 150, 180, 150, 150, 300, 330, 420, 300, 180, // 21-30
  120, 30, 30, 30                                   // 31-34
};

gameaction_t gameaction;

// CPhipps - moved *_loadgame vars here
bool forced_loadgame = false;
bool command_loadgame = false;
gamestate_t gamestate = GS_BAD;
gamestate_t prevgamestate = GS_LEVEL;

// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t wipegamestate = GS_DEMOSCREEN;
gamestate_t oldgamestate = GS_BAD;
skill_t gameskill;
bool respawnmonsters;
int gameepisode;
int gamemap;
bool paused;
bool nodrawers; // for comparative timing purposes
bool noblit;    // for comparative timing purposes
int starttime;  // for comparative timing purposes
int deathmatch; // only if started as net death

/*
 * [CG] Here we go...
 *  bool            playeringame[MAXPLAYERS];
 *  player_t        players[MAXPLAYERS];
 */
int gametic;
int basetic; /* killough 9/29/98: for demo sync */
int totalkills, totallive, totalitems, totalsecret;// for intermission
int show_alive;
int next_map;
bool haswolflevels = false; // jff 4/18/98 wolf levels present
int totalleveltimes; // CPhipps - total time for all completed levels
int bytes_per_tic;
time_t level_start_time;

// jff 3/24/98 define defaultskill here
int defaultskill; // note 1-based

void G_SkipDemoStart(void) {
  saved_fastdemo = fastdemo;
  saved_nodrawers = nodrawers;
  saved_nosfxparm = nosfxparm;
  saved_nomusicparm = nomusicparm;
  saved_render_precise = render_precise;

  paused = false;
  
  doSkip = true;

  S_StopMusic();
  fastdemo = true;
  nodrawers = true;
  nosfxparm = true;
  nomusicparm = true;

  render_precise = render_precise_speed;
  MN_ChangeRenderPrecise();

  I_Init2();
}

void G_SkipDemoStop(void) {
  fastdemo = saved_fastdemo;
  nodrawers = saved_nodrawers;
  nosfxparm = saved_nosfxparm;
  nomusicparm = saved_nomusicparm;
  
  render_precise = saved_render_precise;
  MN_ChangeRenderPrecise();

  demo_stoponnext = false;
  demo_stoponend = false;
  demo_warp = false;
  doSkip = false;
  demo_skiptics = 0;
  startmap = 0;

  I_Init2();

  if (!sound_inited_once && !(nomusicparm && nosfxparm)) {
    I_InitSound();
  }

  S_Init(snd_SfxVolume, snd_MusicVolume);
  S_Stop();
  S_RestartMusic();

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    gld_PreprocessLevel();
  }
#endif
}

void G_SkipDemoCheck(void) {
  if (doSkip && gametic > 0) {
    if (((startmap <= 1) && 
         (gametic > demo_skiptics + (demo_skiptics > 0 ?
                                     0 :
                                     demo_tics_count))) ||
        (demo_warp && gametic - levelstarttic > demo_skiptics)) {
       G_SkipDemoStop();
    }
  }
}

int G_ReloadLevel(void) {
  int result = false;

  if ((gamestate == GS_LEVEL) && !deathmatch && !netgame && !demorecording &&
                                                            !demoplayback &&
                                                            !menuactive) {
    G_DeferedInitNew(gameskill, gameepisode, gamemap);
    result = true;
  }

  return result;
}

int G_GotoNextLevel(void) {
  static unsigned char doom2_next[33] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
  };
  static unsigned char doom_next[4][9] = {
    {12, 13, 19, 15, 16, 17, 18, 21, 14},
    {22, 23, 24, 25, 29, 27, 28, 31, 26},
    {32, 33, 34, 35, 36, 39, 38, 41, 37},
    {42, 49, 44, 45, 46, 47, 48, 11, 43}
  };

  int changed = false;

  // secret level
  doom2_next[14] = (haswolflevels ? 31 : 16);
  
  if (bfgedition && SINGLEPLAYER) {
    if (gamemission == pack_nerve) {
      doom2_next[3] = 9;
      doom2_next[7] = 1;
      doom2_next[8] = 5;
    }
    else {
      doom2_next[1] = 33;
    }
  }

  // shareware doom has only episode 1
  doom_next[0][7] = (gamemode == shareware ? 11 : 21);
  
  doom_next[2][7] = ((gamemode == registered) ||
    // the fourth episode for pre-ultimate complevels is not allowed.
    (compatibility_level < ultdoom_compatibility) ?
    11 : 41);
  
  if ((gamestate == GS_LEVEL) && !deathmatch && !netgame && !demorecording &&
                                                            !demoplayback &&
                                                            !menuactive) {
    //doom2_next and doom_next are 0 biased, unlike gameepisode and gamemap
    int epsd = gameepisode - 1;
    int map = gamemap - 1;

    if (gamemode == commercial) {
      epsd = 1;
      map = doom2_next[BETWEEN(0, 32, map)];
    }
    else {
      int next = doom_next[BETWEEN(0, 3, epsd)][BETWEEN(0, 9, map)];

      epsd = next / 10;
      map = next % 10;
    }

    G_DeferedInitNew(gameskill, epsd, map);
    changed = true;
  }

  return changed;
}

//
// killough 5/15/98: add forced loadgames, which allow user to override checks
//
void G_ForcedLoadGame(void) {
  // CPhipps - net loadgames are always forced, so we
  //           so we only reach here in single player
  G_SetGameAction(ga_loadgame);
  forced_loadgame = true;
}

// CPhipps - do savegame filename stuff here
void G_DoLoadGame(void) {
  char *name;                                      // killough 3/22/98
  size_t length;
  pbuf_t savebuffer;
  char maplump[8];
  int seconds;
  int total_seconds;

  length = G_SaveGameName(NULL, 0, savegameslot, demoplayback);
  name = malloc(length + 1);
  G_SaveGameName(name, length + 1, savegameslot, demoplayback);

  G_SetGameAction(ga_nothing);

  M_PBufInitWithCapacity(&savebuffer, MAX(G_GetAverageSaveSize(), 16384));

  if (!M_PBufSetFile(&savebuffer, name)) {
    D_MsgLocalWarn("Couldn't read file %s: %s\n", name, strerror(errno));
    M_PBufFree(&savebuffer);
    return;
  }

  free(name);

  M_PBufSeek(&savebuffer, 0);

  if (!G_ReadSaveData(&savebuffer, false, true)) {
    I_Error("Error loading save data\n");
  }

  /* Print some information about the save game */
  if (gamemode == commercial) {
    sprintf(maplump, "MAP%02d", gamemap);
  }
  else {
    sprintf(maplump, "E%dM%d", gameepisode, gamemap);
  }

  seconds = leveltime / TICRATE;
  total_seconds = (totalleveltimes + leveltime) / TICRATE;

  wadfile_info_t *wf = g_ptr_array_index(
    resource_files,
    W_GetLumpInfoByNum(W_GetNumForName(maplump))->wadfile
    );

  if (!wf) {
    I_Error("G_DoLoadGame: Couldn't find wadfile for %s\n", maplump);
  }

  D_MsgLocalInfo(
    "G_DoLoadGame: [%d] %s (%s), Skill %d, Level Time %02d:%02d:%02d, "
    "Total Time %02d:%02d:%02d\n",
    savegameslot + 1,
    maplump,
    wf->name,
    gameskill + 1,
    seconds / 3600,
    (seconds % 3600) / 60,
    seconds % 60,
    total_seconds / 3600,
    (total_seconds % 3600) / 60,
    total_seconds % 60
  );

  // done
  M_PBufFree(&savebuffer);

  if (setsizeneeded) {
    R_ExecuteSetViewSize();
  }

  // draw the pattern into the back screen
  R_FillBackScreen();

  /* killough 12/98: support -recordfrom and -loadgame -playdemo */
  if (!command_loadgame) {
    singledemo = false;  /* Clear singledemo flag if loading from menu */
  }
  else if (singledemo) {
    /* Mark that we're loading a game before demo */
    /* This will detect it and won't reinit level */
    G_SetGameAction(ga_loadgame);
    G_DoPlayDemo();
  }
  else if (demorecording) {
    /* Command line + record means it's a recordfrom */
    G_BeginRecording();
  }
}

void G_DoSaveGame(bool menu) {
  char *name;
  int length;
  pbuf_t savebuffer;
  char maplump[8];
  int seconds;
  int total_seconds;
  bool save_succeeded;

  // cph - cancel savegame at top of this function,
  // in case later problems cause a premature exit
  G_SetGameAction(ga_nothing);

  M_PBufInitWithCapacity(&savebuffer, MAX(G_GetAverageSaveSize(), 16384));

  length = G_SaveGameName(NULL, 0, savegameslot, demoplayback && !menu);
  name = malloc(length + 1);
  G_SaveGameName(name, length + 1, savegameslot, demoplayback && !menu);

  G_WriteSaveData(&savebuffer);

  save_succeeded = M_WriteFile(
    name, M_PBufGetData(&savebuffer), M_PBufGetSize(&savebuffer)
    );

  if (save_succeeded) {
    D_MsgLocalInfo("%s\n", s_GGSAVED);/* Ty - externalized */
  }
  else {
    // CPhipps - not externalised
    D_MsgLocalInfo("%s\n", "Game save failed!");
  }

  M_PBufFree(&savebuffer);                   // killough

  /* Print some information about the save game */
  if (gamemode == commercial) {
    sprintf(maplump, "MAP%02d", gamemap);
  }
  else {
    sprintf(maplump, "E%dM%d", gameepisode, gamemap);
  }

  seconds = leveltime / TICRATE;
  total_seconds = (totalleveltimes + leveltime) / TICRATE;

  wadfile_info_t *wf = g_ptr_array_index(
    resource_files,
    W_GetLumpInfoByNum(W_GetNumForName(maplump))->wadfile
    );

  if (!wf) {
    I_Error("G_DoSaveGame: Couldn't find wadfile for %s\n", maplump);
  }

  D_MsgLocalInfo(
    "G_DoSaveGame: [%d] %s (%s), Skill %d, Level Time %02d:%02d:%02d, "
    "Total Time %02d:%02d:%02d\n",
    savegameslot + 1,
    maplump,
    wf->name,
    gameskill + 1,
    seconds / 3600,
    (seconds % 3600) / 60,
    seconds % 60,
    total_seconds / 3600,
    (total_seconds % 3600) / 60,
    total_seconds % 60
  );

  savedescription[0] = 0;
  free(name);
}

//
// G_RestartLevel
//

void G_RestartLevel(void) {
  special_event = BT_SPECIAL | (BTS_RESTARTLEVEL & BT_SPECIALMASK);
}

//
// G_DoLoadLevel
//

void G_DoLoadLevel(void) {
  if (CLIENT) {
    R_Init();
  }

  // Set the sky map.
  // First thing, we have a dummy sky texture name,
  //  a flat. The data is in the WAD only because
  //  we look for an actual index, instead of simply
  //  setting one.

  skyflatnum = R_FlatNumForName(SKYFLATNAME);

  // DOOM determines the sky texture to be used
  // depending on the current episode, and the game version.
  if (gamemode == commercial) {
    // || gamemode == pack_tnt   //jff 3/27/98 sorry guys pack_tnt,pack_plut
    // || gamemode == pack_plut) //aren't gamemodes, this was matching retail
    skytexture = R_TextureNumForName("SKY3");
    if (gamemap < 12) {
      skytexture = R_TextureNumForName("SKY1");
    }
    else if (gamemap < 21) {
      skytexture = R_TextureNumForName("SKY2");
    }
  }
  else {                                     // jff 3/27/98 and lets not forget
                                             // about DOOM and Ultimate DOOM
                                             // huh?
    switch (gameepisode) {
      case 1:
        skytexture = R_TextureNumForName("SKY1");
        break;
      case 2:
        skytexture = R_TextureNumForName("SKY2");
        break;
      case 3:
        skytexture = R_TextureNumForName("SKY3");
        break;
      case 4:                                // Special Edition sky
        skytexture = R_TextureNumForName("SKY4");
        break;
    } // jff 3/27/98 end sky setting fix
  }

  // [RH] Set up details about sky rendering
  R_InitSkyMap();

#ifdef GL_DOOM
  R_SetBoxSkybox(skytexture);
#endif /* ifdef GL_DOOM */

  levelstarttic = gametic;                   // for time calculation

  if (!demo_compatibility && !mbf_features) {// killough 9/29/98
    basetic = gametic;
  }

  if (wipegamestate == GS_LEVEL) {
    G_SetWipeGameState(-1);                  // force a wipe
  }

  G_SetGameState(GS_LEVEL);
  level_start_time = time(NULL);

  PLAYERS_FOR_EACH(iter) {
    if (iter.player->playerstate == PST_DEAD) {
      iter.player->playerstate = PST_REBORN;
    }

    PL_ClearFragsAndDeaths(iter.player);
  }

  // initialize the msecnode_t freelist.                     phares 3/25/98
  // any nodes in the freelist are gone by now, cleared
  // by Z_FreeTags() when the previous level ended or player
  // died.
  P_FreeSecNodeList();

  /* CG 08/10/2014: Reset IDs */
  P_IdentReset();

  P_SetupLevel(gameepisode, gamemap, 0, gameskill);

  if (!demoplayback) { // Don't switch views if playing a demo
    // view the player you are playing
    P_SetDisplayPlayer(P_GetConsolePlayer());
  }

  G_SetGameAction(ga_nothing);
  Z_CheckHeap();

  // clear cmd building stuff
  G_InputClear();

  // killough 5/13/98: in case netdemo has consoleplayer other than green
  ST_Start();
  HU_Start();

  // killough: make -timedemo work on multilevel demos
  // Move to end of function to minimize noise -- killough 2/22/98:

  if (timingdemo) {
    static int first = 1;

    if (first) {
      starttime = I_GetTime_RealTime();
      first = 0;
    }
  }

  if (SERVER) {
    SV_ResyncPeers();
  }
}

//
// G_Responder
// Get info needed to make ticcmd_ts for the players.
//

bool G_Responder(event_t *ev) {
  // allow spy mode changes even during the demo
  // killough 2/22/98: even during DM demo
  //
  // killough 11/98: don't autorepeat spy mode switch

  if (ev->key == key_spy &&
    netgame &&
    (demoplayback || !deathmatch) &&
    gamestate == GS_LEVEL) {
    if (ev->type == ev_key && !ev->pressed) {
      gamekeydown[key_spy] = false;
    }

    if (ev->type == ev_key && ev->pressed && !gamekeydown[key_spy]) {
      player_t *displayplayer = P_GetDisplayPlayer();

      gamekeydown[key_spy] = true;

      PLAYERS_FOR_EACH_AT(iter, displayplayer) {
        if (iter.player != displayplayer) {
          break; // spy mode
        }
      }

      ST_Start();// killough 3/7/98: switch status bar views too
      HU_Start();
      S_UpdateSounds(P_GetDisplayPlayer()->mo);
      R_ActivateSectorInterpolations();
      G_DemoSmoothPlayingReset(NULL);
    }

    return true;
  }

  // any other key pops up menu if in demos
  //
  // killough 8/2/98: enable automap in -timedemo demos
  //
  // killough 9/29/98: make any key pop up menu regardless of
  // which kind of demo, and allow other events during playback

  if (gameaction == ga_nothing &&
    (demoplayback || gamestate == GS_DEMOSCREEN)) {
    // killough 9/29/98: allow user to pause demos during playback
    if (ev->type == ev_key && ev->pressed && ev->key == key_pause) {
      if (paused ^= 2) {
        S_PauseSound();
      }
      else {
        S_ResumeSound();
      }

      return true;
    }

    // killough 10/98:
    // Don't pop up menu, if paused in middle
    // of demo playback, or if automap active.
    // Don't suck up keys, which may be cheats

    // e6y

    /*
     *   return gamestate == GS_DEMOSCREEN &&
     *  !(paused & 2) && !(automapmode & am_active) &&
     *  ((ev->type == ev_key && ev->pressed) ||
     *  (ev->type == ev_mouse && ev->key) ||
     *  (ev->type == ev_joystick && ev->key)) ?
     *  M_StartControlPanel(), true : false;
     */
  }

  if (gamestate == GS_FINALE && F_Responder(ev)) {
    return true; // finale ate the event
  }

  return G_InputHandleEvent(ev);
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//

void G_Ticker(void) {
#if 0

  // CPhipps - player colour changing
  {
    int index = consoleplayer % VANILLA_MAXPLAYERS;

    if (!demoplayback && vanilla_mapplayer_colors[index] != mapcolor_me) {
      if (CLIENT) { // Changed my multiplayer colour - Inform the whole game
        CL_SendColorIndexChange(mapcolor_me);
      }

      G_ChangedPlayerColour(consoleplayer, mapcolor_me);
    }
  }
#endif /* if 0 */

  P_MapStart("G_Ticker");

  // do player reborns if needed
  PLAYERS_FOR_EACH(iter) {
    if (iter.player->playerstate == PST_REBORN) {
      G_DoReborn(iter.player);
    }

    if (iter.player->playerstate == PST_DISCONNECTED) {
      C_Printf("Player %u disconnected\n", iter.player->id);

      if (iter.player->mo) {
        mobj_t *actor = iter.player->mo;
        fixed_t x     = actor->x;
        fixed_t y     = actor->y;
        fixed_t z     = actor->z;

        P_RemoveMobj(iter.player->mo);
        iter.player->mo = NULL;
        P_SpawnMobj(x, y, z, MT_TFOG);
        S_StartSound(actor, sfx_telept);
      }

      P_PlayersIterateRemove(&iter);
    }
  }

  P_MapEnd();

  // do things to change the game state
  while (gameaction != ga_nothing) {
    switch (gameaction) {
      case ga_loadlevel:

        // force players to be initialized on level reload
        PLAYERS_FOR_EACH(iter) {
          iter.player->playerstate = PST_REBORN;
        }

        G_DoLoadLevel();
        break;
      case ga_newgame:
        G_DoNewGame();
        break;
      case ga_loadgame:
        G_DoLoadGame();
        break;
      case ga_savegame:
        G_DoSaveGame(false);
        break;
      case ga_playdemo:
        G_DoPlayDemo();
        break;
      case ga_completed:
        G_DoCompleted();
        break;
      case ga_victory:
        F_StartFinale();
        break;
      case ga_worlddone:
        G_DoWorldDone();
        break;
      case ga_nothing:
        break;
    }
  }

  if (paused & 2 || (!demoplayback && menuactive && !netgame)) {
    basetic++;  // For revenant tracers and RNG -- we must maintain sync
  }
  else if (!MULTINET) {
    PLAYERS_FOR_EACH(iter) {
      ticcmd_t *cmd = &iter.player->cmd;

      // e6y
      if (demoplayback) {
        memset(cmd, 0, sizeof(ticcmd_t));
        G_DemoReadTiccmd(cmd);
      }
      else if (democontinue) {
        memset(cmd, 0, sizeof(ticcmd_t));
        G_DemoReadContinueTiccmd(cmd);
      }

      if (demorecording) {
        G_DemoWriteTiccmd(cmd);
      }

      // check for turbo cheats
      // killough 2/14/98, 2/20/98 -- only warn in netgames and demos

      if ((netgame || demoplayback) &&
          cmd->forwardmove > TURBOTHRESHOLD &&
          !(gametic & 31) &&
          ((gametic >> 5) & 3) == PL_GetVanillaNum(iter.player)) {
        /* cph - don't use sprintf, use doom_printf */
        D_MsgLocalWarn("Player %u is turbo!\n", iter.player->id);
      }
    }

    // check for special buttons
    PLAYERS_FOR_EACH(iter) {
      if (iter.player->cmd.buttons & BT_SPECIAL) {
        switch (iter.player->cmd.buttons & BT_SPECIALMASK) {
          case BTS_PAUSE:
            paused ^= 1;
            if (paused) {
              S_PauseSound();
            }
            else {
              S_ResumeSound();
            }
            break;
          case BTS_SAVEGAME:
            if (!savedescription[0]) {
              strcpy(savedescription, "NET GAME");
            }
            savegameslot =
              (iter.player->cmd.buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
            G_SetGameAction(ga_savegame);
            break;
          case BTS_LOADGAME:          // CPhipps - remote loadgame request
            savegameslot =
              (iter.player->cmd.buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
            G_SetGameAction(ga_loadgame);
            forced_loadgame = netgame;// Force if a netgame
            command_loadgame = false;
            break;
          case BTS_RESTARTLEVEL:      // CPhipps - Restart the level
            if (demoplayback ||
              (compatibility_level < lxdoom_1_compatibility)) {
              break;                  // CPhipps - Ignore in demos or old games
            }

            G_SetGameAction(ga_loadlevel);
            break;
        }
        iter.player->cmd.buttons = 0;
      }
    }
  }

  // cph - if the gamestate changed, we may need to clean up the old gamestate
  if (gamestate != prevgamestate) {
    switch (prevgamestate) {
      case GS_LEVEL:

        // This causes crashes at level end - Neil Stevens
        // The crash is because the sounds aren't stopped before freeing them
        // the following is a possible fix
        // This fix does avoid the crash wowever, with this fix in, the exit
        // switch sound is cut off
        // S_Stop();
        // Z_FreeTags(PU_LEVEL, PU_PURGELEVEL-1);
        break;
      case GS_INTERMISSION:
        WI_End();
      default:
        break;
    }

    G_SetPrevGameState(gamestate);
  }

  // e6y
  // do nothing if a pause has been pressed during playback
  // pausing during intermission can cause desynchs without that
  if (paused & 2 && gamestate != GS_LEVEL) {
    return;
  }

  if (SERVER && gamestate != GS_LEVEL) {
    PLAYERS_FOR_EACH(iter) {
      PL_IgnoreCommands(iter.player);
    }
  }

  // do main actions
  switch (gamestate) {
    case GS_LEVEL:
      P_Ticker();
      P_WalkTicker();
      mlooky = 0;
      AM_Ticker();
      ST_Ticker();

      if (D_DumpEnabled()) {
        D_DumpUpdate();
      }
      break;
    case GS_INTERMISSION:
      WI_Ticker();
      break;
    case GS_FINALE:
      F_Ticker();
      break;
    case GS_DEMOSCREEN:
      D_PageTicker();
      break;
    case GS_BAD:
      break;
  }
}

void G_Drawer(void) {
  static bool borderwillneedredraw = false;
  static bool isborderstate        = false;

  bool viewactive = false;
  bool isborder   = false;

  bool redrawborderstuff;

  // Work out if the player view is visible, and if there is a border
  viewactive = (!(automapmode & am_active) || (automapmode & am_overlay)) &&
    !inhelpscreens;
  if (viewactive) {
    isborder = viewheight != SCREENHEIGHT;
  }
  else {
    isborder = (!inhelpscreens && (automapmode & am_active));
  }

  if (oldgamestate != GS_LEVEL) {
    R_FillBackScreen();    // draw the pattern into the back screen
    redrawborderstuff = isborder;
  }
  else {
    // CPhipps -
    // If there is a border, and either there was no border last time,
    // or the border might need refreshing, then redraw it.
    redrawborderstuff = isborder && (!isborderstate || borderwillneedredraw);

    // The border may need redrawing next time if the border surrounds the
    // screen,
    // and there is a menu being displayed
    borderwillneedredraw = menuactive && isborder && viewactive;

    // e6y
    // I should do it because I call R_RenderPlayerView in all cases,
    // not only if viewactive is true
    borderwillneedredraw = (borderwillneedredraw) || (
      (automapmode & am_active) && !(automapmode & am_overlay)
      );
  }
#ifdef GL_DOOM
  if (redrawborderstuff || (V_GetMode() == VID_MODEGL)) {
    R_DrawViewBorder();
  }
#else  /* ifdef GL_DOOM */
  if (redrawborderstuff) {
    R_DrawViewBorder();
  }
#endif /* ifdef GL_DOOM */

  // e6y
  // Boom colormaps should be applied for everything in R_RenderPlayerView
  use_boom_cm = true;

  R_InterpolateView(P_GetDisplayPlayer());

  R_ClearStats();

  // Now do the drawing
  if (viewactive || map_always_updates) {
    R_RenderPlayerView(P_GetDisplayPlayer());
  }

  // IDRATE cheat
  R_ShowStats();

  // e6y
  // but should NOT be applied for automap, statusbar and HUD
  use_boom_cm = false;
  frame_fixedcolormap = 0;

  if (automapmode & am_active) {
    AM_Drawer();
  }

  R_RestoreInterpolations();

  ST_Drawer(
    ((viewheight != SCREENHEIGHT) || (
    (automapmode & am_active) && !(automapmode & am_overlay))
    ),
    redrawborderstuff || BorderNeedRefresh,
    menuactive == mnact_full
  );

  BorderNeedRefresh = false;

  if (V_GetMode() != VID_MODEGL) {
    R_DrawViewBorder();
  }
}

//
// G_DoReborn
//
void G_DoReborn(player_t *player) {
  if (netgame) {
    mapthing_t *start = NULL;

    // respawn at the start

    // first dissasociate the corpse
    if (player->mo) {
      player->mo->player = NULL;
    }

    // spawn at random spot if in death match
    if (deathmatch) {
      G_DeathMatchSpawnPlayer(player);
      return;
    }

    start = &playerstarts[(player->id - 1) % num_playerstarts];

    if (G_CheckSpot(player, start)) {
      PL_Spawn(player, start);
      return;
    }

    // try to spawn at one of the other players spots
    for (size_t i = 0; i < VANILLA_MAXPLAYERS; i++) {
      mapthing_t *other_start = &playerstarts[i % num_playerstarts];
      if (G_CheckSpot(player, other_start)) {
        PL_Spawn(player, other_start);
        return;
      }
    }

    // They're going to be inside something.  Too bad.
    PL_Spawn(player, start);
  }
  else {
    G_SetGameAction(ga_loadlevel); // reload the level from scratch
  }
}

void G_ExitLevel(void) {
  secretexit = false;
  G_SetGameAction(ga_completed);
}

// Here's for the german edition.
// IF NO WOLF3D LEVELS, NO SECRET EXIT!

void G_SecretExitLevel(void) {
  if (gamemode != commercial || haswolflevels) {
    secretexit = true;
  }
  else {
    secretexit = false;
  }

  G_SetGameAction(ga_completed);
}

//
// G_DoCompleted
//

void G_DoCompleted(void) {
  int par_time = 0;

  G_SetGameAction(ga_nothing);

  PLAYERS_FOR_EACH(iter) {
    G_PlayerFinishLevel(iter.player);// take away cards and stuff
  }

  if (automapmode & am_active) {
    AM_Stop();
  }

  if (gamemode != commercial) {      // kilough 2/7/98
    // Chex Quest ends after 5 levels, rather than 8.
    if (gamemission == chex) {
      if (gamemap == 5) {
        G_SetGameAction(ga_victory);
        return;
      }
    }
    else {
      switch (gamemap) {
        // cph - Remove ExM8 special case, so it gets summary screen displayed
        case 9:
          PLAYERS_FOR_EACH(iter) {
            iter.player->didsecret = true;
          }
          break;
      }
    }
  }

  // next_map is 0 biased, unlike gamemap
  if (gamemode == commercial) {
    if (secretexit) {
      switch (gamemap) {
        case 15:
          next_map = 30;
          break;
        case 31:
          next_map = 31;
          break;
        case 2:
          if (bfgedition && SINGLEPLAYER) {
            next_map = 32;
          }
          break;
        case 4:
          if (gamemission == pack_nerve && SINGLEPLAYER) {
            next_map = 8;
          }
          break;
      }
    }
    else {
      switch (gamemap) {
        case 31:
        case 32:
          next_map = 15;
          break;
        case 33:
          if (bfgedition && SINGLEPLAYER) {
            next_map = 2;
            break;
          }
        default:
          next_map = gamemap;
      }
    }
    if (gamemission == pack_nerve && SINGLEPLAYER && gamemap == 9) {
      next_map = 4;
    }
  }
  else if (secretexit) {
    next_map = 8;  // go to secret level
  }
  else if (gamemap == 9) {
    // returning from secret level
    switch (gameepisode) {
      case 1:
        next_map = 3;
        break;
      case 2:
        next_map = 5;
        break;
      case 3:
        next_map = 6;
        break;
      case 4:
        next_map = 2;
        break;
    }
  }
  else {
    next_map = gamemap;          // go to next level
  }

  if (gamemode == commercial) {
    if (gamemap >= 1 && gamemap <= 34) {
      par_time = TICRATE * cpars[gamemap - 1];
    }
  }
  else if (gameepisode >= 1 && gameepisode <= 4 && gamemap >= 1 &&
    gamemap <= 9) {
    par_time = TICRATE * pars[gameepisode][gamemap];
  }

  /* cph - modified so that only whole seconds are added to the totalleveltimes
   *  value; so our total is compatible with the "naive" total of just adding
   *  the times in seconds shown for each level. Also means our total time
   *  will agree with Compet-n.
   */
  totalleveltimes += (leveltime - leveltime % 35);

  G_SetGameState(GS_INTERMISSION);
  automapmode &= ~am_active;

  if (SERVER) {
    SV_ResyncPeers();
  }

  // lmpwatch.pl engine-side demo testing support
  // print "FINISHED: <mapname>" when the player exits the current map
  if (nodrawers && (demoplayback || timingdemo)) {
    if (gamemode == commercial) {
      D_MsgLocalInfo("FINISHED: MAP%02d\n", gamemap);
    }
    else {
      D_MsgLocalInfo("FINISHED: E%dM%d\n", gameepisode, gamemap);
    }
  }

  e6y_G_DoCompleted();// e6y

  if (demo_compatibility) {
    WI_Start(par_time);
  }
  else if (!X_Call(X_GetState(), "intermission", "start", 1, 0, par_time)) {
    I_Error("Error starting intermission: %s\n", X_GetError(X_GetState()));
  }
}

//
// G_WorldDone
//

void G_WorldDone(void) {
  G_SetGameAction(ga_worlddone);

  if (secretexit) {
    P_GetConsolePlayer()->didsecret = true;
  }

  if (gamemode == commercial && gamemission != pack_nerve) {
    switch (gamemap) {
      case 15:
      case 31:
        if (!secretexit) {
          break;
        }
      case 6:
      case 11:
      case 20:
      case 30:
        F_StartFinale();
        break;
    }
  }
  else if (gamemission == pack_nerve && SINGLEPLAYER && gamemap == 8) {
    F_StartFinale();
  }
  else if (gamemap == 8) {
    // cph - after ExM8 summary screen, show victory stuff
    G_SetGameAction(ga_victory);
  }
}

void G_DoWorldDone(void) {
  idmusnum = -1;      // jff 3/17/98 allow new level's music to be loaded

  if (!CLIENT) {
    G_SetGameState(GS_LEVEL);
  }

  gamemap = next_map + 1;

  if (!CLIENT) {
    G_DoLoadLevel();
  }

  G_SetGameAction(ga_nothing);
  AM_clearMarks();    // jff 4/12/98 clear any marks on the automap
  e6y_G_DoWorldDone();// e6y

  if (CLIENT) {
    CL_Reset();
    CL_ResetSync();
  }
}

// CPhipps - savename variable redundant

/* killough 12/98:
 * This function returns a signature for the current wad.
 * It is used to distinguish between wads, for the purposes
 * of savegame compatibility warnings, and options lookups.
 */

void G_DeferedInitNew(skill_t skill, int episode, int map) {
  d_skill = skill;
  d_episode = episode;
  d_map = map;
  G_SetGameAction(ga_newgame);
}

/* cph -
 * G_Compatibility
 *
 * Initialises the comp[] array based on the compatibility_level
 * For reference, MBF did:
 * for (i=0; i < COMP_TOTAL; i++)
 *   comp[i] = compatibility;
 *
 * Instead, we have a lookup table showing at what version a fix was
 *  introduced, and made optional (replaces comp_options_by_version)
 */

#ifdef DOGS

/* killough 7/19/98: Marine's best friend :) */
static int G_GetHelpers(void) {
  int j = M_CheckParm("-dog");

  if (!j) {
    j = M_CheckParm("-dogs");
  }

  if (!j) {
    return default_dogs;
  }

  if ((j + 1) < myargc) {
    return atoi(myargv[j + 1]);
  }

  return 1;
}

#endif /* ifdef DOGS */

// killough 3/1/98: function to reload all the default parameter
// settings before a new game begins

void G_ReloadDefaults(void) {
  // killough 3/1/98: Initialize options based on config file
  // (allows functions above to load different values for demos
  // and savegames without messing up defaults).

  weapon_recoil = default_weapon_recoil;                // weapon recoil
  player_bobbing = default_player_bobbing;              // whether player bobs
                                                        // or not
  leave_weapons = default_leave_weapons;

  /*
   * cph 2007/06/31 - for some reason, the default_* of the next 2 vars was
   * never implemented
   */
  variable_friction = default_variable_friction;
  allow_pushers     = default_allow_pushers;
  monsters_remember = default_monsters_remember;        // remember former
                                                        // enemies
  monster_infighting = default_monster_infighting;      // killough 7/19/98

#ifdef DOGS
  if (netgame) {
    dogs = 0;
  }
  else {
    dogs = G_GetHelpers();
  }
  dog_jumping = default_dog_jumping;
#endif /* ifdef DOGS */

  distfriend = default_distfriend;                      // killough 8/8/98
  monster_backing = default_monster_backing;            // killough 9/8/98
  monster_avoid_hazards = default_monster_avoid_hazards;// killough 9/9/98
  monster_friction = default_monster_friction;          // killough 10/98
  help_friends = default_help_friends;                  // killough 9/9/98
  monkeys = default_monkeys;

  // jff 1/24/98 reset play mode to command line spec'd version
  // killough 3/1/98: moved to here
  nomonsters  = clnomonsters  = M_CheckParm("-nomonsters");
  respawnparm = clrespawnparm = M_CheckParm("-respawn");
  fastparm    = clfastparm    = M_CheckParm("-fast");

  // jff 3/24/98 set startskill from defaultskill in config file, unless
  // it has already been set by a -skill parameter
  if (startskill == sk_none) {
    startskill = (skill_t)(defaultskill - 1);
  }

  demoplayback = false;
  singledemo = false;                                   // killough 9/29/98:
                                                        // don't stop after 1
                                                        // demo
  netdemo = false;

  // killough 2/21/98:
  // memset(
  //   &playeringame[1],
  //   0,
  //   sizeof(playeringame) - sizeof(playeringame[0])
  // );
  // memset(playeringame + 1, 0, sizeof(*playeringame) * (MAXPLAYERS - 1));

  // consoleplayer = 0;

  compatibility_level = default_compatibility_level;

  {
    int i = M_CheckParm("-complevel");

    if (i && (i + 1) < myargc) {
      int l = atoi(myargv[i + 1]);

      if (l >= -1) {
        compatibility_level = l;
      }
    }
  }

  if (compatibility_level == -1) {
    compatibility_level = best_compatibility;
  }

  if (mbf_features) {
    memcpy(comp, default_comp, sizeof(comp));
  }

  G_Compatibility();

  // killough 3/31/98, 4/5/98: demo sync insurance
  demo_insurance = (default_demo_insurance == 1);

  rngseed += (I_GetRandomTimeSeed() + gametic);         // CPhipps
}

void G_DoNewGame(void) {
  // e6y: allow new level's music to be loaded
  idmusnum = -1;

  G_ReloadDefaults();                                   // killough 3/1/98
  netgame = false;                                      // killough 3/29/98
  deathmatch = false;
  G_InitNew(d_skill, d_episode, d_map);
  G_SetGameAction(ga_nothing);

  // jff 4/26/98 wake up the status bar in case were coming out of a DM demo
  ST_Start();
  walkcamera.mode = camera_mode_disabled;               // e6y
}

// killough 4/10/98: New function to fix bug which caused Doom
// lockups when idclev was used in conjunction with -fast.

void G_SetFastParms(int fast_pending) {
  static int fast = 0;                                  // remembers fast state

  int i;

  if (fast != fast_pending) {                           /* only change if
                                                         * necessary */
    if ((fast = fast_pending)) {
      for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++) {
        if (states[i].tics != 1 || demo_compatibility) {// killough 4/10/98
          states[i].tics >>= 1;                         // don't change 1->0
                                                        // since it causes
                                                        // cycles
        }
      }
      mobjinfo[MT_BRUISERSHOT].speed = 20 * FRACUNIT;
      mobjinfo[MT_HEADSHOT].speed    = 20 * FRACUNIT;
      mobjinfo[MT_TROOPSHOT].speed   = 20 * FRACUNIT;
    }
    else {
      for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++) {
        states[i].tics <<= 1;
      }
      mobjinfo[MT_BRUISERSHOT].speed = 15 * FRACUNIT;
      mobjinfo[MT_HEADSHOT].speed    = 10 * FRACUNIT;
      mobjinfo[MT_TROOPSHOT].speed   = 10 * FRACUNIT;
    }
  }
}

//
// G_InitNew
// Can be called by the startup code or the menu task,
// consoleplayer, displayplayer, playeringame[] should be set.
//

void G_InitNew(skill_t skill, int episode, int map) {
  // e6y
  // This variable is for correct checking for upper limit of episode.
  // Ultimate Doom, Final Doom and Doom95 have
  // "if (episode == 0) episode = 3/4" check instead of
  // "if (episode > 3/4) episode = 3/4"
  bool fake_episode_check = compatibility_level == ultdoom_compatibility ||
    compatibility_level == finaldoom_compatibility;

  if (paused) {
    paused = false;
    S_ResumeSound();
  }

  if (skill > sk_nightmare) {
    skill = sk_nightmare;
  }

  if (episode < 1) {
    episode = 1;
  }

  // e6y: We need to remove the fourth episode for pre-ultimate complevels.
  if (compatibility_level < ultdoom_compatibility && episode > 3) {
    episode = 3;
  }

  // e6y: DosDoom has only this check
  if (compatibility_level == dosdoom_compatibility) {
    if (gamemode == shareware) {
      episode = 1;                        // only start episode 1 on shareware
    }
  }
  else if (gamemode == retail) {
    // e6y: Ability to play any episode with Ultimate Doom,
    // Final Doom or Doom95 compatibility and -warp command line switch
    // E5M1 from 2002ado.wad is an example.
    // Now you can play it with "-warp 5 1 -complevel 3".
    // 'Vanilla' Ultimate Doom executable also allows it.
    if ((fake_episode_check && episode == 0) || episode > 4) {
      episode = 4;
    }
  }
  else if (gamemode == shareware) {
    if (episode > 1) {
      episode = 1;                        // only start episode 1 on shareware
    }
  }
  else if ((fake_episode_check && episode == 0) || episode > 3) {
    // e6y: Ability to play any episode with Ultimate Doom,
    // Final Doom or Doom95 compatibility and -warp command line switch
    episode = 3;
  }

  if (map < 1) {
    map = 1;
  }
  if (map > 9 && gamemode != commercial) {
    map = 9;
  }

  if (fastparm || skill == sk_nightmare) {// killough 4/10/98
    G_SetFastParms(true);
  }
  else {
    G_SetFastParms(false);
  }

  M_ClearRandom();

  if ((skill == sk_nightmare) || respawnparm) {
    respawnmonsters = true;
  }

  // force players to be initialized upon first level load
  PLAYERS_FOR_EACH(iter) {
    iter.player->playerstate = PST_REBORN;
  }

  usergame = true;                        // will be set false if a demo
  paused = false;
  automapmode &= ~am_active;
  gameepisode = episode;
  gamemap = map;
  gameskill = skill;

  totalleveltimes = 0;                    // cph

  // jff 4/16/98 force marks on automap cleared every new level start
  AM_clearMarks();

  G_DoLoadLevel();
}

void G_SetGameState(gamestate_t new_gamestate) {
  gamestate = new_gamestate;
}

void G_SetPrevGameState(gamestate_t new_prevgamestate) {
  prevgamestate = new_prevgamestate;
}

void G_SetOldGameState(gamestate_t new_oldgamestate) {
  oldgamestate = new_oldgamestate;
}

void G_SetWipeGameState(gamestate_t new_wipegamestate) {
  if ((!CLIENT) || (CL_Predicting())) {
    wipegamestate = new_wipegamestate;
  }
}

void G_ResetGameState(void) {
  oldgamestate = gamestate;
  wipegamestate = gamestate;
}

gamestate_t G_GetGameState(void) {
  return gamestate;
}

gameaction_t G_GetGameAction(void) {
  return gameaction;
}

void G_SetGameAction(gameaction_t new_gameaction) {
  if (CLIENT) {
    if (new_gameaction == ga_nothing) {
      gameaction = ga_nothing;
    }
    return;
  }

  gameaction = new_gameaction;

#if 0
  switch (new_gameaction) {
    case ga_nothing:
      puts("ga_nothing");
      break;
    case ga_loadlevel:
      puts("ga_loadlevel");
      break;
    case ga_newgame:
      puts("ga_newgame");
      break;
    case ga_loadgame:
      puts("ga_loadgame");
      break;
    case ga_savegame:
      puts("ga_savegame");
      break;
    case ga_playdemo:
      puts("ga_playdemo");
      break;
    case ga_completed:
      puts("ga_completed");
      break;
    case ga_victory:
      puts("ga_victory");
      break;
    case ga_worlddone:
      puts("ga_worlddone");
      break;
  }
#endif /* if 0 */

  if (SERVER && gameaction != ga_nothing) {
    SV_BroadcastGameActionChange();
  }
}

/* vi: set et ts=2 sw=2: */

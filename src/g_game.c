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
#include "d_event.h"
#include "c_main.h"
#include "d_dump.h"
#include "f_finale.h"
#include "g_state.h"
// #include "i_video.h"
#include "m_argv.h"
#include "m_avg.h"
#include "m_file.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"

#include "r_defs.h"
#include "v_video.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#include "r_defs.h"
#include "p_user.h"
#include "n_main.h"
#include "p_ident.h"
#include "p_setup.h"
#include "p_tick.h"
#include "p_map.h"
#include "p_checksum.h"
#include "d_main.h"
#include "wi_stuff.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "p_map.h"
#include "s_sound.h"
#include "s_advsound.h"
#include "dstrings.h"
#include "sounds.h"
#include "r_patch.h"
#include "r_data.h"
#include "r_sky.h"
#include "d_deh.h"              // Ty 3/27/98 deh declarations
#include "p_inter.h"
#include "g_game.h"
#include "d_net.h"
#include "g_keys.h"
#include "g_save.h"
#include "i_main.h"
#include "i_system.h"
#include "r_demo.h"
#include "r_fps.h"
#include "e6y.h"
#include "cl_main.h"
#include "cl_net.h"
#include "x_main.h"
#include "p_mobj.h"
#include "v_video.h"

extern int forceOldBsp;
extern char *player_names[];
extern bool setsizeneeded;
extern bool doSkip;

#define MAXPLMOVE   (forwardmove[1])
#define TURBOTHRESHOLD  0x32
#define SLOWTURNTICS  6
#define QUICKREVERSE (short)32768 // 180 degree reverse                    // phares

bool        netdemo;

static const unsigned char *demobuffer; /* cph - only used for playback */
static int                  demolength; // check for overrun (missing DEMOMARKER)
static FILE                *demofp;     /* cph - record straight to file */

static int turnheld;       // for accelerative turning

// Set to -1 or +1 to switch to the previous or next weapon.
static int next_weapon = 0;

// Used for prev/next weapon keys.
static const struct {
  weapontype_t weapon;
  weapontype_t weapon_num;
} weapon_order_table[] = {
  { wp_fist,         wp_fist },
  { wp_chainsaw,     wp_fist },
  { wp_pistol,       wp_pistol },
  { wp_shotgun,      wp_shotgun },
  { wp_supershotgun, wp_shotgun },
  { wp_chaingun,     wp_chaingun },
  { wp_missile,      wp_missile },
  { wp_plasma,       wp_plasma },
  { wp_bfg,          wp_bfg }
};

static int mousearray[6];
static int *mousebuttons = &mousearray[1];    // allow [-1]

static unsigned char savegameslot; // Slot to load if gameaction == ga_loadgame

// mouse values are used once
static int   mousex;
static int   mousey;
static int   dclicktime;
static int   dclickstate;
static int   dclicks;
static int   dclicktime2;
static int   dclickstate2;
static int   dclicks2;

// joystick values are repeated
static int   joyxmove;
static int   joyymove;
static bool  joyarray[9];
static bool *joybuttons = &joyarray[1];    // allow [-1]

// Game events info
static buttoncode_t special_event; // Event triggered by local player, to send

static gameaction_t gameaction;

fixed_t forwardmove[2] = {0x19, 0x32};
fixed_t sidemove[2]    = {0x18, 0x28};
fixed_t angleturn[3]   = {640, 1280, 320};  // + slow turn
fixed_t flyspeed[2]    = {1*256, 3*256};

fixed_t forwardmove_normal[2] = {0x19, 0x32};
fixed_t sidemove_normal[2]    = {0x18, 0x28};
fixed_t sidemove_strafe50[2]  = {0x19, 0x32};

// comp_options_by_version removed - see G_Compatibility
#if 0
static unsigned char map_old_comp_levels[] = {
  0, 1, 2, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};
#endif

//e6y static 
const unsigned char *demo_p;
const unsigned char *demo_continue_p = NULL;

// CPhipps - moved *_loadgame vars here
bool            forced_loadgame = false;
bool            command_loadgame = false;
gamestate_t     gamestate = GS_BAD;
gamestate_t     prevgamestate = GS_LEVEL;
// wipegamestate can be set to -1 to force a wipe on the next draw
gamestate_t wipegamestate = GS_DEMOSCREEN;
gamestate_t oldgamestate = GS_BAD;
skill_t         gameskill;
bool            respawnmonsters;
int             gameepisode;
int             gamemap;
bool            paused;
bool            usergame;      // ok to save / end game
bool            timingdemo;    // if true, exit with report on completion
bool            fastdemo;      // if true, run at full speed -- killough
bool            nodrawers;     // for comparative timing purposes
bool            noblit;        // for comparative timing purposes
int             starttime;     // for comparative timing purposes
int             deathmatch;    // only if started as net death
/* [CG] Here we go...
bool            playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];
*/
int             upmove;
int             consoleplayer; // player taking events and displaying
int             displayplayer; // view being displayed
int             gametic;
int             basetic;       /* killough 9/29/98: for demo sync */
int             totalkills, totallive, totalitems, totalsecret;    // for intermission
int             show_alive;
bool            demorecording;
bool            demoplayback;
bool            democontinue = false;
char*           demo_continue_name;
int             demover;
bool            singledemo;           // quit after playing a demo from cmdline
wbstartstruct_t wminfo;               // parms for world map / intermission
bool            haswolflevels = false;// jff 4/18/98 wolf levels present
int             autorun = false;      // always running?          // phares
int             totalleveltimes;      // CPhipps - total time for all completed levels
int             longtics;
int             bytes_per_tic;
time_t          level_start_time;

// e6y
// There is a new command-line switch "-shorttics".
// This makes it possible to practice routes and tricks
// (e.g. glides, where this makes a significant difference)
// with the same mouse behaviour as when recording,
// but without having to be recording every time.
int shorttics;

/*
 * CG: This is set to true when graphics have been initialized; useful for
 *     printf and other routines that would like to fallback to some console
 *     output if the screen is not yet available.
 */
bool graphics_initialized = false;

//jff 3/24/98 define defaultskill here
int defaultskill;               //note 1-based

// killough 2/8/98: make corpse queue variable in size
/*
 * CG 09/25/2014: Use a GQueue for this instead
 *
 * int    bodyqueslot, bodyquesize;        // killough 2/8/98
 * mobj_t **bodyque = 0;                   // phares 8/10/98
 */
int corpse_queue_size; // killough 2/8/98
GQueue *corpse_queue = NULL;

//e6y: save/restore all data which could be changed by G_ReadDemoHeader
static void G_SaveRestoreGameOptions(int save);

//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
static inline signed char fudgef(signed char b) {
/*e6y
  static int c;
  if (!b || !demo_compatibility || longtics) return b;
  if (++c & 0x1f) return b;
  b |= 1; if (b>2) b-=2;*/
  return b;
}

static inline signed short fudgea(signed short b) {
    // e6y
    // There is a new command-line switch "-shorttics".
    // This makes it possible to practice routes and tricks
    // (e.g. glides, where this makes a significant difference)
    // with the same mouse behaviour as when recording,
    // but without having to be recording every time.
/*e6y
  if (!b || !demo_compatibility || !longtics) return b;
  b |= 1; if (b>2) b-=2;*/

  if (shorttics && !demorecording && !demoplayback) {
    return (((b + 128) >> 8) << 8);
  }
  else {
    return b;
  }
}

/* killough 3/22/98: form savegame name in one location
 * (previously code was scattered around in multiple places)
 * cph - Avoid possible buffer overflow problems by passing
 * size to this function and using snprintf */
int G_SaveGameName(char *name, size_t size, int slot, bool demoplayback) {
  const char* sgn;
  
  if (demoplayback) {
    sgn = "demosav";
  }
  else {
    sgn = savegamename;
  }

  return snprintf(name, size, "%s/%s%d.dsg", basesavegame, sgn, slot);
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
  char *name;                // killough 3/22/98
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
    P_Printf(consoleplayer, "Couldn't read file %s: %s\n", name, strerror(errno));
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

  D_Msg(MSG_INFO,
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

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//
void G_SaveGame(int slot, char *description) {
  strcpy(savedescription, description);

  if (demoplayback) {
    savegameslot = slot; // cph - We're doing a user-initiated save game while
    G_DoSaveGame(true);  //       a demo is running so, go outside normal
                         //       mechanisms
  }
  // CPhipps - store info in special_event
  special_event = BT_SPECIAL |
                  (BTS_SAVEGAME & BT_SPECIALMASK) |
                  ((slot << BTS_SAVESHIFT) & BTS_SAVEMASK);
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
    P_Echo(consoleplayer, s_GGSAVED); /* Ty - externalized */
  }
  else {
    P_Echo(consoleplayer, "Game save failed!"); // CPhipps - not externalised
  }

  M_PBufFree(&savebuffer); // killough

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

  D_Msg(MSG_INFO,
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

// killough 3/16/98: add slot info
// killough 5/15/98: add command-line
void G_LoadGame(int slot, bool command) {
  if (!demoplayback && !command) {
    // CPhipps - handle savegame filename in G_DoLoadGame
    //         - Delay load so it can be communicated in net game
    //         - store info in special_event
    special_event = BT_SPECIAL |
                   (BTS_LOADGAME & BT_SPECIALMASK) |
                   ((slot << BTS_SAVESHIFT) & BTS_SAVEMASK);
    forced_loadgame = netgame; // CPhipps - always force load netgames
  }
  else {
    // Do the old thing, immediate load
    G_SetGameAction(ga_loadgame);
    forced_loadgame = false;
    savegameslot = slot;
    demoplayback = false;
    // Don't stay in netgame state if loading single player save
    // while watching multiplayer demo
    netgame = false;
  }
  command_loadgame = command;
  R_SmoothPlaying_Reset(NULL); // e6y
}

// killough 5/15/98:
// Consistency Error when attempting to load savegame.

void G_SetSpeed(void) {
  int p;

  if(movement_strafe50) {
    sidemove[0] = sidemove_strafe50[0];
    sidemove[1] = sidemove_strafe50[1];
  }
  else {
    movement_strafe50onturns = false;
    sidemove[0] = sidemove_normal[0];
    sidemove[1] = sidemove_normal[1];
  }

  if ((p = M_CheckParm ("-turbo"))) {
    int scale = ((p < myargc - 1) ? atoi(myargv[p + 1]) : 200);

    scale = BETWEEN(10, 400, scale);

    D_Msg(MSG_INFO, "turbo scale: %i%%\n", scale);

    forwardmove[0] = forwardmove_normal[0] * scale / 100;
    forwardmove[1] = forwardmove_normal[1] * scale / 100;
    sidemove[0] = sidemove[0] * scale / 100;
    sidemove[1] = sidemove[1] * scale / 100;
  }
}

static bool WeaponSelectable(weapontype_t weapon) {
  if (gamemode == shareware) {
    if (weapon == wp_plasma || weapon == wp_bfg) {
      return false;
    }
  }

  // Can't select the super shotgun in Doom 1.
  if (weapon == wp_supershotgun && gamemission == doom) {
    return false;
  }

  // Can't select a weapon if we don't own it.
  if (!players[consoleplayer].weaponowned[weapon]) {
    return false;
  }

  return true;
}

static int G_NextWeapon(int direction) {
  weapontype_t weapon;
  int i;
  int arrlen;

  // Find index in the table.
  if (players[consoleplayer].pendingweapon == wp_nochange) {
    weapon = players[consoleplayer].readyweapon;
  }
  else {
    weapon = players[consoleplayer].pendingweapon;
  }

  arrlen = sizeof(weapon_order_table) / sizeof(*weapon_order_table);
  for (i = 0; i < arrlen; i++) {
    if (weapon_order_table[i].weapon == weapon) {
      break;
    }
  }

  // Switch weapon.
  do {
    i += direction;
    i = (i + arrlen) % arrlen;
  } while (!WeaponSelectable(weapon_order_table[i].weapon));

  return weapon_order_table[i].weapon_num;
}

void G_BuildTiccmd(ticcmd_t *cmd) {
  int strafe = false;
  int bstrafe;
  int speed = autorun;
  int tspeed;
  int forward;
  int side;
  int newweapon; // phares
  player_t *player = &players[consoleplayer];

  I_StartTic();
  
  if (gamekeydown[key_strafe]) {
    strafe = true;
  }
  else if (mousebuttons[mousebstrafe]) {
    strafe = true;
  }
  else if (joybuttons[joybstrafe]) {
    strafe = true;
  }

  //e6y: the "RUN" key inverts the autorun state

  if (gamekeydown[key_speed]) {
    speed = !autorun;
  }

  if (joybuttons[joybspeed]) {
    speed = !autorun;
  }

  forward = side = 0;

  G_SkipDemoCheck(); //e6y

  if (democontinue) {
    mousex = mousey = 0;
    return;
  }

  // use two stage accelerative turning on the keyboard and joystick
  if (joyxmove < 0 || joyxmove > 0 || gamekeydown[key_right] ||
                                      gamekeydown[key_left]) {
    turnheld++;
  }
  else {
    turnheld = 0;
  }

  if (turnheld < SLOWTURNTICS) {
    tspeed = 2;             // slow turn
  }
  else {
    tspeed = speed;
  }

  // turn 180 degrees in one keystroke?                           // phares
                                                                  //    |
  if (gamekeydown[key_reverse]) {                                 //    V
    cmd->angleturn += QUICKREVERSE;                               //    ^
    gamekeydown[key_reverse] = false;                             //    |
  }                                                               // phares

  // let movement keys cancel each other out

  if (strafe) {
    if (gamekeydown[key_right]) {
      side += sidemove[speed];
    }
    if (gamekeydown[key_left]) {
      side -= sidemove[speed];
    }
    if (joyxmove > 0) {
      side += sidemove[speed];
    }
    if (joyxmove < 0) {
      side -= sidemove[speed];
    }
  }
  else {
    if (gamekeydown[key_right]) {
      cmd->angleturn -= angleturn[tspeed];
    }
    if (gamekeydown[key_left]) {
      cmd->angleturn += angleturn[tspeed];
    }
    if (joyxmove > 0) {
      cmd->angleturn -= angleturn[tspeed];
    }
    if (joyxmove < 0) {
      cmd->angleturn += angleturn[tspeed];
    }
  }

  if (gamekeydown[key_up]) {
    forward += forwardmove[speed];
  }
  if (gamekeydown[key_down]) {
    forward -= forwardmove[speed];
  }
  if (joyymove < 0) {
    forward += forwardmove[speed];
  }
  if (joyymove > 0) {
    forward -= forwardmove[speed];
  }
  if (gamekeydown[key_straferight] || joybuttons[joybstraferight]) {
    side += sidemove[speed];
  }
  if (gamekeydown[key_strafeleft] || joybuttons[joybstrafeleft]) {
    side -= sidemove[speed];
  }

  // buttons
  // cmd->chatchar = HU_dequeueChatChar();

  if (gamekeydown[key_fire]) {
    cmd->buttons |= BT_ATTACK;
  }

  if (mousebuttons[mousebfire]) {
    cmd->buttons |= BT_ATTACK;
  }

  if (gamekeydown[key_use] || mousebuttons[mousebuse] || joybuttons[joybuse]) {
    cmd->buttons |= BT_USE;
    dclicks = 0; // clear double clicks if hit use button
  }

  // Toggle between the top 2 favorite weapons.                   // phares
  // If not currently aiming one of these, switch to              // phares
  // the favorite. Only switch if you possess the weapon.         // phares

  // CG: The above comment is by phares (rofl)

  // killough 3/22/98:
  //
  // Perform automatic weapons switch here rather than in p_pspr.c,
  // except in demo_compatibility mode.
  //
  // killough 3/26/98, 4/2/98: fix autoswitch when no weapons are left

  if ((!demo_compatibility && player->attackdown && // killough
       !P_CheckAmmo(player)) ||
      gamekeydown[key_weapontoggle]) {
    newweapon = P_SwitchWeapon(player);           // phares
  }
  else {                                 // phares 02/26/98: Added gamemode checks
    if (next_weapon) {
      newweapon = G_NextWeapon(next_weapon);
      next_weapon = 0;
    }
    else {
      // killough 5/2/98: reformatted
      // CG: 04/15/2014: re-reformatted
      bool can_check_wp9 = (!demo_compatibility) && gamemode == commercial;

      newweapon = wp_nochange;

      if (gamekeydown[key_weapon1]) {
        newweapon = wp_fist;
      }
      else if (gamekeydown[key_weapon2]) {
        newweapon = wp_pistol;
      }
      else if (gamekeydown[key_weapon3]) {
        newweapon = wp_shotgun;
      }
      else if (gamekeydown[key_weapon4]) {
        newweapon = wp_chaingun;
      }
      else if (gamekeydown[key_weapon5]) {
        newweapon = wp_missile;
      }
      else if (gamekeydown[key_weapon6] && gamemode != shareware) {
        newweapon = wp_plasma;
      }
      else if (gamekeydown[key_weapon7] && gamemode != shareware) {
        newweapon = wp_bfg;
      }
      else if (gamekeydown[key_weapon8]) {
        newweapon = wp_chainsaw;
      }
      else if (can_check_wp9 && gamekeydown[key_weapon9]) {
        newweapon = wp_supershotgun;
      }
    }

    // killough 3/22/98: For network and demo consistency with the
    // new weapons preferences, we must do the weapons switches here
    // instead of in p_user.c. But for old demos we must do it in
    // p_user.c according to the old rules. Therefore demo_compatibility
    // determines where the weapons switch is made.

    // killough 2/8/98:
    // Allow user to switch to fist even if they have chainsaw.
    // Switch to fist or chainsaw based on preferences.
    // Switch to shotgun or SSG based on preferences.

    if (!demo_compatibility) {
      // only select chainsaw from '1' if it's owned, it's
      // not already in use, and the player prefers it or
      // the fist is already in use, or the player does not
      // have the berserker strength.

      if (newweapon == wp_fist && player->weaponowned[wp_chainsaw] &&
                                  player->readyweapon != wp_chainsaw && (
                                  player->readyweapon == wp_fist ||
                                  !player->powers[pw_strength] ||
                                  P_WeaponPreferred(wp_chainsaw, wp_fist))) {
        newweapon = wp_chainsaw;
      }

      // Select SSG from '3' only if it's owned and the player
      // does not have a shotgun, or if the shotgun is already
      // in use, or if the SSG is not already in use and the
      // player prefers it.

      if (newweapon == wp_shotgun && gamemode == commercial &&
                                     player->weaponowned[wp_supershotgun] && (
                                     !player->weaponowned[wp_shotgun] ||
                                     player->readyweapon == wp_shotgun || (
                                     player->readyweapon != wp_supershotgun &&
                                     P_WeaponPreferred(wp_supershotgun, wp_shotgun)))) {
        newweapon = wp_supershotgun;
      }
    }
    // killough 2/8/98, 3/22/98 -- end of weapon selection changes
  }

  if (newweapon != wp_nochange) {
    cmd->buttons |= BT_CHANGE;
    cmd->buttons |= newweapon << BT_WEAPONSHIFT;
  }

  // mouse
  if (mousebuttons[mousebforward]) {
    forward += forwardmove[speed];
  }
  if (mousebuttons[mousebbackward]) {
    forward -= forwardmove[speed];
  }

  if (mouse_doubleclick_as_use) { //e6y
    // forward double click
    if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1) {
      dclickstate = mousebuttons[mousebforward];

      if (dclickstate) {
        dclicks++;
      }

      if (dclicks == 2) {
        cmd->buttons |= BT_USE;
        dclicks = 0;
      }
      else {
        dclicktime = 0;
      }

    }
    else if ((dclicktime++) > 20) {
      dclicks = 0;
      dclickstate = 0;
    }

    // strafe double click

    bstrafe = mousebuttons[mousebstrafe] || joybuttons[joybstrafe];

    if (bstrafe != dclickstate2 && dclicktime2 > 1) {
      dclickstate2 = bstrafe;

      if (dclickstate2) {
        dclicks2++;
      }

      if (dclicks2 == 2) {
        cmd->buttons |= BT_USE;
        dclicks2 = 0;
      }
      else {
        dclicktime2 = 0;
      }
    }
    else if ((dclicktime2++) > 20) {
      dclicks2 = 0;
      dclickstate2 = 0;
    }
  } //e6y: end if (mouse_doubleclick_as_use)

  forward += mousey;

  if (strafe) {
    side += mousex / 4;       /* mead  Don't want to strafe as fast as turns.*/
  }
  else {
    cmd->angleturn -= mousex; /* mead now have enough dynamic range 2-10-00 */
  }

  if (!walkcamera.type || menuactive) { //e6y
    mousex = 0;
    mousey = 0;
    joyxmove = 0;
    joyymove = 0;
  }

#ifdef GL_DOOM
  motion_blur.curr_speed_pow2 = 0;
#endif

  if (forward > MAXPLMOVE) {
    forward = MAXPLMOVE;
  }
  else if (forward < -MAXPLMOVE) {
    forward = -MAXPLMOVE;
  }

  if (side > MAXPLMOVE) {
    side = MAXPLMOVE;
  }
  else if (side < -MAXPLMOVE) {
    side = -MAXPLMOVE;
  }

  //e6y
  if (movement_strafe50) {
    if (!speed) {
      if (side > sidemove_strafe50[0]) {
        side = sidemove_strafe50[0];
      }
      else if (side < -sidemove_strafe50[0]) {
        side = -sidemove_strafe50[0];
      }
    }
    else if (!movement_strafe50onturns && !strafe && cmd->angleturn) {
      if (side > sidemove_normal[1]) {
        side = sidemove_normal[1];
      }
      else if (side < -sidemove_normal[1]) {
        side = -sidemove_normal[1];
      }
    }
  }

  cmd->forwardmove += fudgef((signed char)forward);
  cmd->sidemove += side;
  cmd->angleturn = fudgea(cmd->angleturn);

  upmove = 0;
  if (gamekeydown[key_flyup]) {
    upmove += flyspeed[speed];
  }
  if (gamekeydown[key_flydown]) {
    upmove -= flyspeed[speed];
  }

  // CPhipps - special events (game new/load/save/pause)
  if (special_event & BT_SPECIAL) {
    cmd->buttons = special_event;
    special_event = 0;
  }
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
  int i;

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
  else { //jff 3/27/98 and lets not forget about DOOM and Ultimate DOOM huh?
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
    case 4: // Special Edition sky
      skytexture = R_TextureNumForName("SKY4");
      break;
    } //jff 3/27/98 end sky setting fix
  }

	// [RH] Set up details about sky rendering
	R_InitSkyMap();

#ifdef GL_DOOM
  R_SetBoxSkybox(skytexture);
#endif

  levelstarttic = gametic;        // for time calculation

  if (!demo_compatibility && !mbf_features) { // killough 9/29/98
    basetic = gametic;
  }

  if (wipegamestate == GS_LEVEL) {
    G_SetWipeGameState(-1); // force a wipe
  }

  G_SetGameState(GS_LEVEL);
  level_start_time = time(NULL);

  for (i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i] && players[i].playerstate == PST_DEAD) {
      players[i].playerstate = PST_REBORN;
    }

    memset(players[i].frags, 0, sizeof(players[i].frags));
  }

  // initialize the msecnode_t freelist.                     phares 3/25/98
  // any nodes in the freelist are gone by now, cleared
  // by Z_FreeTags() when the previous level ended or player
  // died.
  P_FreeSecNodeList();

  /* CG 08/10/2014: Reset IDs */
  P_IdentReset();

  wminfo.maxfrags = 0;
  wminfo.partime = 180;

  P_SetupLevel(gameepisode, gamemap, 0, gameskill);

  if (!demoplayback) { // Don't switch views if playing a demo
    displayplayer = consoleplayer;    // view the guy you are playing
  }

  G_SetGameAction(ga_nothing);
  Z_CheckHeap();

  // clear cmd building stuff
  memset(gamekeydown, 0, sizeof(gamekeydown));
  joyxmove = joyymove = 0;
  mousex = mousey = 0;
  mlooky = 0;//e6y
  special_event = 0; paused = false;
  memset(&mousearray, 0, sizeof(mousearray));
  memset(&joyarray, 0, sizeof(joyarray));

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
      gamekeydown[key_spy] = true;
      do {                                        // spy mode
        if (++displayplayer >= MAXPLAYERS) {
          displayplayer = 0;
        }
      } while (!playeringame[displayplayer] && displayplayer != consoleplayer);

      ST_Start(); // killough 3/7/98: switch status bar views too
      HU_Start();
      S_UpdateSounds(players[displayplayer].mo);
      R_ActivateSectorInterpolations();
      R_SmoothPlaying_Reset(NULL);
    }

    return true;
  }

  // any other key pops up menu if in demos
  //
  // killough 8/2/98: enable automap in -timedemo demos
  //
  // killough 9/29/98: make any key pop up menu regardless of
  // which kind of demo, and allow other events during playback

  if (gameaction == ga_nothing && (demoplayback || gamestate == GS_DEMOSCREEN)) {
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

    //e6y
    /*
        return gamestate == GS_DEMOSCREEN &&
    !(paused & 2) && !(automapmode & am_active) &&
    ((ev->type == ev_key && ev->pressed) ||
     (ev->type == ev_mouse && ev->key) ||
     (ev->type == ev_joystick && ev->key)) ?
    M_StartControlPanel(), true : false;
      */
  }

  if (gamestate == GS_FINALE && F_Responder(ev)) {
    return true;  // finale ate the event
  }

  // If the next/previous weapon keys are pressed, set the next_weapon
  // variable to change weapons when the next ticcmd is generated.
  if (ev->type == ev_key && ev->pressed) {
    if (ev->key == key_prevweapon) {
      next_weapon = -1;
    }
    else if (ev->key == key_nextweapon) {
      next_weapon = 1;
    }
  }

  switch (ev->type) {
    case ev_key:
      if (ev->pressed) {
        if (ev->key == key_pause) {         // phares
          special_event = BT_SPECIAL | (BTS_PAUSE & BT_SPECIALMASK);
        }
        else if (ev->key < NUMKEYS) {
          gamekeydown[ev->key] = true;
        }

        /*
         * CG: Don't do this anymore
         *
         * return true;    // eat key down events
         *
         */
      }
      else {
        if (ev->key < NUMKEYS) {
          gamekeydown[ev->key] = false;
        }

        return false;   // always let key up events filter down
      }
    break;
    case ev_mouse:
      if (ev->key == SDL_BUTTON_LEFT) {
        mousebuttons[0] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_MIDDLE) {
        mousebuttons[1] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_RIGHT) {
        mousebuttons[2] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_X1) {
        mousebuttons[3] = ev->pressed;
      }
      else if (ev->key == SDL_BUTTON_X2) {
        mousebuttons[4] = ev->pressed;
      }

      return true;
    break;
    case ev_mouse_movement:
      /*
       * bmead@surfree.com
       * Modified by Barry Mead after adding vastly more resolution
       * to the Mouse Sensitivity Slider in the options menu 1-9-2000
       * Removed the mouseSensitivity "*4" to allow more low end
       * sensitivity resolution especially for lsdoom users.
       */
      //e6y mousex += (ev->data2*(mouseSensitivity_horiz))/10;  /* killough */
      //e6y mousey += (ev->data3*(mouseSensitivity_vert))/10;  /*Mead rm *4 */

      /* killough */
      //e6y
      mousex += (AccelerateMouse(ev->xmove) * (mouseSensitivity_horiz)) / 10;
      if (GetMouseLook()) {
        if (movement_mouseinvert) {
          mlooky += (AccelerateMouse(ev->ymove) * (mouseSensitivity_mlook)) / 10;
        }
        else {
          mlooky -= (AccelerateMouse(ev->ymove) * (mouseSensitivity_mlook)) / 10;
        }
      }
      else {
        mousey += (AccelerateMouse(ev->ymove) * (mouseSensitivity_vert)) / 40;
      }

      return true;    // eat events
    break;
    case ev_joystick:
      if (ev->key >= 0 && ev->key <= 7) {
        joybuttons[ev->key] = ev->pressed;
      }

      return true;
    break;
    case ev_joystick_movement:
      if (ev->jstype == ev_joystick_axis) {
        if (ev->key == 0) {
          joyxmove = ev->value;
        }
        else if (ev->key == 1) {
          joyymove = ev->value;
        }
      }
      else if (ev->jstype == ev_joystick_ball) {
        joyxmove = ev->xmove;
        joyymove = ev->ymove;
      }
      else if (ev->jstype == ev_joystick_hat) {
        if (ev->value == SDL_HAT_CENTERED) {
          joyxmove = 0;
          joyymove = 0;
        }
        else if (ev->value == SDL_HAT_UP) {
          joyymove = 1;
        }
        else if (ev->value == SDL_HAT_RIGHT) {
          joyxmove = 1;
        }
        else if (ev->value == SDL_HAT_DOWN) {
          joyymove = -1;
        }
        else if (ev->value == SDL_HAT_LEFT) {
          joyxmove = -1;
        }
        else if (ev->value == SDL_HAT_RIGHTUP) {
          joyxmove = 1;
          joyymove = 1;
        }
        else if (ev->value == SDL_HAT_RIGHTDOWN) {
          joyxmove = 1;
          joyymove = -1;
        }
        else if (ev->value == SDL_HAT_LEFTUP) {
          joyxmove = -1;
          joyymove = 1;
        }
        else if (ev->value == SDL_HAT_LEFTDOWN) {
          joyxmove = -1;
          joyymove = -1;
        }
      }
      return true;    // eat events
    break;
    default:
    break;
  }

  return false;
}

static void read_one_tick(ticcmd_t* cmd, const unsigned char **data_p) {
  unsigned char at = 0; // e6y: for tasdoom demo format

  cmd->forwardmove = (signed char)(*(*data_p)++);
  cmd->sidemove = (signed char)(*(*data_p)++);

  if (!longtics) {
    cmd->angleturn = ((unsigned char)(at = *(*data_p)++)) << 8;
  }
  else {
    unsigned int lowbyte = (unsigned char)(*(*data_p)++);

    cmd->angleturn = (((signed int)(*(*data_p)++)) << 8) + lowbyte;
  }

  cmd->buttons = (unsigned char)(*(*data_p)++);
  
  // e6y: ability to play tasdoom demos directly
  if (compatibility_level == tasdoom_compatibility) {
    signed char tmp = cmd->forwardmove;

    cmd->forwardmove = cmd->sidemove;
    cmd->sidemove = (signed char)at;
    cmd->angleturn = ((unsigned char)cmd->buttons) << 8;
    cmd->buttons = (unsigned char)tmp;
  }
}

static void read_demo_ticcmd(ticcmd_t *cmd) {
  demo_curr_tic++;

  if (*demo_p == DEMOMARKER) {
    G_CheckDemoStatus();      // end of demo data stream
  }
  else if (demoplayback && demo_p + bytes_per_tic > demobuffer + demolength) {
    D_Msg(MSG_WARN, "read_demo_ticcmd: missing DEMOMARKER\n");
    G_CheckDemoStatus();
  }
  else {
    read_one_tick(cmd, &demo_p);
  }
}

static void read_demo_continue_ticcmd(ticcmd_t* cmd) {
  if (!demo_continue_p) {
    return;
  }

  if (gametic <= demo_tics_count && 
    demo_continue_p + bytes_per_tic <= demobuffer + demolength &&
    *demo_continue_p != DEMOMARKER) {
    read_one_tick(cmd, &demo_continue_p);
  }

  if (gametic >= demo_tics_count ||
    demo_continue_p > demobuffer + demolength ||
    gamekeydown[key_demo_jointogame] || joybuttons[joybuse]) {
    demo_continue_p = NULL;
    democontinue = false;
  }
}

/* Demo limits removed -- killough
 * cph - record straight to file
 */
static void write_demo_ticcmd(ticcmd_t *cmd) {
  char buf[5];
  char *p = buf;

  if (compatibility_level == tasdoom_compatibility) {
    *p++ = cmd->buttons;
    *p++ = cmd->forwardmove;
    *p++ = cmd->sidemove;
    *p++ = (cmd->angleturn + 128) >> 8;
  }
  else {
    *p++ = cmd->forwardmove;
    *p++ = cmd->sidemove;

    if (!longtics) {
      *p++ = (cmd->angleturn + 128) >> 8;
    }
    else {
      signed short a = cmd->angleturn;

      *p++ = a & 0xff;
      *p++ = (a >> 8) & 0xff;
    }
    *p++ = cmd->buttons;
  } //e6y

  if (fwrite(buf, p-buf, 1, demofp) != 1)
    I_Error("write_demo_ticcmd: error writing demo");

  /* cph - alias demo_p to it so we can read it back */
  demo_p = (const unsigned char *)buf;
  read_demo_ticcmd(cmd);         // make SURE it is exactly the same
}

//
// G_Ticker
// Make ticcmd_ts for the players.
//

void G_Ticker(void) {
  int i;

#if 0
  // CPhipps - player colour changing
  {
    int index = consoleplayer % VANILLA_MAXPLAYERS;

    if (!demoplayback && vanilla_mapplayer_colors[index] != mapcolor_me) {
      if (CLIENT) // Changed my multiplayer colour - Inform the whole game
        CL_SendColorIndexChange(mapcolor_me);

      G_ChangedPlayerColour(consoleplayer, mapcolor_me);
    }
  }
#endif

  P_MapStart("G_Ticker");

  // do player reborns if needed
  for (i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i] && players[i].playerstate == PST_REBORN) {
      G_DoReborn(i);
    }

    if (players[i].playerstate == PST_DISCONNECTED) {
      C_Printf("Player %d disconnected\n", i);

      if (players[i].mo) {
        mobj_t *actor = players[i].mo;
        fixed_t x     = actor->x;
        fixed_t y     = actor->y;
        fixed_t z     = actor->z;

        P_RemoveMobj(players[i].mo);
        players[i].mo = NULL;
        P_SpawnMobj(x, y, z, MT_TFOG);
        S_StartSound(actor, sfx_telept);
      }

      playeringame[i] = false;
      players[i].playerstate = PST_LIVE;
    }
  }

  P_MapEnd();

  // do things to change the game state
  while (gameaction != ga_nothing) {
    switch (gameaction) {
      case ga_loadlevel:
        // force players to be initialized on level reload
        for (i = 0; i < MAXPLAYERS; i++) {
          players[i].playerstate = PST_REBORN;
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
    for (i = 0; i < MAXPLAYERS; i++) {
      ticcmd_t *cmd = NULL;

      if (!playeringame[i]) {
        continue;
      }

      cmd = &players[i].cmd;

      //e6y
      if (demoplayback) {
        memset(cmd, 0, sizeof(ticcmd_t));
        read_demo_ticcmd(cmd);
      }
      else if (democontinue) {
        memset(cmd, 0, sizeof(ticcmd_t));
        read_demo_continue_ticcmd(cmd);
      }

      if (demorecording) {
        write_demo_ticcmd(cmd);
      }

      // check for turbo cheats
      // killough 2/14/98, 2/20/98 -- only warn in netgames and demos

      if ((netgame || demoplayback) &&
          cmd->forwardmove > TURBOTHRESHOLD &&
          !(gametic & 31) &&
          ((gametic >> 5) & 3) == i) {

        /* cph - don't use sprintf, use doom_printf */
        P_Printf(consoleplayer, "%s is turbo!\n", player_names[i]);
      }
    }

    // check for special buttons
    for (i = 0; i < MAXPLAYERS; i++) {
      if (!playeringame[i]) {
        continue;
      }

      if (players[i].cmd.buttons & BT_SPECIAL) {
        switch (players[i].cmd.buttons & BT_SPECIALMASK) {
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
              (players[i].cmd.buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
            G_SetGameAction(ga_savegame);
          break;
          case BTS_LOADGAME: // CPhipps - remote loadgame request
            savegameslot =
              (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT;
            G_SetGameAction(ga_loadgame);
            forced_loadgame = netgame; // Force if a netgame
            command_loadgame = false;
          break;
          case BTS_RESTARTLEVEL: // CPhipps - Restart the level
            if (demoplayback ||
                (compatibility_level < lxdoom_1_compatibility)) {
              break; // CPhipps - Ignore in demos or old games
            }

            G_SetGameAction(ga_loadlevel);
            break;
        }
        players[i].cmd.buttons = 0;
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
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (playeringame[i]) {
        P_IgnorePlayerCommands(i);
      }
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
    // The border may need redrawing next time if the border surrounds the screen,
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
#else
  if (redrawborderstuff) {
    R_DrawViewBorder();
  }
#endif

  // e6y
  // Boom colormaps should be applied for everything in R_RenderPlayerView
  use_boom_cm = true;

  R_InterpolateView(&players[displayplayer]);

  R_ClearStats();

  // Now do the drawing
  if (viewactive || map_always_updates) {
    R_RenderPlayerView(&players[displayplayer]);
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
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//

static void G_PlayerFinishLevel(int player) {
  player_t *p = &players[player];

  memset(p->powers, 0, sizeof(p->powers));
  memset(p->cards, 0, sizeof(p->cards));

  p->mo = NULL;           // cph - this is allocated PU_LEVEL so it's gone
  p->extralight = 0;      // cancel gun flashes
  p->fixedcolormap = 0;   // cancel ir gogles
  p->damagecount = 0;     // no palette changes
  p->bonuscount = 0;

  P_ClearPlayerCommands(player);
  p->cmdq.commands_missed = 0;
  p->cmdq.command_limit = 0;
  p->cmdq.commands_run_this_tic = 0;
  p->telefragged_by_spawn = false;
  p->cmdq.latest_command_run_index = 0;

  if (deathmatch) {
    p->playerstate = PST_REBORN;
  }
}

// CPhipps - G_SetPlayerColour
// Player colours stuff
//
// G_SetPlayerColour

void G_ChangedPlayerColour(int pn, int cl) {
  if (!netgame) {
    return;
  }

  vanilla_mapplayer_colors[pn % VANILLA_MAXPLAYERS] = cl;

  // Rebuild colour translation tables accordingly
  R_InitTranslationTables();

  // Change translations on existing player mobj's
  for (int i = 0; i < MAXPLAYERS; i++) {
    if ((gamestate == GS_LEVEL) && playeringame[i] && (players[i].mo != NULL)) {
      players[i].mo->flags &= ~MF_TRANSLATION;
      players[i].mo->flags |= ((uint64_t)playernumtotrans[i]) << MF_TRANSSHIFT;
    }
  }
}

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//

void G_PlayerReborn(int playernum) {
  player_t *p = &players[playernum];

  if (!p->telefragged_by_spawn) {
    p->armorpoints = 0;
    p->armortype = 0;
    memset(p->powers, 0, sizeof(int) * NUMPOWERS);
    memset(p->cards, 0,  sizeof(bool) * NUMCARDS);
    p->backpack = 0;
    memset(p->weaponowned, 0, sizeof(int) * NUMWEAPONS);
    memset(p->ammo, 0, sizeof(int) * NUMAMMO);
    memset(p->maxammo, 0, sizeof(int) * NUMAMMO);
    p->didsecret = 0;
    p->ammo[am_clip] = initial_bullets; // Ty 03/12/98 - use dehacked values

    for (int i = 0; i < NUMAMMO; i++) {
      p->maxammo[i] = maxammo[i];
    }
  }

  p->mo = NULL;
  p->playerstate = PST_LIVE;
  memset(&p->cmd, 0, sizeof(ticcmd_t));
  p->viewz = 0;
  p->viewheight = 0;
  p->deltaviewheight = 0;
  p->bob = 0;
  p->refire = 0;
  p->damagecount = 0;
  p->bonuscount = 0;
  p->attacker = NULL;
  p->extralight = 0;
  p->fixedcolormap = 0;
  p->colormap = 0;
  memset(p->psprites, 0, sizeof(p->psprites));
  p->momx = 0;
  p->momy = 0;
  p->prev_viewz = 0;
  p->prev_viewangle = 0;
  p->prev_viewpitch = 0;
  p->jumpTics = 0;

  p->usedown = p->attackdown = true;  // don't do anything immediately
  p->readyweapon = p->pendingweapon = wp_pistol;
  p->weaponowned[wp_fist] = true;
  p->weaponowned[wp_pistol] = true;

  /*
   * [CG] Yeah, this lets you get all your health back by having a player die
   *      then telefrag you with their spawn.  If you really want to cheat that
   *      way, OK.
   */
  p->health = initial_health; // Ty 03/12/98 - use dehacked values

  P_ClearPlayerCommands(playernum);
  p->cmdq.commands_missed = 0;
  p->cmdq.command_limit = 0;
  p->cmdq.commands_run_this_tic = 0;
  p->cmdq.latest_command_run_index = 0;

  p->telefragged_by_spawn = false;
}

void G_ClearCorpses(void) {
  /*
   * CG: This is only called when loading a new state or map, therefore the
   *     queued corpses don't leak.  This is why the call to P_RemoveMobj or
   *     free is omitted.
   */
  if (corpse_queue_size < 0) {
    I_Error("clear_corpses: corpse_queue_size < 0 (%d)", corpse_queue_size);
  }

  if (corpse_queue) {
    g_queue_free(corpse_queue);
    corpse_queue = NULL;
  }

  if (corpse_queue_size > 0) {
    corpse_queue = g_queue_new();
  }
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//

static bool G_CheckSpot(int playernum, mapthing_t *mthing) {
  fixed_t       x = mthing->x << FRACBITS;
  fixed_t       y = mthing->y << FRACBITS;
  fixed_t      xa;
  fixed_t      ya;
  subsector_t *ss;
  int           i;
  int          an;
  mobj_t      *mo; 

  if (!players[playernum].mo) {
    // first spawn of level, before corpses
    for (i = 0; i < playernum; i++) {
      if (!players[i].mo) {
        continue;
      }

      if (players[i].mo->x == x && players[i].mo->y == y) {
        return false;
      }
    }
    return true;
  }

  // killough 4/2/98: fix bug where P_CheckPosition() uses a non-solid
  // corpse to detect collisions with other players in DM starts
  //
  // Old code:
  // if (!P_CheckPosition (players[playernum].mo, x, y))
  //    return false;

  players[playernum].mo->flags |= MF_SOLID;
  i = P_CheckPosition(players[playernum].mo, x, y);
  players[playernum].mo->flags &= ~MF_SOLID;

  if (!i) {
    return false;
  }

  // flush an old corpse if needed
  // killough 2/8/98: make corpse queue have an adjustable limit
  // killough 8/1/98: Fix bugs causing strange crashes
  if (corpse_queue_size < 0) {
    I_Error("G_CheckSpot: corpse_queue_size < 0 (%d)", corpse_queue_size);
  }

  if (corpse_queue_size == 0) {
    P_RemoveMobj(players[playernum].mo);
  }
  else {
    if (!corpse_queue) {
      corpse_queue = g_queue_new();
    }

    if (g_queue_get_length(corpse_queue) == corpse_queue_size) {
      P_RemoveMobj(g_queue_pop_head(corpse_queue));
    }

    g_queue_push_tail(corpse_queue, players[playernum].mo);
  }

  // spawn a teleport fog
  ss = R_PointInSubsector(x, y);

  // Teleport fog at respawn point

  /* BUG: an can end up negative, because mthing->angle is (signed) short.
   * We have to emulate original Doom's behaviour, deferencing past the start
   * of the array, into the previous array (finetangent) */

  /*
  an = ( ANG45 * ((signed)mthing->angle/45) ) >> ANGLETOFINESHIFT;
  */

  /*
   * [CG] Shifting these signed values is implementation-defined, and it turns
   *      out that clang will (in some configurations) mess this up.  We can
   *      use this lookup here because spawnpoints only have 8 potential
   *      angles.
   */

  switch (mthing->angle) {
    case 45:
      an = 1024;
      break;
    case 90:
      an = 2048;
      break;
    case 135:
      an = 3072;
      break;
    case 180:
      an = -4096;
      break;
    case 225:
      an = -3072;
      break;
    case 270:
      an = -2048;
      break;
    case 315:
      an = -1024;
      break;
    case 360:
    case 0:
      an = 0;
      break;
    default:
      an = 0;
      I_Error("Unexpected angle %d\n", mthing->angle);
      break;
  }

  xa = finecosine[an];
  ya = finesine[an];

  if (compatibility_level <= finaldoom_compatibility ||
      compatibility_level == prboom_4_compatibility) {
    switch (an) {
      case -4096:
        xa = finetangent[2048]; // finecosine[-4096]
        ya = finetangent[0];              // finesine[-4096]
      break;
      case -3072:
        xa = finetangent[3072]; // finecosine[-3072]
        ya = finetangent[1024];           // finesine[-3072]
      break;
      case -2048:
        xa = finesine[0];       // finecosine[-2048]
        ya = finetangent[2048];           // finesine[-2048]
      break;
      case -1024:
        xa = finesine[1024];    // finecosine[-1024]
        ya = finetangent[3072];           // finesine[-1024]
      break;
      case 1024:
      case 2048:
      case 3072:
      case 4096:
      case 0:
      break; /* correct angles set above */
      default:
        I_Error("(%d) G_CheckSpot: unexpected angle %d (%d)\n",
          gametic, an, mthing->angle
        );
        break;
    }
  }

  mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya, ss->sector->floorheight, MT_TFOG);

  if (players[consoleplayer].viewz != 1) {
    S_StartSound(mo, sfx_telept);  // don't start sound on first frame
  }

  return true;
}


// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
void G_DeathMatchSpawnPlayer(int playernum) {
  int selections = deathmatch_p - deathmatchstarts;

  /*
  if (selections < MAXPLAYERS) {
    I_Error("G_DeathMatchSpawnPlayer: Only %i deathmatch spots, %d required",
      selections, MAXPLAYERS
    );
  }
  */

  for (int j = 0; j < 20; j++) {
    int i = P_Random(pr_dmspawn) % selections;

    if (G_CheckSpot(playernum, &deathmatchstarts[i])) {
      deathmatchstarts[i].type = playernum + 1;
      P_SpawnPlayer(playernum, &deathmatchstarts[i % num_deathmatchstarts]);
      return;
    }
  }

  // no good spot, so the player will probably get stuck
  P_SpawnPlayer(playernum, &playerstarts[playernum % num_playerstarts]);
  if (MULTINET) {
    P_StompSpawnPointBlockers(players[playernum].mo);
  }
}

//
// G_DoReborn
//
void G_DoReborn(int playernum) {
  if (netgame) {
    // respawn at the start

    // first dissasociate the corpse
    if (players[playernum].mo) {
      players[playernum].mo->player = NULL;
    }

    // spawn at random spot if in death match
    if (deathmatch) {
      G_DeathMatchSpawnPlayer(playernum);
      return;
    }

    if (G_CheckSpot(playernum, &playerstarts[playernum % num_playerstarts])) {
      P_SpawnPlayer(playernum, &playerstarts[playernum % num_playerstarts]);
      return;
    }

    // try to spawn at one of the other players spots
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (G_CheckSpot(playernum, &playerstarts[i % num_playerstarts])) {
        P_SpawnPlayer(playernum, &playerstarts[i % num_playerstarts]);
        return;
      }
      // They're going to be inside something.  Too bad.
    }
    P_SpawnPlayer(playernum, &playerstarts[playernum % num_playerstarts]);
  }
  else {
    G_SetGameAction(ga_loadlevel); // reload the level from scratch
  }
}

// DOOM Par Times
int pars[5][10] = {
  {0},
  {0,30,75,120,90,165,180,180,30,165},
  {0,90,90,90,120,90,360,240,30,170},
  {0,90,45,90,150,90,90,165,30,135},
  // from Doom 3 BFG Edition
  {0,165,255,135,150,180,390,135,360,180}
};

// DOOM II Par Times
int cpars[34] = {
  30,90,120,120,90,150,120,120,270,90,  //  1-10
  210,150,150,150,210,150,420,150,210,150,  // 11-20
  240,150,180,150,150,300,330,420,300,180,  // 21-30
  120,30,30,30          // 31-34
};

static bool secretexit;

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
  int i;

  G_SetGameAction(ga_nothing);

  for (i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      G_PlayerFinishLevel(i);        // take away cards and stuff
    }
  }

  if (automapmode & am_active) {
    AM_Stop();
  }

  if (gamemode != commercial) { // kilough 2/7/98
    // Chex Quest ends after 5 levels, rather than 8.
    if (gamemission == chex) {
      if (gamemap == 5) {
        G_SetGameAction(ga_victory);
        return;
      }
    }
    else {
      switch(gamemap) {
      // cph - Remove ExM8 special case, so it gets summary screen displayed
        case 9:
          for (i = 0; i < MAXPLAYERS; i++)
            players[i].didsecret = true;
        break;
      }
    }
  }

  wminfo.didsecret = players[consoleplayer].didsecret;
  wminfo.epsd = gameepisode -1;
  wminfo.last = gamemap -1;

  // wminfo.next is 0 biased, unlike gamemap
  if (gamemode == commercial) {
    if (secretexit) {
      switch(gamemap) {
        case 15:
          wminfo.next = 30;
        break;
        case 31:
          wminfo.next = 31;
        break;
        case 2:
          if (bfgedition && SINGLEPLAYER)
            wminfo.next = 32;
        break;
        case 4:
          if (gamemission == pack_nerve && SINGLEPLAYER)
            wminfo.next = 8;
        break;
      }
    }
    else {
      switch (gamemap) {
        case 31:
        case 32:
          wminfo.next = 15;
        break;
        case 33:
          if (bfgedition && SINGLEPLAYER) {
            wminfo.next = 2;
            break;
          }
        default:
          wminfo.next = gamemap;
      }
    }
    if (gamemission == pack_nerve && SINGLEPLAYER && gamemap == 9) {
      wminfo.next = 4;
    }
  }
  else if (secretexit) {
    wminfo.next = 8;  // go to secret level
  }
  else if (gamemap == 9) {
    // returning from secret level
    switch (gameepisode) {
      case 1:
        wminfo.next = 3;
      break;
      case 2:
        wminfo.next = 5;
      break;
      case 3:
        wminfo.next = 6;
      break;
      case 4:
        wminfo.next = 2;
      break;
    }
  }
  else {
    wminfo.next = gamemap;          // go to next level
  }

  wminfo.maxkills = totalkills;
  wminfo.maxitems = totalitems;
  wminfo.maxsecret = totalsecret;
  wminfo.maxfrags = 0;

  if (gamemode == commercial) {
    if (gamemap >= 1 && gamemap <= 34) {
      wminfo.partime = TICRATE * cpars[gamemap - 1];
    }
  }
  else if (gameepisode >= 1 && gameepisode <= 4 && gamemap >= 1 &&
                                                   gamemap <= 9) {
    wminfo.partime = TICRATE*pars[gameepisode][gamemap];
  }

  wminfo.pnum = consoleplayer;

  for (i = 0; i < MAXPLAYERS; i++) {
    wminfo.plyr[i].in = playeringame[i];
    wminfo.plyr[i].skills = players[i].killcount;
    wminfo.plyr[i].sitems = players[i].itemcount;
    wminfo.plyr[i].ssecret = players[i].secretcount;
    wminfo.plyr[i].stime = leveltime;
    memcpy(
      wminfo.plyr[i].frags, players[i].frags, sizeof(wminfo.plyr[i].frags)
    );
  }

  /* cph - modified so that only whole seconds are added to the totalleveltimes
   *  value; so our total is compatible with the "naive" total of just adding
   *  the times in seconds shown for each level. Also means our total time
   *  will agree with Compet-n.
   */
  wminfo.totaltimes = (totalleveltimes += (leveltime - leveltime % 35));

  G_SetGameState(GS_INTERMISSION);
  automapmode &= ~am_active;

  if (SERVER) {
    SV_ResyncPeers();
  }

  // lmpwatch.pl engine-side demo testing support
  // print "FINISHED: <mapname>" when the player exits the current map
  if (nodrawers && (demoplayback || timingdemo)) {
    if (gamemode == commercial) {
      D_Msg(MSG_INFO, "FINISHED: MAP%02d\n", gamemap);
    }
    else {
      D_Msg(MSG_INFO, "FINISHED: E%dM%d\n", gameepisode, gamemap);
    }
  }

  e6y_G_DoCompleted();//e6y

  WI_Start(&wminfo);
}

//
// G_WorldDone
//

void G_WorldDone(void) {
  G_SetGameAction(ga_worlddone);

  if (secretexit) {
    players[consoleplayer].didsecret = true;
  }

  if (gamemode == commercial && gamemission != pack_nerve) {
    switch (gamemap) {
      case 15:
      case 31:
        if (!secretexit)
          break;
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
  idmusnum = -1;             //jff 3/17/98 allow new level's music to be loaded
  if (!CLIENT) {
    G_SetGameState(GS_LEVEL);
  }
  gamemap = wminfo.next + 1;
  if (!CLIENT) {
    G_DoLoadLevel();
  }
  G_SetGameAction(ga_nothing);
  AM_clearMarks();           //jff 4/12/98 clear any marks on the automap
  e6y_G_DoWorldDone();//e6y
  if (CLIENT) {
    CL_Reset();
    CL_ResetSync();
  }
}

//CPhipps - savename variable redundant

/* killough 12/98:
 * This function returns a signature for the current wad.
 * It is used to distinguish between wads, for the purposes
 * of savegame compatibility warnings, and options lookups.
 */

const char * comp_lev_str[MAX_COMPATIBILITY_LEVEL] = {
  "Doom v1.2",
  "Doom v1.666",
  "Doom/Doom2 v1.9",
  "Ultimate Doom/Doom95",
  "Final Doom",
  "early DosDoom",
  "TASDoom",
  "\"boom compatibility\"",
  "boom v2.01",
  "boom v2.02",
  "lxdoom v1.3.2+",
  "MBF",
  "PrBoom 2.03beta",
  "PrBoom v2.1.0-2.1.1",
  "PrBoom v2.1.2-v2.2.6",
  "PrBoom v2.3.x",
  "PrBoom 2.4.0",
  "Current PrBoom"
};

static skill_t d_skill;
static int     d_episode;
static int     d_map;

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

void G_Compatibility(void) {
  static const struct {
    complevel_t fix; // level at which fix/change was introduced
    complevel_t opt; // level at which fix/change was made optional
  } levels[] = {
    // comp_telefrag - monsters used to telefrag only on MAP30, now they do it for spawners only
    { mbf_compatibility, mbf_compatibility },
    // comp_dropoff - MBF encourages things to drop off of overhangs
    { mbf_compatibility, mbf_compatibility },
    // comp_vile - original Doom archville bugs like ghosts
    { boom_compatibility, mbf_compatibility },
    // comp_pain - original Doom limits Pain Elementals from spawning too many skulls
    { boom_compatibility, mbf_compatibility },
    // comp_skull - original Doom let skulls be spit through walls by Pain Elementals
    { boom_compatibility, mbf_compatibility },
    // comp_blazing - original Doom duplicated blazing door sound
    { boom_compatibility, mbf_compatibility },
    // e6y: "Tagged doors don't trigger special lighting" handled wrong
    // http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
    // comp_doorlight - MBF made door lighting changes more gradual
    { boom_compatibility, mbf_compatibility },
    // comp_model - improvements to the game physics
    { boom_compatibility, mbf_compatibility },
    // comp_god - fixes to God mode
    { boom_compatibility, mbf_compatibility },
    // comp_falloff - MBF encourages things to drop off of overhangs
    { mbf_compatibility, mbf_compatibility },
    // comp_floors - fixes for moving floors bugs
    { boom_compatibility_compatibility, mbf_compatibility },
    // comp_skymap
    { mbf_compatibility, mbf_compatibility },
    // comp_pursuit - MBF AI change, limited pursuit?
    { mbf_compatibility, mbf_compatibility },
    // comp_doorstuck - monsters stuck in doors fix
    { boom_202_compatibility, mbf_compatibility },
    // comp_staylift - MBF AI change, monsters try to stay on lifts
    { mbf_compatibility, mbf_compatibility },
    // comp_zombie - prevent dead players triggering stuff
    { lxdoom_1_compatibility, mbf_compatibility },
    // comp_stairs - see p_floor.c
    { boom_202_compatibility, mbf_compatibility },
    // comp_infcheat - FIXME
    { mbf_compatibility, mbf_compatibility },
    // comp_zerotags - allow zero tags in wads */
    { boom_compatibility, mbf_compatibility },
    // comp_moveblock - enables keygrab and mancubi shots going thru walls
    { lxdoom_1_compatibility, prboom_2_compatibility },
    // comp_respawn - objects which aren't on the map at game start respawn at (0,0)
    { prboom_2_compatibility, prboom_2_compatibility },
    // comp_sound - see s_sound.c
    { boom_compatibility_compatibility, prboom_3_compatibility },
    // comp_666 - emulate pre-Ultimate BossDeath behaviour
    { ultdoom_compatibility, prboom_4_compatibility },
    // comp_soul - enables lost souls bouncing (see P_ZMovement)
    { prboom_4_compatibility, prboom_4_compatibility },
    // comp_maskedanim - 2s mid textures don't animate
    { doom_1666_compatibility, prboom_4_compatibility },
    //e6y
    // comp_ouchface - Use Doom's buggy "Ouch" face code
    { prboom_1_compatibility, prboom_6_compatibility },
    // comp_maxhealth - Max Health in DEH applies only to potions
    { boom_compatibility_compatibility, prboom_6_compatibility },
    // comp_translucency - No predefined translucency for some things
    { boom_compatibility_compatibility, prboom_6_compatibility },
  };
  unsigned int i;

  if (sizeof(levels) / sizeof(*levels) != COMP_NUM) {
    I_Error("G_Compatibility: consistency error");
  }

  for (i = 0; i < sizeof(levels) / sizeof(*levels); i++) {
    if (compatibility_level < levels[i].opt) {
      comp[i] = (compatibility_level < levels[i].fix);
    }
  }

  e6y_G_Compatibility();//e6y

  if (!mbf_features) {
    monster_infighting = 1;
    monster_backing = 0;
    monster_avoid_hazards = 0;
    monster_friction = 0;
    help_friends = 0;

#ifdef DOGS
    dogs = 0;
    dog_jumping = 0;
#endif

    monkeys = 0;
  }
}

#ifdef DOGS
/* killough 7/19/98: Marine's best friend :) */
static int G_GetHelpers(void) {
  int j = M_CheckParm ("-dog");

  if (!j) {
    j = M_CheckParm ("-dogs");
  }

  if (!j) {
    return default_dogs;
  }

  if ((j + 1) < myargc) {
    return atoi(myargv[j + 1]);
  }

  return 1;
}
#endif

// killough 3/1/98: function to reload all the default parameter
// settings before a new game begins

void G_ReloadDefaults(void) {
  // killough 3/1/98: Initialize options based on config file
  // (allows functions above to load different values for demos
  // and savegames without messing up defaults).

  weapon_recoil = default_weapon_recoil;    // weapon recoil
  player_bobbing = default_player_bobbing;  // whether player bobs or not
  leave_weapons = default_leave_weapons;

  /*
   * cph 2007/06/31 - for some reason, the default_* of the next 2 vars was
   * never implemented
   */
  variable_friction = default_variable_friction;
  allow_pushers     = default_allow_pushers;
  monsters_remember = default_monsters_remember;   // remember former enemies
  monster_infighting = default_monster_infighting; // killough 7/19/98

#ifdef DOGS
  if (netgame) {
    dogs = 0;
  }
  else {
    dogs = G_GetHelpers();
  }
  dog_jumping = default_dog_jumping;
#endif

  distfriend = default_distfriend;                       // killough 8/8/98
  monster_backing = default_monster_backing;             // killough 9/8/98
  monster_avoid_hazards = default_monster_avoid_hazards; // killough 9/9/98
  monster_friction = default_monster_friction;           // killough 10/98
  help_friends = default_help_friends;                   // killough 9/9/98
  monkeys = default_monkeys;

  // jff 1/24/98 reset play mode to command line spec'd version
  // killough 3/1/98: moved to here
  nomonsters  = clnomonsters  = M_CheckParm("-nomonsters");
  respawnparm = clrespawnparm = M_CheckParm("-respawn");
  fastparm    = clfastparm    = M_CheckParm("-fast");

  //jff 3/24/98 set startskill from defaultskill in config file, unless
  // it has already been set by a -skill parameter
  if (startskill == sk_none) {
    startskill = (skill_t)(defaultskill - 1);
  }

  demoplayback = false;
  singledemo = false; // killough 9/29/98: don't stop after 1 demo
  netdemo = false;

  // killough 2/21/98:
  memset(&playeringame[1], 0, sizeof(playeringame) - sizeof(playeringame[0]));
  // memset(playeringame + 1, 0, sizeof(*playeringame) * (MAXPLAYERS - 1));

  consoleplayer = 0;

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

  rngseed += (I_GetRandomTimeSeed() + gametic); // CPhipps
}

void G_DoNewGame(void) {
  // e6y: allow new level's music to be loaded
  idmusnum = -1;

  G_ReloadDefaults();            // killough 3/1/98
  netgame = false;               // killough 3/29/98
  deathmatch = false;
  G_InitNew(d_skill, d_episode, d_map);
  G_SetGameAction(ga_nothing);

  //jff 4/26/98 wake up the status bar in case were coming out of a DM demo
  ST_Start();
  walkcamera.type = 0; //e6y
}

// killough 4/10/98: New function to fix bug which caused Doom
// lockups when idclev was used in conjunction with -fast.

void G_SetFastParms(int fast_pending) {
  static int fast = 0;            // remembers fast state

  int i;

  if (fast != fast_pending) {     /* only change if necessary */
    if ((fast = fast_pending)) {
      for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++) {
        if (states[i].tics != 1 || demo_compatibility) { // killough 4/10/98
          states[i].tics >>= 1;  // don't change 1->0 since it causes cycles
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
  int i;

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

  //e6y: We need to remove the fourth episode for pre-ultimate complevels.
  if (compatibility_level < ultdoom_compatibility && episode > 3) {
    episode = 3;
  }

  //e6y: DosDoom has only this check
  if (compatibility_level == dosdoom_compatibility) {
    if (gamemode == shareware) {
      episode = 1; // only start episode 1 on shareware
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
      episode = 1; // only start episode 1 on shareware
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

  if (fastparm || skill == sk_nightmare) { // killough 4/10/98
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
  for (i = 0; i < MAXPLAYERS; i++) {
    players[i].playerstate = PST_REBORN;
  }

  usergame = true;                // will be set false if a demo
  paused = false;
  automapmode &= ~am_active;
  gameepisode = episode;
  gamemap = map;
  gameskill = skill;

  totalleveltimes = 0; // cph

  //jff 4/16/98 force marks on automap cleared every new level start
  AM_clearMarks();

  G_DoLoadLevel();
}

//
// DEMO RECORDING
//

//
// G_RecordDemo
//

void G_RecordDemo(const char* name) {
  char *demoname = M_AddDefaultExtension(name, "lmp"); // 1/18/98 killough

  usergame = false;
  demorecording = true;
  
  /* cph - Record demos straight to file
  * If file already exists, try to continue existing demo
  */

  demofp = NULL;
  if (access(demoname, F_OK) || democontinue ||
      (demo_compatibility && demo_overwriteexisting)) {
    if (strlen(demoname) > 4
        && !strcasecmp(demoname + strlen(demoname) - 4, ".wad")) {
      I_Error(
        "G_RecordDemo: Cowardly refusing to record over what appears to be a "
        "WAD. (%s)",
        demoname
      );
    }

    demofp = fopen(demoname, "wb");
  }
  else {
    if (demo_compatibility && !demo_overwriteexisting) {
      I_Error("G_RecordDemo: file %s already exists", name);
    }

    demofp = fopen(demoname, "rb+");
    if (demofp) {
      int slot = -1;
      const unsigned char* pos;
      unsigned char buf[200];
      size_t len;

      //e6y: save all data which can be changed by G_ReadDemoHeader
      G_SaveRestoreGameOptions(true);

      /* Read the demo header for options etc */
      len = fread(buf, 1, sizeof(buf), demofp);
      pos = G_ReadDemoHeader(buf, len);
      if (pos) {
        int rc;
        int bytes_per_tic;
        
        if (longtics) {
          bytes_per_tic = 5;
        }
        else {
          bytes_per_tic = 4;
        }

        fseek(demofp, pos - buf, SEEK_SET);

        /* Now read the demo to find the last save slot */
        do {
          unsigned char buf[5];

          rc = fread(buf, 1, bytes_per_tic, demofp);

          if (buf[0] == DEMOMARKER || rc < bytes_per_tic - 1) {
            break;
          }

          if (buf[bytes_per_tic-1] & BT_SPECIAL) {
            if ((buf[bytes_per_tic - 1] & BT_SPECIALMASK) == BTS_SAVEGAME) {
              slot = (buf[bytes_per_tic - 1] & BTS_SAVEMASK) >> BTS_SAVESHIFT;
            }
          }
        } while (rc == bytes_per_tic);

        if (slot != -1) {
          /* Return to the last save position, and load the relevant savegame */
          fseek(demofp, -rc, SEEK_CUR);
          G_LoadGame(slot, false);
          autostart = false;
          return;
        }
      }

      //demo cannot be continued
      fclose(demofp);
      if (demo_overwriteexisting) {
        //restoration of all data which could be changed by G_ReadDemoHeader
        G_SaveRestoreGameOptions(false);
        demofp = fopen(demoname, "wb");
      }
      else {
        I_Error("G_RecordDemo: No save in demo, can't continue");
      }
    }
  }

  if (!demofp) {
    I_Error("G_RecordDemo: failed to open %s", name);
  }

  free(demoname);
}

// These functions are used to read and write game-specific options in demos
// and savegames so that demo sync is preserved and savegame restoration is
// complete. Not all options (for example "compatibility"), however, should
// be loaded and saved here. It is extremely important to use the same
// positions as before for the variables, so if one becomes obsolete, the
// byte(s) should still be skipped over or padded with 0's.
// Lee Killough 3/1/98

void G_WriteOptions(unsigned char game_options[]) {
  int i = 0;

  memset(game_options, 0, GAME_OPTION_SIZE * sizeof(unsigned char));

  game_options[i++] = monsters_remember; // part of monster AI
  game_options[i++] = variable_friction; // ice & mud
  game_options[i++] = weapon_recoil;     // weapon recoil
  game_options[i++] = allow_pushers;     // MT_PUSH Things

  game_options[i++] = leave_weapons;

  game_options[i++] = player_bobbing;    // whether player bobs or not

  // killough 3/6/98: add parameters to savegame, move around some in demos
  game_options[i++] = respawnparm;
  game_options[i++] = fastparm;
  game_options[i++] = nomonsters;

  game_options[i++] = demo_insurance;    // killough 3/31/98

  // killough 3/26/98: Added rngseed. 3/31/98: moved here
  game_options[i++] = (unsigned char)((rngseed >> 24) & 0xff);
  game_options[i++] = (unsigned char)((rngseed >> 16) & 0xff);
  game_options[i++] = (unsigned char)((rngseed >>  8) & 0xff);
  game_options[i++] = (unsigned char)( rngseed        & 0xff);

  // Options new to v2.03 begin here
  if (mbf_features) {
    game_options[i++] = monster_infighting;

#ifdef DOGS
    game_options[i++] = dogs;
#else
    i++;
#endif

    i += 2;

    game_options[i++] = (distfriend >> 8) & 0xff;  // killough 8/8/98
    game_options[i++] =  distfriend       & 0xff;

    game_options[i++] = monster_backing;       // killough 9/8/98
    game_options[i++] = monster_avoid_hazards; // killough 9/9/98
    game_options[i++] = monster_friction;      // killough 10/98
    game_options[i++] = help_friends;          // killough 9/9/98

#ifdef DOGS
    game_options[i++] = dog_jumping; // killough 10/98
#else
    i++;
#endif

    game_options[i++] = monkeys;

    // killough 10/98: a compatibility vector now
    for (int o = 0; o < COMP_TOTAL; o++) {
      game_options[i + o] = (comp[o] != 0);
    }

    i += COMP_TOTAL;

    // cph 2002/07/20
    if ((compatibility_level >= prboom_2_compatibility) && forceOldBsp) {
      game_options[i++] = 1;
    }
    else {
      game_options[i++] = 0;
    }
  }
}

/* Same, but read instead of write
 * cph - const byte*'s
 */

void G_ReadOptions(unsigned char game_options[]) {
  int i = 0;

  monsters_remember = game_options[i++];
  variable_friction = game_options[i++]; // ice & mud
  weapon_recoil = game_options[i++];     // weapon recoil
  allow_pushers = game_options[i++];     // MT_PUSH Things

  leave_weapons = game_options[i++];

  player_bobbing = game_options[i++];    // Whether player bobs or not
  // killough 3/6/98: add parameters to savegame, move from demo
  respawnparm = game_options[i++];
  fastparm = game_options[i++];
  nomonsters = game_options[i++];
  demo_insurance = game_options[i++]; // killough 3/31/98
  rngseed = ((game_options[i] & 0xFF) << 24) +
            ((game_options[i + 1] & 0xFF) << 16) +
            ((game_options[i + 2] & 0xFF) <<  8) +
            ((game_options[i + 3] & 0xFF));

  i += 4;

  // Options new to v2.03
  if (mbf_features) {
    monster_infighting = game_options[i++]; // killough 7/19/98

#ifdef DOGS
    dogs = game_options[i++]; // killough 7/19/98
#else
    dogs = 0;
    i++;
#endif

    i += 2;

    distfriend = ((game_options[i] & 0xFF) << 8) +
                 ((game_options[i + 1] & 0xFF)); // killough 8/8/98
    i += 2;
    monster_backing = game_options[i++]; // killough 9/8/98
    monster_avoid_hazards = game_options[i++]; // killough 9/9/98
    monster_friction = game_options[i++]; // killough 10/98
    help_friends = game_options[i++]; // killough 9/9/98

#ifdef DOGS
    dog_jumping = game_options[i++]; // killough 10/98
#else
    dog_jumping = 0;
    i++;
#endif

    monkeys = game_options[i++];

    for (int o = 0; o < COMP_TOTAL; o++) {
      comp[o] = game_options[i + o];
    }

    i += COMP_TOTAL;

    forceOldBsp = game_options[i++]; // cph 2002/07/20
  }

  // G_Compatibility();
}

void G_BeginRecording(void) {
  int i;
  unsigned char game_options[GAME_OPTION_SIZE];
  unsigned char *demostart, *demo_p;
  size_t bytes_written;

  demostart = demo_p = malloc(1000);
  longtics = 0;

  /* cph - 3 demo record formats supported: MBF+, BOOM, and Doom v1.9 */
  if (mbf_features) {
    unsigned char v = 0;

    switch(compatibility_level) {
      case mbf_compatibility:
        v = 203;
      break; // e6y: Bug in MBF compatibility mode fixed
      case prboom_2_compatibility:
        v = 210;
      break;
      case prboom_3_compatibility:
        v = 211;
      break;
      case prboom_4_compatibility:
        v = 212;
      break;
      case prboom_5_compatibility:
        v = 213;
      break;
      case prboom_6_compatibility:
			  v = 214; 
			  longtics = 1;
      break;
      default:
        I_Error("G_BeginRecording: PrBoom compatibility level unrecognised");
    }

    *demo_p++ = v; /* Write version code into demo */

    // signature
    *demo_p++ = 0x1d;
    *demo_p++ = 'M';
    *demo_p++ = 'B';
    *demo_p++ = 'F';
    *demo_p++ = 0xe6;
    *demo_p++ = '\0';

    /* killough 2/22/98: save compatibility flag in new demos
     * cph - FIXME? MBF demos will always be not in compat. mode */
    *demo_p++ = 0;

    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = consoleplayer;

    G_WriteOptions(game_options); // killough 3/1/98: Save game options
    for (i = 0; i < GAME_OPTION_SIZE; i++) {
      *demo_p++ = game_options[i];
    }

    for (i = 0; i < MAXPLAYERS; i++) {
      *demo_p++ = playeringame[i];
    }

    // killough 2/28/98:
    // We always store at least MIN_MAXPLAYERS bytes in demo, to
    // support enhancements later w/o losing demo compatibility

    for (; i < MIN_MAXPLAYERS; i++) {
      *demo_p++ = 0;
    }

  }
  /* [FIXME] e6y
   * else if (compatibility_level >= boom_compatibility_compatibility) {
   */
  else if (compatibility_level > boom_compatibility_compatibility) {
    unsigned char v = 0;
    unsigned char c = 0; /* Nominally, version and compatibility bits */

    switch (compatibility_level) {
      case boom_compatibility_compatibility:
        v = 202;
        c = 1;
      break;
      case boom_201_compatibility:
        v = 201;
        c = 0;
      break;
      case boom_202_compatibility:
        v = 202;
        c = 0;
      break;
      default:
        I_Error("G_BeginRecording: Boom compatibility level unrecognised");
    }

    *demo_p++ = v;

    // signature
    *demo_p++ = 0x1d;
    *demo_p++ = 'B';
    *demo_p++ = 'o';
    *demo_p++ = 'o';
    *demo_p++ = 'm';
    *demo_p++ = 0xe6;

    /* CPhipps - save compatibility level in demos */
    *demo_p++ = c;

    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = consoleplayer;

    G_WriteOptions(game_options); // killough 3/1/98: Save game options
    for (i = 0; i < GAME_OPTION_SIZE; i++) {
      *demo_p++ = game_options[i];
    }

    for (i = 0; i < MAXPLAYERS; i++) {
      *demo_p++ = playeringame[i];
    }

    // killough 2/28/98:
    // We always store at least MIN_MAXPLAYERS bytes in demo, to
    // support enhancements later w/o losing demo compatibility

    for (; i < MIN_MAXPLAYERS; i++) {
      *demo_p++ = 0;
    }
  }
  else { // cph - write old v1.9 demos (might even sync)
    unsigned char v = 109;

    longtics = M_CheckParm("-longtics");

    if (longtics) {
      v = 111;
    }
    else {
      switch (compatibility_level) {
        case doom_1666_compatibility:
          v = 106;
        break;
        case tasdoom_compatibility:
          v = 110;
        break;
      }
    }
    *demo_p++ = v;
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = respawnparm;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;

    for (i = 0; i < 4; i++) { // intentionally hard-coded 4 -- killough
      *demo_p++ = playeringame[i];
    }
  }

  bytes_written = fwrite(demostart, 1, demo_p - demostart, demofp);
  
  if (bytes_written != (size_t)(demo_p - demostart)) {
    I_Error("G_BeginRecording: Error writing demo header");
  }

  free(demostart);
}

//
// G_PlayDemo
//

static const char *defdemoname;

void G_DeferedPlayDemo(const char* name) {
  defdemoname = name;
  G_SetGameAction(ga_playdemo);
}

static int demolumpnum = -1;

static int G_GetOriginalDoomCompatLevel(int ver) {
  int lev;
  int i = M_CheckParm("-complevel");

  if (i && (i + 1 < myargc)) {
    lev = atoi(myargv[i + 1]);

    if (lev >= 0) {
      return lev;
    }
  }

  if (ver == 110) {
    return tasdoom_compatibility;
  }

  if (ver < 107) {
    return doom_1666_compatibility;
  }

  if (gamemode == retail) {
    return ultdoom_compatibility;
  }

  if (gamemission == pack_tnt || gamemission == pack_plut) {
    return finaldoom_compatibility;
  }

  return doom2_19_compatibility;
}

//e6y: Check for overrun
static bool CheckForOverrun(const unsigned char *start_p,
                            const unsigned char *current_p,
                            size_t maxsize, size_t size,
                            bool failonerror) {
  size_t pos = current_p - start_p;

  if (pos + size > maxsize) {
    if (failonerror) {
      I_Error("G_ReadDemoHeader: wrong demo header\n");
    }

    return true;
  }

  return false;
}

// e6y
// save/restore all data which could be changed by G_ReadDemoHeader
void G_SaveRestoreGameOptions(int save) {
  typedef struct gameoption_s {
    int type;
    int value_int;
    int *value_p;
  } gameoption_t;

  static gameoption_t gameoptions[] = {
    {1, 0, &demover},
    {1, 0, (int*)&compatibility_level},
    {1, 0, &basetic},
    {3, 0, (int*)&rngseed},

    {1, 0, (int*)&gameskill},
    {1, 0, &gameepisode},
    {1, 0, &gamemap},

    {2, 0, (int*)&deathmatch},
    {2, 0, (int*)&respawnparm},
    {2, 0, (int*)&fastparm},
    {2, 0, (int*)&nomonsters},
    {1, 0, &consoleplayer},
    {2, 0, (int*)&netgame},
    {2, 0, (int*)&netdemo},

    {1, 0, &longtics},
    {1, 0, &monsters_remember},
    {1, 0, &variable_friction},
    {1, 0, &weapon_recoil},
    {1, 0, &allow_pushers},
    {1, 0, &player_bobbing},
    {1, 0, &demo_insurance},
    {1, 0, &monster_infighting},
#ifdef DOGS
    {1, 0, &dogs},
#endif
    {1, 0, &distfriend},
    {1, 0, &monster_backing},
    {1, 0, &monster_avoid_hazards},
    {1, 0, &monster_friction},
    {1, 0, &help_friends},
#ifdef DOGS
    {1, 0, &dog_jumping},
#endif
    {1, 0, &monkeys},
  
    {2, 0, &forceOldBsp},
    {-1, -1, NULL}
  };

  static bool was_saved_once = false;
  static bool playeringame_o[MAXPLAYERS];
  static bool playerscheats_o[MAXPLAYERS];
  static int  comp_o[COMP_TOTAL];

  int i = 0;

  if (save) {
    was_saved_once = true;
  }
  else {
    if (!was_saved_once) {
      I_Error("G_SaveRestoreGameOptions: Trying to restore unsaved data");
    }
  }

  while (gameoptions[i].value_p) {
    switch (gameoptions[i].type) {
      case 1: //int
      case 2: //bool
      case 3: //unsigned long
        if (save)
          gameoptions[i].value_int = *gameoptions[i].value_p;
        else
          *gameoptions[i].value_p = gameoptions[i].value_int;
      break;
      default: // unrecognised type
        I_Error("G_SaveRestoreGameOptions: Unrecognised type of option");
      break;
    }

    i++;
  }

  for (i = 0; i < MAXPLAYERS; i++) {
    if (save) {
      playeringame_o[i] = playeringame[i];
      playerscheats_o[i] = players[i].cheats;
    }
    else {
      playeringame[i] = playeringame_o[i];
      players[i].cheats = playerscheats_o[i];
    }
  }

  for (i = 0; i < COMP_TOTAL; i++) {
    if (save) {
      comp_o[i] = comp[i];
    }
    else {
      comp[i] = comp_o[i];
    }
  }
}

const unsigned char* G_ReadDemoHeader(const unsigned char *demo_p, size_t size) {
  return G_ReadDemoHeaderEx(demo_p, size, 0);
}

const unsigned char* G_ReadDemoHeaderEx(const unsigned char *demo_p, size_t size,
                                        unsigned int params) {
  skill_t skill;
  int i, episode, map;
  unsigned char game_options[GAME_OPTION_SIZE];

  // e6y
  // The local variable should be used instead of demobuffer,
  // because demobuffer can be uninitialized
  const unsigned char *header_p = demo_p;
  bool failonerror = (params & RDH_SAFE);

  basetic = gametic;  // killough 9/29/98

  // killough 2/22/98, 2/28/98: autodetect old demos and act accordingly.
  // Old demos turn on demo_compatibility => compatibility; new demos load
  // compatibility flag, and other flags as well, as a part of the demo.

  //e6y: check for overrun
  if (CheckForOverrun(header_p, demo_p, size, 1, failonerror)) {
    return NULL;
  }

  demover = *demo_p++;
  longtics = 0;

  // e6y
  // Handling of unrecognized demo formats
  // Versions up to 1.2 use a 7-byte header - first byte is a skill level.
  // Versions after 1.2 use a 13-byte header - first byte is a demoversion.
  // BOOM's demoversion starts from 200
  if (!((demover >=   0  && demover <=   4) ||
        (demover >= 104  && demover <= 111) ||
        (demover >= 200  && demover <= 214))) {
    I_Error("G_ReadDemoHeader: Unknown demo format %d.", demover);
  }

  if (demover < 200) {   // Autodetect old demos
    if (demover >= 111) {
      longtics = 1;
    }

    // killough 3/2/98: force these variables to be 0 in demo_compatibility

    variable_friction = 0;
    weapon_recoil = 0;
    allow_pushers = 0;
    monster_infighting = 1;           // killough 7/19/98
#ifdef DOGS
    dogs = 0;                         // killough 7/19/98
    dog_jumping = 0;                  // killough 10/98
#endif
    monster_backing = 0;              // killough 9/8/98
    monster_avoid_hazards = 0;        // killough 9/9/98
    monster_friction = 0;             // killough 10/98
    help_friends = 0;                 // killough 9/9/98
    monkeys = 0;

    // killough 3/6/98: rearrange to fix savegame bugs (moved fastparm,
    // respawnparm, nomonsters flags to G_LoadOptions()/G_SaveOptions())

    skill = demover;

    if (demover >= 100) {                 // For demos from versions >= 1.4
      //e6y: check for overrun
      if (CheckForOverrun(header_p, demo_p, size, 8, failonerror)) {
        return NULL;
      }

      compatibility_level = G_GetOriginalDoomCompatLevel(demover);
      skill = *demo_p++;
      episode = *demo_p++;
      map = *demo_p++;
      deathmatch = *demo_p++;
      respawnparm = *demo_p++;
      fastparm = *demo_p++;
      nomonsters = *demo_p++;
      consoleplayer = *demo_p++;
    }
    else {
      //e6y: check for overrun
      if (CheckForOverrun(header_p, demo_p, size, 2, failonerror)) {
        return NULL;
      }

      compatibility_level = doom_12_compatibility;
      episode = *demo_p++;
      map = *demo_p++;
      deathmatch = 0;
      respawnparm = 0;
      fastparm = 0;
      nomonsters = 0;
      consoleplayer = 0;
      
      // e6y
      // Ability to force -nomonsters and -respawn for playback of 1.2 demos.
      // Demos recorded with Doom.exe 1.2 did not contain any information
      // about whether these parameters had been used. In order to play them
      // back, you should add them to the command-line for playback.
      // There is no more desynch on mesh.lmp @ mesh.wad
      // prboom -iwad doom.wad -file mesh.wad -playdemo mesh.lmp -nomonsters
      // http://www.doomworld.com/idgames/index.php?id=13976
      respawnparm = M_CheckParm("-respawn");
      fastparm = M_CheckParm("-fast");
      nomonsters = M_CheckParm("-nomonsters");

      // e6y: detection of more unsupported demo formats
      if (*(header_p + size - 1) == DEMOMARKER) {
        // file size test;
        // DOOM_old and HERETIC don't use maps>9;
        // 2 at 4,6 means playerclass=mage -> not DOOM_old or HERETIC;
        if ((size >= 8 && (size - 8) % 4 != 0) ||
            (map > 9) ||
            (size >= 6 && (*(header_p + 4) == 2 || *(header_p + 6) == 2))) {
          I_Error("Unrecognised demo format.");
        }
      }

    }

    G_Compatibility();
  }
  else {  // new versions of demos
    demo_p += 6;               // skip signature;

    switch (demover) {
      case 200: /* BOOM */
      case 201:
        //e6y: check for overrun
        if (CheckForOverrun(header_p, demo_p, size, 1, failonerror)) {
          return NULL;
        }

        if (!*demo_p++) {
          compatibility_level = boom_201_compatibility;
        }
        else {
          compatibility_level = boom_compatibility_compatibility;
        }
      break;
      case 202:
        //e6y: check for overrun
        if (CheckForOverrun(header_p, demo_p, size, 1, failonerror)) {
          return NULL;
        }

        if (!*demo_p++) {
          compatibility_level = boom_202_compatibility;
        }
        else {
          compatibility_level = boom_compatibility_compatibility;
        }
      break;
      case 203:
      /* LxDoom or MBF - determine from signature
       * cph - load compatibility level */
      switch (*(header_p + 2)) {
        case 'B': /* LxDoom */
          /* cph - DEMOSYNC - LxDoom demos recorded in compatibility modes
           * support dropped */
          compatibility_level = lxdoom_1_compatibility;
          break;
        case 'M':
          compatibility_level = mbf_compatibility;
          demo_p++;
          break;
      }
      break;
      case 210:
        compatibility_level = prboom_2_compatibility;
        demo_p++;
      break;
      case 211:
        compatibility_level = prboom_3_compatibility;
        demo_p++;
      break;
      case 212:
        compatibility_level = prboom_4_compatibility;
        demo_p++;
      break;
      case 213:
        compatibility_level = prboom_5_compatibility;
        demo_p++;
      break;
      case 214:
        compatibility_level = prboom_6_compatibility;
        longtics = 1;
        demo_p++;
      break;
    }
    //e6y: check for overrun
    if (CheckForOverrun(header_p, demo_p, size, 5, failonerror))
      return NULL;

    skill = *demo_p++;
    episode = *demo_p++;
    map = *demo_p++;
    deathmatch = *demo_p++;
    consoleplayer = *demo_p++;

    //e6y: check for overrun
    if (CheckForOverrun(header_p,
                        demo_p,
                        size,
                        GAME_OPTION_SIZE,
                        failonerror)) {
      return NULL;
    }

    for (i = 0; i < GAME_OPTION_SIZE; i++) {
      game_options[i] = *demo_p++;
    }

    G_ReadOptions(game_options);  // killough 3/1/98: Read game options

    if (demover == 200) {            // killough 6/3/98: partially fix v2.00 demos
      demo_p += 256 - GAME_OPTION_SIZE;
    }
  }

  if (sizeof(comp_lev_str) / sizeof(comp_lev_str[0]) != MAX_COMPATIBILITY_LEVEL) {
    I_Error("G_ReadDemoHeader: compatibility level strings incomplete");
  }

  D_Msg(MSG_INFO, "G_DoPlayDemo: playing demo with %s compatibility\n",
    comp_lev_str[compatibility_level]
  );

  //e6y
  // only 4 players can exist in old demos
  if (demo_compatibility || demover < 200) {
    //e6y: check for overrun
    if (CheckForOverrun(header_p, demo_p, size, 4, failonerror)) {
      return NULL;
    }

    /*
    for (i = 0; i < 4; i++) { // intentionally hard-coded 4 -- killough
    */

    /*
     * [CG] VANILLA_MAXPLAYERS is now 4, which is the original intent of the
     * hardcoded 4.
     */
    for (i = 0; i < VANILLA_MAXPLAYERS; i++) {
      playeringame[i] = *demo_p++;
    }

    for (; i < MAXPLAYERS; i++) {
      playeringame[i] = 0;
    }
  }
  else {
    //e6y: check for overrun
    if (CheckForOverrun(header_p, demo_p, size, MAXPLAYERS, failonerror)) {
      return NULL;
    }

    for (i = 0; i < MAXPLAYERS; i++) {
      playeringame[i] = *demo_p++;
    }

    demo_p += MIN_MAXPLAYERS - MAXPLAYERS;
  }

  if (playeringame[1]) {
    netgame = true;
    netdemo = true;
  }

  if (!(params & RDH_SKIP_HEADER)) {
    if (gameaction != ga_loadgame) { /* killough 12/98: support -loadgame */
      G_InitNew(skill, episode, map);
    }
  }

  for (i = 0; i < MAXPLAYERS; i++) { // killough 4/24/98
    players[i].cheats = 0;
  }

  // e6y
  // additional params
  {
    const unsigned char *p = demo_p;

    if (longtics) {
      bytes_per_tic = 5;
    }
    else {
      bytes_per_tic = 4;
    }

    demo_playerscount = 0;
    demo_tics_count = 0;
    demo_curr_tic = 0;
    strcpy(demo_len_st, "-");

    for (i = 0; i < MAXPLAYERS; i++) {
      if (playeringame[i]) {
        demo_playerscount++;
      }
    }

    if (demo_playerscount > 0 && demolength > 0) {
      do {
        demo_tics_count++;
        p += bytes_per_tic;
      } while ((p < demobuffer + demolength) && (*p != DEMOMARKER));

      demo_tics_count /= demo_playerscount;

      sprintf(demo_len_st, "\x1b\x35/%d:%02d", 
        demo_tics_count / TICRATE / 60, 
        (demo_tics_count % (60 * TICRATE)) / TICRATE
      );
    }
  }

  return demo_p;
}

void G_DoPlayDemo(void) {
  if (LoadDemo(defdemoname, &demobuffer, &demolength, &demolumpnum)) {
    demo_p = G_ReadDemoHeaderEx(demobuffer, demolength, RDH_SAFE);

    G_SetGameAction(ga_nothing);
    usergame = false;

    demoplayback = true;
    R_SmoothPlaying_Reset(NULL); // e6y
  }
  else {
    // e6y
    // Do not exit if corresponding demo lump is not found.
    // It makes sense for Plutonia and TNT IWADs, which have no DEMO4 lump,
    // but DEMO4 should be in a demo cycle as real Plutonia and TNT have.
    //
    // Plutonia/Tnt executables exit with "W_GetNumForName: DEMO4 not found"
    // message after playing of DEMO3, because DEMO4 is not present
    // in the corresponding IWADs.
    usergame = false;
    D_StartTitle();                // Start the title screen
    G_SetGameState(GS_DEMOSCREEN); // And set the game state accordingly
  }
}

/* G_CheckDemoStatus
 *
 * Called after a death or level completion to allow demos to be cleaned up
 * Returns true if a new demo loop action will take place
 */
bool G_CheckDemoStatus(void) {
  //e6y
  if (doSkip && (demo_stoponend || demo_stoponnext)) {
    G_SkipDemoStop();
  }

  P_ChecksumFinal();

  if (demorecording) {
    demorecording = false;
    fputc(DEMOMARKER, demofp);
    
    //e6y
    G_WriteDemoFooter(demofp);

    D_Msg(MSG_INFO, "G_CheckDemoStatus: Demo recorded\n");

    return false;  // killough
  }

  if (timingdemo) {
    int endtime = I_GetTime_RealTime ();
    // killough -- added fps information and made it work for longer demos:
    unsigned realtics = endtime-starttime;

    M_SaveDefaults();

    I_Error("Timed %u gametics in %u realtics = %-.1f frames per second",
      (unsigned) gametic, realtics,
      (unsigned) gametic * (double) TICRATE / realtics
    );
  }

  if (demoplayback) {
    if (singledemo) {
      exit(0);  // killough
    }

    if (demolumpnum != -1) {
      // cph - unlock the demo lump
      W_UnlockLumpNum(demolumpnum);
      demolumpnum = -1;
    }
    G_ReloadDefaults();    // killough 3/1/98
    netgame = false;       // killough 3/29/98
    deathmatch = false;
    D_AdvanceDemo();
    return true;
  }

  return false;
}

//e6y
void P_WalkTicker() {
  int strafe;
  int speed;
  int tspeed;
  int turnheld;
  int forward;
  int side;
  int angturn;
  subsector_t *subsec = NULL;

  if (!walkcamera.type || menuactive) {
    return;
  }

  strafe = gamekeydown[key_strafe]    ||
           mousebuttons[mousebstrafe] ||
           joybuttons[joybstrafe];
  speed = autorun || gamekeydown[key_speed] || joybuttons[joybspeed]; // phares

  forward = side = 0;
  angturn = 0;
  turnheld = 0;

    // use two stage accelerative turning
    // on the keyboard and joystick
  if (joyxmove < 0 || joyxmove > 0 || gamekeydown[key_right] ||
                                      gamekeydown[key_left]) {
    turnheld++;
  }
  else {
    turnheld = 0;
  }

  if (turnheld < SLOWTURNTICS) {
    tspeed = 0;             // slow turn
  }
  else {
    tspeed = speed;                                                             // phares
  }

  // let movement keys cancel each other out

  if (strafe) {
    if (gamekeydown[key_right]) {
      side += sidemove[speed];
    }
    if (gamekeydown[key_left]) {
      side -= sidemove[speed];
    }
    if (joyxmove > 0) {
      side += sidemove[speed];
    }
    if (joyxmove < 0) {
      side -= sidemove[speed];
    }
  }
  else {
    if (gamekeydown[key_right]) {
      angturn -= angleturn[tspeed];
    }
    if (gamekeydown[key_left]) {
      angturn += angleturn[tspeed];
    }
    if (joyxmove > 0) {
      angturn -= angleturn[tspeed];
    }
    if (joyxmove < 0) {
      angturn += angleturn[tspeed];
    }
  }

  if (gamekeydown[key_up]) {
    forward += forwardmove[speed];
  }
  if (gamekeydown[key_down]) {
    forward -= forwardmove[speed];
  }
  if (joyymove < 0) {
    forward += forwardmove[speed];
  }
  if (joyymove > 0) {
    forward -= forwardmove[speed];
  }
  if (gamekeydown[key_straferight]) {
    side += sidemove[speed];
  }
  if (gamekeydown[key_strafeleft]) {
    side -= sidemove[speed];
  }

  //mouse
  if (mousebuttons[mousebforward]) {
    forward += forwardmove[speed];
  }

  forward += mousey;

  if (strafe) {
    side += mousex / 4;       /* mead  Don't want to strafe as fast as turns.*/
  }
  else {
    angturn -= mousex; /* mead now have enough dynamic range 2-10-00 */
  }

  walkcamera.angle += ((angturn / 8) << ANGLETOFINESHIFT);
  if (GetMouseLook()) {
    walkcamera.pitch += ((mlooky / 8) << ANGLETOFINESHIFT);
    CheckPitch((signed int *) &walkcamera.pitch);
  }

  if (gamekeydown[key_fire] ||
      mousebuttons[mousebfire] ||
      joybuttons[joybfire]) {
    walkcamera.x = players[0].mo->x;
    walkcamera.y = players[0].mo->y;
    walkcamera.angle = players[0].mo->angle;
    walkcamera.pitch = players[0].mo->pitch;
  }

  if (forward > MAXPLMOVE) {
    forward = MAXPLMOVE;
  }
  else if (forward < -MAXPLMOVE) {
    forward = -MAXPLMOVE;
  }
  if (side > MAXPLMOVE) {
    side = MAXPLMOVE;
  }
  else if (side < -MAXPLMOVE) {
    side = -MAXPLMOVE;
  }

  // moving forward
  walkcamera.x += FixedMul(
    (ORIG_FRICTION / 4) * forward,
    finecosine[walkcamera.angle >> ANGLETOFINESHIFT]
  );
  walkcamera.y += FixedMul(
    (ORIG_FRICTION / 4) * forward,
    finesine[walkcamera.angle >> ANGLETOFINESHIFT]
  );

  // strafing
  walkcamera.x += FixedMul(
    (ORIG_FRICTION / 6) * side,
    finecosine[(walkcamera.angle - ANG90) >> ANGLETOFINESHIFT]
  );
  walkcamera.y += FixedMul(
    (ORIG_FRICTION / 6) * side,
    finesine[(walkcamera.angle - ANG90) >> ANGLETOFINESHIFT]
  );

  subsec = R_PointInSubsector(walkcamera.x, walkcamera.y);
  walkcamera.z = subsec->sector->floorheight + 41 * FRACUNIT;

  mousex = mousey = 0;
}

void P_ResetWalkcam(void) {
  if (walkcamera.type) {
    walkcamera.PrevX = walkcamera.x;
    walkcamera.PrevY = walkcamera.y;
    walkcamera.PrevZ = walkcamera.z;
    walkcamera.PrevAngle = walkcamera.angle;
    walkcamera.PrevPitch = walkcamera.pitch;
  }
}

void P_SyncWalkcam(bool sync_coords, bool sync_sight) {
  if (!walkcamera.type) {
    return;
  }

  if (players[displayplayer].mo) {
    if (sync_sight) {
      walkcamera.angle = players[displayplayer].mo->angle;
      walkcamera.pitch = players[displayplayer].mo->pitch;
    }

    if(sync_coords) {
      walkcamera.x = players[displayplayer].mo->x;
      walkcamera.y = players[displayplayer].mo->y;
    }
  }
}

//e6y
void G_CheckDemoContinue(void) {
  if (democontinue) {
    if (LoadDemo(defdemoname, &demobuffer, &demolength, &demolumpnum)) {
      demo_continue_p = G_ReadDemoHeaderEx(demobuffer, demolength, RDH_SAFE);

      singledemo = true;
      autostart = true;
      G_RecordDemo(demo_continue_name);
      G_BeginRecording();
      usergame = true;
    }
  }
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
#endif

  if (SERVER && gameaction != ga_nothing) {
    SV_BroadcastGameActionChange();
  }
}

/* vi: set et ts=2 sw=2: */


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


#ifndef P_USER__
#define P_USER__

struct mobj_s;
typedef struct mobj_s mobj_t;

/*
 * Frame flags:
 * handles maximum brightness (torches, muzzle flare, light sources)
 */

#define FF_FULLBRIGHT   0x8000  /* flag in thing->frame */
#define FF_FRAMEMASK    0x7fff

/*
 * Overlay psprites are scaled shapes
 * drawn directly on the view screen,
 * coordinates are given for a 320*200 view screen.
 */

typedef enum
{
  ps_weapon,
  ps_flash,
  NUMPSPRITES
} psprnum_t;

typedef struct
{
  state_t *state;       /* a NULL state means not active */
  int     tics;
  fixed_t sx;
  fixed_t sy;
} pspdef_t;

typedef enum {
  WSOP_NONE   = 0,
  WSOP_WEAPON = 1,
  WSOP_AMMO   = 2,
  WSOP_MAX    = 4,
} wsop_e;

extern int weapon_preferences[2][NUMWEAPONS + 1]; /* killough 5/2/98 */
int P_WeaponPreferred(int w1, int w2);

typedef struct netticcmd_s {
  uint32_t      index;
  uint32_t      tic;
  uint32_t      server_tic;
  char          forward;
  char          side;
  short         angle;
  unsigned char buttons;
} netticcmd_t;

typedef struct {
  GPtrArray *commands;
  uint32_t   latest_synchronized_index;
  uint32_t   commands_missed;
  uint32_t   command_limit;
  uint32_t   commands_run_this_tic;
  uint32_t   latest_command_run_index;
} command_queue_t;

void A_Light0();
void A_WeaponReady();
void A_Lower();
void A_Raise();
void A_Punch();
void A_ReFire();
void A_FirePistol();
void A_Light1();
void A_FireShotgun();
void A_Light2();
void A_FireShotgun2();
void A_CheckReload();
void A_OpenShotgun2();
void A_LoadShotgun2();
void A_CloseShotgun2();
void A_FireCGun();
void A_GunFlash();
void A_FireMissile();
void A_Saw();
void A_FirePlasma();
void A_BFGsound();
void A_FireBFG();
void A_BFGSpray();
void A_FireOldBFG();

//
// Player states.
//

typedef enum
{
  // Playing or camping.
  PST_LIVE,
  // Dead on the ground, view follows killer.
  PST_DEAD,
  // Ready to restart/respawn???
  PST_REBORN,
  // Disconnected
  PST_DISCONNECTED
} playerstate_t;


//
// Player internal flags, for cheats and debug.
//
typedef enum
{
  // No clipping, walk through barriers.
  CF_NOCLIP           = 1,
  // No damage, no health loss.
  CF_GODMODE          = 2,
  // Not really a cheat, just a debug aid.
  CF_NOMOMENTUM       = 4,

  // [RH] Monsters don't target
  CF_NOTARGET         = 8,
  // [RH] Flying player
  CF_FLY              = 16,
} cheat_t;

typedef struct player_messages_s {
  GPtrArray *messages;
  bool updated;
} player_messages_t;

typedef struct player_message_s {
  char *content;
  bool centered;
  bool processed;
  int sfx;
} player_message_t;

/* The data sampled per tick (single player)
 * and transmitted to other peers (multiplayer).
 * Mainly movements/button commands per game tick,
 * plus a checksum for internal state consistency.
 * CPhipps - explicitely signed the elements, since they have to be signed to work right
 */

 /*
  * CG 04/23/2014: Un-explicitly sign the elements.  If a platform's char's are
  *                implicitly unsigned, that's too bad.  They'll be fixed again
  *                when the grand "use explicitly sized types where needed"
  *                initiative is completed.
  */

typedef struct {
  char          forwardmove;  /* *2048 for move       */
  char          sidemove;     /* *2048 for move       */
  short         angleturn;    /* <<16 for angle delta */
  short         consistancy;  /* checks for net game  */
  unsigned char chatchar;
  unsigned char buttons;
} ticcmd_t;

//
// Extended player object info: player_t
//
typedef struct player_s
{
  mobj_t*             mo;
  playerstate_t       playerstate;
  ticcmd_t            cmd;

  // Determine POV,
  //  including viewpoint bobbing during movement.
  // Focal origin above r.z
  fixed_t             viewz;
  // Base height above floor for viewz.
  fixed_t             viewheight;
  // Bob/squat speed.
  fixed_t             deltaviewheight;
  // bounded/scaled total momentum.
  fixed_t             bob;

  // This is only used between levels,
  // mo->health is used during levels.
  int                 health;
  int                 armorpoints;
  // Armor type is 0-2.
  int                 armortype;

  // Power ups. invinc and invis are tic counters.
  int                 powers[NUMPOWERS];
  bool                cards[NUMCARDS];
  bool                backpack;

  // Frags, kills of other players.
  int                 frags[MAXPLAYERS];
  weapontype_t        readyweapon;

  // Is wp_nochange if not changing.
  weapontype_t        pendingweapon;

  int                 weaponowned[NUMWEAPONS];
  int                 ammo[NUMAMMO];
  int                 maxammo[NUMAMMO];

  // True if button down last tic.
  int                 attackdown;
  int                 usedown;

  // Bit flags, for cheats and debug.
  // See cheat_t, above.
  int                 cheats;

  // Refired shots are less accurate.
  int                 refire;

  // For intermission stats.
  int                 killcount;
  int                 itemcount;
  int                 secretcount;

  // Hint messages.
  // CPhipps - const
  // const char         *message;

  player_messages_t   messages;

  // For screen flashing (red or bright).
  int                 damagecount;
  int                 bonuscount;

  // Who did damage (NULL for floors/ceilings).
  mobj_t             *attacker;

  // So gun flashes light up areas.
  int                 extralight;

  // Current PLAYPAL, ???
  //  can be set to REDCOLORMAP for pain, etc.
  int                 fixedcolormap;

  // Player skin colorshift,
  //  0-3 for which color to draw player.
  int                 colormap;

  // Overlay view sprites (gun, etc).
  pspdef_t            psprites[NUMPSPRITES];

  // True if secret level has been done.
  bool                didsecret;

  // e6y
  // All non original (new) fields of player_t struct are moved to bottom
  // for compatibility with overflow (from a deh) of player_t::ammo[NUMAMMO]

  /* killough 10/98: used for realistic bobbing (i.e. not simply overall speed)
   * mo->momx and mo->momy represent true momenta experienced by player.
   * This only represents the thrust that the player applies himself.
   * This avoids anomolies with such things as Boom ice and conveyors.
   */
  fixed_t             momx, momy;      // killough 10/98

  //e6y
  int                 resurectedkillcount;
  //not used, not removed because of savagame compatibility
  const char*         centermessage;

  fixed_t prev_viewz;
  angle_t prev_viewangle;
  angle_t prev_viewpitch;
  fixed_t jumpTics;      // delay the next jump for a moment

  /* CG 4/3/2014: New fields for netcode */
  const char      *name;
  unsigned char    team;
  command_queue_t  cmdq;
  int              ping;
  int              connect_tic;
  bool             telefragged_by_spawn;
  fixed_t          saved_momx;
  fixed_t          saved_momy;
  fixed_t          saved_momz;
  int              saved_damagecount;
} player_t;

// Bookkeeping on players - state.
extern  player_t  players[MAXPLAYERS];

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct {
  bool in;     // whether the player is in game
  int  skills; // Player stats, kills, collected items etc.
  int  sitems;
  int  ssecret;
  int  stime;
  int  frags[MAXPLAYERS];
  int  score;  // current score on entry, modified on return

} wbplayerstruct_t;

typedef struct {
  int              epsd;   // episode # (0-2)
  bool             didsecret; // if true, splash the secret level
  int              last; // previous and next levels, origin 0
  int              next;
  int              maxkills;
  int              maxitems;
  int              maxsecret;
  int              maxfrags;
  int              partime; // the par time
  int              pnum; // index of this player in game
  wbplayerstruct_t plyr[MAXPLAYERS];
  int              totaltimes; // CPhipps - total game time for completed levels so far
} wbstartstruct_t;

typedef bool (*TrimFunc)(gpointer data, gpointer user_data);

int  P_SwitchWeapon(player_t *player);
bool P_CheckAmmo(player_t *player);
void P_SetupPsprites(player_t *curplayer);
void P_MovePsprites(player_t *curplayer);
void P_DropWeapon(player_t *player);

void P_PlayerThink(int playernum);
void P_CalcHeight(player_t *player);
void P_DeathThink(player_t *player);
void P_MovePlayer(player_t *player);
void P_Thrust(player_t *player, angle_t angle, fixed_t move);
void P_SetPitch(player_t *player);
void P_SetName(int playernum, const char *name);
void P_InitPlayerMessages(int playernum);
void P_AddMessage(int playernum, player_message_t *message);
void P_ClearMessagesUpdated(int playernum);

unsigned int P_GetLatestServerRunCommandIndex(int playernum);
void         P_UpdateCommandServerTic(int playernum, uint32_t command_index,
                                                     uint32_t server_tic);
void         P_UpdateLatestSynchronizedCommandIndex(int originating_playernum,
                                                    int receiving_playernum,
                                                    unsigned int command_index);
void         P_ResetLatestSynchronizedCommandIndex(int playernum);
bool         P_LatestSynchronizedCommandIndexReady(int playernum);
unsigned int P_GetLatestSynchronizedCommandIndex(int playernum);

void         P_InitCommandQueues(void);
bool         P_HasCommands(int playernum);
unsigned int P_GetCommandCount(int playernum);
netticcmd_t* P_GetCommand(int playernum, unsigned int index);
void         P_InsertCommandSorted(int playernum, netticcmd_t *tmp_ncmd);
void         P_QueuePlayerCommand(int playernum, netticcmd_t *ncmd);
void         P_AppendNewCommand(int playernum, netticcmd_t *tmp_ncmd);
netticcmd_t* P_GetEarliestCommand(int playernum);
int          P_GetEarliestCommandIndex(int playernum);
netticcmd_t* P_GetLatestCommand(int playernum);
unsigned int P_GetLatestCommandIndex(int playernum);
void         P_UpdateLatestCommandIndex(int originating_playernum,
                                        int receiving_playernum,
                                        unsigned int command_index);
void         P_ForEachCommand(int playernum, GFunc func, gpointer user_data);
void         P_ClearPlayerCommands(int playernum);
void         P_IgnorePlayerCommands(int playernum);
void         P_TrimCommands(int playernum, TrimFunc should_trim,
                                           gpointer user_data);
void         P_TrimCommandsByTic(int playernum, int tic);
void         P_TrimCommandsByIndex(int playernum, int command_index);
void         P_BuildCommand(void);
bool         P_RunPlayerCommands(int playernum);
void         P_PrintCommands(int playernum);

void P_Printf(int playernum, const char *fmt, ...) PRINTF_DECL(2, 3);
void P_VPrintf(int playernum, const char *fmt, va_list args);
void P_Echo(int playernum, const char *message);
void P_Write(int playernum, const char *message);
void P_MPrintf(int playernum, const char *fmt, ...) PRINTF_DECL(2, 3);
void P_MVPrintf(int playernum, const char *fmt, va_list args);
void P_MEcho(int playernum, const char *message);
void P_MWrite(int playernum, const char *message);
void P_SPrintf(int playernum, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void P_SVPrintf(int playernum, int sfx, const char *fmt, va_list args);
void P_SEcho(int playernum, int sfx, const char *message);
void P_SWrite(int playernum, int sfx, const char *message);
void P_MSPrintf(int playernum, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void P_MSVPrintf(int playernum, int sfx, const char *fmt, va_list args);
void P_MSEcho(int playernum, int sfx, const char *message);
void P_MSWrite(int playernum, int sfx, const char *message);
void P_CenterPrintf(int playernum, const char *fmt, ...) PRINTF_DECL(2, 3);
void P_CenterVPrintf(int playernum, const char *fmt, va_list args);
void P_CenterEcho(int playernum, const char *message);
void P_CenterWrite(int playernum, const char *message);
void P_CenterMPrintf(int playernum, const char *fmt, ...) PRINTF_DECL(2, 3);
void P_CenterMVPrintf(int playernum, const char *fmt, va_list args);
void P_CenterMEcho(int playernum, const char *message);
void P_CenterMWrite(int playernum, const char *message);
void P_CenterSPrintf(int playernum, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void P_CenterSVPrintf(int playernum, int sfx, const char *fmt, va_list args);
void P_CenterSEcho(int playernum, int sfx, const char *message);
void P_CenterSWrite(int playernum, int sfx, const char *message);
void P_CenterMSPrintf(int playernum, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void P_CenterMSVPrintf(int playernum, int sfx, const char *fmt, va_list args);
void P_CenterMSEcho(int playernum, int sfx, const char *message);
void P_CenterMSWrite(int playernum, int sfx, const char *message);
void P_DestroyMessage(gpointer data);

void P_SendMessage(const char *message);
void P_ClearMessages(int playernum);

void P_SetPlayerName(int playernum, const char *name);
int  P_PlayerGetFragCount(int playernum);
int  P_PlayerGetDeathCount(int playernum);
int  P_PlayerGetPing(int playernum);
int  P_PlayerGetTime(int playernum);

#endif  /* P_USER__ */

/* vi: set et ts=2 sw=2: */


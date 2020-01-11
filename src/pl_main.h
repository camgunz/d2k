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


#ifndef PL_MAIN_H__
#define PL_MAIN_H__

struct mobj_s;
typedef struct mobj_s mobj_t;

typedef enum {
  ps_weapon,
  ps_flash,
  NUMPSPRITES
} psprnum_e;

typedef struct {
  state_t *state; /* a NULL state means not active */
  int      tics;
  fixed_t  sx;
  fixed_t  sy;
} pspdef_t;

typedef enum {
  PST_LIVE,        /* Playing or camping. */
  PST_DEAD,        /* Dead on the ground, view follows killer. */
  PST_REBORN,      /* Ready to restart/respawn??? */
  PST_DISCONNECTED /* Disconnected */
} playerstate_t;

typedef struct {
  char          forwardmove;  /* *2048 for move       */
  char          sidemove;     /* *2048 for move       */
  short         angleturn;    /* <<16 for angle delta */
  short         consistancy;  /* checks for net game  */
  unsigned char chatchar;
  unsigned char buttons;
} ticcmd_t;

typedef struct idxticcmd_s {
  unsigned int  index;
  unsigned int  tic;
  unsigned int  server_tic;
  char          forward;
  char          side;
  short         angle;
  unsigned char buttons;
} idxticcmd_t;

typedef struct {
  GPtrArray *commands;
  uint32_t   commands_missed;
  uint32_t   command_limit;
  uint32_t   commands_run_this_tic;
  uint32_t   latest_command_run_index;
} command_queue_t;

typedef struct player_message_s {
  char *content;
  bool centered;
  bool processed;
  int sfx;
} player_message_t;

typedef struct player_messages_s {
  GPtrArray *messages;
  bool updated;
} player_messages_t;

typedef struct player_s {
  mobj_t *mo;
  playerstate_t playerstate;
  ticcmd_t cmd;

  // Determine POV,
  //  including viewpoint bobbing during movement.
  // Focal origin above r.z
  fixed_t viewz;
  // Base height above floor for viewz.
  fixed_t viewheight;
  // Bob/squat speed.
  fixed_t deltaviewheight;
  // bounded/scaled total momentum.
  fixed_t bob;

  // This is only used between levels,
  // mo->health is used during levels.
  int health;
  int armorpoints;
  // Armor type is 0-2.
  int armortype;

  // Power ups. invinc and invis are tic counters.
  int powers[NUMPOWERS];
  bool cards[NUMCARDS];
  bool backpack;

  // Frags, kills of other players.
  /*
   * [CG] Frags/Deaths are now tracked in hash tables; keps for DeHackEd
   *      overflow compatibility
   */
  int old_frags[VANILLA_MAXPLAYERS];
  weapontype_t readyweapon;

  // Is wp_nochange if not changing.
  weapontype_t pendingweapon;

  int weaponowned[NUMWEAPONS];
  int ammo[NUMAMMO];
  int maxammo[NUMAMMO];

  // True if button down last tic.
  int attackdown;
  int usedown;

  // Bit flags, for cheats and debug.
  // See cheat_t, above.
  int cheats;

  // Refired shots are less accurate.
  int refire;

  // For intermission stats.
  int killcount;
  int itemcount;
  int secretcount;

  // Hint messages.
  // CPhipps - const
  const char *message;

  // For screen flashing (red or bright).
  int damagecount;
  int bonuscount;

  // Who did damage (NULL for floors/ceilings).
  mobj_t *attacker;

  // So gun flashes light up areas.
  int extralight;

  // Current PLAYPAL, ???
  //  can be set to REDCOLORMAP for pain, etc.
  int fixedcolormap;

  // Player skin colorshift,
  //  0-3 for which color to draw player.
  int colormap;

  // Overlay view sprites (gun, etc).
  pspdef_t psprites[NUMPSPRITES];

  // True if secret level has been done.
  bool didsecret;

  /* [CG] [FIXME] Fix this field thing described below; it's not ideal */

  // e6y
  // All non original (new) fields of player_t struct are moved to bottom
  // for compatibility with overflow (from a deh) of player_t::ammo[NUMAMMO]

  /* killough 10/98: used for realistic bobbing (i.e. not simply overall speed)
   * mo->momx and mo->momy represent true momenta experienced by player.
   * This only represents the thrust that the player applies himself.
   * This avoids anomolies with such things as Boom ice and conveyors.
   */
  fixed_t momx; // killough 10/98
  fixed_t momy;

  //e6y
  int resurectedkillcount;

  fixed_t prev_viewz;
  angle_t prev_viewangle;
  angle_t prev_viewpitch;
  fixed_t jumpTics;      // delay the next jump for a moment

  /* CG 4/3/2014: New fields for netcode */
  uint32_t id;
  const char *name;
  unsigned char team;
  command_queue_t cmdq;
  player_messages_t messages;
  GHashTable *frags;
  GHashTable *deaths;
  int ping;
  int connect_tic;
  bool telefragged_by_spawn;
  fixed_t saved_momx;
  fixed_t saved_momy;
  fixed_t saved_momz;
  int saved_damagecount;
} player_t;

typedef struct {
  size_t index;
  player_t *player;
} players_iterator_t;

#define PLAYERS_FOR_EACH(p) \
  for (players_iterator_t (p) = {0, NULL}; P_PlayersIter(&p.index, &p.player);)

void      P_PlayersInit(void);
uint32_t  P_PlayersGetCount(void);
bool      P_PlayersIter(size_t *index, player_t **start);
player_t* P_PlayersGetNew(void);
player_t* P_PlayersLookup(uint32_t id);
void      P_PlayerRemove(player_t *player);

void      P_SetConsolePlayerID(player_t *player);
void      P_SetDisplayPlayerID(player_t *player);
player_t* P_GetConsolePlayer(void);
player_t* P_GetDisplayPlayer(void);

player_t* PL_New(void);
bool      PL_IsConsolePlayer(player_t *player);
bool      PL_IsDisplayPlayer(player_t *player);
void      PL_AddDeath(player_t *victim, player_t *fragger);
uint32_t  PL_GetFrags(player_t *fragger, player_t *victim);
uint32_t  PL_GetTotalFrags(player_t *player);
uint32_t  PL_GetMurders(player_t *victim, player_t *fragger);
uint32_t  PL_GetTotalMurders(player_t *victim);
uint32_t  PL_GetTotalDeaths(player_t *victim);
uint32_t  PL_GetTotalSuicides(player_t *victim);
int32_t   PL_GetFragScore(player_t *player);
void      PL_ClearFragsAndDeaths(player_t *player);
int       P_PlayerGetPing(player_t *player);
int       P_PlayerGetTime(player_t *player);
void      PL_Spawn(player_t *player, const mapthing_t *mthing);

#endif

/* vi: set et ts=2 sw=2: */

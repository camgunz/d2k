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

#include "am_map.h"
#include "p_setup.h"
#include "pl_main.h"
#include "pl_msg.h"
#include "pl_weap.h"
#include "g_game.h"
#include "r_defs.h"
#include "r_patch.h"
#include "r_data.h"
#include "p_mobj.h"
#include "p_inter.h"
#include "p_tick.h"
#include "m_cheat.h"
#include "m_argv.h"
#include "n_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "dstrings.h"
#include "r_main.h"
#include "p_map.h"
#include "d_deh.h"  // Ty 03/27/98 - externalized strings

/* cph 2006/07/23 - needs direct access to thinkercap */
#include "p_tick.h"

// e6y: for speedup
static int boom_cheat_route[MAX_COMPATIBILITY_LEVEL];

// -----------------------------------------------------------------------------
//
// CHEAT SEQUENCE PACKAGE
//
// -----------------------------------------------------------------------------

static void cheat_mus();

static void cheat_choppers();

static void cheat_god();

static void cheat_fa();

static void cheat_k();

static void cheat_kfa();

static void cheat_noclip();

static void cheat_pw();

static void cheat_behold();

static void cheat_clev();

static void cheat_mypos();

static void cheat_rate();

static void cheat_comp();

static void cheat_friction();

static void cheat_pushers();

static void cheat_tnttran();

static void cheat_massacre();

static void cheat_ddt();

static void cheat_hom();

static void cheat_fast();

static void cheat_tntkey();

static void cheat_tntkeyx();

static void cheat_tntkeyxx();

static void cheat_tntweap();

static void cheat_tntweapx();

static void cheat_tntammo();

static void cheat_tntammox();

static void cheat_smart();

static void cheat_pitch();

static void cheat_megaarmour();

static void cheat_health();

static void cheat_notarget();

static void cheat_fly();

// -----------------------------------------------------------------------------
//
// List of cheat codes, functions, and special argument indicators.
//
// The first argument is the cheat code.
//
// The second argument is its DEH name, or NULL if it's not supported by -deh.
//
// The third argument is a combination of the bitmasks:
// {always, not_dm, not_coop, not_net, not_menu, not_demo, not_deh},
// which excludes the cheat during certain modes of play.
//
// The fourth argument is the handler function.
//
// The fifth argument is passed to the handler function if it's non-negative;
// if negative, then its negative indicates the number of extra characters
// expected after the cheat code, which are passed to the handler function
// via a pointer to a buffer (after folding any letters to lowercase).
//
// -----------------------------------------------------------------------------

cheatseq_t cheat[] = {
  CHEAT("idmus", "Change music", always, cheat_mus, -2),
  CHEAT("idchoppers", "Chainsaw", cht_never, cheat_choppers, 0),
  CHEAT("iddqd", "God mode", cht_never, cheat_god, 0),
  CHEAT("idkfa", "Ammo & Keys", cht_never, cheat_kfa, 0),
  CHEAT("idfa", "Ammo", cht_never, cheat_fa, 0),
  CHEAT("idspispopd", "No Clipping 1", cht_never, cheat_noclip, 0),
  CHEAT("idclip", "No Clipping 2", cht_never, cheat_noclip, 0),
  CHEAT("idbeholdh", "Invincibility", cht_never, cheat_health, 0),
  CHEAT("idbeholdm", "Invincibility", cht_never, cheat_megaarmour, 0),
  CHEAT("idbeholdv", "Invincibility", cht_never, cheat_pw, pw_invulnerability),
  CHEAT("idbeholds", "Berserk", cht_never, cheat_pw, pw_strength),
  CHEAT("idbeholdi", "Invisibility", cht_never, cheat_pw, pw_invisibility),
  CHEAT("idbeholdr", "Radiation Suit", cht_never, cheat_pw, pw_ironfeet),
  CHEAT("idbeholda", "Auto-map", not_dm, cheat_pw, pw_allmap),
  CHEAT("idbeholdl", "Lite-Amp Goggles", not_dm, cheat_pw, pw_infrared),
  CHEAT("idbehold", "BEHOLD menu", not_dm, cheat_behold, 0),
  CHEAT("idclev", "Level Warp", cht_never | not_menu, cheat_clev, -2),
  CHEAT("idmypos", "Player Position", not_dm, cheat_mypos, 0),
  CHEAT("idrate", "Frame rate", always, cheat_rate, 0),

  // phares
  CHEAT("tntcomp", NULL, cht_never, cheat_comp, 0),

  // jff 2/01/98 kill all monsters
  CHEAT("tntem", NULL, cht_never, cheat_massacre, 0),

  // killough 2/07/98: moved from am_map.c
  CHEAT("iddt", "Map cheat", not_dm, cheat_ddt, 0),

  // killough 2/07/98: HOM autodetector
  CHEAT("tnthom", NULL, always, cheat_hom, 0),

  // killough 2/16/98: generalized key cheats
  CHEAT("tntkey", NULL, cht_never, cheat_tntkey, 0),
  CHEAT("tntkeyr", NULL, cht_never, cheat_tntkeyx, 0),
  CHEAT("tntkeyy", NULL, cht_never, cheat_tntkeyx, 0),
  CHEAT("tntkeyb", NULL, cht_never, cheat_tntkeyx, 0),
  CHEAT("tntkeyrc", NULL, cht_never, cheat_tntkeyxx, it_redcard),
  CHEAT("tntkeyyc", NULL, cht_never, cheat_tntkeyxx, it_yellowcard),
  CHEAT("tntkeybc", NULL, cht_never, cheat_tntkeyxx, it_bluecard),
  CHEAT("tntkeyrs", NULL, cht_never, cheat_tntkeyxx, it_redskull),
  CHEAT("tntkeyys", NULL, cht_never, cheat_tntkeyxx, it_yellowskull),

  // killough 2/16/98: end generalized keys
  CHEAT("tntkeybs", NULL, cht_never, cheat_tntkeyxx, it_blueskull),

  // Ty 04/11/98 - Added TNTKA
  CHEAT("tntka", NULL, cht_never, cheat_k, 0),

  // killough 2/16/98: generalized weapon cheats
  CHEAT("tntweap", NULL, cht_never, cheat_tntweap, 0),
  CHEAT("tntweap", NULL, cht_never, cheat_tntweapx, -1),
  CHEAT("tntammo", NULL, cht_never, cheat_tntammo, 0),

  // killough 2/16/98: end generalized weapons
  CHEAT("tntammo", NULL, cht_never, cheat_tntammox, -1),

  // invoke translucency         // phares
  CHEAT("tnttran", NULL, always, cheat_tnttran, 0),

  // killough 2/21/98: smart monster toggle
  CHEAT("tntsmart", NULL, cht_never, cheat_smart, 0),

  // killough 2/21/98: pitched sound toggle
  CHEAT("tntpitch", NULL, always, cheat_pitch, 0),

  // killough 2/21/98: reduce RSI injury by adding simpler alias sequences:
  // killough 2/21/98: same as tnttran
  CHEAT("tntran", NULL, always, cheat_tnttran, 0),

  // killough 2/21/98: same as tntammo
  CHEAT("tntamo", NULL, cht_never, cheat_tntammo, 0),

  // killough 2/21/98: same as tntammo
  CHEAT("tntamo", NULL, cht_never, cheat_tntammox, -1),

  // killough 3/6/98: -fast toggle
  CHEAT("tntfast", NULL, cht_never, cheat_fast, 0),

  // phares 3/10/98: toggle variable friction effects
  CHEAT("tntice", NULL, cht_never, cheat_friction, 0),

  // phares 3/10/98: toggle pushers
  CHEAT("tntpush", NULL, cht_never, cheat_pushers, 0),

  // [RH] Monsters don't target
  CHEAT("notarget", NULL, cht_never, cheat_notarget, 0),

  // fly mode is active
  CHEAT("fly", NULL, cht_never, cheat_fly, 0),

  // end-of-list marker
  { NULL }
};

// -----------------------------------------------------------------------------

static void cheat_mus(char buf[3]) {
  int musnum;

  // jff 3/20/98 note: this cheat allowed in netgame/demorecord

  // jff 3/17/98 avoid musnum being negative and crashing
  if (!isdigit(buf[0]) || !isdigit(buf[1])) {
    return;
  }

  PL_Echo(P_GetConsolePlayer(), s_STSTR_MUS); // Ty 03/27/98 - externalized

  if (gamemode == commercial) {
    musnum = mus_runnin + (buf[0] - '0') * 10 + buf[1] - '0' - 1;

    // jff 4/11/98 prevent IDMUS00 in DOOMII and IDMUS36 or greater
    if (musnum < mus_runnin || ((buf[0] - '0') * 10 + buf[1] - '0') > 35) {
      // Ty 03/27/98 - externalized
      PL_Echo(P_GetConsolePlayer(), s_STSTR_NOMUS);
    }
    else {
      S_ChangeMusic(musnum, 1);
      idmusnum = musnum; // jff 3/17/98 remember idmus number for restore
    }
  }
  else {
    musnum = mus_e1m1 + (buf[0] - '1') * 9 + (buf[1] - '1');

    // jff 4/11/98 prevent IDMUS0x IDMUSx0 in DOOMI and greater than introa
    if (buf[0] < '1' ||
        buf[1] < '1' ||
        ((buf[0] - '1') * 9 + buf[1] - '1') > 31) {
      // Ty 03/27/98 - externalized
      PL_Echo(P_GetConsolePlayer(), s_STSTR_NOMUS);
    }
    else {
      S_ChangeMusic(musnum, 1);
      idmusnum = musnum; // jff 3/17/98 remember idmus number for restore
    }
  }
}

// 'choppers' invulnerability & chainsaw
static void cheat_choppers() {
  P_GetConsolePlayer()->weaponowned[wp_chainsaw] = true;
  P_GetConsolePlayer()->powers[pw_invulnerability] = true;
  // Ty 03/27/98 - externalized
  PL_Echo(P_GetConsolePlayer(), s_STSTR_CHOPPERS);
}

// 'dqd' cheat for toggleable god mode
static void cheat_god() {
  P_GetConsolePlayer()->cheats ^= CF_GODMODE;

  if (P_GetConsolePlayer()->cheats & CF_GODMODE) {
    if (P_GetConsolePlayer()->mo) {
      P_GetConsolePlayer()->mo->health = god_health; // Ty 03/09/98 - deh
    }
    P_GetConsolePlayer()->health = god_health;
    // Ty 03/27/98 - externalized
    PL_Echo(P_GetConsolePlayer(), s_STSTR_DQDON);
  }
  else {
    // Ty 03/27/98 - externalized
    PL_Echo(P_GetConsolePlayer(), s_STSTR_DQDOFF);
  }
}

// CPhipps - new health and armour cheat codes
static void cheat_health() {
  if (!(P_GetConsolePlayer()->cheats & CF_GODMODE)) {
    if (P_GetConsolePlayer()->mo) {
      P_GetConsolePlayer()->mo->health = mega_health;
    }
    P_GetConsolePlayer()->health = mega_health;
    // Ty 03/27/98 - externalized
    PL_Echo(P_GetConsolePlayer(), s_STSTR_BEHOLDX);
  }
}

static void cheat_megaarmour() {
  P_GetConsolePlayer()->armorpoints = idfa_armor;     // Ty 03/09/98 - deh
  P_GetConsolePlayer()->armortype = idfa_armor_class; // Ty 03/09/98 - deh
  // Ty 03/27/98 - externalized
  PL_Echo(P_GetConsolePlayer(), s_STSTR_BEHOLDX);
}

static void cheat_fa() {
  int i;

  if (!P_GetConsolePlayer()->backpack) {
    for (i = 0; i < NUMAMMO; i++) {
      P_GetConsolePlayer()->maxammo[i] *= 2;
    }
    P_GetConsolePlayer()->backpack = true;
  }

  P_GetConsolePlayer()->armorpoints = idfa_armor;     // Ty 03/09/98 - deh
  P_GetConsolePlayer()->armortype = idfa_armor_class; // Ty 03/09/98 - deh

  // You can't own weapons that aren't in the game // phares 02/27/98
  for (i = 0; i < NUMWEAPONS; i++) {
    if (!(((i == wp_plasma || i == wp_bfg) && gamemode == shareware) ||
      (i == wp_supershotgun && gamemode != commercial))) {
      P_GetConsolePlayer()->weaponowned[i] = true;
    }
  }

  for (i = 0; i < NUMAMMO; i++) {
    if (i != am_cell || gamemode != shareware) {
      P_GetConsolePlayer()->ammo[i] = P_GetConsolePlayer()->maxammo[i];
    }
  }

  PL_Echo(P_GetConsolePlayer(), s_STSTR_FAADDED);
}

static void cheat_k() {
  int i;

  for (i = 0; i < NUMCARDS; i++) {
    // only print message if at least one key added
    // however, caller may overwrite message anyway
    if (!P_GetConsolePlayer()->cards[i]) {
      P_GetConsolePlayer()->cards[i] = true;
      PL_Echo(P_GetConsolePlayer(), "Keys Added");
    }
  }
}

static void cheat_kfa() {
  cheat_k();
  cheat_fa();
  PL_Echo(P_GetConsolePlayer(), STSTR_KFAADDED);
}

static void cheat_noclip() {
  // Simplified, accepting both "noclip" and "idspispopd".
  // no clipping mode cheat

  P_GetConsolePlayer()->cheats ^= CF_NOCLIP;

  // Ty 03/27/98 - externalized
  if (P_GetConsolePlayer()->cheats & CF_NOCLIP) {
    PL_Echo(P_GetConsolePlayer(), s_STSTR_NCON);
  }
  else {
    PL_Echo(P_GetConsolePlayer(), s_STSTR_NCOFF);
  }
}

// 'behold?' power-up cheats (modified for infinite duration -- killough)
static void cheat_pw(int pw) {
  // killough
  if (P_GetConsolePlayer()->powers[pw]) {
    P_GetConsolePlayer()->powers[pw] = pw != pw_strength && pw != pw_allmap;
  }
  else {
    P_GivePower(P_GetConsolePlayer(), pw);

    // infinite duration -- killough
    if (pw != pw_strength) {
      P_GetConsolePlayer()->powers[pw] = -1;
    }
  }
  PL_Echo(P_GetConsolePlayer(), s_STSTR_BEHOLDX); // Ty 03/27/98 - externalized
}

// 'behold' power-up menu
static void cheat_behold() {
  PL_Echo(P_GetConsolePlayer(), s_STSTR_BEHOLD); // Ty 03/27/98 - externalized
}

// 'clev' change-level cheat
static void cheat_clev(char buf[3]) {
  int epsd, map;

  if (gamemode == commercial) {
    epsd = 1; // jff was 0, but espd is 1-based
    map = (buf[0] - '0') * 10 + buf[1] - '0';
  }
  else {
    epsd = buf[0] - '0';
    map = buf[1] - '0';
  }

  // Catch invalid maps.
  if (epsd < 1 || map < 1 || // Ohmygod - this is not going to work.
    // e6y: The fourth episode for pre-ultimate complevels is not allowed.
    (compatibility_level < ultdoom_compatibility && (epsd > 3)) ||
    (gamemode == retail     && (epsd > 4 || map > 9)) ||
    (gamemode == registered && (epsd > 3 || map > 9)) ||
    (gamemode == shareware  && (epsd > 1 || map > 9)) ||
    (gamemode == commercial && (epsd > 1 || map > 33))) { // jff no 33 and 34
    return;                                               // 8/14/98 allowed
  }

  if (!bfgedition && map == 33) {
    return;
  }

  if (gamemission == pack_nerve && map > 9) {
    return;
  }

  // Chex.exe always warps to episode 1.
  if (gamemission == chex) {
    epsd = 1;
  }

  // So be it.
  PL_Echo(P_GetConsolePlayer(), s_STSTR_CLEV); // Ty 03/27/98 - externalized

  G_DeferedInitNew(gameskill, epsd, map);
}

// 'mypos' for player position
// killough 2/7/98: simplified using dprintf and made output more user-friendly
static void cheat_mypos() {
  PL_Printf(P_GetConsolePlayer(), "Position (%d,%d,%d)\tAngle %-.0f",
    P_GetConsolePlayer()->mo->x >> FRACBITS,
    P_GetConsolePlayer()->mo->y >> FRACBITS,
    P_GetConsolePlayer()->mo->z >> FRACBITS,
    P_GetConsolePlayer()->mo->angle * (90.0 / ANG90)
  );
}

// cph - cheat to toggle frame rate/rendering stats display
static void cheat_rate() {
  rendering_stats ^= 1;
}

// compatibility cheat

static void cheat_comp() {
  // CPhipps - modified for new compatibility system
  compatibility_level++;
  compatibility_level %= MAX_COMPATIBILITY_LEVEL;

  // must call G_Compatibility after changing compatibility_level
  // (fixes sf bug number 1558738)
  G_Compatibility();
  PL_Printf(P_GetConsolePlayer(), "New compatibility level:\n%s",
    comp_lev_str[compatibility_level]
  );
}

// variable friction cheat
static void cheat_friction() {
  variable_friction = !variable_friction;

  // Ty 03/27/98 - *not* externalized
  if (variable_friction) {
    PL_Echo(P_GetConsolePlayer(), "Variable Friction enabled");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Variable Friction disabled");
  }
}

// Pusher cheat
// phares 3/10/98
static void cheat_pushers() {
  allow_pushers = !allow_pushers;

  // Ty 03/27/98 - *not* externalized
  if (allow_pushers) {
    PL_Echo(P_GetConsolePlayer(), "Pushers enabled");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Pushers disabled");
  }
}

// translucency cheat
static void cheat_tnttran() {
  general_translucency = !general_translucency;

  // Ty 03/27/98 - *not* externalized
  if (general_translucency) {
    PL_Echo(P_GetConsolePlayer(), "Translucency enabled");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Translucency disabled");
  }

  // killough 3/1/98, 4/11/98: cache translucency map on a demand basis
  if (general_translucency && !main_tranmap) {
    R_InitTranMap(0);
  }
}

static void cheat_massacre() {                        // jff 2/01/98 kill all
                                                      // monsters
  // jff 02/01/98 'em' cheat - kill all monsters
  // partially taken from Chi's .46 port
  //
  // killough 2/7/98: cleaned up code and changed to use dprintf;
  // fixed lost soul bug (LSs left behind when PEs are killed)

  int killcount = 0;
  thinker_t *currentthinker = NULL;
  extern void A_PainDie(mobj_t *);

  // killough 7/20/98: kill friendly monsters only if no others to kill
  uint64_t mask = MF_FRIEND;

  P_MapStart("cheat_massacre");
  do {
    while ((currentthinker = P_NextThinker(currentthinker, th_all)) != NULL) {
      if (currentthinker->function == P_MobjThinker &&
        !(((mobj_t *)currentthinker)->flags & mask) && // killough 7/20/98
        (((mobj_t *)currentthinker)->flags & MF_COUNTKILL ||
        ((mobj_t *)currentthinker)->type == MT_SKULL)) {
        // killough 3/6/98: kill even if PE is dead
        if (((mobj_t *)currentthinker)->health > 0) {
          killcount++;
          P_DamageMobj((mobj_t *)currentthinker, NULL, NULL, 10000);
        }
        if (((mobj_t *)currentthinker)->type == MT_PAIN) {
          A_PainDie((mobj_t *)currentthinker);        // killough 2/8/98
          P_SetMobjState((mobj_t *)currentthinker, S_PAIN_DIE6);
        }
      }
    }
  } while (!killcount && mask ? mask = 0, 1 : 0);     // killough 7/20/98

  P_MapEnd();

  // killough 3/22/98: make more intelligent about plural
  // Ty 03/27/98 - string(s) *not* externalized

  if (killcount == 1) {
    PL_Printf(P_GetConsolePlayer(), "%d Monsters Killed\n", killcount);
  }
  else {
    PL_Printf(P_GetConsolePlayer(), "1 Monster Killed\n");
  }
}

// killough 2/7/98: move iddt cheat from am_map.c to here
// killough 3/26/98: emulate Doom better
static void cheat_ddt() {
  extern int ddt_cheating;

  if (automapmode & am_active) {
    ddt_cheating = (ddt_cheating + 1) % 3;
  }
}

// killough 2/7/98: HOM autodetection
static void cheat_hom() {
  flashing_hom = !flashing_hom;

  if (flashing_hom) {
    PL_Echo(P_GetConsolePlayer(), "HOM Detection On");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "HOM Detection Off");
  }
}

// killough 3/6/98: -fast parameter toggle
static void cheat_fast() {
  fastparm = !fastparm;

  // Ty 03/27/98 - *not* externalized
  if (fastparm) {
    PL_Echo(P_GetConsolePlayer(), "Fast Monsters On");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Fast Monsters Off");
  }

  G_SetFastParms(fastparm);                  // killough 4/10/98: set -fast
                                             // parameter correctly
}

// killough 2/16/98: keycard/skullkey cheat functions
static void cheat_tntkey() {
  // Ty 03/27/98 - *not* externalized
  PL_Echo(P_GetConsolePlayer(), "Red, Yellow, Blue");
}

static void cheat_tntkeyx() {
  // Ty 03/27/98 - *not* externalized
  PL_Echo(P_GetConsolePlayer(), "Card, Skull");
}

static void cheat_tntkeyxx(int key) {
  P_GetConsolePlayer()->cards[key] = !P_GetConsolePlayer()->cards[key];

  // Ty 03/27/98 - *not* externalized
  if (P_GetConsolePlayer()->cards[key]) {
    PL_Echo(P_GetConsolePlayer(), "Key Added");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Key Removed");
  }
}

// killough 2/16/98: generalized weapon cheats
static void cheat_tntweap() {
  // Ty 03/27/98 - *not* externalized
  // killough 2/28/98
  if (gamemode == commercial) {
    PL_Echo(P_GetConsolePlayer(), "Weapon number 1-9");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Weapon number 1-8");
  }
}

static void cheat_tntweapx(char buf[3]) {
  int w = *buf - '1';

  if ((w == wp_supershotgun && gamemode != commercial) || // killough 2/28/98
      ((w == wp_bfg || w == wp_plasma) && gamemode == shareware)) {
    return;
  }

  // make '1' apply beserker strength toggle
  if (w == wp_fist) {
    cheat_pw(pw_strength);
  }
  else if (w >= 0 && w < NUMWEAPONS) {
    bool weaponowned = P_GetConsolePlayer()->weaponowned[w];

    P_GetConsolePlayer()->weaponowned[w] = !weaponowned;
    ;
    if (P_GetConsolePlayer()->weaponowned[w]) {
      // Ty 03/27/98 - *not* externalized
      PL_Echo(P_GetConsolePlayer(), "Weapon Added");
    }
    else {
      // Ty 03/27/98 - *not* externalized
      PL_Echo(P_GetConsolePlayer(), "Weapon Removed");

      // maybe switch if weapon removed
      if (w == P_GetConsolePlayer()->readyweapon) {
        P_GetConsolePlayer()->pendingweapon = PL_SwitchWeapon(
          P_GetConsolePlayer()
        );
      }
    }
  }
}

// killough 2/16/98: generalized ammo cheats
static void cheat_tntammo() {
  // Ty 03/27/98 - *not* externalized
  PL_Echo(P_GetConsolePlayer(), "Ammo 1-4, Backpack");
}

static void cheat_tntammox(char buf[1]) {
  int a = *buf - '1';

  if (*buf == 'b') { // Ty 03/27/98 - strings *not* externalized
    P_GetConsolePlayer()->backpack = !P_GetConsolePlayer()->backpack;
    if (P_GetConsolePlayer()->backpack) {
      PL_Echo(P_GetConsolePlayer(), "Backpack Added");

      for (a = 0; a < NUMAMMO; a++) {
        P_GetConsolePlayer()->maxammo[a] <<= 1;
      }
    }
    else {
      PL_Echo(P_GetConsolePlayer(), "Backpack Removed");

      for (a = 0; a < NUMAMMO; a++) {
        if (P_GetConsolePlayer()->ammo[a] >
          (P_GetConsolePlayer()->maxammo[a] >>= 1)) {
          P_GetConsolePlayer()->ammo[a] = P_GetConsolePlayer()->maxammo[a];
        }
      }
    }
  }
  else if (a >= 0 && a < NUMAMMO) { // Ty 03/27/98 - *not* externalized
    // killough 5/5/98: switch plasma and rockets for now -- KLUDGE
    // HACK
    if (a == am_cell) {
      a = am_misl;
    }
    else if (a == am_misl) {
      a = am_cell;
    }

    P_GetConsolePlayer()->ammo[a] = !P_GetConsolePlayer()->ammo[a];

    if (P_GetConsolePlayer()->ammo[a]) {
      PL_Echo(P_GetConsolePlayer(), "Ammo Added");
    }
    else {
      PL_Echo(P_GetConsolePlayer(), "Ammo Removed");
    }
  }
}

static void cheat_smart() {
  monsters_remember = !monsters_remember;

  if (monsters_remember) {
    PL_Echo(P_GetConsolePlayer(), "Smart Monsters Enabled");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Smart Monsters Disabled");
  }
}

static void cheat_pitch() {
  pitched_sounds = !pitched_sounds;

  if (pitched_sounds) {
    PL_Echo(P_GetConsolePlayer(), "Pitch Effects Enabled");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "Pitch Effects Disabled");
  }
}

static void cheat_notarget() {
  P_GetConsolePlayer()->cheats ^= CF_NOTARGET;

  if (P_GetConsolePlayer()->cheats & CF_NOTARGET) {
    PL_Echo(P_GetConsolePlayer(), "No-target Mode On");
  }
  else {
    PL_Echo(P_GetConsolePlayer(), "No-target Mode Off");
  }
}

static void cheat_fly() {
  if (P_GetConsolePlayer()->mo != NULL) {
    P_GetConsolePlayer()->cheats ^= CF_FLY;

    if (P_GetConsolePlayer()->cheats & CF_FLY) {
      P_GetConsolePlayer()->mo->flags |= MF_NOGRAVITY;
      P_GetConsolePlayer()->mo->flags |= MF_FLY;
      PL_Echo(P_GetConsolePlayer(), "Fly mode On");
    }
    else {
      P_GetConsolePlayer()->mo->flags &= ~MF_NOGRAVITY;
      P_GetConsolePlayer()->mo->flags &= ~MF_FLY;
      PL_Echo(P_GetConsolePlayer(), "Fly mode Off");
    }
  }
}

// -----------------------------------------------------------------------------
// 2/7/98: Cheat detection rewritten by Lee Killough, to avoid
// scrambling and to use a more general table-driven approach.
// -----------------------------------------------------------------------------

static int M_FindCheats_Boom(int key) {
  static uint64_t sr;
  static char argbuf[CHEAT_ARGS_MAX + 1];
  static char *arg;
  static int init;
  static int argsleft;
  static int cht;

  int i;
  int ret;
  int matchedbefore;

  // If we are expecting arguments to a cheat
  // (e.g. idclev), put them in the arg buffer

  if (argsleft) {
    *arg++ = tolower(key);                        // store key in arg buffer

    if (!--argsleft) {                            // if last key in arg list,
      cheat[cht].func(argbuf);                    // process the arg buffer
    }
    return 1;                                     // affirmative response
  }

  key = tolower(key) - 'a';
  if (key < 0 || key >= 32) {                     // ignore most non-alpha cheat
                                                  // letters
    sr = 0;                                       // clear shift register
    return 0;
  }

  if (!init) {                                    // initialize aux entries of
                                                  // table
    init = 1;
    for (i = 0; cheat[i].cheat; i++) {
      uint64_t c = 0, m = 0;
      const char *p;

      for (p = cheat[i].cheat; *p; p++) {
        unsigned key = tolower(*p) - 'a';         // convert to 0-31

        if (key >= 32) {                          // ignore most non-alpha cheat
                                                  // letters
          continue;
        }

        c = (c << 5) + key;                       // shift key into code
        m = (m << 5) + 31;                        // shift 1's into mask
      }
      cheat[i].code = c;                          // code for this cheat key
      cheat[i].mask = m;                          // mask for this cheat key
    }
  }

  sr = (sr << 5) + key;                           // shift this key into shift
                                                  // register

  for (matchedbefore = ret = i = 0; cheat[i].cheat; i++) {
    if ((sr & cheat[i].mask) == cheat[i].code &&  // if match found
      !(cheat[i].when & not_dm   && deathmatch) &&// and if cheat allowed
      !(cheat[i].when & not_coop && netgame && !deathmatch) &&
      !(cheat[i].when & not_demo && (demorecording || demoplayback)) &&
      !(cheat[i].when & not_menu && menuactive) &&
      !(cheat[i].when & not_deh  && M_CheckParm("-deh"))) {
      if (cheat[i].arg < 0) {                     // if additional args are
                                                  // required
        cht = i;                                  // remember this cheat code
        arg = argbuf;                             // point to start of arg
                                                  // buffer
        argsleft = -cheat[i].arg;                 // number of args expected
        ret = 1;                                  // responder has eaten key
      }
      else if (!matchedbefore) {                  // allow only one cheat at a
                                                  // time
        matchedbefore = ret = 1;                  // responder has eaten key
        cheat[i].func(cheat[i].arg);              // call cheat handler
      }
    }
  }

  return ret;
}

//
// CHEAT SEQUENCE PACKAGE
//

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
static int M_FindCheats_Doom(int key) {
  int rc = 0;
  cheatseq_t *cht;
  char char_key;

  char_key = (char)key;

  for (cht = cheat; cht->cheat; cht++) {
    if (!(cht->when & not_dm   && deathmatch) &&  // and if cheat allowed
      !(cht->when & not_coop && netgame && !deathmatch) &&
      !(cht->when & not_demo && (demorecording || demoplayback)) &&
      !(cht->when & not_menu && menuactive) &&
      !(cht->when & not_deh  && M_CheckParm("-deh"))) {
      // if we make a short sequence on a cheat with parameters, this
      // will not work in vanilla doom.  behave the same.

      if (demo_compatibility || compatibility_level == lxdoom_1_compatibility) {
        if (cht->arg < 0 && cht->deh_sequence_len < cht->sequence_len) {
          continue;
        }
      }

      if (cht->chars_read < cht->deh_sequence_len) {
        // still reading characters from the cheat code
        // and verifying.  reset back to the beginning
        // if a key is wrong

        if (char_key == cht->cheat[cht->chars_read]) {
          ++cht->chars_read;
        }
        else {
          cht->chars_read = 0;
        }

        cht->param_chars_read = 0;
      }
      else if (cht->param_chars_read < -cht->arg) {
        // we have passed the end of the cheat sequence and are
        // entering parameters now

        cht->parameter_buf[cht->param_chars_read] = char_key;

        ++cht->param_chars_read;

        // affirmative response
        rc = 1;
      }

      if (cht->chars_read >= cht->deh_sequence_len &&
        cht->param_chars_read >= -cht->arg) {
        if (cht->param_chars_read) {
          static char argbuf[CHEAT_ARGS_MAX + 1];

          // process the arg buffer
          memcpy(argbuf, cht->parameter_buf, -cht->arg);

          cht->func(argbuf);
        }
        else {
          // call cheat handler
          cht->func(cht->arg);
        }

        cht->chars_read = cht->param_chars_read = 0;
        rc = 1;
      }
    }
  }

  return rc;
}

static void cht_InitCheats(void) {
  static int init = false;

  if (!init) {
    cheatseq_t *cht;

    init = true;

    memset(boom_cheat_route, 0, sizeof(boom_cheat_route));
    boom_cheat_route[boom_compatibility_compatibility] = 1;
    boom_cheat_route[boom_201_compatibility] = 1;
    boom_cheat_route[boom_202_compatibility] = 1;
    boom_cheat_route[mbf_compatibility] = 1;

    for (cht = cheat; cht->cheat; cht++) {
      cht->deh_sequence_len = strlen(cht->cheat);
    }
  }
}

bool M_FindCheats(int key) {
  cht_InitCheats();

  if (boom_cheat_route[compatibility_level]) {
    return M_FindCheats_Boom(key);
  }

  return M_FindCheats_Doom(key);
}

/* vi: set et ts=2 sw=2: */

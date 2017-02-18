-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- Copyright (C) 2014: See COPYRIGHT file                                    --
--                                                                           --
-- This file is part of D2K.                                                 --
--                                                                           --
-- D2K is free software: you can redistribute it and/or modify it under the  --
-- terms of the GNU General Public License as published by the Free Software --
-- Foundation, either version 2 of the License, or (at your option) any      --
-- later version.                                                            --
--                                                                           --
-- D2K is distributed in the hope that it will be useful, but WITHOUT ANY    --
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS --
-- FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    --
-- details.                                                                  --
--                                                                           --
-- You should have received a copy of the GNU General Public License along   --
-- with D2K.  If not, see <http:--www.gnu.org/licenses/>.                    --
--                                                                           --
-------------------------------------------------------------------------------

local debug = require('debug')

CheatEngine = {}

CheatEngine.ALWAYS   = 0
CheatEngine.NOT_DM   = 1
CheatEngine.NOT_COOP = 2
CheatEngine.NOT_DEMO = 4
CheatEngine.NOT_MENU = 8
CheatEngine.NOT_DEH  = 16
CheatEngine.SINGLEPLAYER  = CheatEngine.NOT_DM | CheatEngine.NOT_COOP
CheatEngine.SINGLEPLAYER = CheatEngine.NOT_NET | CheatEngine.NOT_DEMO

function get_give_power_func(pow)
    return function()
        local player = d2k.Game.get_consoleplayer()
        player.add_permanent_power(pow)
    end
end

cheat_engine.add_cheat({
    'idbeholdr',
    'Radiation Suit',
    Cheat.NEVER,
    get_give_power_func(d2k.Game.powers.ironfeet)
})

function CheatEngine:add_cheat(cht)
    local cheat_code = cht[1]

    cht.cheats[cheat_code] = {
        code = cht[1],
        description = cht[2],
        when = cht[3],
        run = cht[4],
        arity = debug.getinfo(cht.func, 'u').nparams
    }
end

function CheatEngine:initialize(ce)
    ce = ce or {}

    self.buf = ce.buf or {}
    self.input = ce.input or {}
    self.cheats = ce.input or {}
end

function CheatEngine:_clear_input_buffer()
    while #self.buf do
        table.remove(self.buf)
    end
end

function CheatEngine:_rebuild_input()
    self.input = ''

    for i, c in ipairs(self.buf) do
        self.input = self.input .. c
    end
end

function CheatEngine:_clip_starting_junk()
    local junk_char_count
    local found_junk = false

    for junk_char_count=1,#self.input do
        local found_potential_match = false
        local token = self.input:sub(junk_char_count, #self.input)

        for i, cheat in ipairs(self.cheats) do
            found_potential_match = cheat.name:startswith(token)
            if found_potential_match then
                break
            end
        end

        if found_potential_match then
            break
        end

        found_junk = true
    end

    if not found_junk then
        return
    end

    for i=1,junk_char_count do
        table.remove(self.buf)
    end

    self:_rebuild_input()
end

function CheatEngine:_find_matching_cheat()
    for i=#self.input,1,-1 do
        local cheat = self.cheats[self.input:sub(1, i)

        if cheat ~= nil then
            return cheat
        end
    end
end

function CheatEngine:run_matching_cheat()
    self:_clip_starting_junk()

    local cheat = self:_find_matching_cheat()

    if cheat == nil then
        return
    end

    local args = {}

    for i=1,cheat.arity do
        local arg = self.buf[#cheat.code + i]

        if arg:is_digit() then
            arg = tonumber(arg)
        end

        args.insert(arg)
    end

    cheat.run(unpack(args))

    for n, cheat in ipairs(self.cheats) do
        if 
    end
end

function CheatEngine:handle_input(event)
    if not event.is_key() then
        return false
    end

    if not event.is_press() then
        return false
    end

    local key_name = event.get_key_name()

    if key_name:len() ~= 1 then
        return false
    end

    local letter = key_name:match('%a')

    if letter ~= nil then
        table.insert(self.buf, letter:lower())
        self:run_matching_cheat()
        return true
    end

    local digit = key_name:match('%d')

    if digit ~= nil then
        table.insert(self.buf, digit)
        self:_run_matching_cheat()
        return true
    end

    self:_clear_input_buffer() -- [CG] Clear on non-alphanumeric key press
    return false
end

cheats = {
    {"idmus",      "Change music",     Cheat.ALWAYS, cheat_mus, arity = 2},
    {"idchoppers", "Chainsaw",         Cheat.NEVER, cheat_choppers, 0},
    {"iddqd",      "God mode",         Cheat.NEVER, cheat_god, 0},
    {"idkfa",      "Ammo & Keys",      Cheat.NEVER, cheat_kfa, 0},
    {"idfa",       "Ammo",             Cheat.NEVER, cheat_fa, 0},
    {"idspispopd", "No Clipping 1",    Cheat.NEVER, cheat_noclip, 0},
    {"idclip",     "No Clipping 2",    Cheat.NEVER, cheat_noclip, 0},
    {"idbeholdh",  "Invincibility",    Cheat.NEVER, cheat_health, 0},
    {"idbeholdm",  "Invincibility",    Cheat.NEVER, cheat_megaarmour, 0},
    {"idbeholdv",  "Invincibility",    Cheat.NEVER, cheat_pw, arg = pw_invulnerability},
    {"idbeholds",  "Berserk",          Cheat.NEVER, cheat_pw, pw_strength},
    {"idbeholdi",  "Invisibility",     Cheat.NEVER, cheat_pw, pw_invisibility},
    {"idbeholdr",  "Radiation Suit",   Cheat.NEVER, cheat_pw, pw_ironfeet},
    {"idbeholda",  "Auto-map",         not_dm, cheat_pw, pw_allmap},
    {"idbeholdl",  "Lite-Amp Goggles", not_dm, cheat_pw, pw_infrared},
    {"idbehold",   "BEHOLD menu",      not_dm, cheat_behold, 0},
    {"idclev",     "Level Warp",       Cheat.NEVER | not_menu, cheat_clev, -2},
    {"idmypos",    "Player Position",  not_dm, cheat_mypos, 0},
    {"idrate",     "Frame rate",       Cheat.ALWAYS, cheat_rate, 0},
    -- phares
    {"tntcomp",    NULL,               Cheat.NEVER, cheat_comp, 0},
    -- jff 2/01/98 kill all monsters
    {"tntem",      NULL,               Cheat.NEVER, cheat_massacre, 0},
    -- killough 2/07/98: moved from am_map.c
    {"iddt",       "Map cheat",        not_dm, cheat_ddt, 0},
    -- killough 2/07/98: HOM autodetector
    {"tnthom",     NULL,               Cheat.ALWAYS, cheat_hom, 0},
    -- killough 2/16/98: generalized key cheats
    {"tntkey",     NULL,               Cheat.NEVER, cheat_tntkey, 0},
    {"tntkeyr",    NULL,               Cheat.NEVER, cheat_tntkeyx, 0},
    {"tntkeyy",    NULL,               Cheat.NEVER, cheat_tntkeyx, 0},
    {"tntkeyb",    NULL,               Cheat.NEVER, cheat_tntkeyx, 0},
    {"tntkeyrc",   NULL,               Cheat.NEVER, cheat_tntkeyxx, it_redcard},
    {"tntkeyyc",   NULL,               Cheat.NEVER, cheat_tntkeyxx, it_yellowcard},
    {"tntkeybc",   NULL,               Cheat.NEVER, cheat_tntkeyxx, it_bluecard},
    {"tntkeyrs",   NULL,               Cheat.NEVER, cheat_tntkeyxx, it_redskull},
    {"tntkeyys",   NULL,               Cheat.NEVER, cheat_tntkeyxx, it_yellowskull},
    -- killough 2/16/98: end generalized keys
    {"tntkeybs",   NULL,               Cheat.NEVER, cheat_tntkeyxx, it_blueskull},
    -- Ty 04/11/98 - Added TNTKA
    {"tntka",      NULL,               Cheat.NEVER, cheat_k, 0},
    -- killough 2/16/98: generalized weapon cheats
    {"tntweap",    NULL,               Cheat.NEVER, cheat_tntweap, 0},
    {"tntweap",    NULL,               Cheat.NEVER, cheat_tntweapx, -1},
    {"tntammo",    NULL,               Cheat.NEVER, cheat_tntammo, 0},
    -- killough 2/16/98: end generalized weapons
    {"tntammo",    NULL,               Cheat.NEVER, cheat_tntammox, -1},
    -- invoke translucency         -- phares
    {"tnttran",    NULL,               Cheat.ALWAYS, cheat_tnttran, 0},
    -- killough 2/21/98: smart monster toggle
    {"tntsmart",   NULL,               Cheat.NEVER, cheat_smart, 0},
    -- killough 2/21/98: pitched sound toggle
    {"tntpitch",   NULL,               Cheat.ALWAYS, cheat_pitch, 0},
    -- killough 2/21/98: reduce RSI injury by adding simpler alias sequences:
    -- killough 2/21/98: same as tnttran
    {"tntran",     NULL,               Cheat.ALWAYS, cheat_tnttran, 0},
    -- killough 2/21/98: same as tntammo
    {"tntamo",     NULL,               Cheat.NEVER, cheat_tntammo, 0},
    -- killough 2/21/98: same as tntammo
    {"tntamo",     NULL,               Cheat.NEVER, cheat_tntammox, -1},
    -- killough 3/6/98: -fast toggle
    {"tntfast",    NULL,               Cheat.NEVER, cheat_fast, 0},
    -- phares 3/10/98: toggle variable friction effects
    {"tntice",     NULL,               Cheat.NEVER, cheat_friction, 0},
    -- phares 3/10/98: toggle pushers
    {"tntpush",    NULL,               Cheat.NEVER, cheat_pushers, 0},
    -- [RH] Monsters don't target
    {"notarget",   NULL,               Cheat.NEVER, cheat_notarget, 0},
    -- fly mode is active
    {"fly",        NULL,               Cheat.NEVER, cheat_fly, 0},
};

-------------------------------------------------------------------------------

static void cheat_mus(char buf[3]) {
  int musnum;

  --jff 3/20/98 note: this cheat allowed in netgame/demorecord

  --jff 3/17/98 avoid musnum being negative and crashing
  if (!isdigit(buf[0]) || !isdigit(buf[1]))
    return;

  P_Echo(consoleplayer, s_STSTR_MUS); -- Ty 03/27/98 - externalized

  if (gamemode == commercial) {
    musnum = mus_runnin + (buf[0] - '0') * 10 + buf[1] - '0' - 1;

    --jff 4/11/98 prevent IDMUS00 in DOOMII and IDMUS36 or greater
    if (musnum < mus_runnin || ((buf[0] - '0') * 10 + buf[1] - '0') > 35) {
      P_Echo(consoleplayer, s_STSTR_NOMUS); -- Ty 03/27/98 - externalized
    }
    else {
      S_ChangeMusic(musnum, 1);
      idmusnum = musnum; --jff 3/17/98 remember idmus number for restore
    }
  }
  else {
    musnum = mus_e1m1 + (buf[0] - '1') * 9 + (buf[1] - '1');

    --jff 4/11/98 prevent IDMUS0x IDMUSx0 in DOOMI and greater than introa
    if (buf[0] < '1' ||
        buf[1] < '1' ||
        ((buf[0] - '1') * 9 + buf[1] - '1') > 31) {
      P_Echo(consoleplayer, s_STSTR_NOMUS); -- Ty 03/27/98 - externalized
    }
    else {
      S_ChangeMusic(musnum, 1);
      idmusnum = musnum; --jff 3/17/98 remember idmus number for restore
    }
  }
}

-- 'choppers' invulnerability & chainsaw
static void cheat_choppers() {
  players[consoleplayer].weaponowned[wp_chainsaw] = true;
  players[consoleplayer].powers[pw_invulnerability] = true;
  P_Echo(consoleplayer, s_STSTR_CHOPPERS); -- Ty 03/27/98 - externalized
}

-- 'dqd' cheat for toggleable god mode
static void cheat_god() {
  players[consoleplayer].cheats ^= CF_GODMODE;

  if (players[consoleplayer].cheats & CF_GODMODE) {
    if (players[consoleplayer].mo)
      players[consoleplayer].mo->health = god_health;  -- Ty 03/09/98 - deh

    players[consoleplayer].health = god_health;
    P_Echo(consoleplayer, s_STSTR_DQDON); -- Ty 03/27/98 - externalized
  }
  else {
    P_Echo(consoleplayer, s_STSTR_DQDOFF); -- Ty 03/27/98 - externalized
  }
}

-- CPhipps - new health and armour cheat codes
static void cheat_health() {
  if (!(players[consoleplayer].cheats & CF_GODMODE)) {
    if (players[consoleplayer].mo)
      players[consoleplayer].mo->health = mega_health;
    players[consoleplayer].health = mega_health;
    P_Echo(consoleplayer, s_STSTR_BEHOLDX); -- Ty 03/27/98 - externalized
  }
}

static void cheat_megaarmour() {
  players[consoleplayer].armorpoints = idfa_armor;      -- Ty 03/09/98 - deh
  players[consoleplayer].armortype = idfa_armor_class;  -- Ty 03/09/98 - deh
  P_Echo(consoleplayer, s_STSTR_BEHOLDX); -- Ty 03/27/98 - externalized
}

static void cheat_fa() {
  int i;

  if (!players[consoleplayer].backpack) {
    for (i = 0 ; i < NUMAMMO; i++)
      players[consoleplayer].maxammo[i] *= 2;
    players[consoleplayer].backpack = true;
  }

  players[consoleplayer].armorpoints = idfa_armor;      -- Ty 03/09/98 - deh
  players[consoleplayer].armortype = idfa_armor_class;  -- Ty 03/09/98 - deh

  -- You can't own weapons that aren't in the game -- phares 02/27/98
  for (i = 0; i < NUMWEAPONS; i++) {
    if (!(((i == wp_plasma || i == wp_bfg) && gamemode == shareware) ||
          (i == wp_supershotgun && gamemode != commercial))) {
      players[consoleplayer].weaponowned[i] = true;
    }
  }

  for (i = 0; i < NUMAMMO; i++) {
    if (i != am_cell || gamemode != shareware)
      players[consoleplayer].ammo[i] = players[consoleplayer].maxammo[i];
  }

  P_Echo(consoleplayer, s_STSTR_FAADDED);
}

static void cheat_k() {
  int i;

  for (i = 0; i < NUMCARDS; i++) {
    -- only print message if at least one key added
    -- however, caller may overwrite message anyway
    if (!players[consoleplayer].cards[i]) {
      players[consoleplayer].cards[i] = true;
      P_Echo(consoleplayer, "Keys Added");
    }
  }
}

static void cheat_kfa() {
  cheat_k();
  cheat_fa();
  P_Echo(consoleplayer, STSTR_KFAADDED);
}

static void cheat_noclip() {
  -- Simplified, accepting both "noclip" and "idspispopd".
  -- no clipping mode cheat

  players[consoleplayer].cheats ^= CF_NOCLIP;

  -- Ty 03/27/98 - externalized
  if (players[consoleplayer].cheats & CF_NOCLIP)
    P_Echo(consoleplayer, s_STSTR_NCON);
  else
    P_Echo(consoleplayer, s_STSTR_NCOFF);
}

-- 'behold?' power-up cheats (modified for infinite duration -- killough)
static void cheat_pw(int pw) {
  -- killough
  if (players[consoleplayer].powers[pw]) {
    players[consoleplayer].powers[pw] = pw != pw_strength && pw != pw_allmap;
  }
  else {
    P_GivePower(plyr, pw);

    -- infinite duration -- killough
    if (pw != pw_strength)
      players[consoleplayer].powers[pw] = -1;
  }
  P_Echo(consoleplayer, s_STSTR_BEHOLDX); -- Ty 03/27/98 - externalized
}

-- 'behold' power-up menu
static void cheat_behold() {
  P_Echo(consoleplayer, s_STSTR_BEHOLD); -- Ty 03/27/98 - externalized
}

-- 'clev' change-level cheat
static void cheat_clev(char buf[3]) {
  int epsd, map;

  if (gamemode == commercial) {
    epsd = 1; --jff was 0, but espd is 1-based
    map = (buf[0] - '0')*10 + buf[1] - '0';
  }
  else {
    epsd = buf[0] - '0';
    map = buf[1] - '0';
  }

  -- Catch invalid maps.
  if (epsd < 1 || map < 1 ||   -- Ohmygod - this is not going to work.
      --e6y: The fourth episode for pre-ultimate complevels is not allowed.
      (compatibility_level < ultdoom_compatibility && (epsd > 3)) ||
      (gamemode == retail     && (epsd > 4 || map > 9  )) ||
      (gamemode == registered && (epsd > 3 || map > 9  )) ||
      (gamemode == shareware  && (epsd > 1 || map > 9  )) ||
      (gamemode == commercial && (epsd > 1 || map > 33 ))) { --jff no 33 and 34
    return;                                                  --8/14/98 allowed
  }

  if (!bfgedition && map == 33)
    return;

  if (gamemission == pack_nerve && map > 9)
    return;

  -- Chex.exe always warps to episode 1.
  if (gamemission == chex)
    epsd = 1;

  -- So be it.
  P_Echo(consoleplayer, s_STSTR_CLEV); -- Ty 03/27/98 - externalized

  G_DeferedInitNew(gameskill, epsd, map);
}

-- 'mypos' for player position
-- killough 2/7/98: simplified using dprintf and made output more user-friendly
static void cheat_mypos() {
  P_Printf(consoleplayer, "Position (%d,%d,%d)\tAngle %-.0f",
    players[consoleplayer].mo->x >> FRACBITS,
    players[consoleplayer].mo->y >> FRACBITS,
    players[consoleplayer].mo->z >> FRACBITS,
    players[consoleplayer].mo->angle * (90.0 / ANG90)
  );
}

-- cph - cheat to toggle frame rate/rendering stats display
static void cheat_rate() {
  rendering_stats ^= 1;
}

-- compatibility cheat

static void cheat_comp() {
  -- CPhipps - modified for new compatibility system
  compatibility_level++; compatibility_level %= MAX_COMPATIBILITY_LEVEL;
  -- must call G_Compatibility after changing compatibility_level
  -- (fixes sf bug number 1558738)
  G_Compatibility();
  P_Printf(consoleplayer, "New compatibility level:\n%s",
    comp_lev_str[compatibility_level]
  );
}

-- variable friction cheat
static void cheat_friction() {
  variable_friction = !variable_friction;

  -- Ty 03/27/98 - *not* externalized
  if (variable_friction)
    P_Echo(consoleplayer, "Variable Friction enabled");
  else
    P_Echo(consoleplayer, "Variable Friction disabled");
}

-- Pusher cheat
-- phares 3/10/98
static void cheat_pushers() {
  allow_pushers = !allow_pushers;

  -- Ty 03/27/98 - *not* externalized
  if (allow_pushers)
    P_Echo(consoleplayer, "Pushers enabled");
  else
    P_Echo(consoleplayer, "Pushers disabled");
}

-- translucency cheat
static void cheat_tnttran() {
  general_translucency = !general_translucency;

  -- Ty 03/27/98 - *not* externalized
  if (general_translucency)
    P_Echo(consoleplayer, "Translucency enabled");
  else
    P_Echo(consoleplayer, "Translucency disabled");

  -- killough 3/1/98, 4/11/98: cache translucency map on a demand basis
  if (general_translucency && !main_tranmap)
    R_InitTranMap(0);
}

static void cheat_massacre() {  -- jff 2/01/98 kill all monsters
  -- jff 02/01/98 'em' cheat - kill all monsters
  -- partially taken from Chi's .46 port
  --
  -- killough 2/7/98: cleaned up code and changed to use dprintf;
  -- fixed lost soul bug (LSs left behind when PEs are killed)

  int killcount = 0;
  thinker_t *currentthinker = NULL;
  extern void A_PainDie(mobj_t *);

  -- killough 7/20/98: kill friendly monsters only if no others to kill
  uint64_t mask = MF_FRIEND;
  P_MapStart("cheat_massacre");
  do {
    while ((currentthinker = P_NextThinker(currentthinker, th_all)) != NULL)
      if (currentthinker->function == P_MobjThinker &&
          !(((mobj_t *) currentthinker)->flags & mask) && -- killough 7/20/98
          (((mobj_t *) currentthinker)->flags & MF_COUNTKILL ||
           ((mobj_t *) currentthinker)->type == MT_SKULL)) {
        -- killough 3/6/98: kill even if PE is dead
        if (((mobj_t *) currentthinker)->health > 0) {
          killcount++;
          P_DamageMobj((mobj_t *)currentthinker, NULL, NULL, 10000);
        }
        if (((mobj_t *) currentthinker)->type == MT_PAIN) {
          A_PainDie((mobj_t *) currentthinker);    -- killough 2/8/98
          P_SetMobjState((mobj_t *) currentthinker, S_PAIN_DIE6);
        }
      }
  } while (!killcount && mask ? mask = 0, 1 : 0); -- killough 7/20/98

  P_MapEnd();

  -- killough 3/22/98: make more intelligent about plural
  -- Ty 03/27/98 - string(s) *not* externalized

  if (killcount == 1)
    P_Printf(consoleplayer, "%d Monsters Killed\n", killcount);
  else
    P_Printf(consoleplayer, "1 Monster Killed\n");
}

-- killough 2/7/98: move iddt cheat from am_map.c to here
-- killough 3/26/98: emulate Doom better
static void cheat_ddt() {
  extern int ddt_cheating;

  if (automapmode & am_active)
    ddt_cheating = (ddt_cheating + 1) % 3;
}

-- killough 2/7/98: HOM autodetection
static void cheat_hom() {
  flashing_hom = !flashing_hom;

  if (flashing_hom)
    P_Echo(consoleplayer, "HOM Detection On");
  else
    P_Echo(consoleplayer, "HOM Detection Off");
}

-- killough 3/6/98: -fast parameter toggle
static void cheat_fast() {
  fastparm = !fastparm;

  -- Ty 03/27/98 - *not* externalized
  if (fastparm)
    P_Echo(consoleplayer, "Fast Monsters On");
  else
    P_Echo(consoleplayer, "Fast Monsters Off");

  G_SetFastParms(fastparm); -- killough 4/10/98: set -fast parameter correctly
}

-- killough 2/16/98: keycard/skullkey cheat functions
static void cheat_tntkey() {
  P_Echo(consoleplayer, "Red, Yellow, Blue");  -- Ty 03/27/98 - *not* externalized
}

static void cheat_tntkeyx() {
  P_Echo(consoleplayer, "Card, Skull");        -- Ty 03/27/98 - *not* externalized
}

static void cheat_tntkeyxx(int key) {
  players[consoleplayer].cards[key] = !players[consoleplayer].cards[key];

  -- Ty 03/27/98 - *not* externalized
  if (players[consoleplayer].cards[key])
    P_Echo(consoleplayer, "Key Added");
  else
    P_Echo(consoleplayer, "Key Removed");
}

-- killough 2/16/98: generalized weapon cheats
static void cheat_tntweap() {
  -- Ty 03/27/98 - *not* externalized
  -- killough 2/28/98
  if (gamemode == commercial)
    P_Echo(consoleplayer, "Weapon number 1-9");
  else
    P_Echo(consoleplayer, "Weapon number 1-8");
}

static void cheat_tntweapx(char buf[3]) {
  int w = *buf - '1';

  if ((w == wp_supershotgun && gamemode != commercial) ||      -- killough 2/28/98
      ((w == wp_bfg || w == wp_plasma) && gamemode == shareware)) {
    return;
  }

  -- make '1' apply beserker strength toggle
  if (w == wp_fist) {
    cheat_pw(pw_strength);
  }
  else if (w >= 0 && w < NUMWEAPONS) {
    players[consoleplayer].weaponowned[w] =
      !players[consoleplayer].weaponowned[w];
    if (players[consoleplayer].weaponowned[w]) {
      -- Ty 03/27/98 - *not* externalized
      P_Echo(consoleplayer, "Weapon Added");
    }
    else {
      -- Ty 03/27/98 - *not* externalized
      P_Echo(consoleplayer, "Weapon Removed");

      -- maybe switch if weapon removed
      if (w == players[consoleplayer].readyweapon)
        players[consoleplayer].pendingweapon = P_SwitchWeapon(plyr);
    }
  }
}

-- killough 2/16/98: generalized ammo cheats
static void cheat_tntammo() {
  -- Ty 03/27/98 - *not* externalized
  P_Echo(consoleplayer, "Ammo 1-4, Backpack");
}

static void cheat_tntammox(char buf[1]) {
  int a = *buf - '1';

  if (*buf == 'b') { -- Ty 03/27/98 - strings *not* externalized
    players[consoleplayer].backpack = !players[consoleplayer].backpack;
    if (players[consoleplayer].backpack) {
      P_Echo(consoleplayer, "Backpack Added");

      for (a = 0; a < NUMAMMO; a++)
        players[consoleplayer].maxammo[a] <<= 1;
    }
    else {
      P_Echo(consoleplayer, "Backpack Removed");

      for (a = 0; a < NUMAMMO; a++) {
        if (players[consoleplayer].ammo[a] >
            (players[consoleplayer].maxammo[a] >>= 1)) {
          players[consoleplayer].ammo[a] = players[consoleplayer].maxammo[a];
        }
      }
    }
  }
  else if (a >= 0 && a < NUMAMMO) { -- Ty 03/27/98 - *not* externalized
    -- killough 5/5/98: switch plasma and rockets for now -- KLUDGE
    -- HACK
    if (a == am_cell)
      a = am_misl;
    else if (a == am_misl)
      a = am_cell;

    players[consoleplayer].ammo[a] = !players[consoleplayer].ammo[a];

    if (players[consoleplayer].ammo[a])
      P_Echo(consoleplayer, "Ammo Added");
    else
      P_Echo(consoleplayer, "Ammo Removed");
  }
}

static void cheat_smart() {
  monsters_remember = !monsters_remember;

  if (monsters_remember)
    P_Echo(consoleplayer, "Smart Monsters Enabled");
  else
    P_Echo(consoleplayer, "Smart Monsters Disabled");
}

static void cheat_pitch() {
  pitched_sounds = !pitched_sounds;

  if (pitched_sounds)
    P_Echo(consoleplayer, "Pitch Effects Enabled");
  else
    P_Echo(consoleplayer, "Pitch Effects Disabled");
}

static void cheat_notarget() {
  players[consoleplayer].cheats ^= CF_NOTARGET;

  if (players[consoleplayer].cheats & CF_NOTARGET)
    P_Echo(consoleplayer, "No-target Mode On");
  else
    P_Echo(consoleplayer, "No-target Mode Off");
}

static void cheat_fly() {
  if (players[consoleplayer].mo != NULL) {
    players[consoleplayer].cheats ^= CF_FLY;

    if (players[consoleplayer].cheats & CF_FLY) {
      players[consoleplayer].mo->flags |= MF_NOGRAVITY;
      players[consoleplayer].mo->flags |= MF_FLY;
      P_Echo(consoleplayer, "Fly mode On");
    }
    else {
      players[consoleplayer].mo->flags &= ~MF_NOGRAVITY;
      players[consoleplayer].mo->flags &= ~MF_FLY;
      P_Echo(consoleplayer, "Fly mode Off");
    }
  }
}

-- vi: et ts=4 sw=4

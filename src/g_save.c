/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include "cmp.h"

#include "doomstat.h"
#include "m_avg.h"
#include "m_pbuf.h"
#include "d_main.h"
#include "g_game.h"
#include "g_save.h"
#include "lprintf.h"
#include "m_menu.h"
#include "n_net.h"
#include "p_map.h"
#include "p_saveg.h"
#include "r_demo.h"
#include "r_defs.h"
#include "r_fps.h"
#include "r_state.h"
#include "s_advsound.h"
#include "s_sound.h"

#include "p_tick.h"

#include "gl_intern.h"
#include "gl_struct.h"

// CPhipps - size of version header
#define VERSIONSIZE 16
#define SAVESTRINGSIZE  24

static avg_t average_savebuffer_size;
static bool average_savebuffer_size_initialized = false;

static const struct {
  int comp_level;
  const char* ver_printf;
  int version;
} version_headers[] = {
  /* cph - we don't need a new version_header for prboom_3_comp/v2.1.1, since
   *  the file format is unchanged. */
  { prboom_3_compatibility,            "PrBoom %d", 210},
  { prboom_5_compatibility,            "PrBoom %d", 211},
  { prboom_6_compatibility,            "PrBoom %d", 212}
  //e6y
  ,{ doom_12_compatibility,            "PrBoom %d", 100}
  ,{ doom_1666_compatibility,          "PrBoom %d", 101}
  ,{ doom2_19_compatibility,           "PrBoom %d", 102}
  ,{ ultdoom_compatibility,            "PrBoom %d", 103}
  ,{ finaldoom_compatibility,          "PrBoom %d", 104}
  ,{ dosdoom_compatibility,            "PrBoom %d", 105}
  ,{ tasdoom_compatibility,            "PrBoom %d", 106}
  ,{ boom_compatibility_compatibility, "PrBoom %d", 107}
  ,{ boom_201_compatibility,           "PrBoom %d", 108}
  ,{ boom_202_compatibility,           "PrBoom %d", 109}
  ,{ lxdoom_1_compatibility,           "PrBoom %d", 110}
  ,{ mbf_compatibility,                "PrBoom %d", 111}
  ,{ prboom_2_compatibility,           "PrBoom %d", 113}
};

static const size_t num_version_headers =
  sizeof(version_headers) / sizeof(version_headers[0]);

// Description to save in savegame if gameaction == ga_savegame
char savedescription[SAVEDESCLEN];

//e6y
static unsigned int GetPackageVersion(void) {
  static unsigned int PACKAGEVERSION = 0;

  //e6y: "2.4.8.2" -> 0x02040802
  if (PACKAGEVERSION == 0) {
    int b[4];
    int i;
    int k = 1;

    memset(b, 0, sizeof(b));

    sscanf(PACKAGE_VERSION, "%d.%d.%d.%d", &b[0], &b[1], &b[2], &b[3]);

    for (i = 3; i >= 0; i--, k *= 256) {
#ifdef RANGECHECK
      if (b[i] >= 256)
        I_Error("Wrong version number of package: %s", PACKAGE_VERSION);
#endif
      PACKAGEVERSION += b[i] * k;
    }
  }
  return PACKAGEVERSION;
}

static uint_64_t G_UpdateSignature(uint_64_t s, const char *name) {
  int i;
  int lump = W_CheckNumForName(name);

  if (lump != -1 && (i = lump + 10) < numlumps) {
    do {
      int size = W_LumpLength(i);
      const byte *p = W_CacheLumpNum(i);

      while (size--) {
        s <<= 1;
        s += *p++;
      }

      W_UnlockLumpNum(i);
    } while (--i > lump);
  }
  return s;
}

static uint_64_t G_Signature(void) {
  static uint_64_t s = 0;
  static dboolean computed = false;

  char name[9];
  int episode, map;

  if (computed)
    return s;

  computed = true;

  if (gamemode == commercial) {
    if (haswolflevels)
      map = 32;
    else
      map = 30;

    while (map--) {
      sprintf(name, "map%02d", map);
      s = G_UpdateSignature(s, name);
    }
  }
  else {
    map = 9;
    if (gamemode == retail)
      episode = 4;
    else if (gamemode == shareware)
      episode = 1;
    else
      episode = 3;

    while (episode--) {
      while (map--) {
        sprintf(name, "E%dM%d", episode, map);
        s = G_UpdateSignature(s, name);
      }
    }
  }

  return s;
}

static void G_LoadGameErr(const char *msg) {
  M_ForcedLoadGame(msg);             // Print message asking for 'Y' to force
  if (command_loadgame) {            // If this was a command-line -loadgame
    D_StartTitle();                // Start the title screen
    gamestate = GS_DEMOSCREEN;     // And set the game state accordingly
  }
}

//==========================================================================
//
// RecalculateDrawnSubsectors
//
// In case the subsector data is unusable this function tries to reconstruct
// if from the linedefs' ML_MAPPED info.
//
//==========================================================================
void RecalculateDrawnSubsectors(void) {
#ifdef GL_DOOM
  for (int i = 0; i < numsubsectors; i++) {
    subsector_t *sub = &subsectors[i];
    seg_t *seg = &segs[sub->firstline];

    for (int j = 0; j < sub->numlines; j++, seg++) {
      if (seg->linedef && seg->linedef->flags & ML_MAPPED)
        map_subsectors[i] = 1;
    }
  }

  gld_ResetTexturedAutomap();
#endif
}

void G_UpdateAverageSaveSize(int new_size) {
  if (!average_savebuffer_size_initialized) {
    M_AverageInit(&average_savebuffer_size);
    average_savebuffer_size_initialized = true;
  }

  M_AverageUpdate(&average_savebuffer_size, new_size);
}

int G_GetAverageSaveSize(void) {
  if (average_savebuffer_size_initialized)
    return average_savebuffer_size.value;

  return 0;
}

void G_WriteSaveData(pbuf_t *savebuffer) {
  int i;
  char name2[VERSIONSIZE];
  byte game_options[GAME_OPTION_SIZE];
  //e6y: numeric version number of package
  unsigned int packageversion = GetPackageVersion();
  char *description = savedescription;

  M_PBufWriteBytes(savebuffer, description, SAVESTRINGSIZE);

  memset(name2, 0, sizeof(name2));

  // CPhipps - scan for the version header
  for (i = 0; i < num_version_headers; i++) {
    if (version_headers[i].comp_level == best_compatibility) {
      // killough 2/22/98: "proprietary" version string :-)
      sprintf(name2, version_headers[i].ver_printf, version_headers[i].version);
      i = num_version_headers + 1;
    }
  }
  M_PBufWriteBytes(savebuffer, name2, VERSIONSIZE);

  /* killough 3/16/98, 12/98: store lump name checksum */
  if (MULTINET)
    M_PBufWriteLong(savebuffer, 1);
  else
    M_PBufWriteLong(savebuffer, G_Signature());

  /*-----------------*/
  /* CG: TODO: PWADs */
  /*-----------------*/

  M_PBufWriteUInt(savebuffer, packageversion);
  M_PBufWriteInt(savebuffer, compatibility_level);
  M_PBufWriteInt(savebuffer, gameskill);
  M_PBufWriteInt(savebuffer, gameepisode);
  M_PBufWriteInt(savebuffer, gamemap);
  M_PBufWriteInt(savebuffer, gametic);

  for (i = 0; i < MAXPLAYERS; i++)
    M_PBufWriteBool(savebuffer, playeringame[i]);

  for (i = MAXPLAYERS; i < MIN_MAXPLAYERS; i++)
    M_PBufWriteBool(savebuffer, false);

  M_PBufWriteInt(savebuffer, idmusnum);

  G_WriteOptions(game_options);    // killough 3/1/98: save game options

  if (mbf_features) {
    M_PBufWriteArray(savebuffer, GAME_OPTION_SIZE);

    for (i = 0; i < GAME_OPTION_SIZE; i++)
      M_PBufWriteUChar(savebuffer, game_options[i]);
  }
  else {
    M_PBufWriteArray(savebuffer, OLD_GAME_OPTION_SIZE);

    for (i = 0; i < OLD_GAME_OPTION_SIZE; i++)
      M_PBufWriteUChar(savebuffer, game_options[i]);
  }

  M_PBufWriteInt(savebuffer, leveltime);
  M_PBufWriteInt(savebuffer, totalleveltimes);
  // killough 11/98: save revenant tracer state
  M_PBufWriteUChar(savebuffer, (gametic - basetic) & 255);

  // phares 9/13/98: Move mobj_t->index out of P_ArchiveThinkers so the
  // indices can be used by P_ArchiveWorld when the sectors are saved.
  // This is so we can save the index of the mobj_t of the thinker that
  // caused a sound, referenced by sector_t->soundtarget.
  // P_ThinkerToIndex();
  P_UpdateMobjIndices();

  P_ArchivePlayers(savebuffer);
  P_ArchiveWorld(savebuffer);
  P_ArchiveThinkers(savebuffer);

  // phares 9/13/98: Move index->mobj_t out of P_ArchiveThinkers, simply
  // for symmetry with the P_ThinkerToIndex call above.

  // P_IndexToThinker();

  P_ArchiveSpecials(savebuffer);
  P_ArchiveRNG(savebuffer);    // killough 1/18/98: save RNG information
  P_ArchiveMap(savebuffer);    // killough 1/22/98: save automap information
  M_PBufWriteUChar(savebuffer, 0xe6); // consistency marker

  G_UpdateAverageSaveSize(M_PBufGetCapacity(savebuffer));
}

dboolean G_ReadSaveData(pbuf_t *savebuffer, dboolean bail_on_errors,
                                            dboolean init_new) {
  int i;
  int savegame_compatibility = -1;
  //e6y: numeric version number of package should be zero before initializing
  //     from savegame
  unsigned int packageversion = 0;
  unsigned char description[SAVESTRINGSIZE];
  unsigned char save_version[VERSIONSIZE];
  byte game_options[GAME_OPTION_SIZE];
  unsigned int game_option_count;
  unsigned char tic_delta;
  unsigned char safety_byte;
  buf_t byte_buf;

  M_BufferInit(&byte_buf);

  memset(description, 0, SAVESTRINGSIZE);
  memset(save_version, 0, VERSIONSIZE);

  M_PBufReadBytes(savebuffer, &byte_buf);

  if (M_BufferGetSize(&byte_buf) != SAVESTRINGSIZE) {
    I_Error("G_ReadSaveData: save string size mismatch (%zu != %u)",
      M_BufferGetSize(&byte_buf), SAVESTRINGSIZE
    );
  }

  memcpy(description, M_BufferGetData(&byte_buf), M_BufferGetSize(&byte_buf));

  M_BufferClear(&byte_buf);

  M_PBufReadBytes(savebuffer, &byte_buf);

  if (M_BufferGetSize(&byte_buf) != VERSIONSIZE) {
    I_Error("G_ReadSaveData: version size mismatch (%zu != %u)",
      M_BufferGetSize(&byte_buf), VERSIONSIZE
    );
  }

  memcpy(save_version, M_BufferGetData(&byte_buf), M_BufferGetSize(&byte_buf));

  M_BufferFree(&byte_buf);

  // CPhipps - read the description field, compare with supported ones
  for (i = 0; i < num_version_headers; i++) {
    char vcheck[VERSIONSIZE];

    // killough 2/22/98: "proprietary" version string :-)
    sprintf(vcheck, version_headers[i].ver_printf, version_headers[i].version);

    if (!strncmp((const char *)save_version, vcheck, VERSIONSIZE)) {
      savegame_compatibility = version_headers[i].comp_level;
      break;
    }
  }

  if (savegame_compatibility == -1) {
    fprintf(stderr, "savegame_compatibility == -1 (%s)\n", save_version);
    if (bail_on_errors) {
      return false;
    }
    else if (forced_loadgame) {
      savegame_compatibility = MAX_COMPATIBILITY_LEVEL - 1;
    }
    else {
      G_LoadGameErr("Unrecognized savegame version!\nAre you sure? (y/n) ");
      return false;
    }
  }

  // CPhipps - always check savegames even when forced,
  //           only print a warning if forced
  {
    // killough 3/16/98: check lump name checksum (independent of order)
    uint_64_t checksum;
    uint_64_t save_checksum = 0;

    if (MULTINET)
      checksum = 1;
    else
      checksum = G_Signature();

    M_PBufReadLong(savebuffer, (int_64_t *)&save_checksum);

    if (save_checksum != checksum) {
      fprintf(stderr, "bad checksum: %" PRIu64 " != %" PRIu64 "\n",
        checksum, save_checksum
      );

      if (bail_on_errors)
        return false;

      if (!forced_loadgame) {
        G_LoadGameErr("Incompatible Savegame!!!\n\nAre you sure?");
        return false;
      }
      else {
        lprintf(LO_WARN, "G_DoLoadGame: Incompatible savegame\n");
      }
    }
  }

  /*-----------------*/
  /* CG: TODO: PWADs */
  /*-----------------*/

  M_PBufReadUInt(savebuffer, &packageversion);
  M_PBufReadInt(savebuffer, &compatibility_level);
  M_PBufReadInt(savebuffer, &gameskill);
  M_PBufReadInt(savebuffer, &gameepisode);
  M_PBufReadInt(savebuffer, &gamemap);
  M_PBufReadInt(savebuffer, &gametic);

  for (i = 0; i < MAXPLAYERS; i++)
    M_PBufReadBool(savebuffer, &playeringame[i]);

  // killough 2/28/98
  for (dboolean b = false, i = MAXPLAYERS; i < MIN_MAXPLAYERS; i++)
    M_PBufReadBool(savebuffer, &b);

  M_PBufReadInt(savebuffer, &idmusnum); // jff 3/17/98 restore idmus music

  /* killough 3/1/98: Read game options
   * killough 11/98: move down to here
   */
  M_PBufReadArray(savebuffer, &game_option_count);
  if (mbf_features && (game_option_count > GAME_OPTION_SIZE)) {
    I_Error("G_ReadSaveData: too many options (%d > %d)",
      game_option_count, GAME_OPTION_SIZE
    );
  }
  else if ((!mbf_features) && game_option_count > OLD_GAME_OPTION_SIZE) {
    I_Error("G_ReadSaveData: too many options (%d > %d)",
      game_option_count, OLD_GAME_OPTION_SIZE
    );
  }

  for (i = 0; i < game_option_count; i++)
    M_PBufReadUChar(savebuffer, &game_options[i]);

  G_ReadOptions(game_options);  // killough 3/1/98: Read game options

  // load a base level
  if (init_new)
    G_InitNew(gameskill, gameepisode, gamemap);

  /* get the times - killough 11/98: save entire word */
  M_PBufReadInt(savebuffer, &leveltime);

  /* cph - total episode time */
  //e6y: total level times are always saved since 2.4.8.1
  M_PBufReadInt(savebuffer, &totalleveltimes);

  // killough 11/98: load revenant tracer state
  M_PBufReadUChar(savebuffer, &tic_delta);

  basetic = gametic - tic_delta;

  // dearchive all the modifications
  P_MapStart();
  P_UnArchivePlayers(savebuffer);
  P_UnArchiveWorld(savebuffer);
  P_UnArchiveThinkers(savebuffer);
  P_UnArchiveSpecials(savebuffer);
  P_UnArchiveRNG(savebuffer);    // killough 1/18/98: load RNG information
  P_UnArchiveMap(savebuffer);    // killough 1/22/98: load automap information
  P_MapEnd();
  R_ActivateSectorInterpolations();//e6y
  R_SmoothPlaying_Reset(NULL); // e6y

  if (musinfo.current_item != -1)
    S_ChangeMusInfoMusic(musinfo.current_item, true);

  RecalculateDrawnSubsectors();

  M_PBufReadUChar(savebuffer, &safety_byte);
  if (safety_byte != 0xe6)
    I_Error("G_ReadSaveData: Bad savegame");

  G_UpdateAverageSaveSize(M_PBufGetCapacity(savebuffer));

  return true;
}

/* vi: set et ts=2 sw=2: */

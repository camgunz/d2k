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


// killough 3/7/98: modified to allow arbitrary listeners in spy mode
// killough 5/2/98: reindented, removed useless code, beautified

#include "z_zone.h"

#include "doomstat.h"
#include "d_main.h"
#include "e6y.h"
#include "i_sound.h"
#include "i_system.h"
#include "m_random.h"
#include "n_net.h"
#include "n_main.h"
#include "cl_main.h"
#include "p_cmd.h"
#include "p_ident.h"
#include "r_main.h"
#include "s_advsound.h"
#include "sounds.h"
#include "s_sound.h"
#include "sc_man.h"
#include "w_wad.h"

typedef struct {
  sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  mobj_t *origin;      // origin of sound
  uint32_t origin_id;  // sound origin ID
  int handle;          // handle of the sound being played
  int tic;             // TIC at which this sound was started
  int command_index;   // command index at which this sound was started
  int is_pickup;       // killough 4/25/98: whether sound is a player's weapon
  int pitch;
} channel_t;

typedef struct played_sound_s {
  uint32_t origin_id;
  int      sfx_id;
  bool     is_pickup;
  int      tic;
  int      command_index;
  bool     found;
} played_sound_t;

extern int numChannels;
extern int idmusnum;
extern bool mus_paused;
extern musicinfo_t *mus_playing;
extern int musicnum_current;

static GArray *channels;
static GList *played_sounds = NULL;

static void log_channel(int channel_num) {
  channel_t *c = &g_array_index(channels, channel_t, channel_num);

  D_Msg(MSG_SOUND, "%d, %s, %u/%u, %d, %d\n", 
    channel_num,
    c->sfxinfo != NULL ? c->sfxinfo->name : "(nil)",
    c->origin != NULL ? c->origin->id : 0,
    c->origin_id,
    c->tic,
    c->command_index
  );
}

static void log_played_sound(played_sound_t *ps) {
  D_Msg(MSG_SOUND, "{%d, %u, %d, %d}", 
    ps->sfx_id,
    ps->origin_id,
    ps->tic,
    ps->command_index
  );
}

static void log_played_sounds(void) {
  for (GList *node = played_sounds; node != NULL; node = node->next) {
    played_sound_t *ps = (played_sound_t *)node->data;

    log_played_sound(ps);
  }
}

static int adjust_sound_params(mobj_t *listener, mobj_t *source,
                               int *vol, int *sep, int *pitch) {
  fixed_t adx, ady, approx_dist;
  angle_t angle;

  if (!source)
    return 0;

  // e6y
  // Fix crash when the program wants to adjust_sound_params() for player
  // which is not displayplayer and displayplayer was not spawned at the moment.
  // It happens in multiplayer demos only.
  //
  // Stack trace is:
  //   - P_SetupLevel()
  //   - P_LoadThings()
  //   - P_SpawnMapThing()
  //   - P_SpawnPlayer(players[0])
  //   - P_SetupPsprites()
  //   - P_BringUpWeapon()
  //   - S_StartSound(players[0]->mo, sfx_sawup)
  //   - start_sound()
  //   - adjust_sound_params(players[displayplayer]->mo, ...);
  //   - players[displayplayer]->mo is NULL
  //
  // There is no more crash on e1cmnet3.lmp between e1m2 and e1m3
  // http://competn.doom2.net/pub/compet-n/doom/coop/movies/e1cmnet3.zip
  if (!listener)
    return 0;

  // calculate the distance to sound origin
  //  and clip it if necessary
  if (walkcamera.type > 1) {
    adx = walkcamera.x - source->x;
    ady = walkcamera.y - source->y;
  }
  else {
    adx = D_abs(listener->x - source->x);
    ady = D_abs(listener->y - source->y);
  }

  // From _GG1_ p.428. Appox. eucledian distance fast.
  approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

  if (!approx_dist) {  // killough 11/98: handle zero-distance as special case
    *sep = NORM_SEP;
    *vol = snd_SfxVolume;
    return *vol > 0;
  }

  if (approx_dist > S_CLIPPING_DIST)
    return 0;

  // angle of source to listener
  angle = R_PointToAngle2(listener->x, listener->y, source->x, source->y);

  if (angle <= listener->angle)
    angle += 0xffffffff;

  angle -= listener->angle;
  angle >>= ANGLETOFINESHIFT;

  // stereo separation
  *sep = 128 - (FixedMul(S_STEREO_SWING, finesine[angle]) >> FRACBITS);

  // volume calculation
  if (approx_dist < S_CLOSE_DIST) {
    *vol = snd_SfxVolume * 8;
  }
  else {
    // distance effect
    *vol = (snd_SfxVolume * ((S_CLIPPING_DIST - approx_dist) >> FRACBITS) * 8)
      / S_ATTENUATOR;
  }

  return (*vol > 0);
}

static int get_channel_index(channel_t *c) {
  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *c2 = &g_array_index(channels, channel_t, i);

    if (c2 == c)
      return i;
  }

  I_Error("get_channel_index: Channel not found\n");

  return -1;
}

static void stop_channel(channel_t *c) {
  if (!c->sfxinfo)
    return;

  // stop the sound playing
  if (I_SoundIsPlaying(c->handle))
    I_StopSound(c->handle);

  D_Msg(MSG_SOUND, "(%d) stopped channel, sfxinfo => NULL: ", gametic);
  log_channel(get_channel_index(c));
  D_Msg(MSG_SOUND, "\n");

  // degrade usefulness of sound data
  c->sfxinfo->usefulness--;
  c->sfxinfo = NULL;
}

static void init_channel(channel_t *c, mobj_t *mo, sfxinfo_t *sfx, int pu) {
  c->sfxinfo = sfx;
  c->tic = gametic;
  c->origin = mo;
  c->command_index = CL_GetCurrentCommandIndex();
  c->is_pickup = pu;         // killough 4/25/98
  if (mo)
    c->origin_id = mo->id;
  else
    c->origin_id = 0;
}

static bool is_duplicate(mobj_t *origin, sfxinfo_t *sfx, bool is_pickup) {
  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *c = &g_array_index(channels, channel_t, i);

    if (c->origin != NULL && origin == NULL)
      continue;

    if (c->origin == NULL && origin != NULL)
      continue;

    if (origin != NULL && c->origin_id != origin->id)
      continue;

    if (c->sfxinfo != sfx)
      continue;

    if (c->is_pickup != is_pickup)
      continue;

    if (c->command_index != CL_GetCurrentCommandIndex() &&
        c->tic != gametic) {
      continue;
    }

    return i + 1;
  }

  return 0;
}

static bool already_played(mobj_t *origin, sfxinfo_t *sfx, bool is_pickup) {
  for (GList *node = played_sounds; node != NULL; node = node->next) {
    played_sound_t *sound = (played_sound_t *)node->data;

    D_Msg(MSG_SOUND, "Checking {%td, %u, %d, %d, %d} against ",
      sfx - S_sfx,
      origin != NULL ? origin->id : 0,
      gametic,
      CL_GetCurrentCommandIndex(),
      is_pickup
    );

    log_played_sound(sound);
    D_Msg(MSG_SOUND, "(%d) \n", CL_OccurredDuringRePrediction(sound->tic));

    if (sound->origin_id == 0 && origin != NULL)
      continue;

    if (sound->origin_id != 0 && origin == NULL)
      continue;

    if (sound->origin_id != 0 && sound->origin_id != origin->id)
      continue;

    if ((S_sfx + sound->sfx_id) != sfx)
      continue;

    if (sound->is_pickup != is_pickup)
      continue;

    if (sound->tic != gametic &&
        sound->command_index != CL_GetCurrentCommandIndex()) {
      continue;
    }

    sound->found = true;

    return true;
  }

  return false;
}

static int get_channel(mobj_t *mobj, sfxinfo_t *sfxinfo, int is_pickup) {

  if (channels->len < numChannels)
    g_array_set_size(channels, numChannels);

  /*
   * CG: TODO: Truncate channel array when numChannels is decreased.  I think
   *           using numChannels instead of channels->len in the below loops
   *           would do it (eventually), but I need to think it through.
   */

  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *c = &g_array_index(channels, channel_t, i);

    if (c->sfxinfo == NULL) {
      D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel (1)\n",
        gametic, __func__
      );
      log_channel(i);
      D_Msg(MSG_SOUND, "\n");

      stop_channel(c);
      init_channel(c, mobj, sfxinfo, is_pickup);

      return i;
    }

    /*
     * CG: The channel is currently in use, but we can reuse it if it is
     *     playing a sound for this actor (actors only make a single sound at a
     *     time).
     */

    if (mobj == NULL)
      continue;

    if (c->origin_id != mobj->id)
      continue;

    if ((c->is_pickup != is_pickup) && (!comp[comp_sound]))
      continue;

    D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel (1)\n",
      gametic, __func__
    );
    log_channel(i);
    D_Msg(MSG_SOUND, "\n");

    stop_channel(c);
    init_channel(c, mobj, sfxinfo, is_pickup);

    return i;
  }

  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *c = &g_array_index(channels, channel_t, i);

    if (c->sfxinfo->priority >= sfxinfo->priority)
      continue;

    D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel (2)\n",
      gametic, __func__
    );
    log_channel(i);
    D_Msg(MSG_SOUND, "\n");

    stop_channel(c);
    init_channel(c, mobj, sfxinfo, is_pickup);

    return i;
  }

  return -1;
}

static gint compare_played_sounds(gconstpointer a, gconstpointer b) {
  played_sound_t *ps1 = (played_sound_t *)a;
  played_sound_t *ps2 = (played_sound_t *)b;

  if (ps1->tic < ps2->tic)
    return -1;

  if (ps2->tic < ps1->tic)
    return 1;

  if (ps1->command_index < ps2->command_index)
    return -1;

  if (ps2->command_index < ps1->command_index)
    return 1;

  return 0;
}

static void add_to_sound_log(uint32_t origin_id, int sfx_id, bool is_pickup,
                             int tic, int command_index) {
  played_sound_t *ps = calloc(1, sizeof(played_sound_t));

  if (ps == NULL)
    I_Error("add_to_sound_log: Error allocating new played sound");

  ps->origin_id = origin_id;
  ps->sfx_id = sfx_id;
  ps->is_pickup = is_pickup;
  ps->tic = tic;
  ps->command_index = command_index;

  played_sounds = g_list_insert_sorted(
    played_sounds, ps, compare_played_sounds
  );
}

static void init(void) {
  channels = g_array_sized_new(false, true, sizeof(channel_t), numChannels);
}

static void start_sound(mobj_t *origin, int sfx_id, int volume) {
  sfxinfo_t *sfx;
  channel_t *channel;
  int sep;
  int pitch;
  int priority;
  int cnum;
  int is_pickup;

  is_pickup = sfx_id & PICKUP_SOUND ||
              sfx_id == sfx_oof ||
              (compatibility_level >= prboom_2_compatibility &&
               sfx_id == sfx_noway); // killough 4/25/98
  sfx_id &= ~PICKUP_SOUND;

  D_Msg(MSG_SOUND, "(%d | %d) start_sound(%u, %s, %d, %d/%d/%d/%d)\n",
    gametic,
    CL_GetCurrentCommandIndex(),
    origin != NULL ? origin->id : 0,
    S_sfx[sfx_id].name,
    volume,
    CL_RunningConsoleplayerCommands(),
    CL_Synchronizing(),
    CL_RePredicting(),
    CL_Predicting()
  );

  // check for bogus sound #
  if (sfx_id < 1 || sfx_id > NUMSFX)
    I_Error("start_sound: Bad sfx #: %d", sfx_id);

  sfx = &S_sfx[sfx_id];

  if (CLIENT) {
    /* [CG] Ohhhh what a hack */

    if (CL_Predicting() && CL_RunningConsoleplayerCommands()) {
    }
    else if (CL_Synchronizing() && !CL_RunningConsoleplayerCommands()) {
      if (gametic <= CL_StateTIC()) {
        return;
      }
    }
    else if (CL_Predicting() && !CL_RunningThinkers()) {
    }
    else {
      return;
    }

    if ((CL_Synchronizing() || CL_RePredicting()) &&
        is_duplicate(origin, sfx, is_pickup)) {
      D_Msg(MSG_SOUND,
        "(%d) start_sound: skipping sound (duplicate)\n\n",
        gametic
      );

      return;
    }
    else if (origin != NULL && already_played(origin, sfx, is_pickup)) {
      D_Msg(MSG_SOUND,
        "(%d) start_sound: skipping sound (already played)\n\n",
        gametic
      );
      return;
    }
    log_played_sounds();
  }

  // Initialize sound parameters
  if (sfx->link) {
    pitch = sfx->pitch;
    priority = sfx->priority;
    volume += sfx->volume;

    if (volume < 1)
      return;

    if (volume > snd_SfxVolume)
      volume = snd_SfxVolume;
  }
  else {
    pitch = NORM_PITCH;
    priority = NORM_PRIORITY;
  }

  // Check to see if it is audible, modify the params
  // killough 3/7/98, 4/25/98: code rearranged slightly

  if (!origin || (origin == players[displayplayer].mo && walkcamera.type < 2)) {
    sep = NORM_SEP;
    volume *= 8;
  }
  else if (!adjust_sound_params(players[displayplayer].mo, origin, &volume,
                                &sep, &pitch)) {
    return;
  }
  else if (origin->x == players[displayplayer].mo->x &&
           origin->y == players[displayplayer].mo->y) {
    sep = NORM_SEP;
  }

  /*
   * CG: Not totally sure about D_RandomRange here.  I think it's necessary
   *     because differences in displayplayer between clients & server will
   *     cause volumes to be different, which will cause adjust_sound_params to
   *     exit sometimes and not others, which will desync the RNG.  On the
   *     other hand, this desyncs old demos, so you can't watch them together
   *     as a netgame.  Needs more research to resolve fully.
   */

  // hacks to vary the sfx pitches
  if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit)
    pitch += 8 - D_RandomRange(0, 15);
  else if (sfx_id != sfx_itemup && sfx_id != sfx_tink)
    pitch += 16 - D_RandomRange(0, 31);

  if (pitch < 0)
    pitch = 0;

  if (pitch > 255)
    pitch = 255;

  // kill old sound
  /*
   * CG: Can't just use silence_actor here because it doesn't handle BOOM-style
   *     pickup sounds.
   */
  if (origin) {
    for (unsigned int i = 0; i < channels->len; i++) {
      channel_t *channel = &g_array_index(channels, channel_t, i);

      if (channel->sfxinfo == NULL)
        continue;

      if (channel->origin_id != origin->id)
        continue;

      if ((channel->is_pickup != is_pickup) && (!comp[comp_sound]))
        continue;

      D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel\n", gametic, __func__);
      log_channel(i);
      D_Msg(MSG_SOUND, "\n");

      stop_channel(channel);

      break;
    }
  }

  // try to find a channel
  cnum = get_channel(origin, sfx, is_pickup);

  if (cnum < 0)
    return;

  // get lumpnum if necessary
  // killough 2/28/98: make missing sounds non-fatal
  if (sfx->lumpnum < 0 && (sfx->lumpnum = I_GetSfxLumpNum(sfx)) < 0)
    return;

  // increase the usefulness
  if (sfx->usefulness++ < 0)
    sfx->usefulness = 1;

  if (CL_RePredicting()) {
    D_Msg(MSG_SOUND, "||| (%d | %d) Repredicting sound: ",
      gametic, CL_GetCurrentCommandIndex()
    );
  }
  else {
    D_Msg(MSG_SOUND, "||| (%d | %d) Starting sound: ",
      gametic, CL_GetCurrentCommandIndex()
    );
  }
  log_channel(cnum);
  D_Msg(MSG_SOUND, "\n");

  // Assigns the handle to one of the channels in the mix/output buffer.
  // e6y: [Fix] Crash with zero-length sounds.
  int h = I_StartSound(sfx_id, cnum, volume, sep, pitch, priority);

  if (h == -1)
    return;

  channel = &g_array_index(channels, channel_t, cnum);

  channel->handle = h;
  channel->pitch = pitch;

  if (CLIENT) {
    add_to_sound_log(
      channel->origin_id,
      (int)(channel->sfxinfo - S_sfx),
      channel->is_pickup,
      gametic,
      CL_GetCurrentCommandIndex()
    );
  }
}

static void silence_actor(mobj_t *mobj) {
  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *channel = &g_array_index(channels, channel_t, i);

    if (channel->sfxinfo && channel->origin_id == mobj->id) {
      D_Msg(MSG_SOUND, "(%d) (silencing actor %u) %s: calling stop_channel\n",
        gametic, mobj->id, __func__
      );
      log_channel(i);
      D_Msg(MSG_SOUND, "\n");

      stop_channel(channel);
      break;
    }
  }
}

static void stop_sounds(void) {
  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *channel = &g_array_index(channels, channel_t, i);

    if (channel->sfxinfo) {
      D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel\n", gametic, __func__);
      log_channel(i);
      D_Msg(MSG_SOUND, "\n");

      stop_channel(channel);
    }
  }
}

static void set_music(int musicnum, bool looping) {
  musicinfo_t *music;
  int music_file_failed; // cournia - if true load the default MIDI music
  char* music_filename;  // cournia

  // current music which should play
  musicnum_current = musicnum;
  musinfo.current_item = -1;

  if (musicnum <= mus_None || musicnum >= NUMMUSIC)
    I_Error("S_ChangeMusic: Bad music number %d", musicnum);

  music = &S_music[musicnum];

  if (mus_playing == music)
    return;

  // shutdown old music
  S_StopMusic();

  // get lumpnum if neccessary
  if (!music->lumpnum) {
    char namebuf[9];
    sprintf(namebuf, "d_%s", music->name);
    music->lumpnum = W_GetNumForName(namebuf);
  }

  music_file_failed = 1;

  // proff_fs - only load when from IWAD
  if (lumpinfo[music->lumpnum].source == source_iwad) {
    // cournia - check to see if we can play a higher quality music file
    //           rather than the default MIDI
    music_filename = I_FindFile(S_music_files[musicnum], "");

    if (music_filename) {
      music_file_failed = I_RegisterMusic(music_filename, music);
      free(music_filename);
    }
  }

  if (music_file_failed) {
    //cournia - could not load music file, play default MIDI music

    // load & register it
    music->data = W_CacheLumpNum(music->lumpnum);
    music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));
  }

  // play it
  I_PlaySong(music->handle, looping);

  mus_playing = music;

  musinfo.current_item = -1;
}

static void set_musinfo_music(int lumpnum, bool looping) {
  musicinfo_t *music;

  if (doSkip) {
    musinfo.current_item = lumpnum;
    return;
  }

  if (mus_playing && mus_playing->lumpnum == lumpnum)
    return;

  music = &S_music[NUMMUSIC];

  if (music->lumpnum == lumpnum)
    return;

  // shutdown old music
  S_StopMusic();

  // save lumpnum
  music->lumpnum = lumpnum;

  // load & register it
  music->data = W_CacheLumpNum(music->lumpnum);
  music->handle = I_RegisterSong(music->data, W_LumpLength(music->lumpnum));

  // play it
  I_PlaySong(music->handle, looping);

  mus_playing = music;

  musinfo.current_item = lumpnum;
}

static void stop_music(void) {
  if (mus_playing) {
    if (mus_paused)
      I_ResumeSong(mus_playing->handle);

    I_StopSong(mus_playing->handle);
    I_UnRegisterSong(mus_playing->handle);

    if (mus_playing->lumpnum >= 0)
      W_UnlockLumpNum(mus_playing->lumpnum); // cph - release the music data

    mus_playing->data = 0;
    mus_playing = 0;
  }
}

static void restart_music(void) {
  if (musinfo.current_item != -1)
    S_ChangeMusInfoMusic(musinfo.current_item, true);
  else if (musicnum_current > mus_None && musicnum_current < NUMMUSIC)
    S_ChangeMusic(musicnum_current, true);
}

static void pause_music(void) {
  if (mus_playing && !mus_paused) {
    I_PauseSong(mus_playing->handle);
    mus_paused = true;
  }
}

static void resume_music(void) {
  if (mus_playing && mus_paused) {
    I_ResumeSong(mus_playing->handle);
    mus_paused = false;
  }
}

static void handle_level_start(void) {
  int mnum;

  // kill all playing sounds at start of level
  //  (trust me - a good idea)

  S_Stop();

  // start new music for the level
  mus_paused = 0;

  if (idmusnum != -1) {
    mnum = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
  }
  else if (gamemode == commercial) {
      mnum = mus_runnin + gamemap - 1;
  }
  else {
    static const int spmus[] = {   // Song - Who? - Where?
      mus_e3m4,     // American     e4m1
      mus_e3m2,     // Romero       e4m2
      mus_e3m3,     // Shawn        e4m3
      mus_e1m5,     // American     e4m4
      mus_e2m7,     // Tim  e4m5
      mus_e2m4,     // Romero       e4m6
      mus_e2m6,     // J.Anderson   e4m7 CHIRON.WAD
      mus_e2m5,     // Shawn        e4m8
      mus_e1m9      // Tim          e4m9
    };

    if (gameepisode < 4)
      mnum = mus_e1m1 + (gameepisode - 1) * 9 + gamemap - 1;
    else
      mnum = spmus[gamemap - 1];
  }
  S_ChangeMusic(mnum, true);
}

static void reposition_sounds(mobj_t *listener) {
  int volume;
  int pitch;
  int sep;

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return;

#ifdef UPDATE_MUSIC
  I_UpdateMusic();
#endif

  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *channel = &g_array_index(channels, channel_t, i);
    sfxinfo_t *sfx = channel->sfxinfo;

    if (!sfx)
      continue;

    // if channel is allocated but sound has stopped, free it
    if (!I_SoundIsPlaying(channel->handle)) {
      D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel (1)\n",
        gametic, __func__
      );
      log_channel(i);
      D_Msg(MSG_SOUND, "\n");

      stop_channel(channel);
      continue;
    }

    volume = snd_SfxVolume;
    pitch = channel->pitch; // use channel's pitch!
    sep = NORM_SEP;

    if (sfx->link) {
      pitch = sfx->pitch;
      volume += sfx->volume;

      if (volume < 1) {
        D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel (2)\n",
          gametic, __func__
        );
        log_channel(i);
        D_Msg(MSG_SOUND, "\n");

        stop_channel(channel);
        continue;
      }

      if (volume > snd_SfxVolume)
        volume = snd_SfxVolume;
    }

    // check non-local sounds for distance clipping or modify their params
    // killough 3/20/98
    if (channel->origin_id && listener->id != channel->origin_id) {
      mobj_t *source = P_IdentLookup(channel->origin_id);

      if (!source)
        continue;

      if (!adjust_sound_params(listener, source, &volume, &sep, &pitch)) {
        D_Msg(MSG_SOUND, "(%d) %s: calling stop_channel (3)\n",
          gametic, __func__
        );
        log_channel(i);
        D_Msg(MSG_SOUND, "\n");

        stop_channel(channel);
      }
      else {
        I_UpdateSoundParams(channel->handle, volume, sep, pitch);
      }
    }
  }
}

void S_ReloadChannelOrigins(void) {
  if (!CLIENT)
    return;

  for (unsigned int i = 0; i < channels->len; i++) {
    channel_t *c = &g_array_index(channels, channel_t, i);
    uint32_t origin_id = c->origin_id;
    mobj_t *mobj;

    if (!origin_id)
      continue;

    mobj = P_IdentLookup(origin_id);

    if (mobj) {
      c->origin = mobj;
    }
    else {
      c->origin = NULL;

      if (!I_SoundIsPlaying(c->handle))
        stop_channel(c);
    }
  }
}

void S_ResetSoundLog(void) {
  if (!CLIENT)
    return;

  for (GList *node = played_sounds; node != NULL; node = node->next) {
    played_sound_t *sound = (played_sound_t *)node->data;

    sound->found = false;
  }
}

void S_TrimSoundLog(int tic, int command_index) {
  if (!CLIENT)
    return;

  while (played_sounds != NULL) {
    played_sound_t *sound = (played_sound_t *)played_sounds->data;

    if (sound->tic >= tic || sound->command_index >= command_index)
      break;

    D_Msg(MSG_SOUND, "(%d) Removing played sound ", gametic);
    log_played_sound(sound);
    D_Msg(MSG_SOUND, "\n");

    played_sounds = g_list_delete_link(played_sounds, played_sounds);
    free(sound);
  }
}

sound_engine_t* S_GetNewSoundEngine(void) {
  sound_engine_t *se = calloc(1, sizeof(sound_engine_t));

  if (se == NULL)
    I_Error("S_GetNewSoundEngine: Error allocating new sound engine\n");

  se->init = init;
  se->start_sound = start_sound;
  se->silence_actor = silence_actor;
  se->stop_sounds = stop_sounds;
  se->set_music = set_music;
  se->set_musinfo_music = set_musinfo_music;
  se->stop_music = stop_music;
  se->restart_music = restart_music;
  se->pause_music = pause_music;
  se->resume_music = resume_music;
  se->handle_level_start = handle_level_start;
  se->reposition_sounds = reposition_sounds;

  return se;
}

/* vi: set et ts=2 sw=2: */


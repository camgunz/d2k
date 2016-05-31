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

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "e6y.h"
#include "i_sound.h"
#include "i_system.h"
#include "m_random.h"
#include "r_main.h"
#include "s_advsound.h"
#include "sounds.h"
#include "s_sound.h"
#include "sc_man.h"
#include "w_wad.h"
#include "p_setup.h"
#include "p_mobj.h"
#include "p_user.h"
#include "g_game.h"

extern bool doSkip;

extern int numChannels;
extern int idmusnum;
extern bool mus_paused;
extern musicinfo_t *mus_playing;
extern int musicnum_current;

typedef struct {
  sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  void *origin;        // origin of sound
  int handle;          // handle of the sound being played
  int is_pickup;       // killough 4/25/98: whether sound is a player's weapon
  int pitch;
} channel_t;

static channel_t *channels;      // the set of channels available

static void stop_channel(int cnum) {
  int i;
  channel_t *c = &channels[cnum];

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return;

  if (c->sfxinfo) {
    // stop the sound playing
    if (I_SoundIsPlaying(c->handle))
      I_StopSound(c->handle);

    // check to see
    //  if other channels are playing the sound
    for (i = 0; i < numChannels; i++) {
      if (cnum != i && c->sfxinfo == channels[i].sfxinfo) {
        break;
      }
    }

    // degrade usefulness of sound data
    c->sfxinfo->usefulness--;
    c->sfxinfo = 0;
  }
}

//
// get_channel:
//
// If none available, return -1.  Otherwise channel #.
//
// killough 4/25/98: made static, added is_pickup argument
//
static int get_channel(mobj_t *origin, sfxinfo_t *sfxinfo, int is_pickup) {
  int cnum; // channel number to use
  channel_t *c;

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return -1;

  // Find an open channel
  for (cnum = 0; cnum < numChannels && channels[cnum].sfxinfo; cnum++) {
    if (origin &&
        channels[cnum].origin == origin &&
        channels[cnum].is_pickup == is_pickup) {
      stop_channel(cnum);
      break;
    }
  }

  // None available
  if (cnum == numChannels) {      // Look for lower priority
    for (cnum = 0; cnum < numChannels; cnum++) {
      if (channels[cnum].sfxinfo->priority >= sfxinfo->priority) {
        break;
      }
    }

    if (cnum == numChannels)
      return -1;                  // No lower priority.  Sorry, Charlie.
    else
      stop_channel(cnum);        // Otherwise, kick out lower priority.
  }

  c = &channels[cnum];              // channel is decided to be cnum.
  c->sfxinfo = sfxinfo;
  c->origin = origin;
  c->is_pickup = is_pickup;         // killough 4/25/98

  return cnum;
}

static int adjust_sound_params(mobj_t *listener, mobj_t *source,
                               int *vol, int *sep, int *pitch) {
  fixed_t adx, ady,approx_dist;
  angle_t angle;

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return 0;

  // e6y
  // Fix crash when the program wants to adjust_sound_params() for player
  // which is not displayplayer and displayplayer was not spawned at the moment.
  // It happens in multiplayer demos only.
  //
  // Stack trace is:
  //   P_SetupLevel() -
  //   P_LoadThings() -
  //   P_SpawnMapThing() -
  //   P_SpawnPlayer(players[0]) -
  //   P_SetupPsprites() -
  //   P_BringUpWeapon() -
  //   S_StartSound(players[0]->mo, sfx_sawup) -
  //   start_sound(players[0]->mo, sfx_sawup, snd_SfxVolume) -
  //   adjust_sound_params(players[displayplayer]->mo, ...)
  //
  // players[displayplayer]->mo is NULL
  //
  // There is no more crash on e1cmnet3.lmp between e1m2 and e1m3
  // http://competn.doom2.net/pub/compet-n/doom/coop/movies/e1cmnet3.zip
  if (!listener)
    return 0;

  // calculate the distance to sound origin
  //  and clip it if necessary
  if (walkcamera.type > 1) {
    adx = D_abs(walkcamera.x - source->x);
    ady = D_abs(walkcamera.y - source->y);
  }
  else {
    adx = D_abs(listener->x - source->x);
    ady = D_abs(listener->y - source->y);
  }

  // From _GG1_ p.428. Appox. eucledian distance fast.
  approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

  if (!approx_dist) { // killough 11/98: handle zero-distance as special case
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
  *sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

  // volume calculation
  if (approx_dist < S_CLOSE_DIST) {
    *vol = snd_SfxVolume * 8;
  }
  else { // distance effect
    *vol = (
      snd_SfxVolume * ((
          S_CLIPPING_DIST - approx_dist
        ) >> FRACBITS
      ) * 8
    ) / S_ATTENUATOR;
  }

  return ((*vol) > 0);
}

static void init(void) {
  channels = calloc(numChannels, sizeof(channel_t));

  if (channels == NULL)
    I_Error("old sound engine init: error allocating channels");
}

static void start_sound(mobj_t *origin, int sfx_id, int volume) {
  int sep, pitch, priority, cnum, is_pickup, handle;
  sfxinfo_t *sfx;

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return;

  is_pickup = sfx_id & PICKUP_SOUND ||
              sfx_id == sfx_oof ||
              (compatibility_level >= prboom_2_compatibility &&
               sfx_id == sfx_noway); // killough 4/25/98
  sfx_id &= ~PICKUP_SOUND;

  // check for bogus sound #
  if (sfx_id < 1 || sfx_id > NUMSFX)
    I_Error("S_StartSoundAtVolume: Bad sfx #: %d", sfx_id);

  sfx = &S_sfx[sfx_id];

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
  else if (!adjust_sound_params(players[displayplayer].mo,
                                origin, &volume, &sep, &pitch)) {
      return;
  }
  else if (origin->x == players[displayplayer].mo->x &&
           origin->y == players[displayplayer].mo->y) {
    sep = NORM_SEP;
  }

  // hacks to vary the sfx pitches
  if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit) {
    pitch += 8 - (M_Random() & 15);
  }
  else if (sfx_id != sfx_itemup && sfx_id != sfx_tink) {
    pitch += 16 - (M_Random() & 31);
  }

  if (pitch < 0)
    pitch = 0;

  if (pitch > 255)
    pitch = 255;

  // kill old sound
  for (cnum = 0; cnum < numChannels; cnum++) {
    if (channels[cnum].sfxinfo &&
        channels[cnum].origin == origin &&
        (comp[comp_sound] ||
         channels[cnum].is_pickup == is_pickup)) {
      stop_channel(cnum);
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

  // Assigns the handle to one of the channels in the mix/output buffer.
  // e6y: [Fix] Crash with zero-length sounds.
  handle = I_StartSound(sfx_id, cnum, volume, sep, pitch, priority);
  if (handle != -1) {
    channels[cnum].handle = handle;
    channels[cnum].pitch = pitch;
  }
}

static void silence_actor(mobj_t *origin) {
  int cnum;

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return;

  for (cnum = 0; cnum < numChannels; cnum++) {
    if (channels[cnum].sfxinfo && channels[cnum].origin == origin) {
      stop_channel(cnum);
      break;
    }
  }
}

static void stop_sounds(void) {
  int cnum;

  //jff 1/22/98 skip sound init if sound not enabled
  if (SOUND_DISABLED)
    return;

  for (cnum = 0; cnum < numChannels; cnum++) {
    if (channels[cnum].sfxinfo)
      stop_channel(cnum);
  }
}

static void set_music(int musicnum, bool looping) {
  musicinfo_t *music;
  int music_file_failed; // cournia - if true load the default MIDI music
  char *music_filename;  // cournia

  // current music which should play
  musicnum_current = musicnum;
  musinfo.current_item = -1;

  //jff 1/22/98 return if music is not enabled
  if (MUSIC_DISABLED)
    return;

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

  //jff 1/22/98 return if music is not enabled
  if (MUSIC_DISABLED)
    return;

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
  //jff 1/22/98 return if music is not enabled
  if (MUSIC_DISABLED)
    return;

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
  //jff 1/22/98 return if music is not enabled
  if (MUSIC_DISABLED)
    return;

  if (mus_playing && !mus_paused) {
    I_PauseSong(mus_playing->handle);
    mus_paused = true;
  }
}

static void resume_music(void) {
  //jff 1/22/98 return if music is not enabled
  if (MUSIC_DISABLED)
    return;

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
    static const int spmus[] = { // Song - Who? - Where?
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
      mnum = mus_e1m1 + (gameepisode - 1)* 9 + gamemap - 1;
    else
      mnum = spmus[gamemap - 1];
  }

  S_ChangeMusic(mnum, true);
}

static void reposition_sounds(mobj_t *listener) {
  int cnum;

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return;

#ifdef UPDATE_MUSIC
  I_UpdateMusic();
#endif

  for (cnum = 0; cnum < numChannels; cnum++) {
    sfxinfo_t *sfx;
    channel_t *c = &channels[cnum];

    if ((sfx = c->sfxinfo))
    {
      if (I_SoundIsPlaying(c->handle))
      {
        // initialize parameters
        int volume = snd_SfxVolume;
        int pitch = c->pitch; // use channel's pitch!
        int sep = NORM_SEP;

        if (sfx->link)
        {
          pitch = sfx->pitch;
          volume += sfx->volume;
          if (volume < 1)
            {
              stop_channel(cnum);
              continue;
            }
          else
            if (volume > snd_SfxVolume)
              volume = snd_SfxVolume;
        }

        // check non-local sounds for distance clipping
        // or modify their params
        if (c->origin && listener != c->origin) { // killough 3/20/98
          if (!adjust_sound_params(listener, c->origin,
                                   &volume, &sep, &pitch))
            stop_channel(cnum);
          else
            I_UpdateSoundParams(c->handle, volume, sep, pitch);
        }
      }
      else { // if channel is allocated but sound has stopped, free it
        stop_channel(cnum);
      }
    }
  }
}

sound_engine_t* S_GetOldSoundEngine(void) {
  sound_engine_t *se = calloc(1, sizeof(sound_engine_t));

  if (se == NULL)
    I_Error("S_GetOldSoundEngine: Error allocating new sound engine\n");

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


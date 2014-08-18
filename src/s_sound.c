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
#include "s_sound.h"
#include "s_advsound.h"
#include "i_sound.h"
#include "i_system.h"
#include "d_main.h"
#include "r_main.h"
#include "m_random.h"
#include "w_wad.h"
#include "lprintf.h"
#include "sc_man.h"
#include "p_ident.h"
#include "e6y.h"

#include "n_net.h"
#include "n_main.h"

// when to clip out sounds
// Does not fit the large outdoor areas.
#define S_CLIPPING_DIST (1200 << FRACBITS)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).

#define S_CLOSE_DIST (160 << FRACBITS)
#define S_ATTENUATOR ((S_CLIPPING_DIST - S_CLOSE_DIST) >> FRACBITS)

// Adjustable by menu.
#define NORM_PITCH 128
#define NORM_PRIORITY 64
#define NORM_SEP 128
#define S_STEREO_SWING (96 << FRACBITS)

const char* S_music_files[NUMMUSIC]; // cournia - stores music file names

typedef struct {
  sfxinfo_t *sfxinfo;  // sound information (if null, channel avail.)
  mobj_t *origin;      // origin of sound
  uint32_t origin_id;  // sound origin ID
  int handle;          // handle of the sound being played
  int is_pickup;       // killough 4/25/98: whether sound is a player's weapon
  int pitch;
} channel_t;

static int saved_sfx_volume;

// whether songs are mus_paused
static dboolean mus_paused;

// music currently being played
static musicinfo_t *mus_playing;

// music currently should play
static int musicnum_current;

// the set of channels available
static cbuf_t *channels;

// These are not used, but should be (menu).
// Maximum volume of a sound effect.
// Internal default is max out of 0-15.
int snd_SfxVolume = 15;

// Maximum volume of music. Useless so far.
int snd_MusicVolume = 15;

// following is set
//  by the defaults code in M_misc:
// number of channels available
int default_numChannels;
int numChannels;

//jff 3/17/98 to keep track of last IDMUS specified music num
int idmusnum;

//
// Internals.
//

//
// adjust_sound_params
//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
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
  // P_SetupLevel() - P_LoadThings() - P_SpawnMapThing() \ P_SpawnPlayer(players[0]) -
  // P_SetupPsprites() - P_BringUpWeapon() - S_StartSound(players[0]->mo, sfx_sawup) -
  // start_sound_at_volume() - adjust_sound_params(players[displayplayer]->mo, ...);
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

static void stop_channel(channel_t *c) {
  if (!c->sfxinfo)
    return;

  if (CL_RePredicting())
    return;

  // stop the sound playing
  if (I_SoundIsPlaying(c->handle))
    I_StopSound(c->handle);

  // degrade usefulness of sound data
  c->sfxinfo->usefulness--;
  c->sfxinfo = NULL;
}

static void init_channel(channel_t *c, mobj_t *mo, sfxinfo_t *sfx, int pu) {
  c->sfxinfo = sfx;
  c->origin = mo;
  if (mo)
    c->origin_id = mo->id;
  else
    c->origin_id = 0;
  c->is_pickup = pu;         // killough 4/25/98
}

//
// get_channel :
//   If none available, return -1.  Otherwise channel #.
//
// killough 4/25/98: made static, added is_pickup argument
//
static int get_channel(mobj_t *mobj, sfxinfo_t *sfxinfo, int is_pickup) {
  int channel_count = M_CBufGetObjectCount(channels);

  if (channel_count < numChannels) {
    channel_t dummy;

    init_channel(&dummy, mobj, sfxinfo, is_pickup);
    M_CBufConsolidate(channels);
    M_CBufAppend(channels, &dummy);

    return channel_count;
  }

  CBUF_FOR_EACH(channels, entry) {
    channel_t *c = (channel_t *)entry.obj;

    if ((c->sfxinfo == NULL) ||
        ((mobj && c->origin_id == mobj->id) && c->is_pickup == is_pickup)) {
      stop_channel(c);
      init_channel(c, mobj, sfxinfo, is_pickup);
      return entry.index;
    }
  }

  CBUF_FOR_EACH(channels, entry) {
    channel_t *c = (channel_t *)entry.obj;

    if (c->sfxinfo->priority < sfxinfo->priority) {
      stop_channel(c);
      init_channel(c, mobj, sfxinfo, is_pickup);
      return entry.index;
    }
  }

  return -1;
}

static void start_sound_at_volume(mobj_t *origin, int sfx_id, int volume) {
  int sep, pitch, priority, cnum, is_pickup;
  sfxinfo_t *sfx;
  channel_t *channel;

  //jff 1/22/98 return if sound is not enabled
  is_pickup = sfx_id & PICKUP_SOUND ||
              sfx_id == sfx_oof ||
              (compatibility_level >= prboom_2_compatibility &&
               sfx_id == sfx_noway); // killough 4/25/98
  sfx_id &= ~PICKUP_SOUND;

  // check for bogus sound #
  if (sfx_id < 1 || sfx_id > NUMSFX)
    I_Error("start_sound_at_volume: Bad sfx #: %d", sfx_id);

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
  if (sfx_id >= sfx_sawup && sfx_id <= sfx_sawhit) {
    if (DELTASYNC)
      pitch += 8 - D_RandomRange(0, 15);
    else
      pitch += 8 - (M_Random() & 15);
  }
  else if (sfx_id != sfx_itemup && sfx_id != sfx_tink) {
    if (DELTASYNC)
      pitch += 16 - D_RandomRange(0, 31);
    else
      pitch += 16 - (M_Random() & 31);
  }

  if (pitch < 0)
    pitch = 0;

  if (pitch > 255)
    pitch = 255;

  // kill old sound
  CBUF_FOR_EACH(channels, entry) {
    channel = (channel_t *)entry.obj;

    if (!channel->sfxinfo)
      continue;

    if (origin == NULL && channel->origin == NULL) {
      stop_channel(channel);
      break;
    }

    if (!origin)
      continue;

    if (channel->origin_id != origin->id)
      continue;

    if (comp[comp_sound] || channel->is_pickup == is_pickup) {
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

  // Assigns the handle to one of the channels in the mix/output buffer.
  // e6y: [Fix] Crash with zero-length sounds.
  int h = I_StartSound(sfx_id, cnum, volume, sep, pitch, priority);

  if (h != -1) {
    channel = M_CBufGet(channels, cnum);

    if (!channel)
      return;

    channel->handle = h;
    channel->pitch = pitch;
  }
}

//
// S_Init
//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume) {
  idmusnum = -1; //jff 3/17/98 insure idmus number is blank

  //jff 1/22/98 skip sound init if sound not enabled
  numChannels = default_numChannels;

  lprintf(LO_CONFIRM, "S_Init: default sfx volume %d\n", sfxVolume);

  // Whatever these did with DMX, these are rather dummies now.
  I_SetChannels();

  S_SetSfxVolume(sfxVolume);

  // Allocating the internal channels for mixing
  // (the maximum numer of sounds rendered
  // simultaneously) within zone memory.
  // CPhipps - calloc
  channels = M_CBufNewWithCapacity(numChannels, sizeof(channel_t));

  // Note that sounds have not been cached (yet).
  for (int i = 1; i < NUMSFX; i++)
    S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;

  S_SetMusicVolume(musicVolume);

  // no sounds are playing, and they are not mus_paused
  mus_paused = 0;
}

void S_Stop(void) {
  //jff 1/22/98 skip sound init if sound not enabled
  CBUF_FOR_EACH(channels, entry) {
    channel_t *channel = (channel_t *)entry.obj;

    if (channel->sfxinfo)
      stop_channel(channel);
  }
}

//
// S_Start
//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void) {
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
      mnum = mus_e1m1 + (gameepisode-1)*9 + gamemap-1;
    else
      mnum = spmus[gamemap-1];
  }
  S_ChangeMusic(mnum, true);
}

void S_StartSound(mobj_t *mobj, int sfx_id) {
  start_sound_at_volume(mobj, sfx_id, snd_SfxVolume);
}

void S_StopSound(mobj_t *mobj) {
  CBUF_FOR_EACH(channels, entry) {
    channel_t *channel = (channel_t *)entry.obj;

    if (channel->sfxinfo && channel->origin_id == mobj->id) {
      stop_channel(channel);
      break;
    }
  }
}

//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void) {
  if (mus_playing && !mus_paused) {
    I_PauseSong(mus_playing->handle);
    mus_paused = true;
  }
}

void S_ResumeSound(void) {
  if (mus_playing && mus_paused) {
    I_ResumeSong(mus_playing->handle);
    mus_paused = false;
  }
}

//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t *listener) {
  int volume;
  int pitch;
  int sep;

  //jff 1/22/98 return if sound is not enabled
  if (SOUND_DISABLED)
    return;

#ifdef UPDATE_MUSIC
  I_UpdateMusic();
#endif

  CBUF_FOR_EACH(channels, entry) {
    channel_t *channel = (channel_t *)entry.obj;
    sfxinfo_t *sfx = channel->sfxinfo;

    if (!sfx)
      continue;

    if (!I_SoundIsPlaying(channel->handle)) {
      // if channel is allocated but sound has stopped, free it
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
        stop_channel(channel);
        continue;
      }
      else if (volume > snd_SfxVolume) {
        volume = snd_SfxVolume;
      }
    }

    // check non-local sounds for distance clipping or modify their params
    // killough 3/20/98
    if (channel->origin_id && listener->id != channel->origin_id) {
      mobj_t *source = P_IdentLookup(channel->origin_id);

      if (!adjust_sound_params(listener, source, &volume, &sep, &pitch))
        stop_channel(channel);
      else
        I_UpdateSoundParams(channel->handle, volume, sep, pitch);
    }
  }
}

void S_SetMusicVolume(int volume) {
  if (volume < 0 || volume > 15)
    I_Error("S_SetMusicVolume: Attempt to set music volume at %d", volume);

  I_SetMusicVolume(volume);
  snd_MusicVolume = volume;
}

void S_SetSfxVolume(int volume) {
  if (volume < 0 || volume > 127)
    I_Error("S_SetSfxVolume: Attempt to set sfx volume at %d", volume);

  snd_SfxVolume = volume;
  saved_sfx_volume = snd_SfxVolume;
}

// Starts some music with the music id found in sounds.h.
void S_StartMusic(int m_id) {
  S_ChangeMusic(m_id, false);
}

void S_ChangeMusic(int musicnum, int looping) {
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

void S_RestartMusic(void) {
  if (musinfo.current_item != -1)
    S_ChangeMusInfoMusic(musinfo.current_item, true);
  else if (musicnum_current > mus_None && musicnum_current < NUMMUSIC)
    S_ChangeMusic(musicnum_current, true);
}

void S_ChangeMusInfoMusic(int lumpnum, int looping) {
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

void S_StopMusic(void) {
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

void S_ReloadChannelOrigins(void) {
  CBUF_FOR_EACH(channels, entry) {
    channel_t *c = (channel_t *)entry.obj;
    uint32_t origin_id = c->origin_id;

    if (origin_id) {
      mobj_t *mobj = P_IdentLookup(origin_id);

      if (mobj) {
        c->origin = mobj;
      }
      else {
        c->origin = NULL;
        c->origin_id = 0;
        stop_channel(c);
      }
    }
  }
}

void S_MuteSound(void) {
  saved_sfx_volume = snd_SfxVolume;
  snd_SfxVolume = 0;
}

void S_UnMuteSound(void) {
  snd_SfxVolume = saved_sfx_volume;
}

/* vi: set et ts=2 sw=2: */


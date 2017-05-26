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

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_mutex.h>

#if SDL_VERSION_ATLEAST(1, 3, 0)
#include <SDL_endian.h>
#else
#include <SDL_byteorder.h>
#endif

#include <SDL_version.h>
#include <SDL_thread.h>
#ifdef HAVE_MIXER
#define USE_RWOPS
#include <SDL_mixer.h>
#endif

#include "i_pcsound.h"
#include "i_sound.h"

#include "d_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"

extern int gametic;

bool sound_inited_once = false;

// from pcsound_sdl.c
void PCSound_Mix_Callback(void *udata, Uint8 *stream, int len);

#ifndef HAVE_OWN_MUSIC
void Exp_UpdateMusic(void *buff, unsigned nsamp);
#endif

typedef struct {
  int id;                       // SFX id of the playing sound effect.  Used to
                                // catch duplicates (like chainsaw).
  unsigned int step;            // The channel step amount...
  unsigned int stepremainder;   // ... and a 0.16 bit remainder of last step.
  unsigned int samplerate;
  const unsigned char *data;    // The channel data pointers, start...
  const unsigned char *enddata; // ...and end.
  int starttime;                // Time/gametic that the channel started
                                // playing, used to determine oldest, which
                                // automatically has lowest priority. In case
                                // number of active sounds exceeds available channels.
  int leftvol;                  // left channel volume (0-127)
  int rightvol;                 // right channel volume (0-127)
} channel_info_t;

static int SAMPLECOUNT =   512; // Needed for calling the actual sound output.
static bool sound_inited = false;
static bool first_sound_init = true;
static int dumping_sound = 0; // NSM

int snd_pcspeaker;
int snd_card = 1;           // Variables used by Boom from Allegro
int mus_card = 1;           // created here to avoid changes to core Boom files
int detect_voices = 0;      // God knows
int snd_samplerate = 11025; // MWM 2000-01-08: Sample rate in samples/second
int audio_fd;               // The actual output device.
int steptable[256];         // Pitch to stepping lookup, unused.
SDL_mutex *sfxmutex;        // lock for updating any params related to sfx
SDL_mutex *musmutex;        // lock for updating any params related to music

/* CG TODO: Use a GArray */
channel_info_t channelinfo[MAX_SOUND_CHANNELS];

#ifndef HAVE_OWN_MUSIC
int use_experimental_music = -1;
#endif

// int vol_lookup[128 * 256]; // Volume lookups.

//
// cph
// stop_chan
//
// Stops a sound, unlocks the data
//
static void stop_chan(int i) {
  if (SOUND_DISABLED) {
    return;
  }

  if (channelinfo[i].data) { /* cph - prevent excess unlocks */
    channelinfo[i].data = NULL;
    W_UnlockLumpNum(S_sfx[channelinfo[i].id].lumpnum);
  }
}

//
// add_sfx
//
// This function adds a sound to the list of currently active sounds, which is
// maintained as a given number (eight, usually) of internal channels.
//
// Returns a handle.
//
static int add_sfx(int sfxid, int channel, const unsigned char *data,
                                           size_t len) {
  if (SOUND_DISABLED) {
    return 0;
  }

  stop_chan(channel);

  channelinfo[channel].data = data;
  /* Set pointer to end of raw data. */
  channelinfo[channel].enddata = channelinfo[channel].data + len - 1;
  channelinfo[channel].samplerate =
    (channelinfo[channel].data[3] << 8) + channelinfo[channel].data[2];
  channelinfo[channel].data += 8; /* Skip header */

  channelinfo[channel].stepremainder = 0;
  // Should be gametic, I presume.
  channelinfo[channel].starttime = gametic;

  // Preserve sound SFX id,
  //  e.g. for avoiding duplicates of chainsaw.
  channelinfo[channel].id = sfxid;

  return channel;
}

static void update_sound_params(int handle, int volume, int separation,
                                                        int pitch) {
  int slot = handle;
  int rightvol;
  int leftvol;
  int step = steptable[pitch];

#ifdef RANGECHECK
  if ((handle < 0) || (handle >= MAX_SOUND_CHANNELS)) {
    I_Error("update_sound_params: handle out of range");
  }
#endif

  if (SOUND_DISABLED) {
    return;
  }

  if (snd_pcspeaker) {
    return;
  }

  // Set stepping
  // MWM 2000-12-24: Calculates proportion of channel samplerate to global
  // samplerate for mixing purposes.
  // Patched to shift left *then* divide, to minimize roundoff errors as well
  // as to use SAMPLERATE as defined above, not to assume 11025 Hz
  if (pitched_sounds) {
    channelinfo[slot].step = step + (
      ((channelinfo[slot].samplerate << 16) / snd_samplerate) - 65536
    );
  }
  else {
    channelinfo[slot].step = (
      (channelinfo[slot].samplerate << 16) / snd_samplerate
    );
  }

  // Separation, that is, orientation/stereo.
  //  range is: 1 - 256
  separation += 1;

  // Per left/right channel.
  //  x^2 separation,
  //  adjust volume properly.
  leftvol = volume - ((volume * separation * separation) >> 16);
  separation = separation - 257;
  rightvol = volume - ((volume * separation * separation) >> 16);

  // Sanity check, clamp volume.
  if (rightvol < 0 || rightvol > 127) {
    I_Error("rightvol out of bounds");
  }

  if (leftvol < 0 || leftvol > 127) {
    I_Error("leftvol out of bounds");
  }

  // Get the proper lookup table piece for this volume level???
  channelinfo[slot].leftvol = leftvol;
  channelinfo[slot].rightvol = rightvol;
}

//
// This function loops all active (internal) sound channels, retrieves a given
// number of samples from the raw sound data, modifies it according to the
// current (internal) channel parameters, mixes the per channel samples into
// the given mixing buffer, and clamping it to the allowed range.
//
// This function currently supports only 16bit.
//
static void update_sound(void *unused, Uint8 *stream, int len) {
  // Mix current sound data.
  // Data, from raw sound, for right and left.
  // register unsigned char sample;
  register int    dl;
  register int    dr;

  // Pointers in audio stream, left, right, end.
  signed short   *leftout;
  signed short   *rightout;
  signed short   *leftend;
  // Step in stream, left and right, thus two.
  int       step;

  // Mixing channel index.
  int       chan;

  // Number of channels.
  int channel_count;

  if (SOUND_DISABLED) {
    return;
  }

  // NSM: when dumping sound, ignore the callback calls and only
  // service dumping calls
  if (dumping_sound && unused != (void *)0xDEADBEEF) {
    return;
  }

#ifndef HAVE_OWN_MUSIC
  // do music update
  if (use_experimental_music) {
    SDL_LockMutex(musmutex);
    Exp_UpdateMusic(stream, len / 4);
    SDL_UnlockMutex(musmutex);
  }
#endif

  if (snd_pcspeaker) {
    PCSound_Mix_Callback(NULL, stream, len);
    // no sfx mixing
    return;
  }

  SDL_LockMutex(sfxmutex);
  // Left and right channel
  //  are in audio stream, alternating.
  leftout = (signed short *)stream;
  rightout = ((signed short *)stream) + 1;
  step = 2;

  // Determine end, for left channel only
  //  (right channel is implicit).
  leftend = leftout + (len / 4) * step;

  // Mix sounds into the mixing buffer.
  // Loop over step*SAMPLECOUNT,
  //  that is 512 values for two channels.
  while (leftout != leftend) {
    // Reset left/right value.
    //dl = 0;
    //dr = 0;
    dl = *leftout;
    dr = *rightout;

    // Love thy L2 cache - made this a loop.
    // Now more channels could be set at compile time
    //  as well. Thus loop those channels.
    channel_count = S_GetChannelCount();
    for (chan = 0; chan < channel_count; chan++) {
      // Check channel, if active.
      if (channelinfo[chan].data) {
        // Get the raw data from the channel.
        // no filtering
        //int s = channelinfo[chan].data[0] * 0x10000 - 0x800000;

        // linear filtering the old SRC did linear interpolation back into 8
        // bit, and then expanded to 16 bit.  this does interpolation and 8->16
        // at same time, allowing slightly higher quality

        // convert to signed
        int s = 
          ((unsigned int)channelinfo[chan].data[0] *
           (0x10000 - channelinfo[chan].stepremainder)) +
          ((unsigned int)channelinfo[chan].data[1] *
           (channelinfo[chan].stepremainder)) - 0x800000;


        // Add left and right part
        //  for this channel (sound)
        //  to the current data.
        // Adjust volume accordingly.

        // full loudness (vol=127) is actually 127/191

        dl += channelinfo[chan].leftvol * s / 49152;  // >> 15;
        dr += channelinfo[chan].rightvol * s / 49152; // >> 15;

        // Increment index ???
        channelinfo[chan].stepremainder += channelinfo[chan].step;
        // MSB is next sample???
        channelinfo[chan].data += channelinfo[chan].stepremainder >> 16;
        // Limit to LSB???
        channelinfo[chan].stepremainder &= 0xffff;

        // Check whether we are done.
        if (channelinfo[chan].data >= channelinfo[chan].enddata)
          stop_chan(chan);
      }
    }

    // Clamp to range. Left hardware channel.
    // Has been char instead of short.
    // if (dl > 127) *leftout = 127;
    // else if (dl < -128) *leftout = -128;
    // else *leftout = dl;

    if (dl > SHRT_MAX) {
      *leftout = SHRT_MAX;
    }
    else if (dl < SHRT_MIN) {
      *leftout = SHRT_MIN;
    }
    else {
      *leftout = (signed short)dl;
    }

    // Same for right hardware channel.
    if (dr > SHRT_MAX) {
      *rightout = SHRT_MAX;
    }
    else if (dr < SHRT_MIN) {
      *rightout = SHRT_MIN;
    }
    else {
      *rightout = (signed short)dr;
    }

    // Increment current pointers in stream
    leftout += step;
    rightout += step;
  }
  SDL_UnlockMutex(sfxmutex);
}

void I_UpdateSoundParams(int handle, int volume, int separation, int pitch) {
  if (SOUND_DISABLED) {
    return;
  }

  SDL_LockMutex(sfxmutex);
  update_sound_params(handle, volume, separation, pitch);
  SDL_UnlockMutex(sfxmutex);
}

//
// SFX API
//
// Note: this was called by S_Init.  However, whatever they did in the old DPMS
// based DOS version, this were simply dummies in the Linux version.
//
// See soundserver initdata().
//

//
// I_SetChannels
//
// Init internal lookups (raw data, mixing buffer, channels).  This function
// sets up internal lookups used during the mixing process.
//
void I_SetChannels(void) {
  int i;
  int *steptablemid = steptable + 128;

  // Okay, reset internal mixing channels to zero.
  for (i = 0; i < MAX_SOUND_CHANNELS; i++) {
    memset(&channelinfo[i], 0, sizeof(channel_info_t));
  }

  // This table provides step widths for pitch parameters.
  // I fail to see that this is currently used.
  for (i = -128; i < 128; i++) {
    steptablemid[i] = (int)(pow(
      1.2, ((double)i / (64.0 * snd_samplerate / 11025))
    ) * 65536.0);
  }


  // Generates volume lookup tables
  //  which also turn the unsigned samples
  //  into signed samples.
  /*
  for (i = 0 ; i < 128 ; i++)
    for (j = 0 ; j < 256 ; j++)
    {
      // proff - made this a little bit softer, because with
      // full volume the sound clipped badly
      vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 191;
      //vol_lookup[i*256+j] = (i*(j-128)*256)/127;
    }
  */
}

//
// I_GetSfxLumpNum
//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t *sfx) {
  char namebuf[9];
  const char *prefix = (snd_pcspeaker ? "dp" : "ds");

  // Different prefix for PC speaker sound effects.
  snprintf(namebuf, 9, "%s%s", prefix, sfx->name);

  return W_SafeGetNumForName(namebuf); //e6y: make missing sounds non-fatal
}

//
// Starting a sound means adding it to the current list of active sounds in the
// internal channels.
//
// As the SFX info struct contains e.g. a pointer to the raw data, it is
// ignored.
//
// As our sound handling does not handle priority, it is ignored.
//
// Pitching (that is, increased speed of playback) is set, but currently not
// used by mixing.
//
int I_StartSound(int id, int channel, int vol, int sep, int pitch,
                                                        int priority) {
  const unsigned char *data;
  int lump;
  size_t len;

  if ((channel < 0) || (channel >= MAX_SOUND_CHANNELS)) {
#ifdef RANGECHECK
    I_Error("I_StartSound: handle out of range");
#else
    return -1;
#endif
  }

  if (SOUND_DISABLED) {
    return 0;
  }

  if (snd_pcspeaker) {
    return I_PCS_StartSound(id, channel, vol, sep, pitch, priority);
  }

  lump = S_sfx[id].lumpnum;

  // We will handle the new SFX.
  // Set pointer to raw data.
  len = W_LumpLength(lump);

  // e6y: Crash with zero-length sounds.
  // Example wad: dakills (http://www.doomworld.com/idgames/index.php?id=2803)
  // The entries DSBSPWLK, DSBSPACT, DSSWTCHN and DSSWTCHX are all zero-length sounds
  if (len <= 8) {
    return -1;
  }

  /* Find padded length */
  len -= 8;
  // do the lump caching outside the SDL_LockAudio/SDL_UnlockAudio pair
  // use locking which makes sure the sound data is in a malloced area and
  // not in a memory mapped one
  data = W_LockLumpNum(lump);

  SDL_LockMutex(sfxmutex);
  add_sfx(id, channel, data, len); // Returns a handle (not used).
  update_sound_params(channel, vol, sep, pitch);
  SDL_UnlockMutex(sfxmutex);

  return channel;
}

void I_StopSound (int handle) {
#ifdef RANGECHECK
  if ((handle < 0) || (handle >= MAX_SOUND_CHANNELS)) {
    I_Error("I_StopSound: handle out of range");
  }
#endif

  if (SOUND_DISABLED) {
    return;
  }

  if (snd_pcspeaker) {
    I_PCS_StopSound(handle);
    return;
  }

  SDL_LockMutex(sfxmutex);
  stop_chan(handle);
  SDL_UnlockMutex(sfxmutex);
}

bool I_SoundIsPlaying(int handle) {
#ifdef RANGECHECK
  if ((handle < 0) || (handle >= MAX_SOUND_CHANNELS)) {
    I_Error("I_SoundIsPlaying: handle out of range");
  }
#endif

  if (SOUND_DISABLED) {
    return false;
  }

  if (snd_pcspeaker) {
    return I_PCS_SoundIsPlaying(handle);
  }

  return channelinfo[handle].data != NULL;
}

bool I_AnySoundStillPlaying(void) {
  int i;

  if (SOUND_DISABLED) {
    return false;
  }

  if (snd_pcspeaker) {
    return false;
  }

  for (i = 0; i < MAX_SOUND_CHANNELS; i++) {
    if (channelinfo[i].data != NULL) {
      return true;
    }
  }

  return false;
}

void I_ShutdownSound(void) {
  if (SOUND_DISABLED) {
    return;
  }

  if (sound_inited) {
    D_MsgLocalInfo("I_ShutdownSound: ");
#ifdef HAVE_MIXER
    Mix_CloseAudio();
#endif
    SDL_CloseAudio();
    D_MsgLocalInfo("\n");
    sound_inited = false;

    if (sfxmutex) {
      SDL_DestroyMutex(sfxmutex);
      sfxmutex = NULL;
    }
  }
}

void I_InitSound(void) {
  int audio_rate;
  int audio_channels;
  int audio_buffers;
  int res;
  SDL_AudioSpec audio;

  if (SOUND_DISABLED) {
    return;
  }

  // haleyjd: the docs say we should do this
  if (SDL_InitSubSystem(SDL_INIT_AUDIO)) {
    D_MsgLocalInfo("Couldn't initialize SDL audio (%s))\n", SDL_GetError());
    nosfxparm = true;
    nomusicparm = true;
    return;
  }

  if (sound_inited) {
    I_ShutdownSound();
  }

  // Secure and configure sound device first.
  D_MsgLocalInfo("I_InitSound: ");

  if (!use_experimental_music) {
#ifdef HAVE_MIXER

    /* Initialize variables */
    audio_rate = snd_samplerate;
    audio_channels = 2;
    SAMPLECOUNT = 512;
    audio_buffers = SAMPLECOUNT*snd_samplerate/11025;

    res = Mix_OpenAudio(
      audio_rate, MIX_DEFAULT_FORMAT, audio_channels, audio_buffers
    );
    
    if (res < 0) {
      D_MsgLocalInfo("couldn't open audio with desired format (%s)\n",
        SDL_GetError()
      );
      nosfxparm = true;
      nomusicparm = true;

      return;
    }

    sound_inited_once = true;//e6y
    sound_inited = true;
    SAMPLECOUNT = audio_buffers;
    Mix_SetPostMix(update_sound, NULL);
    D_MsgLocalInfo(" configured audio device with %d samples/slice\n",
      SAMPLECOUNT
    );
  }
  else
#else // HAVE_MIXER
  }
#endif // HAVE_MIXER
  {
    // Open the audio device
    audio.freq = snd_samplerate;
#if ( SDL_BYTEORDER == SDL_BIG_ENDIAN )
    audio.format = AUDIO_S16MSB;
#else
    audio.format = AUDIO_S16LSB;
#endif
    audio.channels = 2;
    audio.samples = SAMPLECOUNT * snd_samplerate / 11025;
    audio.callback = update_sound;

    if (SDL_OpenAudio(&audio, NULL) < 0) {
      D_MsgLocalInfo("couldn't open audio with desired format (%s))\n",
        SDL_GetError()
      );
      nosfxparm = true;
      nomusicparm = true;

      return;
    }

    sound_inited_once = true;//e6y
    sound_inited = true;
    SAMPLECOUNT = audio.samples;
    D_MsgLocalInfo(" configured audio device with %d samples/slice\n",
      SAMPLECOUNT
    );
  }

  if (first_sound_init) {
    atexit(I_ShutdownSound);
    first_sound_init = false;
  }

  sfxmutex = SDL_CreateMutex();

  // If we are using the PC speaker, we now need to initialise it.
  if (snd_pcspeaker) {
    I_PCS_InitSound();
  }

  if (!nomusicparm) {
    I_InitMusic();
  }

  // Finished initialization.
  D_MsgLocalInfo("I_InitSound: sound module ready\n");
  SDL_PauseAudio(0);
}


// NSM sound capture routines

// silences sound output, and instead allows sound capture to work
// call this before sound startup
void I_SetSoundCap(void) {
  dumping_sound = 1;
}

// grabs len samples of audio (16 bit interleaved)
unsigned char* I_GrabSound (int len) {
  static unsigned char *buffer = NULL;
  static size_t buffer_size = 0;

  size_t size;

  if (SOUND_DISABLED) {
    return false;
  }

  if (!dumping_sound) {
    return NULL;
  }

  size = len * 4;
  if (!buffer || size > buffer_size) {
    buffer_size = size * 4;
    buffer = realloc(buffer, buffer_size);
  }

  if (buffer) {
    memset (buffer, 0, size);
    update_sound((void *)0xDEADBEEF, buffer, size);
  }

  return buffer;
}

// NSM helper routine for some of the streaming audio
// assumes 16 bit signed interleaved stereo
void I_ResampleStream(void *dest, unsigned nsamp,
                      void (*proc) (void *dest, unsigned nsamp),
                      unsigned sratein, unsigned srateout) {
  static short *sin = NULL;
  static unsigned int sinsamp = 0;
  static unsigned int remainder = 0;

  unsigned int i;
  int j = 0;
  short *sout = dest;
  unsigned step = (sratein << 16) / (unsigned) srateout;
  unsigned nreq = (step * nsamp + remainder) >> 16;

  if (nreq > sinsamp) {
    sin = realloc(sin, (nreq + 1) * 4);

    if (!sinsamp) { // avoid pop when first starting stream
      sin[0] = sin[1] = 0;
    }

    sinsamp = nreq;
  }

  proc(sin + 2, nreq);

  for (i = 0; i < nsamp; i++) {
    *sout++ = (
      (unsigned) sin[j + 0] * (0x10000 - remainder) +
      (unsigned) sin[j + 2] * remainder
    ) >> 16;
    *sout++ = (
      (unsigned) sin[j + 1] * (0x10000 - remainder) +
      (unsigned) sin[j + 3] * remainder
    ) >> 16;
    remainder += step;
    j += remainder >> 16 << 1;
    remainder &= 0xffff;
  }

  sin[0] = sin[nreq * 2];
  sin[1] = sin[nreq * 2 + 1];
}  
  
/* vi: set et ts=2 sw=2: */

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


/********************************************************

experimental music API

********************************************************/

#ifndef HAVE_OWN_MUSIC

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

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "i_sound.h"
#include "m_file.h"
#include "p_mobj.h"
#include "s_sound.h"

#include "mus2mid.h"

#include "MUSIC/musicplayer.h"
#include "MUSIC/oplplayer.h"
#include "MUSIC/madplayer.h"
#include "MUSIC/dumbplayer.h"
#include "MUSIC/flplayer.h"
#include "MUSIC/vorbisplayer.h"
#include "MUSIC/portmidiplayer.h"

extern SDL_mutex *musmutex; // From i_sound.c
extern int mus_pause_opt;   // From m_misc.c

#define PLAYER_VORBIS     "vorbis player"
#define PLAYER_MAD        "mad mp3 player"
#define PLAYER_DUMB       "dumb tracker player"
#define PLAYER_FLUIDSYNTH "fluidsynth midi player"
#define PLAYER_OPL2       "opl2 synth player"
#define PLAYER_PORTMIDI   "portmidi midi player"
#define NUM_MUS_PLAYERS ( \
  (int)(sizeof (music_players) / sizeof (music_player_t *) - 1) \
)

// list of possible music players
static const music_player_t *music_players[] = {
  // until some ui work is done, the order these appear is the autodetect order.
  // of particular importance:  things that play mus have to be last, because
  // mus2midi very often succeeds even on garbage input
  &vorb_player,      // vorbisplayer.h
  &mp_player,        // madplayer.h
  &db_player,        // dumbplayer.h
  &fl_player,        // flplayer.h
  &opl_synth_player, // oplplayer.h
  &pm_player,        // portmidiplayer.h
  NULL
};

static int music_player_was_init[NUM_MUS_PLAYERS];
static int current_player = -1;
static const void *music_handle = NULL;

// songs played directly from wad (no mus->mid conversion)
// won't have this
static void *song_data = NULL;

// note that the "handle" passed around by s_sound is ignored
// however, a handle is maintained for the individual music players
const char *snd_soundfont;  // soundfont name for synths that use it
const char *snd_mididev;    // midi device to use (portmidiplayer)
const char *snd_midiplayer; // prefered MIDI device
const char *midiplayers[midi_player_last + 1] = {
  "sdl", "fluidsynth", "opl2", "portmidi", NULL
};
// order in which players are to be tried
const char *music_player_order[NUM_MUS_PLAYERS] = {
  PLAYER_VORBIS,
  PLAYER_MAD,
  PLAYER_DUMB,
  PLAYER_FLUIDSYNTH,
  PLAYER_OPL2,
  PLAYER_PORTMIDI,
};

int mus_fluidsynth_gain; // NSM  fine tune fluidsynth output level
int mus_opl_gain;        // NSM  fine tune OPL output level

void Exp_ShutdownMusic(void) {
  int i;

  if (MUSIC_DISABLED)
    return;

  S_StopMusic();

  for (i = 0; music_players[i]; i++) {
    if (music_player_was_init[i])
      music_players[i]->shutdown();
  }

  if (musmutex) {
    SDL_DestroyMutex(musmutex);
    musmutex = NULL;
  }
}

void Exp_InitMusic(void) {
  int i;
  musmutex = SDL_CreateMutex();

  if (MUSIC_DISABLED)
    return;

  // TODO: not so greedy
  for (i = 0; music_players[i]; i++)
    music_player_was_init[i] = music_players[i]->init (snd_samplerate);

  atexit(Exp_ShutdownMusic);
}

void Exp_PlaySong(int handle, int looping) {
  if (MUSIC_DISABLED)
    return;

  if (music_handle) {
    SDL_LockMutex(musmutex);
    music_players[current_player]->play(music_handle, looping);
    music_players[current_player]->setvolume(snd_MusicVolume);
    SDL_UnlockMutex(musmutex);
  }
}

void Exp_PauseSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (!music_handle)
    return;

  SDL_LockMutex(musmutex);
  switch (mus_pause_opt) {
    case 0:
      music_players[current_player]->stop();
    break;
    case 1:
      music_players[current_player]->pause();
    break;
    default: // Default - let music continue
    break;
  }  
  SDL_UnlockMutex(musmutex);
}

void Exp_ResumeSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (!music_handle)
    return;
  
  SDL_LockMutex(musmutex);
  switch (mus_pause_opt) {
    case 0: // i'm not sure why we can guarantee looping=true here,
            // but that's what the old code did
      music_players[current_player]->play(music_handle, 1);
    break;
    case 1:
      music_players[current_player]->resume();
    break;
    default: // Default - music was never stopped
    break;
  }
  SDL_UnlockMutex(musmutex);
}

void Exp_StopSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (music_handle) {
    SDL_LockMutex(musmutex);
    music_players[current_player]->stop();
    SDL_UnlockMutex(musmutex);
  }
}

void Exp_UnRegisterSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (music_handle) {
    SDL_LockMutex(musmutex);
    music_players[current_player]->unregistersong(music_handle);
    music_handle = NULL;
    if (song_data) {
      free(song_data);
      song_data = NULL;
    }
    SDL_UnlockMutex(musmutex);
  }
}

void Exp_SetMusicVolume(int volume) {
  if (MUSIC_DISABLED)
    return;

  if (music_handle) {
    SDL_LockMutex(musmutex);
    music_players[current_player]->setvolume(volume);
    SDL_UnlockMutex(musmutex);
  }
}

// returns 1 on success, 0 on failure
int Exp_RegisterSongEx (const void *data, size_t len, int try_mus2mid) {
  int i;
  int j;
  MEMFILE *instream;
  MEMFILE *outstream;
  void *outbuf;
  size_t outbuf_len;
  int result;

  if (MUSIC_DISABLED)
    return 1;

  if (music_handle)
    Exp_UnRegisterSong(0);

  // e6y: new logic by me
  // Now you can hear title music in deca.wad
  // http://www.doomworld.com/idgames/index.php?id=8808
  // Ability to use mp3 and ogg as inwad lump

  if (len > 4 && memcmp(data, "MUS", 3) != 0) {
    // The header has no MUS signature
    // Let's try to load this song directly
  
    // go through music players in order
    int found = 0;

    for (j = 0; j < NUM_MUS_PLAYERS; j++) {
      found = 0;

      for (i = 0; music_players[i]; i++) {
        if (strcmp (music_players[i]->name (), music_player_order[j]) == 0) {
          found = 1;

          if (music_player_was_init[i]) {
            const void *temp_handle = music_players[i]->registersong(data, len);

            if (temp_handle) {
              SDL_LockMutex (musmutex);
              current_player = i;
              music_handle = temp_handle;
              SDL_UnlockMutex(musmutex);
              D_Msg(MSG_INFO,
                "Exp_RegisterSongEx: Using player %s\n", music_players[i]->name()
              );
              return 1;
            }
          }
          else {
            D_Msg(MSG_INFO,
              "Exp_RegisterSongEx: Music player %s on preferred list but it "
              "failed to init\n",
              music_players[i]-> name()
            );
          }
        }
      }
      if (!found) {
        D_Msg(MSG_INFO,
          "Exp_RegisterSongEx: Couldn't find preferred music player %s in "
          "list\n  (typo or support not included at compile time)\n",
          music_player_order[j]
        );
      }
    }
    // load failed
  }

  // load failed? try mus2mid
  if (try_mus2mid) {
    instream = mem_fopen_read(data, len);
    outstream = mem_fopen_write();

    // e6y: from chocolate-doom
    // New mus -> mid conversion code thanks to Ben Ryves <benryves@benryves.com>
    // This plays back a lot of music closer to Vanilla Doom - eg. tnt.wad map02
    result = mus2mid(instream, outstream);

    if (result != 0) {
      size_t muslen = len;
      const unsigned char *musptr = data;

      // haleyjd 04/04/10: scan forward for a MUS header. Evidently DMX was
      // capable of doing this, and would skip over any intervening data. That,
      // or DMX doesn't use the MUS header at all somehow.
      while (musptr < (const unsigned char*)data + len - sizeof(musheader)) {
        // if we found a likely header start, reset the mus pointer to that location,
        // otherwise just leave it alone and pray.
        if (!strncmp ((const char *)musptr, "MUS\x1a", 4)) {
          mem_fclose(instream);
          instream = mem_fopen_read(musptr, muslen);
          result = mus2mid(instream, outstream);
          break;
        }

        musptr++;
        muslen--;
      }
    }

    if (result == 0) {
      mem_get_buf(outstream, &outbuf, &outbuf_len);

      // recopy so we can free the MEMFILE
      song_data = malloc(outbuf_len);
      if (song_data)
        memcpy(song_data, outbuf, outbuf_len);

      mem_fclose(instream);
      mem_fclose(outstream);

      if (song_data)
        return Exp_RegisterSongEx(song_data, outbuf_len, 0);
    }
  }

  D_Msg(MSG_ERROR, "Exp_RegisterSongEx: Failed\n");
  return 0;
}


int Exp_RegisterSong(const void *data, size_t len) {
  if (MUSIC_DISABLED)
    return 0;

  Exp_RegisterSongEx(data, len, 1);
  return 0;
}

// try register external music file (not in WAD)

int Exp_RegisterMusic(const char *filename, musicinfo_t *song) {
  size_t len;

  if (MUSIC_DISABLED)
    return 0;

  if (!M_ReadFile(filename, (char **)&song_data, &len)) {
    D_Msg(MSG_WARN,
      "Couldn't read %s\nAttempting to load default MIDI music.\n", filename
    );
    return 1;
  }

  if (!Exp_RegisterSongEx(song_data, len, 1)) {
    free (song_data);
    song_data = NULL;

    D_Msg(MSG_WARN,
      "Couldn't load music from %s\nAttempting to load default MIDI music.\n",
      filename
    );

    return 1; // failure
  }

  song->data = 0;
  song->handle = 0;
  song->lumpnum = 0;

  return 0;
}

void Exp_UpdateMusic(void *buff, unsigned nsamp) {
  if (MUSIC_DISABLED)
    return;

  if (!music_handle) {
    memset(buff, 0, nsamp * 4);
    return;
  }

  music_players[current_player]->render(buff, nsamp);
}

void M_ChangeMIDIPlayer(void) {
  int experimental_music;

  if (MUSIC_DISABLED)
    return;

#ifdef HAVE_OWN_MUSIC
  // do not bother about small memory leak
  snd_midiplayer = strdup(midiplayers[midi_player_sdl]);
  use_experimental_music = 0;

  return;
#endif

  if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_sdl])) {
    experimental_music = false;
  }
  else {
    experimental_music = true;

    if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_fluidsynth])) {
      music_player_order[3] = PLAYER_FLUIDSYNTH;
      music_player_order[4] = PLAYER_OPL2;
      music_player_order[5] = PLAYER_PORTMIDI;
    }
    else if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_opl2])) {
      music_player_order[3] = PLAYER_OPL2;
      music_player_order[4] = PLAYER_FLUIDSYNTH;
      music_player_order[5] = PLAYER_PORTMIDI;
    }
    else if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_portmidi])) {
      music_player_order[3] = PLAYER_PORTMIDI;
      music_player_order[4] = PLAYER_FLUIDSYNTH;
      music_player_order[5] = PLAYER_OPL2;
    }
  }

  if (use_experimental_music == -1) {
    use_experimental_music = experimental_music;
  }
  else if (experimental_music && use_experimental_music) {
    S_StopMusic();
    S_RestartMusic();
  }
}

#endif // !HAVE_OWN_MUSIC
  
/* vi: set et ts=2 sw=2: */


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


//
// MUSIC API.
//

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
#include "i_pcsound.h"
#include "i_sound.h"
#include "m_file.h"
#include "p_mobj.h"
#include "s_sound.h"

#ifdef HAVE_MIXER
#include "mus2mid.h"
#endif

extern int mus_pause_opt; // From m_misc.c

#ifdef HAVE_MIXER
static Mix_Music *music[2] = { NULL, NULL };
static SDL_RWops *rw_midi = NULL; // Some tracks are directly streamed from the RWops;
                                  // we need to free them in the end
static char *music_tmp = NULL;    /* cph - name of music temporary file */
static const char *music_tmp_ext[] = { // List of extensions that can be
  "", ".mp3", ".ogg"                   // appended to music_tmp. First must be
};                                     // "".
#define MUSIC_TMP_EXT (sizeof(music_tmp_ext) / sizeof(*music_tmp_ext))
#endif

extern void Exp_UpdateMusic(void *buff, unsigned nsamp);
extern int  Exp_RegisterMusic(const char *filename, musicinfo_t *song);
extern int  Exp_RegisterSong(const void *data, size_t len);
extern int  Exp_RegisterSongEx(const void *data, size_t len, int try_mus2mid);
extern void Exp_SetMusicVolume(int volume);
extern void Exp_UnRegisterSong(int handle);
extern void Exp_StopSong(int handle);
extern void Exp_ResumeSong(int handle);
extern void Exp_PauseSong(int handle);
extern void Exp_PlaySong(int handle, int looping);
extern void Exp_InitMusic(void);
extern void Exp_ShutdownMusic(void);

void I_ShutdownMusic(void) {
  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_ShutdownMusic();
    return;
  }
#ifdef HAVE_MIXER
  if (music_tmp) {
    S_StopMusic();

    for (int i = 0; i < MUSIC_TMP_EXT; i++) {
      size_t name_size = strlen(music_tmp) + strlen(music_tmp_ext[i]) + 1;
      char *name = malloc(name_size);

      snprintf(name, name_size, "%s%s", music_tmp, music_tmp_ext[i]);

      if (!unlink(name))
        D_Msg(MSG_DEBUG, "I_ShutdownMusic: removed %s\n", name);

      free(name);
    }

    free(music_tmp);
    music_tmp = NULL;
  }
#endif
}

void I_InitMusic(void) {
#ifndef _WIN32
  int fd;
#endif

  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_InitMusic();
    return;
  }
#ifdef HAVE_MIXER
  if (!music_tmp) {
#ifndef _WIN32
    music_tmp = strdup("/tmp/"PACKAGE_TARNAME"-music-XXXXXX");

    fd = mkstemp(music_tmp);

    if (fd < 0) {
      D_Msg(MSG_ERROR, "I_InitMusic: failed to create music temp file %s",
        music_tmp
      );

      free(music_tmp);
      music_tmp = NULL;

      return;
    }
    else {
      close(fd);
    }
#else /* !_WIN32 */
    music_tmp = strdup("doom.tmp");
#endif
    atexit(I_ShutdownMusic);
  }

  return;
#endif
  D_Msg(MSG_INFO,
    "I_InitMusic: Was compiled without SDL_Mixer support.  "
    "You should enable experimental music.\n"
  );
}

void I_PlaySong(int handle, bool looping) {
  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_PlaySong (handle, looping);
    return;
  }
#ifdef HAVE_MIXER
  if (music[handle]) {
    Mix_PlayMusic(music[handle], looping ? -1 : 0);

    // haleyjd 10/28/05: make sure volume settings remain consistent
    I_SetMusicVolume(snd_MusicVolume);
  }
#endif
}

void I_PauseSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_PauseSong (handle);
    return;
  }

#ifdef HAVE_MIXER
  switch (mus_pause_opt) {
  case 0:
    I_StopSong(handle);
  break;
  case 1:
    switch (Mix_GetMusicType(NULL)) {
      case MUS_NONE:
      break;
      case MUS_MID:
        // SDL_mixer's native MIDI music playing does not pause properly.  As a
        // workaround, set the volume to 0 when paused.
        I_SetMusicVolume(0);
      break;
      default:
        Mix_PauseMusic();
      break;
    }
  break;
  }
#endif
  // Default - let music continue
}

void I_ResumeSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_ResumeSong (handle);
    return;
  }

#ifdef HAVE_MIXER
  switch(mus_pause_opt) {
    case 0:
      I_PlaySong(handle, 1);
    break;
    case 1:
      switch (Mix_GetMusicType(NULL)) {
        case MUS_NONE:
        break;
        case MUS_MID:
          I_SetMusicVolume(snd_MusicVolume);
        break;
        default:
          Mix_ResumeMusic();
        break;
      }
      break;
  }
#endif
  /* Otherwise, music wasn't stopped */
}

void I_StopSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_StopSong(handle);
    return;
  }

#ifdef HAVE_MIXER
  Mix_HaltMusic(); // halt music playback
#endif
}

void I_UnRegisterSong(int handle) {
  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_UnRegisterSong (handle);
    return;
  }
#ifdef HAVE_MIXER
  if (music[handle]) {
    Mix_FreeMusic(music[handle]);
    music[handle] = NULL;

    // Free RWops
    if (rw_midi != NULL) {
      rw_midi = NULL;
    }
  }
#endif
}

int I_RegisterSong(const void *data, size_t len) {
  int i;
  char *name;
  size_t name_size;
  bool io_errors = false;

  if (MUSIC_DISABLED)
    return 0;

  if (use_experimental_music)
    return Exp_RegisterSong(data, len);

#ifdef HAVE_MIXER
  if (music_tmp == NULL)
    return 0;

  // e6y: new logic by me
  // Now you can hear title music in deca.wad
  // http://www.doomworld.com/idgames/index.php?id=8808
  // Ability to use mp3 and ogg as inwad lump

  music[0] = NULL;

  if (len > 4 && memcmp(data, "MUS", 3) != 0) {
    // The header has no MUS signature
    // Let's try to load this song with SDL
    for (i = 0; i < MUSIC_TMP_EXT; i++) {
      // Current SDL_mixer (up to 1.2.8) cannot load some MP3 and OGG
      // without proper extension
      name_size = strlen(music_tmp) + strlen(music_tmp_ext[i]) + 1;
      name = malloc(name_size);
      snprintf(name, name_size, "%s%s", music_tmp, music_tmp_ext[i]);

      if (strlen(music_tmp_ext[i]) == 0) {
        //midi
        rw_midi = SDL_RWFromConstMem(data, len);
        if (rw_midi)
          music[0] = Mix_LoadMUS_RW(rw_midi);
      }

      if (!music[0]) {
        io_errors = (M_WriteFile(name, data, len) == 0);

        if (!io_errors)
          music[0] = Mix_LoadMUS(name);
      }

      free(name);

      if (music[0])
        break; // successfully loaded
    }
  }

  // e6y: from Chocolate-Doom
  // Assume a MUS file and try to convert
  if (!music[0]) {
    MEMFILE *instream;
    MEMFILE *outstream;
    void *outbuf;
    size_t outbuf_len;
    int result;

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
        // if we found a likely header start, reset the mus pointer to that
        // location, otherwise just leave it alone and pray.
        if (!strncmp((const char*)musptr, "MUS\x1a", 4)) {
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
      
      rw_midi = SDL_RWFromMem(outbuf, outbuf_len);
      if (rw_midi)
        music[0] = Mix_LoadMUS_RW(rw_midi);
      
      if (!music[0]) {
        io_errors = M_WriteFile(music_tmp, outbuf, outbuf_len) == 0;

        if (!io_errors)
          music[0] = Mix_LoadMUS(music_tmp); // Load the MUS
      }
    }

    mem_fclose(instream);
    mem_fclose(outstream);
  }
  
  // Failed to load
  if (!music[0]) {
    // Conversion failed, free everything
    if (rw_midi != NULL)
      rw_midi = NULL;

    if (io_errors)
      D_Msg(MSG_ERROR, "Error writing song\n");
    else
      D_Msg(MSG_ERROR, "Error loading song: %s\n", Mix_GetError());
  }

#endif
  return 0;
}

// cournia - try to load a music file into SDL_Mixer
//           returns true if could not load the file
int I_RegisterMusic(const char* filename, musicinfo_t *song) {
  if (MUSIC_DISABLED)
    return 0;

  if (use_experimental_music)
    return Exp_RegisterMusic(filename, song);

#ifdef HAVE_MIXER
  if (!filename)
    return 1;

  if (!song)
    return 1;

  music[0] = Mix_LoadMUS(filename);
  if (music[0] == NULL) {
    D_Msg(MSG_WARN,
      "Couldn't load music from %s: %s\nAttempting to load default "
      "MIDI music.\n",
      filename,
      Mix_GetError()
    );
    return 1;
  }
  else {
    song->data = 0;
    song->handle = 0;
    song->lumpnum = 0;

    return 0;
  }
#else
  return 1;
#endif
}

void I_SetMusicVolume(int volume) {
  if (MUSIC_DISABLED)
    return;

  if (use_experimental_music) {
    Exp_SetMusicVolume(volume);
    return;
  }

#ifdef HAVE_MIXER
  Mix_VolumeMusic(volume * 8);

#if defined(_WIN32) && !defined(__MINGW32__)
  // e6y: workaround
  if (mus_extend_volume && Mix_GetMusicType(NULL) == MUS_MID)
    I_midiOutSetVolumes(volume);
#endif

#endif
}

#endif // !HAVE_OWN_MUSIC
  
/* vi: set et ts=2 sw=2: */


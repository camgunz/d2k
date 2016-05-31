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

#include "doomdef.h"
#include "d_main.h"
#include "i_sound.h"
#include "n_net.h"
#include "p_setup.h"
#include "p_mobj.h"
#include "sounds.h"
#include "s_sound.h"

#define DEBUG_SOUND 0

static int saved_sfx_volume;

// Internal volumes are 0-15.
int snd_SfxVolume = 15;   // Maximum volume of a sound effect.
int snd_MusicVolume = 15; // Maximum volume of music.
int default_numChannels;
int numChannels;
int idmusnum; //jff 3/17/98 to keep track of last IDMUS specified music num
bool mus_paused;
musicinfo_t *mus_playing;
int musicnum_current;

sound_engine_t *SoundEngine = NULL;

const char *S_music_files[NUMMUSIC]; // cournia - stores music file names

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume) {
  int i;

#if DEBUG_SOUND
  if (CLIENT)
    D_MsgActivateWithFile(MSG_SOUND, "client-sound.log");
  else if (SERVER)
    D_MsgActivateWithFile(MSG_SOUND, "server-sound.log");
  else
    D_MsgActivateWithFile(MSG_SOUND, "sound.log");
#endif

  idmusnum = -1; //jff 3/17/98 insure idmus number is blank

  numChannels = default_numChannels;

  //jff 1/22/98 skip sound init if sound not enabled
  if (!SOUND_DISABLED) {
    D_Msg(MSG_INFO, "S_Init: default sfx volume %d\n", sfxVolume);

    // Whatever these did with DMX, these are rather dummies now.
    I_SetChannels();

    S_SetSfxVolume(sfxVolume);

    // Allocating the internal channels for mixing
    // (the maximum numer of sounds rendered
    // simultaneously) within zone memory.
    // CPhipps - calloc

    // Note that sounds have not been cached (yet).
    for (i = 1; i < NUMSFX; i++)
      S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
  }

  // CPhipps - music init reformatted
  if (!MUSIC_DISABLED) {
    S_SetMusicVolume(musicVolume);
    mus_paused = 0; // no sounds are playing, and they are not mus_paused
  }

  if (MULTINET)
    SoundEngine = S_GetNewSoundEngine();
  else
    SoundEngine = S_GetOldSoundEngine();

  SoundEngine->init();
}

int S_GetChannelCount(void) {
  return numChannels;
}

void S_Stop(void) {
  SoundEngine->stop_sounds();
}

void S_Start(void) {
  SoundEngine->handle_level_start();
}

void S_StartSound(mobj_t *origin, int sfx_id) {
  SoundEngine->start_sound(origin, sfx_id, snd_SfxVolume);
}

void S_StartSoundAtVolume(mobj_t *origin, int sfx_id, int volume) {
  SoundEngine->start_sound(origin, sfx_id, volume);
}

void S_StopSound(mobj_t *origin) {
  SoundEngine->silence_actor(origin);
}

void S_StartMusic(int music_id) {
  SoundEngine->set_music(music_id, false);
}

void S_ChangeMusic(int music_id, bool looping) {
  SoundEngine->set_music(music_id, looping);
}

void S_ChangeMusInfoMusic(int lumpnum, bool looping) {
  SoundEngine->set_musinfo_music(lumpnum, looping);
}

void S_RestartMusic(void) {
  SoundEngine->restart_music();
}

void S_StopMusic(void) {
  SoundEngine->stop_music();
}

void S_PauseSound(void) {
  SoundEngine->pause_music();
}

void S_ResumeSound(void) {
  SoundEngine->resume_music();
}

void S_MuteSound(void) {
  saved_sfx_volume = snd_SfxVolume;
  snd_SfxVolume = 0;
}

void S_UnMuteSound(void) {
  snd_SfxVolume = saved_sfx_volume;
}

void S_UpdateSounds(mobj_t *listener) {
  SoundEngine->reposition_sounds(listener);
}

void S_SetMusicVolume(int volume) {
  //jff 1/22/98 return if music is not enabled
  if (MUSIC_DISABLED)
    return;

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

/* vi: set et ts=2 sw=2: */


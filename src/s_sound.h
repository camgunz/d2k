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


#ifndef S_SOUND_H__
#define S_SOUND_H__

#ifdef __GNUG__
#pragma interface
#endif

// The number of internal mixing channels, the samples calculated for each
// mixing step, the size of the 16bit, 2 hardware channel (stereo) mixing
// buffer, and the samplerate of the raw data.
// CG 08/15/2014: This used to be the number of internal mixing channels, but
//                it is tightly linked to the number of "external" mixing
//                channels, so now it is "MAX_SOUND_CHANNELS" instead of
//                "MAX_CHANNELS", it lives in s_sound.h instead of i_sound.c,
//                and m_misc.c uses it to configure "snd_channels".
#define MAX_SOUND_CHANNELS 32

#define SOUND_DISABLED (!snd_card || nosfxparm)
#define MUSIC_DISABLED (!mus_card || nomusicparm)

// killough 4/25/98: mask used to indicate sound origin is player item pickup
#define PICKUP_SOUND (0x8000)

typedef struct sound_engine_s {
  void (stop_sounds)(void);                                   // S_Stop
  void (start_sound)(mobj_t *origin, int sfx_id, int volume); // S_StartSoundAtVolume
  void (silence_actor)(mobj_t *origin);                       // S_StopSound
  void (reposition_sounds)(mobj_t *listener);                 // S_UpdateSounds
  void (set_music)(int musicnum, bool looping);               // S_ChangeMusic
  void (restart_music)(void);                                 // S_RestartMusic
  void (set_musinfo_music)(int lumpnum, bool looping);        // S_ChangeMusInfoMusic
  void (stop_music)                                           // S_StopMusic
  void (pause_music)(void);                                   // S_PauseSound
  void (resume_music)(void);                                  // S_ResumeSound
  void (handle_level_start)(void);                            // S_Start
} sound_engine_t;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init(int sfxVolume, int musicVolume);

// Kills all sounds
void S_Stop(void);

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_Start(void);

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(mobj_t *origin, int sound_id);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(mobj_t *origin, int sound_id, int volume);

// Stop sound for thing at <origin>
void S_StopSound(mobj_t *origin);

// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h, and set whether looping
void S_ChangeMusic(int music_id, bool looping);
void S_ChangeMusInfoMusic(int lumpnum, bool looping);
void S_RestartMusic(void);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);
void S_MuteSound(void);
void S_UnMuteSound(void);

//
// Updates music & sounds
//
void S_UpdateSounds(mobj_t *listener);
void S_SetMusicVolume(int volume);
void S_SetSfxVolume(int volume);

//
// Serialization routines
//
void S_ReloadChannelOrigins(void);

//
// Sound Log routines
//
void S_ResetSoundLog(void);
void S_TrimSoundLog(int tic, int command_index);

// machine-independent sound params
extern int default_numChannels;
extern int numChannels;

//jff 3/17/98 holds last IDMUS number, or -1
extern int idmusnum;

#endif

/* vi: set et ts=2 sw=2: */


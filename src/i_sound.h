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
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef __I_SOUND__
#define __I_SOUND__

#include "sounds.h"
#include "doomtype.h"

#define SNDSERV
#undef SNDINTR

#ifndef SNDSERV
#include "l_soundgen.h"
#endif

extern int snd_pcspeaker;

// Init at program start...
void I_InitSound(void);

// ... shut down and relase at program termination.
void I_ShutdownSound(void);

//
//  SFX I/O
//

// Initialize channels?
void I_SetChannels(void);

// Get raw data lump index for sound descriptor.
int I_GetSfxLumpNum (sfxinfo_t *sfxinfo);

// Starts a sound in a particular sound channel.
int I_StartSound(int id, int channel, int vol, int sep, int pitch, int priority);

// Stops a sound channel.
void I_StopSound(int handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
dboolean I_SoundIsPlaying(int handle);

// Called by m_menu.c to let the quit sound play and quit right after it stops
dboolean I_AnySoundStillPlaying(void);

// Updates the volume, separation,
//  and pitch of a sound channel.
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch);

// NSM sound capture routines
// silences sound output, and instead allows sound capture to work
// call this before sound startup
void I_SetSoundCap (void);
// grabs len samples of audio (16 bit interleaved)
unsigned char *I_GrabSound (int len);

// NSM helper routine for some of the streaming audio
void I_ResampleStream (void *dest, unsigned nsamp, void (*proc) (void *dest, unsigned nsamp), unsigned sratein, unsigned srateout);

//
//  MUSIC I/O
//
extern const char *snd_soundfont;
extern const char *snd_mididev;
extern char music_player_order[][200];

void I_InitMusic(void);
void I_ShutdownMusic(void);

// Volume.
void I_SetMusicVolume(int volume);

// PAUSE game handling.
void I_PauseSong(int handle);
void I_ResumeSong(int handle);

// Registers a song handle to song data.
int I_RegisterSong(const void *data, size_t len);

// cournia - tries to load a music file
int I_RegisterMusic( const char* filename, musicinfo_t *music );

// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
void I_PlaySong(int handle, int looping);

// Stops a song over 3 seconds.
void I_StopSong(int handle);

// See above (register), then think backwards
void I_UnRegisterSong(int handle);

// Allegro card support jff 1/18/98
extern int snd_card;
extern int mus_card;
// CPhipps - put these in config file
extern int snd_samplerate;

extern int use_experimental_music;

extern int mus_fluidsynth_gain; // NSM  fine tune fluidsynth output level
extern int mus_opl_gain; // NSM  fine tune OPL output level

// prefered MIDI player
typedef enum
{
  midi_player_sdl,
  midi_player_fluidsynth,
  midi_player_opl2,
  midi_player_portmidi,

  midi_player_last
} midi_player_name_t;

extern const char *snd_midiplayer;
extern const char *midiplayers[];

void M_ChangeMIDIPlayer(void);

#endif


// NSM: reworked to function without sdl_mixer (needs new i_sound.c)

#include "z_zone.h"

#include <SDL.h>

#include "pcsound.h"
#include "sounds.h"
#include "i_sound.h"

#define SQUARE_WAVE_AMP 0x2000

static pcsound_callback_func callback;

// Output sound format

static int mixing_freq;
//static Uint16 mixing_format;
//static int mixing_channels;

// Currently playing sound
// current_remaining is the number of remaining samples that must be played
// before we invoke the callback to get the next frequency.

static int current_remaining;
static int current_freq;

static int phase_offset = 0;

static int pcsound_inited = 0;

// Mixer function that does the PC speaker emulation

void PCSound_Mix_Callback(void *udata, Uint8 *stream, int len) {
  Sint16 *leftptr;
  Sint16 *rightptr;
  Sint16  this_value;
  int     oldfreq;
  int     i;
  int     nsamples;

  // safe to call even if not active
  if (!pcsound_inited)
    return;

  // Number of samples is quadrupled, because of 16-bit and stereo

  nsamples = len / 4;

  leftptr = (Sint16 *) stream;
  rightptr = ((Sint16 *) stream) + 1;

  // Fill the output buffer

  for (i = 0; i < nsamples; i++) {
    // Has this sound expired? If so, invoke the callback to get
    // the next frequency.

    while (current_remaining == 0) {
      oldfreq = current_freq;

      // Get the next frequency to play

      callback(&current_remaining, &current_freq);

      if (current_freq != 0) {
        // Adjust phase to match to the new frequency.
        // This gives us a smooth transition between different tones,
        // with no impulse changes.

        phase_offset = (phase_offset * oldfreq) / current_freq;
      }

      current_remaining = (current_remaining * mixing_freq) / 1000;
    }

    // Set the value for this sample.

    if (current_freq == 0) { // Silence
      this_value = 0;
    }
    else {
      int frac;

      // Determine whether we are at a peak or trough in the current
      // sound.  Multiply by 2 so that frac % 2 will give 0 or 1
      // depending on whether we are at a peak or trough.

      frac = (phase_offset * current_freq * 2) / mixing_freq;

      if ((frac % 2) == 0)
        this_value = SQUARE_WAVE_AMP;
      else
        this_value = -SQUARE_WAVE_AMP;

      ++phase_offset;
    }

    --current_remaining;

    // Use the same value for the left and right channels.

    *leftptr += this_value;
    *rightptr += this_value;

    leftptr += 2;
    rightptr += 2;
  }
}
//#endif // HAVE_LIBSDL_MIXER

static int PCSound_SDL_Init(pcsound_callback_func callback_func) {
  mixing_freq = snd_samplerate;

  callback = callback_func;
  current_freq = 0;
  current_remaining = 0;
  pcsound_inited = 1;

  return 1;
}

static void PCSound_SDL_Shutdown(void) {
  pcsound_inited = 0;
}

pcsound_driver_t pcsound_sdl_driver = {
  "SDL",
  PCSound_SDL_Init,
  PCSound_SDL_Shutdown,
};

/* vi: set et ts=2 sw=2: */


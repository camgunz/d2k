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

#include <cairo/cairo.h>

#include <SDL.h>

#include "doomdef.h"
#include "doomstat.h"
#include "am_map.h"
#include "d_deh.h"
#include "d_event.h"
#include "d_main.h"
#include "e6y.h"//e6y
#include "f_wipe.h"
#include "g_game.h"
#include "i_capture.h"
#include "i_input.h"
#include "i_joy.h"
#include "i_main.h"
#include "i_mouse.h"
#include "i_smp.h"
#include "i_video.h"
#include "lprintf.h"
#include "m_argv.h"
#include "p_user.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_screenmultiply.h"
#include "r_things.h"
#include "st_stuff.h"
#include "v_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "xd_main.h"

#ifdef GL_DOOM
#include "gl_struct.h"

extern GLuint* last_glTexID;
#endif

#define MAX_RESOLUTIONS_COUNT 128
#define NO_PALETTE_CHANGE 1000

/* CG: FIXME: This is duplicated in overlay.lua */
#define OVERLAY_FORMAT CAIRO_FORMAT_RGB24

static int newpal = 0;

overlay_t overlay;

const char *screen_resolutions_list[MAX_RESOLUTIONS_COUNT] = {NULL};
const char *screen_resolution_lowest;
const char *screen_resolution = NULL;

int process_affinity_mask;
int process_priority;
int try_to_reduce_cpu_cache_misses;

bool window_focused;

// Window resize state.
#if 0
static void ApplyWindowResize(SDL_Event *resize_event);
#endif

const char *sdl_videodriver;
const char *sdl_video_window_pos;

int gl_colorbuffer_bits = 16;
int gl_depthbuffer_bits = 16;

#ifdef DISABLE_DOUBLEBUFFER
int use_doublebuffer = 0;
#else
int use_doublebuffer = 1; // Included not to break m_misc, but not relevant to SDL
#endif

int use_fullscreen;
int desired_fullscreen;
SDL_Surface *screen;

#ifdef GL_DOOM
vid_8ingl_t vid_8ingl;
int use_gl_surface;
#endif

int leds_always_off = 0; // Expected by m_misc, not relevant

int I_GetModeFromString(const char *modestr);

static void initialize_overlay(void) {
  overlay.pixels          = NULL;
  overlay.owns_pixels     = false;
#ifdef GL_DOOM
  overlay.tex_id          = 0;
#endif
  overlay.needs_resetting = false;
}

void get_screen_resolution(void) {
  int width, height;

  desired_screenwidth = 640;
  desired_screenheight = 480;

  if (screen_resolution) {
    if (sscanf(screen_resolution, "%dx%d", &width, &height) == 2) {
      desired_screenwidth = width;
      desired_screenheight = height;
    }
  }
}

static void fill_screen_resolution_list(void) {
  SDL_Rect **modes;
  int i;
  int j;
  int list_size;
  int current_resolution_index;
  char mode[256];
  Uint32 flags;

  // do it only once
  if (screen_resolutions_list[0])
    return;

  if (desired_screenwidth == 0 || desired_screenheight == 0)
    get_screen_resolution();

  flags = SDL_FULLSCREEN;
#ifdef GL_DOOM
  flags |= SDL_OPENGL;
#endif

  // Don't call SDL_ListModes if SDL has not been initialized
  if (nodrawers)
    modes = NULL;
  else
    modes = SDL_ListModes(NULL, flags);

  list_size = 0;
  current_resolution_index = -1;

  if (modes) {
    int count = 0;

    for (i = 0; modes[i]; i++)
      count++;

    // (-2) is for NULL at the end of list and for custom resolution
    count = MIN(count, MAX_RESOLUTIONS_COUNT - 2);

    for (i = count - 1; i >= 0; i--) {
      int in_list = false;

      doom_snprintf(mode, sizeof(mode), "%dx%d", modes[i]->w, modes[i]->h);

      if (i == count - 1)
        screen_resolution_lowest = strdup(mode);

      for (j = 0; j < list_size; j++) {
        if (!strcmp(mode, screen_resolutions_list[j])) {
          in_list = true;
          break;
        }
      }

      if (!in_list) {
        screen_resolutions_list[list_size] = strdup(mode);

        if (modes[i]->w == desired_screenwidth &&
            modes[i]->h == desired_screenheight) {
          current_resolution_index = list_size;
        }

        list_size++;
      }
    }
    screen_resolutions_list[list_size] = NULL;
  }

  if (list_size == 0) {
    doom_snprintf(
      mode, sizeof(mode), "%dx%d", desired_screenwidth, desired_screenheight
    );
    screen_resolutions_list[0] = strdup(mode);
    current_resolution_index = 0;
    list_size = 1;
  }

  if (current_resolution_index == -1) {
    doom_snprintf(
      mode, sizeof(mode), "%dx%d", desired_screenwidth, desired_screenheight
    );

    // make it first
    list_size++;
    for (i = list_size - 1; i > 0; i--)
      screen_resolutions_list[i] = screen_resolutions_list[i - 1];
    screen_resolutions_list[0] = strdup(mode);
    current_resolution_index = 0;
  }

  screen_resolutions_list[list_size] = NULL;
  screen_resolution = screen_resolutions_list[current_resolution_index];
}

static void get_closest_resolution(int *width, int *height, int flags) {
  SDL_Rect **modes;
  int twidth;
  int theight;
  int cwidth = 0;
  int cheight = 0;
  unsigned int closest = UINT_MAX;
  unsigned int dist;

  if (!SDL_WasInit(SDL_INIT_VIDEO))
    return;

  modes = SDL_ListModes(NULL, flags);

  if (modes == (SDL_Rect **)-1) // any dimension is okay for the given format
    return;

  if (!modes)
    return;

  for (int i = 0; modes[i]; i++) {
    twidth = modes[i]->w;
    theight = modes[i]->h;

    if (twidth == *width && theight == *height)
      return;

    //if (iteration == 0 && (twidth < *width || theight < *height))
    //  continue;

    dist = (twidth  - *width)  * (twidth  - *width) +
           (theight - *height) * (theight - *height);

    if (dist < closest) {
      closest = dist;
      cwidth = twidth;
      cheight = theight;
    }
  }

  if (closest != 4294967295u)
    *height = cheight;

}

void I_VideoUpdateFocus(void) {
  Uint8 state;

  state = SDL_GetAppState();

  window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);

  if (desired_fullscreen && window_focused) {
    if (st_palette < 0)
      st_palette = 0;

    V_SetPalette(st_palette);
  }

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    if (gl_hardware_gamma) {
      // e6y: Restore of startup gamma if window loses focus
      if (!window_focused)
        gld_SetGammaRamp(-1);
      else
        gld_SetGammaRamp(useglgamma);
    }
  }
#endif
}

int I_GetScreenStride(void) {
  int stride;

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    stride = cairo_format_stride_for_width(OVERLAY_FORMAT, SCREENWIDTH);
  else if (try_to_reduce_cpu_cache_misses && render_screen_multiply > 1)
    stride = REAL_SCREENPITCH;
  else
    stride = cairo_format_stride_for_width(OVERLAY_FORMAT, SCREENWIDTH);
#else
  if (try_to_reduce_cpu_cache_misses && render_screen_multiply > 1)
    stride = REAL_SCREENPITCH;
  else
    stride = cairo_format_stride_for_width(OVERLAY_FORMAT, SCREENWIDTH);
#endif

  return stride;
}

void I_UpdateNoBlit(void) {
}

void I_FinishUpdate(void) {
  I_MouseUpdateGrab();

#ifdef MONITOR_VISIBILITY
  if (!(SDL_GetAppState() & SDL_APPACTIVE))
    return;
#endif

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    if (!overlay.needs_resetting) {
      glBindTexture(GL_TEXTURE_2D, overlay.tex_id);
      glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        REAL_SCREENWIDTH,
        REAL_SCREENHEIGHT,
        0,
        GL_BGRA,
        GL_UNSIGNED_BYTE,
        overlay.pixels
      );
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(0.0f, 0.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(0.0f, REAL_SCREENHEIGHT);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(REAL_SCREENWIDTH, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(REAL_SCREENWIDTH, REAL_SCREENHEIGHT);
      glEnd();

      last_glTexID = &overlay.tex_id;
    }

    gld_Finish();

    return;
  }
#endif

  if ((screen_multiply > 1) || SDL_MUSTLOCK(screen)) {
    int h;
    byte *src;
    byte *dest;

    if (SDL_LockSurface(screen) < 0) {
      lprintf(LO_INFO,"I_FinishUpdate: %s\n", SDL_GetError());
      return;
    }

    if (screen_multiply > 1) {
      R_ProcessScreenMultiply(
        screens[0].data,
        screen->pixels,
        V_GetPixelDepth(),
        screens[0].byte_pitch,
        screen->pitch);
    }
    else {
      dest = screen->pixels;
      src = screens[0].data;

      for (h = screen->h; h > 0; h--) {
        memcpy(dest, src, SCREENWIDTH * V_GetPixelDepth());
        dest += screen->pitch;
        src += screens[0].byte_pitch;
      }
    }

    SDL_UnlockSurface(screen);
  }

#ifdef GL_DOOM
  if (vid_8ingl.enabled)
    gld_Draw8InGL();
#endif

  SDL_Flip(screen);
}

void I_SetPalette(int pal) {
  newpal = pal;
}

void I_PreInitGraphics(void) {
  int p;
  char *video_driver = strdup(sdl_videodriver);
  unsigned int flags = 0;

  if (!(M_CheckParm("-nodraw") && M_CheckParm("-nosound")))
    flags = SDL_INIT_VIDEO;

#ifdef DEBUG
  flags |= SDL_INIT_NOPARACHUTE;
#endif

  /*
   * e6y: Forcing "directx" video driver for Win9x.
   * The "windib" video driver is the default for SDL > 1.2.9, to prevent
   * problems with certain laptops, 64-bit Windows, and Windows Vista.  The
   * DirectX driver is still available, and can be selected by setting the
   * environment variable SDL_VIDEODRIVER to "directx".
   */

  if ((p = M_CheckParm("-videodriver")) && (p < myargc - 1)) {
    free(video_driver);
    video_driver = strdup(myargv[p + 1]);
  }

  if (strcasecmp(video_driver, "default")) { // videodriver != default
    char buf[80];

    strcpy(buf, "SDL_VIDEODRIVER=");
    strncat(buf, video_driver, sizeof(buf) - sizeof(buf[0]) - strlen(buf));
    putenv(buf);
  }
#ifdef _WIN32
#ifdef GL_DOOM
  else if ((int)GetVersion() < 0 && V_GetMode() != VID_MODEGL) {
#else
  else if ((int)GetVersion() < 0) {
#endif
    free(video_driver);
    video_driver = strdup("directx");
    putenv("SDL_VIDEODRIVER=directx");
  }
#endif

  p = SDL_Init(flags);

  if (p < 0 && strcasecmp(video_driver, "default")) {
    static const union {
      const char *c;
      char *s;
    } u = { "SDL_VIDEODRIVER=" };

    //e6y: wrong videodriver?
    lprintf(
      LO_ERROR,
      "Could not initialize SDL with SDL_VIDEODRIVER=%s [%s]\n",
      video_driver,
      SDL_GetError()
    );

    putenv(u.s);

    p = SDL_Init(flags);
  }

  free(video_driver);

  if (p < 0)
    I_Error("Could not initialize SDL [%s]", SDL_GetError());

  atexit(SDL_Quit);
}

// e6y: resolution limitation is removed
void I_InitBuffersRes(void) {
  R_InitMeltRes();
  R_InitSpritesRes();
  R_InitBuffersRes();
  R_InitPlanesRes();
  R_InitVisplanesRes();
}

// e6y
// It is a simple test of CPU cache misses.
unsigned int I_TestCPUCacheMisses(int width, int height,
                                             unsigned int mintime) {
  int   k = 0;
  char *s = malloc(width * height);
  char *d = malloc(width * height);
  char *ps;
  char *pd;
  unsigned int tickStart = SDL_GetTicks();

  do {
    pd = d;
    ps = s;

    for (int i = 0; i < height; i++) {
      pd[0] = ps[0];
      pd += width;
      ps += width;
    }

    k++;

  } while (SDL_GetTicks() - tickStart < mintime);

  free(d);
  free(s);

  return k;
}

void I_CalculateRes(int width, int height) {
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    if (desired_fullscreen)
      get_closest_resolution(&width, &height, SDL_OPENGL | SDL_FULLSCREEN);

    SCREENWIDTH = width;
    SCREENHEIGHT = height;
    SCREENPITCH = SCREENWIDTH;

    REAL_SCREENWIDTH  = SCREENWIDTH;
    REAL_SCREENHEIGHT = SCREENHEIGHT;
    REAL_SCREENPITCH  = SCREENPITCH;

    return;
  }
#endif

  unsigned int count1;
  unsigned int count2;
  int pitch1;
  int pitch2;

  SCREENWIDTH = width;//(width+15) & ~15;
  SCREENHEIGHT = height;

  // e6y
  // Trying to optimise screen pitch for reducing of CPU cache misses.
  // It is extremally important for wiping in software.
  // I have ~20x improvement in speed with using 1056 instead of 1024 on Pentium4
  // and only ~10% for Core2Duo
  if (try_to_reduce_cpu_cache_misses) {
    unsigned int mintime = 100;
    int w = (width + 15) & ~15;

    pitch1 = w * V_GetPixelDepth();
    pitch2 = w * V_GetPixelDepth() + 32;

    count1 = I_TestCPUCacheMisses(pitch1, SCREENHEIGHT, mintime);
    count2 = I_TestCPUCacheMisses(pitch2, SCREENHEIGHT, mintime);

    lprintf(LO_INFO, "I_CalculateRes: trying to optimize screen pitch\n");
    lprintf(LO_INFO,
      " test case for pitch=%d is processed %d times for %d msec\n",
      pitch1, count1, mintime
    );
    lprintf(LO_INFO,
      " test case for pitch=%d is processed %d times for %d msec\n",
      pitch2, count2, mintime
    );

    SCREENPITCH = (count2 > count1 ? pitch2 : pitch1);

    lprintf(LO_INFO, " optimized screen pitch is %d\n", SCREENPITCH);
  }
  else {
    SCREENPITCH = SCREENWIDTH * V_GetPixelDepth();
  }

  // e6y: processing of screen_multiply
  REAL_SCREENWIDTH  = SCREENWIDTH  * render_screen_multiply;
  REAL_SCREENHEIGHT = SCREENHEIGHT * render_screen_multiply;
  REAL_SCREENPITCH  = SCREENPITCH  * render_screen_multiply;
}

// CPhipps -
// I_InitScreenResolution
// Sets the screen resolution
// e6y: processing of screen_multiply
void I_InitScreenResolution(void) {
  int i, p, w, h;
  char c, x;
  video_mode_t mode;
  int init = screen == NULL;

  get_screen_resolution();

  if (init) {
    //e6y: ability to change screen resolution from GUI
    fill_screen_resolution_list();

    // Video stuff
    if ((p = M_CheckParm("-width"))) {
      if (myargv[p + 1])
        desired_screenwidth = atoi(myargv[p + 1]);
    }

    if ((p = M_CheckParm("-height"))) {
      if (myargv[p + 1])
        desired_screenheight = atoi(myargv[p + 1]);
    }

    if ((p = M_CheckParm("-fullscreen")))
      use_fullscreen = 1;

    if ((p = M_CheckParm("-nofullscreen")))
      use_fullscreen = 0;

    // e6y
    // New command-line options for setting a window (-window)
    // or fullscreen (-nowindow) mode temporarily which is not saved in cfg.
    // It works like "-geom" switch
    desired_fullscreen = use_fullscreen;
    if ((p = M_CheckParm("-window")))
      desired_fullscreen = 0;

    if ((p = M_CheckParm("-nowindow")))
      desired_fullscreen = 1;

    // e6y
    // change the screen size for the current session only
    // syntax: -geom WidthxHeight[w|f]
    // examples: -geom 320x200f, -geom 640x480w, -geom 1024x768
    w = desired_screenwidth;
    h = desired_screenheight;

    if (!(p = M_CheckParm("-geom")))
      p = M_CheckParm("-geometry");

    if (p && p + 1 < myargc) {
      int count = sscanf(myargv[p+1], "%d%c%d%c", &w, &x, &h, &c);

      // at least width and height must be specified
      // restoring original values if not
      if (count < 3 || tolower(x) != 'x') {
        w = desired_screenwidth;
        h = desired_screenheight;
      }
      else if (count >= 4) {
        if (tolower(c) == 'w')
          desired_fullscreen = 0;
        if (tolower(c) == 'f')
          desired_fullscreen = 1;
      }
    }
  }
  else {
    w = desired_screenwidth;
    h = desired_screenheight;
  }

  mode = I_GetModeFromString(default_videomode);
  if ((i = M_CheckParm("-vidmode")) && i < myargc - 1)
    mode = I_GetModeFromString(myargv[i + 1]);

  V_InitMode(mode);

  I_CalculateRes(w, h);
  V_DestroyUnusedTrueColorPalettes();
  V_FreeScreens();

  // set first three to standard values
  for (i = 0; i < 5; i++) {
    screens[i].width = REAL_SCREENWIDTH;
    screens[i].height = REAL_SCREENHEIGHT;
    screens[i].byte_pitch = REAL_SCREENPITCH;
    screens[i].int_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE32);
  }

  // statusbar
  /*
  screens[4].width = REAL_SCREENWIDTH;
  screens[4].height = REAL_SCREENHEIGHT;
  screens[4].byte_pitch = REAL_SCREENPITCH;
  screens[4].int_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE32);
  */

  I_InitBuffersRes();

  lprintf(LO_INFO, "I_InitScreenResolution: Using resolution %dx%d\n",
    REAL_SCREENWIDTH, REAL_SCREENHEIGHT
  );
}

void I_InitGraphics(void) {
  static bool initialized = false;

  if (initialized)
    return;

  initialized = true;

  lprintf(LO_INFO, "I_InitGraphics: %dx%d\n", SCREENWIDTH, SCREENHEIGHT);

  initialize_overlay();

  I_UpdateVideoMode();

  I_SetWindowCaption();

  I_SetWindowIcon();

  I_InputInit();

  I_VideoUpdateFocus();
  I_MouseUpdateGrab();
}

int I_GetModeFromString(const char *modestr) {
  if (!stricmp(modestr, "32"))
    return VID_MODE32;

  if (!stricmp(modestr, "32bit"))
    return VID_MODE32;

#ifdef GL_DOOM
  if (!stricmp(modestr, "gl"))
    return VID_MODEGL;

  if (!stricmp(modestr, "opengl"))
    return VID_MODEGL;
#endif

  return VID_MODE32;
}

void I_UpdateVideoMode(void) {
  int init_flags;

  if (screen) {
    // video capturing cannot be continued with new screen settings
    I_CaptureFinish();

#ifdef GL_DOOM
    if (V_GetMode() == VID_MODEGL) {
      gld_CleanMemory();
      gld_CleanStaticMemory(); // hires patches
    }
#endif

    I_InitScreenResolution();

    SDL_FreeSurface(screen);
    screen = NULL;

#ifdef GL_DOOM
    if (vid_8ingl.surface) {
      SDL_FreeSurface(vid_8ingl.surface);
      vid_8ingl.surface = NULL;
    }
#endif

    SMP_Free();
  }

  // e6y: initialisation of screen_multiply
  screen_multiply = render_screen_multiply;

  // Initialize SDL with this graphics mode
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    init_flags = SDL_OPENGL;
  else if (use_doublebuffer)
    init_flags = SDL_DOUBLEBUF;
  else
    init_flags = SDL_SWSURFACE;
#ifndef DEBUG
  init_flags |= SDL_HWPALETTE;
#endif
#else
  if (use_doublebuffer)
    init_flags = SDL_DOUBLEBUF;
  else
    init_flags = SDL_SWSURFACE;
#ifndef DEBUG
  init_flags |= SDL_HWPALETTE;
#endif
#endif

  if (desired_fullscreen)
    init_flags |= SDL_FULLSCREEN;

  // In windowed mode, the window can be resized while the game is
  // running.  This feature is disabled on OS X, as it adds an ugly
  // scroll handle to the corner of the screen.
#ifndef MACOSX
  if (!desired_fullscreen)
    init_flags |= SDL_RESIZABLE;
#endif

  if (sdl_video_window_pos && sdl_video_window_pos[0]) {
    char buf[80];

    strcpy(buf, "SDL_VIDEO_WINDOW_POS=");
    strncat(buf, sdl_video_window_pos, sizeof(buf) - sizeof(buf[0]) - strlen(buf));
    putenv(buf);
  }

#ifdef GL_DOOM
  vid_8ingl.enabled = false;

  if (V_GetMode() == VID_MODEGL) {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, gl_colorbuffer_bits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gl_depthbuffer_bits);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    //e6y: vertical sync
#if !SDL_VERSION_ATLEAST(1, 3, 0)
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (gl_vsync ? 1 : 0));
#endif

    //e6y: anti-aliasing
    gld_MultisamplingInit();

    screen = SDL_SetVideoMode(
      REAL_SCREENWIDTH, REAL_SCREENHEIGHT, gl_colorbuffer_bits, init_flags
    );

    gld_CheckHardwareGamma();
  }
  else if (use_gl_surface) {
    int flags = SDL_OPENGL;

    if (desired_fullscreen)
      flags |= SDL_FULLSCREEN;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, gl_colorbuffer_bits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gl_depthbuffer_bits);

    //e6y: vertical sync
#if !SDL_VERSION_ATLEAST(1, 3, 0)
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, (gl_vsync ? 1 : 0));
#endif

    vid_8ingl.surface = SDL_SetVideoMode(
      REAL_SCREENWIDTH, REAL_SCREENHEIGHT, gl_colorbuffer_bits, flags
    );

    if (vid_8ingl.surface == NULL) {
      I_Error("Couldn't set %dx%d video mode [%s]",
        REAL_SCREENWIDTH, REAL_SCREENHEIGHT, SDL_GetError()
      );
    }

    screen = SDL_CreateRGBSurface(
      init_flags & ~SDL_FULLSCREEN,
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT,
      V_GetNumPixelBits(),
      0x00FF0000,
      0x0000FF00,
      0x000000FF,
      0x00000000
    );

    vid_8ingl.screen = screen;

    vid_8ingl.enabled = true;
  }
  else {
    screen = SDL_SetVideoMode(
      REAL_SCREENWIDTH, REAL_SCREENHEIGHT, V_GetNumPixelBits(), init_flags
    );
  }
#else
  screen = SDL_SetVideoMode(
    REAL_SCREENWIDTH, REAL_SCREENHEIGHT, V_GetNumPixelBits(), init_flags
  );
#endif

  if (screen == NULL) {
    I_Error("Couldn't set %dx%d video mode [%s]",
      REAL_SCREENWIDTH, REAL_SCREENHEIGHT, SDL_GetError()
    );
  }

  SMP_Init();

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
#if SDL_VERSION_ATLEAST(1, 3, 0)
    SDL_GL_SetSwapInterval((gl_vsync ? 1 : 0));
#endif
    // gld_MultisamplingCheck();
  }
#endif

  lprintf(LO_INFO, "I_UpdateVideoMode: 0x%x, %s, %s\n",
    init_flags,
    screen->pixels ? "SDL buffer" : "own buffer",
    SDL_MUSTLOCK(screen) ? "lock-and-copy": "direct access"
  );

  // Get the info needed to render to the display
  if (screen_multiply == 1 && !SDL_MUSTLOCK(screen)) {
    screens[0].not_on_heap = true;
    screens[0].data = (unsigned char *) (screen->pixels);
    screens[0].byte_pitch = screen->pitch;
    screens[0].int_pitch = screen->pitch / V_GetModePixelDepth(VID_MODE32);
  }
  else {
    screens[0].not_on_heap = false;
  }

  V_AllocScreens();

  R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);

  // e6y: wide-res
  // Need some initialisations before level precache
  R_ExecuteSetViewSize();

  V_SetPalette(0);
#if 0
  upload_new_palette(0, true);
#endif

  ST_SetResolution();
  AM_SetResolution();

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
#if 0
    int temp;

    lprintf(LO_INFO,"SDL OpenGL PixelFormat:\n");
    SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_RED_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_GREEN_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_BLUE_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_STENCIL_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_RED_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_RED_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_GREEN_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_GREEN_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_BLUE_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_BLUE_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_ALPHA_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &temp );
    lprintf(LO_INFO,"    SDL_GL_DOUBLEBUFFER: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_BUFFER_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_BUFFER_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_DEPTH_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &temp );
    lprintf(LO_INFO,"    SDL_GL_MULTISAMPLESAMPLES: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_MULTISAMPLEBUFFERS, &temp );
    lprintf(LO_INFO,"    SDL_GL_MULTISAMPLEBUFFERS: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_STENCIL_SIZE: %i\n",temp);
#endif

    gld_Init(SCREENWIDTH, SCREENHEIGHT);
  }

  if (vid_8ingl.enabled)
    gld_Init8InGLMode();

  if (V_GetMode() == VID_MODEGL) {
    M_ChangeFOV();
    M_ChangeRenderPrecise();
    deh_changeCompTranslucency();
  }
#endif
  overlay.needs_resetting = true;
}

#if 0
static void ApplyWindowResize(SDL_Event *resize_event) {
  int i, k;
  char mode[80];

  memset(mode, 0, 80);

  int w = MAX(320, resize_event->resize.w);
  int h = MAX(200, resize_event->resize.h);

  if (!screen_resolution)
    return;

  if (!(SDL_GetModState() & KMOD_SHIFT)) {
    // Find the biggest screen mode that will fall within these
    // dimensions, falling back to the smallest mode possible if
    // none is found.

    Uint32 flags = SDL_FULLSCREEN;

    if (V_GetMode() == VID_MODEGL)
      flags |= SDL_OPENGL;

    get_closest_resolution(&w, &h, flags);
  }

  w = MAX(320, w);
  h = MAX(200, h);
  snprintf(mode, 80, "%dx%d", w, h);

  for (i = 0; i < MAX_RESOLUTIONS_COUNT; i++) {
    if (screen_resolutions_list[i] == NULL)
      break;

    if (!strcmp(mode, screen_resolutions_list[i])) {
      screen_resolution = screen_resolutions_list[i];
      V_ChangeScreenResolution();
      return;
    }
  }

  // custom resolution
  if (screen_resolution_lowest &&
      !strcmp(screen_resolution_lowest, screen_resolutions_list[0])) {
    // there is no "custom resolution" entry in the list
    for(k = MAX_RESOLUTIONS_COUNT - 1; k > 0; k--)
      screen_resolutions_list[k] = screen_resolutions_list[k - 1];
  }
  else {
    free((char *)screen_resolutions_list[0]);
  }

  // add it
  screen_resolutions_list[0] = strdup(mode);
  screen_resolution = screen_resolutions_list[0];

  V_ChangeScreenResolution();
}
#endif

/* vi: set et ts=2 sw=2: */


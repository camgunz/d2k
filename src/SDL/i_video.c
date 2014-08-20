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

#if CAIRO_HAS_XLIB_SURFACE
#include <cairo/cairo-xlib.h>

#if CAIRO_HAS_XLIB_XRENDER_SURFACE
#include <cairo/cairo-xlib-xrender.h>
#endif
#endif

#if CAIRO_HAS_QUARTZ_SURFACE
#include <cairo/cairo-quartz.h>
#endif

#if CAIRO_HAS_WIN32_SURFACE
#include <cairo/cairo-win32.h>
#endif

#include <SDL.h>
#include <SDL_syswm.h>

#include "m_argv.h"
#include "doomstat.h"
#include "doomdef.h"
#include "v_video.h"
#include "r_draw.h"
#include "r_things.h"
#include "r_plane.h"
#include "r_main.h"
#include "f_wipe.h"
#include "d_main.h"
#include "d_event.h"
#include "d_deh.h"
#include "i_joy.h"
#include "i_video.h"
#include "i_smp.h"
#include "i_capture.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "am_map.h"
#include "g_game.h"
#include "lprintf.h"
#include "p_user.h"

#ifdef GL_DOOM
#include "gl_struct.h"
#endif

#include "r_screenmultiply.h"
#include "e6y.h"//e6y
#include "i_main.h"

//e6y: new mouse code
static SDL_Cursor* cursors[2] = {NULL, NULL};

static unsigned char *local_pixels = NULL;
static cairo_t *render_context = NULL;
static cairo_surface_t *render_surface = NULL;
#ifdef GL_DOOM
static GLuint overlay_tex_id = 0;
#endif

dboolean window_focused;

// Window resize state.
static void ApplyWindowResize(SDL_Event *resize_event);

const char *sdl_videodriver;
const char *sdl_video_window_pos;

static void ActivateMouse(void);
static void DeactivateMouse(void);
//static int AccelerateMouse(int val);
static void CenterMouse(void);
static void I_ReadMouse(void);
static dboolean MouseShouldBeGrabbed();
static void UpdateFocus(void);

int gl_colorbuffer_bits = 16;
int gl_depthbuffer_bits = 16;

extern void M_QuitDOOM(int choice);
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

////////////////////////////////////////////////////////////////////////////
// Input code
int             leds_always_off = 0; // Expected by m_misc, not relevant

// Mouse handling
extern int     usemouse;        // config file var
static dboolean mouse_enabled; // usemouse, but can be overriden by -nomouse

int I_GetModeFromString(const char *modestr);

/////////////////////////////////////////////////////////////////////////////////
// Keyboard handling

//
//  Translates the key currently in key
//

const char* I_GetKeyString(int keycode) {
  return SDL_GetKeyName(keycode);
}

/////////////////////////////////////////////////////////////////////////////////
// Main input code

/* cph - pulled out common button code logic */
//e6y static 
int I_SDLtoDoomMouseState(Uint8 buttonstate) {
  return 0
      | (buttonstate & SDL_BUTTON(1) ? 1 : 0)
      | (buttonstate & SDL_BUTTON(2) ? 2 : 0)
      | (buttonstate & SDL_BUTTON(3) ? 4 : 0)
#if SDL_VERSION_ATLEAST(1, 2, 14)
      | (buttonstate & SDL_BUTTON(6) ? 8 : 0)
      | (buttonstate & SDL_BUTTON(7) ? 16 : 0)
#endif
      ;
}

static void I_GetEvent(void) {
  event_t event;

  event.type  = ev_none;
  event.data1 = 0;
  event.data2 = 0;
  event.data3 = 0;
  event.wchar = 0;

  SDL_Event SDLEvent;
  SDL_Event *Event = &SDLEvent;

  static int mwheeluptic = 0, mwheeldowntic = 0;

  while (SDL_PollEvent(Event)) {
    switch (Event->type) {
      case SDL_KEYDOWN:
#ifdef MACOSX
        if (Event->key.keysym.mod & KMOD_META) {
          // Switch windowed<->fullscreen if pressed <Command-F>
          if (Event->key.keysym.sym == SDLK_f) {
            V_ToggleFullscreen();
            break;
          }
        }
#else
        if (Event->key.keysym.mod & KMOD_LALT) {
          // Prevent executing action on Alt-Tab
          if (Event->key.keysym.sym == SDLK_TAB)
            break;

          // Switch windowed<->fullscreen if pressed Alt-Enter
          if (Event->key.keysym.sym == SDLK_RETURN) {
            V_ToggleFullscreen();
            break;
          }

          // Immediately exit on Alt+F4 ("Boss Key")
          if (Event->key.keysym.sym == SDLK_F4) {
            I_SafeExit(0);
            break;
          }
        }
#endif
        event.type = ev_keydown;
        event.data1 = Event->key.keysym.sym;
        event.wchar = Event->key.keysym.unicode;
        D_PostEvent(&event);
      break;

      case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = Event->key.keysym.sym;
        D_PostEvent(&event);
      break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if (mouse_enabled && window_focused) {
          event.type = ev_mouse;
          event.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
          event.data2 = event.data3 = 0;

          if (Event->type == SDL_MOUSEBUTTONDOWN) {
            switch(Event->button.button) {
              case SDL_BUTTON_WHEELUP:
                event.type = ev_keydown;
                event.data1 = SDL_BUTTON_WHEELUP;
                mwheeluptic = gametic;
              break;
              case SDL_BUTTON_WHEELDOWN:
                event.type = ev_keydown;
                event.data1 = SDL_BUTTON_WHEELDOWN;
                mwheeldowntic = gametic;
              break;
            }
          }

          D_PostEvent(&event);
        }
      break;
      //e6y: new mouse code
      case SDL_ACTIVEEVENT:
        UpdateFocus();
      break;
      case SDL_VIDEORESIZE:
        ApplyWindowResize(Event);
      break;
      case SDL_QUIT:
        S_StartSound(NULL, sfx_swtchn);
      M_QuitDOOM(0);
      default:
      break;
    }
  }

  if (mwheeluptic && mwheeluptic + 1 < gametic) {
    event.type = ev_keyup;
    event.data1 = SDL_BUTTON_WHEELUP;
    D_PostEvent(&event);
    mwheeluptic = 0;
  }

  if (mwheeldowntic && mwheeldowntic + 1 < gametic) {
    event.type = ev_keyup;
    event.data1 = SDL_BUTTON_WHEELDOWN;
    D_PostEvent(&event);
    mwheeldowntic = 0;
  }
}

//
// I_StartTic
//

void I_StartTic(void) {
  I_GetEvent();
  I_ReadMouse();
  I_PollJoystick();
}

//
// I_StartFrame
//
void I_StartFrame(void) {
  cairo_set_operator(render_context, CAIRO_OPERATOR_CLEAR);
  cairo_paint(render_context);
}

//
// I_InitInputs
//

static void I_InitInputs(void) {
  static Uint8 empty_cursor_data = 0;

  int nomouse_parm = M_CheckParm("-nomouse");

  // check if the user wants to use the mouse
  mouse_enabled = usemouse && !nomouse_parm;
  
  SDL_PumpEvents();

  // Save the default cursor so it can be recalled later
  cursors[0] = SDL_GetCursor();
  // Create an empty cursor
  cursors[1] = SDL_CreateCursor(&empty_cursor_data, &empty_cursor_data, 8, 1, 0, 0);

  if (mouse_enabled) {
    CenterMouse();
    MouseAccelChanging();
  }

  I_InitJoystick();
}

/////////////////////////////////////////////////////////////////////////////

// I_SkipFrame
//
// Returns true if it thinks we can afford to skip this frame

#if 0
inline static dboolean I_SkipFrame(void)
{
  static int frameno;

  frameno++;
  switch (gamestate) {
  case GS_LEVEL:
    if (!paused)
      return false;
  default:
    // Skip odd frames
    return (frameno & 1) ? true : false;
  }
}
#endif

void* I_GetRenderContext(void) {
  cairo_status_t status;
  SDL_SysWMinfo wm_info;

  if (nodrawers)
    return NULL;

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    if (render_surface && render_context && overlay_tex_id)
      return render_context;
  }
  else
#endif
  if (render_surface && render_context)
    return render_context;

  SDL_VERSION(&wm_info.version);
  SDL_GetWMInfo(&wm_info);

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    if (render_surface == NULL) {
      if (local_pixels)
        free(local_pixels);

      local_pixels = malloc(
        REAL_SCREENWIDTH * REAL_SCREENHEIGHT * sizeof(unsigned int)
      );

      if (local_pixels == NULL)
        I_Error("I_GetRenderContext: malloc'ing pixels failed");

      render_surface = cairo_image_surface_create_for_data(
        local_pixels,
        CAIRO_FORMAT_ARGB32,
        REAL_SCREENWIDTH,
        REAL_SCREENHEIGHT,
        cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, REAL_SCREENWIDTH)
      );
    }
  }
#endif

#if CAIRO_HAS_WIN32_SURFACE
  if (render_surface == NULL) {
    render_surface = cairo_win32_surface_create_with_dib(
      CAIRO_FORMAT_RGB24,
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT
    );
  }
#endif

#if CAIRO_HAS_XLIB_SURFACE
  if (render_surface == NULL) {
    Display *dpy = wm_info.info.x11.gfxdisplay;

    render_surface = cairo_xlib_surface_create(
      wm_info.info.x11.gfxdisplay,
      (Drawable)wm_info.info.x11.window,
      DefaultVisual(dpy, 0),
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT
    );

#if CAIRO_HAS_XLIB_XRENDER_SURFACE
    status = cairo_surface_status(render_surface);

    if (status != CAIRO_STATUS_SUCCESS) {
      I_Error("I_GetRenderContext: Error creating cairo surface (%s)",
        cairo_status_to_string(status)
      );
    }

    cairo_surface_t *xr = cairo_xlib_surface_create_with_xrender_format(
      wm_info.info.x11.gfxdisplay,
      cairo_xlib_surface_get_drawable(render_surface),
      cairo_xlib_surface_get_screen(render_surface),
      cairo_xlib_surface_get_xrender_format(render_surface),
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT
    );

    cairo_surface_destroy(render_surface);
    render_surface = xr;
#endif
  }
#endif

#if CAIRO_HAS_QUARTZ_SURFACE
  if (render_surface == NULL) {
    render_surface = cairo_quartz_surface_create(
      CAIRO_FORMAT_ARGB32,
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT
    );
  }
#endif
  if (render_surface == NULL) {
    if (local_pixels)
      free(local_pixels);

    local_pixels = malloc(
      REAL_SCREENWIDTH * REAL_SCREENHEIGHT * sizeof(unsigned int)
    );

    if (local_pixels == NULL)
      I_Error("I_GetRenderContext: malloc'ing pixels failed");

    render_surface = cairo_image_surface_create_for_data(
      local_pixels,
      CAIRO_FORMAT_ARGB32,
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT,
      cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, REAL_SCREENWIDTH)
    );
  }

  status = cairo_surface_status(render_surface);

  if (status != CAIRO_STATUS_SUCCESS) {
    I_Error("I_GetRenderContext: Error creating cairo surface (%s)",
      cairo_status_to_string(status)
    );
  }

  if (render_context)
    cairo_set_source_surface(render_context, render_surface, 0.0, 0.0);
  else
    render_context = cairo_create(render_surface);

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL && !overlay_tex_id) {
    glGenTextures(1, &overlay_tex_id);

    glBindTexture(GL_TEXTURE_2D, overlay_tex_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLEXT_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLEXT_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  }
#endif

  return render_context;
}

void I_ResetRenderContext(void) {
  if (nodrawers)
    return;

  if (render_surface) {
    cairo_surface_destroy(render_surface);
    render_surface = NULL;
  }

  if (render_context) {
    cairo_destroy(render_context);
    render_context = NULL;
  }

#ifdef GL_DOOM
  if (overlay_tex_id) {
    glDeleteTextures(1, &overlay_tex_id);
    overlay_tex_id = 0;
  }
#endif
}

///////////////////////////////////////////////////////////
// Palette stuff.
//
static void I_UploadNewPalette(int pal, int force)
{
  // This is used to replace the current 256 colour cmap with a new one
  // Used by 256 colour PseudoColor modes

  // Array of SDL_Color structs used for setting the 256-colour palette
  static SDL_Color* colours;
  static int cachedgamma;
  static size_t num_pals;

  if (V_GetMode() == VID_MODEGL)
    return;

  if ((colours == NULL) || (cachedgamma != usegamma) || force) {
    int pplump = W_GetNumForName("PLAYPAL");
    int gtlump = (W_CheckNumForName)("GAMMATBL",ns_prboom);
    register const byte * palette = W_CacheLumpNum(pplump);
    register const byte * const gtable = (const byte *)W_CacheLumpNum(gtlump) + 256*(cachedgamma = usegamma);
    register int i;

    num_pals = W_LumpLength(pplump) / (3*256);
    num_pals *= 256;

    if (!colours) {
      // First call - allocate and prepare colour array
      colours = malloc(sizeof(*colours)*num_pals);
#ifdef GL_DOOM
      vid_8ingl.colours = malloc(sizeof(vid_8ingl.colours[0]) * 4 * num_pals);
#endif
    }

    // set the colormap entries
    for (i=0 ; (size_t)i<num_pals ; i++) {
#ifdef GL_DOOM
      if (!vid_8ingl.enabled)
#endif
      {
        colours[i].r = gtable[palette[0]];
        colours[i].g = gtable[palette[1]];
        colours[i].b = gtable[palette[2]];
      }

      palette += 3;
    }

    W_UnlockLumpNum(pplump);
    W_UnlockLumpNum(gtlump);
    num_pals/=256;
  }

#ifdef RANGECHECK
  if ((size_t)pal >= num_pals)
    I_Error("I_UploadNewPalette: Palette number out of range (%d>=%d)",
      pal, num_pals);
#endif

  // store the colors to the current display
  // SDL_SetColors(SDL_GetVideoSurface(), colours+256*pal, 0, 256);
#ifdef GL_DOOM
  if (vid_8ingl.enabled) {
    vid_8ingl.palette = pal;
  }
  else
#endif
  {
    SDL_SetPalette(
      SDL_GetVideoSurface(),
      SDL_LOGPAL | SDL_PHYSPAL,
      colours + 256 * pal,
      0,
      256
    );
  }
}

//////////////////////////////////////////////////////////////////////////////
// Graphics API

void I_ShutdownGraphics(void) {
  SDL_FreeCursor(cursors[1]);
  DeactivateMouse();
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void) {
}

void I_ReadOverlay(void) {
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    glBindTexture(GL_TEXTURE_2D, overlay_tex_id);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      cairo_image_surface_get_data(render_surface)
    );

    return;
  }
#endif

  if (SDL_MUSTLOCK(screen)) {
    if (SDL_LockSurface(screen) < 0) {
      lprintf(LO_INFO, "I_ReadOverlay: %s\n", SDL_GetError());
      return;
    }
  }

  cairo_surface_t *csurf = cairo_surface_map_to_image(render_surface, NULL);
  SDL_Surface *screen_surf = SDL_GetVideoSurface();
  SDL_Surface *sdl_surf;
  
  if (csurf == NULL)
    I_Error("I_ReadOverlay: Error getting cairo image surface");

  if (screen_surf == NULL) {
    I_Error("I_ReadOverlay: Error getting current SDL display surface (%s)",
      SDL_GetError()
    );
  }

  sdl_surf = SDL_CreateRGBSurfaceFrom(
    cairo_image_surface_get_data(csurf),
    cairo_image_surface_get_width(csurf),
    cairo_image_surface_get_height(csurf),
    V_GetNumPixelBits(),
    cairo_image_surface_get_width(csurf) * (V_GetNumPixelBits() / 8),
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0xFF000000
  );

  if (sdl_surf == NULL) {
    I_Error("I_ReadOverlay: Error getting SDL surface (%s)",
      SDL_GetError()
    );
  }

#if CAIRO_HAS_WIN32_SURFACE
  SDL_SetAlpha(sdl_surf, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
#else
  SDL_SetAlpha(sdl_surf, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_TRANSPARENT);
#endif

  SDL_BlitSurface(sdl_surf, NULL, screen_surf, NULL);

  SDL_FreeSurface(sdl_surf);

  cairo_surface_destroy(csurf);

  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void I_RenderOverlay(void) {
  if (render_context == NULL || render_surface == NULL)
    I_GetRenderContext();

  cairo_set_operator(render_context, CAIRO_OPERATOR_SOURCE);

  I_ReadOverlay();

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    cairo_set_operator(render_context, CAIRO_OPERATOR_OVER);
    cairo_surface_flush(render_surface);

    glBindTexture(GL_TEXTURE_2D, overlay_tex_id);

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
  }
#endif
}

//
// I_FinishUpdate
//
static int newpal = 0;
#define NO_PALETTE_CHANGE 1000

void I_FinishUpdate(void) {
  //e6y: new mouse code
  UpdateGrab();

  // The screen wipe following pressing the exit switch on a level
  // is noticably jerkier with I_SkipFrame
  // if (I_SkipFrame())return;

#ifdef MONITOR_VISIBILITY
  if (!(SDL_GetAppState() & SDL_APPACTIVE)) {
    return;
  }
#endif

  I_RenderOverlay();

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    // proff 04/05/2000: swap OpenGL buffers
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

    // e6y: processing of screen_multiply
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
      h = screen->h;

      while (h-- > 0) {
        memcpy(dest, src, SCREENWIDTH * V_GetPixelDepth()); //e6y
        dest += screen->pitch;
        src += screens[0].byte_pitch;
      }
    }

    SDL_UnlockSurface(screen);
  }

  /* Update the display buffer (flipping video pages if supported)
   * If we need to change palette, that implicitely does a flip */
  if (newpal != NO_PALETTE_CHANGE) {
    I_UploadNewPalette(newpal, false);
    newpal = NO_PALETTE_CHANGE;
  }

#ifdef GL_DOOM
  if (vid_8ingl.enabled)
    gld_Draw8InGL();
  else
#endif
    SDL_Flip(screen);
}

//
// I_ScreenShot - moved to i_sshot.c
//

//
// I_SetPalette
//
void I_SetPalette (int pal)
{
  newpal = pal;
}

// I_PreInitGraphics

static void I_ShutdownSDL(void)
{
  SDL_Quit();
  return;
}

void I_PreInitGraphics(void)
{
  int p;
  char *video_driver = strdup(sdl_videodriver);

  // Initialize SDL
  unsigned int flags = 0;
  if (!(M_CheckParm("-nodraw") && M_CheckParm("-nosound")))
    flags = SDL_INIT_VIDEO;
#ifdef DEBUG
  flags |= SDL_INIT_NOPARACHUTE;
#endif

  // e6y: Forcing "directx" video driver for Win9x.
  // The "windib" video driver is the default for SDL > 1.2.9, 
  // to prevent problems with certain laptops, 64-bit Windows, and Windows Vista.  
  // The DirectX driver is still available, and can be selected by setting 
  // the environment variable SDL_VIDEODRIVER to "directx".

  if ((p = M_CheckParm("-videodriver")) && (p < myargc - 1))
  {
    free(video_driver);
    video_driver = strdup(myargv[p + 1]);
  }

  if (strcasecmp(video_driver, "default"))
  {
    // videodriver != default
    char buf[80];
    strcpy(buf, "SDL_VIDEODRIVER=");
    strncat(buf, video_driver, sizeof(buf) - sizeof(buf[0]) - strlen(buf));
    putenv(buf);
  }
#ifdef _WIN32
  else if ((int)GetVersion() < 0 && V_GetMode() != VID_MODEGL ) { // win9x
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
  {
    I_Error("Could not initialize SDL [%s]", SDL_GetError());
  }

  atexit(I_ShutdownSDL);
}

// e6y: resolution limitation is removed
void I_InitBuffersRes(void)
{
  R_InitMeltRes();
  R_InitSpritesRes();
  R_InitBuffersRes();
  R_InitPlanesRes();
  R_InitVisplanesRes();
}

#define MAX_RESOLUTIONS_COUNT 128
const char *screen_resolutions_list[MAX_RESOLUTIONS_COUNT] = {NULL};
const char *screen_resolution_lowest;
const char *screen_resolution = NULL;

//
// I_GetScreenResolution
// Get current resolution from the config variable (WIDTHxHEIGHT format)
// 640x480 if screen_resolution variable has wrong data
//
void I_GetScreenResolution(void)
{
  int width, height;

  desired_screenwidth = 640;
  desired_screenheight = 480;

  if (screen_resolution)
  {
    if (sscanf(screen_resolution, "%dx%d", &width, &height) == 2)
    {
      desired_screenwidth = width;
      desired_screenheight = height;
    }
  }
}

//
// I_FillScreenResolutionsList
// Get all the supported screen resolutions
// and fill the list with them
//
static void I_FillScreenResolutionsList(void) {
  SDL_Rect **modes;
  int i;
  int j;
  int list_size;
  int current_resolution_index;
  int count;
  char mode[256];
  Uint32 flags;

  // do it only once
  if (screen_resolutions_list[0])
    return;

  if (desired_screenwidth == 0 || desired_screenheight == 0)
    I_GetScreenResolution();

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
    count = 0;

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

// e6y
// GLBoom use this function for trying to set the closest supported resolution if the requested mode can't be set correctly.
// For example glboom.exe -geom 1025x768 -nowindow will set 1024x768.
// It should be used only for fullscreen modes.
static void I_ClosestResolution (int *width, int *height, int flags)
{
  SDL_Rect **modes;
  int twidth, theight;
  int cwidth = 0, cheight = 0;
  int i;
  unsigned int closest = UINT_MAX;
  unsigned int dist;

  if (!SDL_WasInit(SDL_INIT_VIDEO))
    return;

  modes = SDL_ListModes(NULL, flags);

  if (modes == (SDL_Rect **)-1)
  {
    // any dimension is okay for the given format
    return;
  }

  if (modes)
  {
    for(i=0; modes[i]; ++i)
    {
      twidth = modes[i]->w;
      theight = modes[i]->h;

      if (twidth == *width && theight == *height)
        return;

      //if (iteration == 0 && (twidth < *width || theight < *height))
      //  continue;

      dist = (twidth - *width) * (twidth - *width) + 
             (theight - *height) * (theight - *height);

      if (dist < closest)
      {
        closest = dist;
        cwidth = twidth;
        cheight = theight;
      }
    }
    if (closest != 4294967295u)
    {
      *width = cwidth;
      *height = cheight;
      return;
    }
  }
}  

int process_affinity_mask;
int process_priority;
int try_to_reduce_cpu_cache_misses;

// e6y
// It is a simple test of CPU cache misses.
unsigned int I_TestCPUCacheMisses(int width, int height, unsigned int mintime)
{
  int i, k;
  char *s, *d, *ps, *pd;
  unsigned int tickStart;
  
  s = malloc(width * height);
  d = malloc(width * height);

  tickStart = SDL_GetTicks();
  k = 0;
  do
  {
    ps = s;
    pd = d;
    for(i = 0; i < height; i++)
    {
      pd[0] = ps[0];
      pd += width;
      ps += width;
    }
    k++;
  }
  while (SDL_GetTicks() - tickStart < mintime);

  free(d);
  free(s);

  return k;
}

// CPhipps -
// I_CalculateRes
// Calculates the screen resolution, possibly using the supplied guide
void I_CalculateRes(int width, int height)
{
// e6y
// GLBoom will try to set the closest supported resolution 
// if the requested mode can't be set correctly.
// For example glboom.exe -geom 1025x768 -nowindow will set 1024x768.
// It affects only fullscreen modes.
  if (V_GetMode() == VID_MODEGL) {
    if ( desired_fullscreen )
    {
      I_ClosestResolution(&width, &height, SDL_OPENGL|SDL_FULLSCREEN);
    }
    SCREENWIDTH = width;
    SCREENHEIGHT = height;
    SCREENPITCH = SCREENWIDTH;
  } else {
    unsigned int count1, count2;
    int pitch1, pitch2;

    SCREENWIDTH = width;//(width+15) & ~15;
    SCREENHEIGHT = height;

    // e6y
    // Trying to optimise screen pitch for reducing of CPU cache misses.
    // It is extremally important for wiping in software.
    // I have ~20x improvement in speed with using 1056 instead of 1024 on Pentium4
    // and only ~10% for Core2Duo
    if (try_to_reduce_cpu_cache_misses)
    {
      unsigned int mintime = 100;
      int w = (width+15) & ~15;
      pitch1 = w * V_GetPixelDepth();
      pitch2 = w * V_GetPixelDepth() + 32;

      count1 = I_TestCPUCacheMisses(pitch1, SCREENHEIGHT, mintime);
      count2 = I_TestCPUCacheMisses(pitch2, SCREENHEIGHT, mintime);

      lprintf(LO_INFO, "I_CalculateRes: trying to optimize screen pitch\n");
      lprintf(LO_INFO, " test case for pitch=%d is processed %d times for %d msec\n", pitch1, count1, mintime);
      lprintf(LO_INFO, " test case for pitch=%d is processed %d times for %d msec\n", pitch2, count2, mintime);

      SCREENPITCH = (count2 > count1 ? pitch2 : pitch1);

      lprintf(LO_INFO, " optimized screen pitch is %d\n", SCREENPITCH);
    }
    else
    {
      SCREENPITCH = SCREENWIDTH * V_GetPixelDepth();
    }
  }

  // e6y: processing of screen_multiply
  {
    int factor = ((V_GetMode() == VID_MODEGL) ? 1 : render_screen_multiply);
    REAL_SCREENWIDTH = SCREENWIDTH * factor;
    REAL_SCREENHEIGHT = SCREENHEIGHT * factor;
    REAL_SCREENPITCH = SCREENPITCH * factor;
  }
}

// CPhipps -
// I_InitScreenResolution
// Sets the screen resolution
// e6y: processing of screen_multiply
void I_InitScreenResolution(void)
{
  int i, p, w, h;
  char c, x;
  video_mode_t mode;
  int init = screen == NULL;

  I_GetScreenResolution();

  if (init) {
    //e6y: ability to change screen resolution from GUI
    I_FillScreenResolutionsList();

    // Video stuff
    if ((p = M_CheckParm("-width"))) {
      if (myargv[p+1])
        desired_screenwidth = atoi(myargv[p + 1]);
    }

    if ((p = M_CheckParm("-height"))) {
      if (myargv[p+1])
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
  for (i = 0; i < 3; i++) {
    screens[i].width = REAL_SCREENWIDTH;
    screens[i].height = REAL_SCREENHEIGHT;
    screens[i].byte_pitch = REAL_SCREENPITCH;
    screens[i].int_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE32);
  }

  // statusbar
  screens[4].width = REAL_SCREENWIDTH;
  screens[4].height = REAL_SCREENHEIGHT;
  screens[4].byte_pitch = REAL_SCREENPITCH;
  screens[4].int_pitch = REAL_SCREENPITCH / V_GetModePixelDepth(VID_MODE32);

  I_InitBuffersRes();

  lprintf(LO_INFO, "I_InitScreenResolution: Using resolution %dx%d\n",
    REAL_SCREENWIDTH, REAL_SCREENHEIGHT
  );
}

// 
// Set the window caption
//

void I_SetWindowCaption(void) {
  size_t len = strlen(PACKAGE_NAME) + strlen(PACKAGE_VERSION) + 3;
  char *buf = calloc(len, sizeof(char));

  if (buf == NULL)
    I_Error("I_SetWindowCaption: calloc failed\n");

  snprintf(buf, len, "%s v%s", PACKAGE_NAME, PACKAGE_VERSION);

  SDL_WM_SetCaption(buf, NULL);

  free(buf);
}

// 
// Set the application icon
// 

#include "icon.c"

void I_SetWindowIcon(void)
{
  static SDL_Surface *surface = NULL;

  // do it only once, because of crash in SDL_InitVideoMode in SDL 1.3
  if (!surface)
  {
    surface = SDL_CreateRGBSurfaceFrom(icon_data,
      icon_w, icon_h, 32, icon_w * 4,
      0xff << 0, 0xff << 8, 0xff << 16, 0xff << 24);
  }

  if (surface)
  {
    SDL_WM_SetIcon(surface, NULL);
  }
}

void I_InitGraphics(void)
{
  static int    firsttime=1;

  if (firsttime)
  {
    firsttime = 0;

    atexit(I_ShutdownGraphics);
    lprintf(LO_INFO, "I_InitGraphics: %dx%d\n", SCREENWIDTH, SCREENHEIGHT);

    /* Set the video mode */
    I_UpdateVideoMode();

    //e6y: setup the window title
    I_SetWindowCaption();

    //e6y: set the application icon
    I_SetWindowIcon();

    /* Initialize the input system */
    I_InitInputs();

    //e6y: new mouse code
    UpdateFocus();
    UpdateGrab();
  }
}

int I_GetModeFromString(const char *modestr)
{
  if (!stricmp(modestr, "32"))
    return VID_MODE32;

  if (!stricmp(modestr, "32bit"))
    return VID_MODE32;

  if (!stricmp(modestr, "gl"))
    return VID_MODEGL;

  if (!stricmp(modestr, "opengl"))
    return VID_MODEGL;

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
  if (V_GetMode() == VID_MODEGL) {
    init_flags = SDL_OPENGL;
  }
  else
#endif
  {
    if (use_doublebuffer)
      init_flags = SDL_DOUBLEBUF;
    else
      init_flags = SDL_SWSURFACE;
#ifndef DEBUG
    init_flags |= SDL_HWPALETTE;
#endif
  }

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
#endif

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
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
      REAL_SCREENWIDTH,
      REAL_SCREENHEIGHT,
      gl_colorbuffer_bits,
      init_flags
    );

    gld_CheckHardwareGamma();
#endif
  }
  else
  {
#ifdef GL_DOOM
    if (use_gl_surface)
    {
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
        REAL_SCREENWIDTH, REAL_SCREENHEIGHT,
        gl_colorbuffer_bits, flags
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
    else
#endif
    {
      screen = SDL_SetVideoMode(
        REAL_SCREENWIDTH,
        REAL_SCREENHEIGHT,
        V_GetNumPixelBits(),
        init_flags
      );
    }
  }

  if (screen == NULL) {
    I_Error("Couldn't set %dx%d video mode [%s]",
      REAL_SCREENWIDTH, REAL_SCREENHEIGHT, SDL_GetError()
    );
  }

  SMP_Init();

#if SDL_VERSION_ATLEAST(1, 3, 0)
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    SDL_GL_SetSwapInterval((gl_vsync ? 1 : 0));
#endif
#endif

#ifdef GL_DOOM
  /*if (V_GetMode() == VID_MODEGL)
    gld_MultisamplingCheck();*/
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
  I_UploadNewPalette(0, true);

  ST_SetResolution();
  AM_SetResolution();

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
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

    gld_Init(SCREENWIDTH, SCREENHEIGHT);
  }

  if (vid_8ingl.enabled)
  {
    gld_Init8InGLMode();
  }

  if (V_GetMode() == VID_MODEGL)
  {
    M_ChangeFOV();
    M_ChangeRenderPrecise();
    deh_changeCompTranslucency();
  }
#endif

  I_ResetRenderContext();
  I_GetRenderContext();
}

static void ActivateMouse(void)
{
  SDL_WM_GrabInput(SDL_GRAB_ON);
#if SDL_VERSION_ATLEAST(1, 3, 0)
  SDL_ShowCursor(0);
#else
  SDL_SetCursor(cursors[1]);
  SDL_ShowCursor(1);
#endif
}

static void DeactivateMouse(void)
{
  SDL_WM_GrabInput(SDL_GRAB_OFF);
#if !SDL_VERSION_ATLEAST(1, 3, 0)
  SDL_SetCursor(cursors[0]);
#endif
  SDL_ShowCursor(1);
}

// Warp the mouse back to the middle of the screen
static void CenterMouse(void)
{
  // Warp the the screen center
  SDL_WarpMouse((unsigned short)(REAL_SCREENWIDTH/2), (unsigned short)(REAL_SCREENHEIGHT/2));

  // Clear any relative movement caused by warping
  SDL_PumpEvents();
  SDL_GetRelativeMouseState(NULL, NULL);
}

//
// Read the change in mouse state to generate mouse motion events
//
// This is to combine all mouse movement for a tic into one mouse
// motion event.
static void I_ReadMouse(void)
{
  static int mouse_currently_grabbed = true;
  int x, y;
  event_t ev;

  if (!usemouse)
    return;

  if (!MouseShouldBeGrabbed())
  {
    mouse_currently_grabbed = false;
    return;
  }

  if (!mouse_currently_grabbed && !desired_fullscreen)
  {
    CenterMouse();
    mouse_currently_grabbed = true;
  }

  SDL_GetRelativeMouseState(&x, &y);

  if (x != 0 || y != 0) 
  {
    ev.type = ev_mouse;
    ev.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
    ev.data2 = x << 5;
    ev.data3 = (-y) << 5;

    D_PostEvent(&ev);
  }

  CenterMouse();
}

static dboolean MouseShouldBeGrabbed()
{
  // never grab the mouse when in screensaver mode

  //if (screensaver_mode)
  //    return false;

  // if the window doesnt have focus, never grab it
  if (!window_focused)
    return false;

  // always grab the mouse when full screen (dont want to 
  // see the mouse pointer)
  if (desired_fullscreen)
    return true;

  // if we specify not to grab the mouse, never grab
  if (!mouse_enabled)
    return false;

  // always grab the mouse in camera mode when playing levels 
  // and menu is not active
  if (walkcamera.type)
    return (demoplayback && gamestate == GS_LEVEL && !menuactive);

  // when menu is active or game is paused, release the mouse 
  if (menuactive || paused)
    return false;

  // only grab mouse when playing levels (but not demos)
  return (gamestate == GS_LEVEL) && !demoplayback;
}

// Update the value of window_focused when we get a focus event
//
// We try to make ourselves be well-behaved: the grab on the mouse
// is removed if we lose focus (such as a popup window appearing),
// and we dont move the mouse around if we aren't focused either.
static void UpdateFocus(void)
{
  Uint8 state;

  state = SDL_GetAppState();

  // We should have input (keyboard) focus and be visible 
  // (not minimised)
  window_focused = (state & SDL_APPINPUTFOCUS) && (state & SDL_APPACTIVE);

  // e6y
  // Reuse of a current palette to avoid black screen at software fullscreen modes
  // after switching to OS and back
  if (desired_fullscreen && window_focused)
  {
    // currentPaletteIndex?
    if (st_palette < 0)
      st_palette = 0;

    V_SetPalette(st_palette);
  }

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    if (gl_hardware_gamma)
    {
      if (!window_focused)
      {
        // e6y: Restore of startup gamma if window loses focus
        gld_SetGammaRamp(-1);
      }
      else
      {
        gld_SetGammaRamp(useglgamma);
      }
    }
  }
#endif

  // Should the screen be grabbed?
  //    screenvisible = (state & SDL_APPACTIVE) != 0;
}

void UpdateGrab(void) {
  static dboolean currently_grabbed = false;
  dboolean grab;

  grab = MouseShouldBeGrabbed();

  if (grab && !currently_grabbed)
    ActivateMouse();

  if (!grab && currently_grabbed)
    DeactivateMouse();

  currently_grabbed = grab;
}

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

    I_ClosestResolution(&w, &h, flags);
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

/* vi: set et ts=2 sw=2: */


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

#ifndef PRBOOM_SERVER
#include "m_argv.h"
#endif

#include "lprintf.h"
#include "doomtype.h"
#include "doomdef.h"
#include "m_file.h"

#ifndef PRBOOM_SERVER
#include "d_player.h"
#include "m_fixed.h"
#include "r_fps.h"
#include "e6y.h"
#endif

#include "i_system.h"

#ifdef __GNUG__
#pragma implementation "i_system.h"
#endif

static int basetime = 0;

int ms_to_next_tick;

uint32_t I_GetTicks(void) {
  return SDL_GetTicks();
}

void I_Sleep(unsigned long ms) {
  SDL_Delay(ms);
}

int I_GetTime_RealTime(void) {
  int i;
  int t = SDL_GetTicks();

  //e6y: removing startup delay
  if (basetime == 0)
    basetime = t;

  t -= basetime;

  i = t * (TICRATE / 5) / 200;

  ms_to_next_tick = (i + 1) * 200 / (TICRATE / 5) - t;
  if (ms_to_next_tick > 1000 / TICRATE || ms_to_next_tick < 1)
      ms_to_next_tick = 1;

  return i;
}

#ifndef PRBOOM_SERVER
static unsigned int start_displaytime;
static unsigned int displaytime;
static dboolean InDisplay = false;
static int saved_gametic = -1;
dboolean realframe = false;

dboolean I_StartDisplay(void)
{
  if (InDisplay)
    return false;

  realframe = (!movement_smooth) || (gametic > saved_gametic);

  if (realframe)
    saved_gametic = gametic;

  start_displaytime = SDL_GetTicks();
  InDisplay = true;

  return true;
}

void I_EndDisplay(void)
{
  displaytime = SDL_GetTicks() - start_displaytime;
  InDisplay = false;
}

static int subframe = 0;
static int prevsubframe = 0;
int interpolation_method;
fixed_t I_GetTimeFrac (void)
{
  unsigned long now;
  fixed_t frac;

  now = SDL_GetTicks();

  subframe++;

  if (tic_vars.step == 0)
  {
    frac = FRACUNIT;
  }
  else
  {
    extern int renderer_fps;
    if ((interpolation_method == 0) || (prevsubframe <= 0) || (renderer_fps <= 0))
    {
      frac = (fixed_t)((now - tic_vars.start + displaytime) * FRACUNIT / tic_vars.step);
    }
    else
    {
      frac = (fixed_t)((now - tic_vars.start) * FRACUNIT / tic_vars.step);
      frac = (unsigned int)((float)FRACUNIT * TICRATE * subframe / renderer_fps);
    }
    frac = BETWEEN(0, FRACUNIT, frac);
  }

  return frac;
}

void I_GetTime_SaveMS(void)
{
  if (!movement_smooth)
    return;

  tic_vars.start = SDL_GetTicks();
  tic_vars.next = (unsigned int) ((tic_vars.start * tic_vars.msec + 1.0f) / tic_vars.msec);
  tic_vars.step = tic_vars.next - tic_vars.start;
  prevsubframe = subframe;
  subframe = 0;
}
#endif

/*
 * I_GetRandomTimeSeed
 *
 * CPhipps - extracted from G_ReloadDefaults because it is O/S based
 */
unsigned long I_GetRandomTimeSeed(void)
{
/* This isnt very random */
  return(SDL_GetTicks());
}

/* cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char* I_GetVersionString(char* buf, size_t sz)
{
  snprintf(buf, sz, "%s v%s (" PACKAGE_URL ")", PACKAGE_NAME, PACKAGE_VERSION);
  return buf;
}

/* cphipps - I_SigString
 * Returns a string describing a signal number
 */
const char* I_SigString(char* buf, size_t sz, int signum)
{
#if HAVE_DECL_SYS_SIGLIST // NSM: automake defines this symbol as 0 or 1
  if (strlen(sys_siglist[signum]) < sz)
    strcpy(buf,sys_siglist[signum]);
  else
#endif
  snprintf(buf,sz,"signal %d",signum);
  return buf;
}

#if 0
#ifndef PRBOOM_SERVER
dboolean I_FileToBuffer(const char *filename, byte **data, int *size)
{
  FILE *hfile;

  dboolean result = false;
  byte *buffer = NULL;
  size_t filesize = 0;

  hfile = fopen(filename, "rb");
  if (hfile)
  {
    fseek(hfile, 0, SEEK_END);
    filesize = ftell(hfile);
    fseek(hfile, 0, SEEK_SET);

    buffer = malloc(filesize);
    if (buffer)
    {
      if (fread(buffer, filesize, 1, hfile) == 1)
      {
        result = true;

        if (data)
        {
          *data = buffer;
        }

        if (size)
        {
          *size = filesize;
        }
      }
    }

    fclose(hfile);
  }

  if (!result)
  {
    free(buffer);
    buffer = NULL;
  }

  return result;
}
#endif // PRBOOM_SERVER

/*
 * I_Read
 *
 * cph 2001/11/18 - wrapper for read(2) which handles partial reads and aborts
 * on error.
 */
void I_Read(int fd, void* vbuf, size_t sz)
{
  unsigned char* buf = vbuf;

  while (sz) {
    int rc = read(fd,buf,sz);
    if (rc <= 0) {
      I_Error("I_Read: read failed: %s", rc ? strerror(errno) : "EOF");
    }
    sz -= rc; buf += rc;
  }
}

/*
 * I_Filelength
 *
 * Return length of an open file.
 */

int I_Filelength(int handle)
{
  struct stat   fileinfo;
  if (fstat(handle,&fileinfo) == -1)
    I_Error("I_Filelength: %s",strerror(errno));
  return fileinfo.st_size;
}
#endif

#ifndef PRBOOM_SERVER

// Return the path where the executable lies -- Lee Killough
// proff_fs 2002-07-04 - moved to i_system
#ifdef _WIN32

void I_SwitchToWindow(HWND hwnd) {
  typedef BOOL (WINAPI *TSwitchToThisWindow) (HWND wnd, BOOL restore);
  static TSwitchToThisWindow SwitchToThisWindow = NULL;

  if (!SwitchToThisWindow)
    SwitchToThisWindow = (TSwitchToThisWindow)GetProcAddress(GetModuleHandle("user32.dll"), "SwitchToThisWindow");

  if (SwitchToThisWindow) {
    HWND hwndLastActive = GetLastActivePopup(hwnd);

    if (IsWindowVisible(hwndLastActive))
      hwnd = hwndLastActive;

    SetForegroundWindow(hwnd);
    Sleep(100);
    SwitchToThisWindow(hwnd, TRUE);
  }
}

const char* I_DoomExeDir(void) {
  static char *base;

  /*
   * CG: This code is largely from GLib, released under the GPLv2 (or any later
   *     version)
   */

  char *utf8_buf = NULL;
  wchar_t buf[MAX_PATH + 1];

  if (!base) { // cache multiple requests
    if (GetModuleFileNameW(GetModuleHandle(NULL), buf, G_N_ELEMENTS(buf)) > 0)
      utf8_buf = g_utf16_to_utf8(buf, -1, NULL, NULL, NULL);

    if (utf8_buf) {
      base = g_path_get_dirname(utf8_buf);
      g_free(utf8_buf);
    }
  }

  return base;
}

const char* I_GetTempDir(void) {
  static char tmp_path[PATH_MAX] = {0};

  if (tmp_path[0] == 0)
    GetTempPath(sizeof(tmp_path), tmp_path);

  return tmp_path;
}

#elif defined(AMIGA)

const char* I_DoomExeDir(void) {
  return "PROGDIR:";
}

const char* I_GetTempDir(void) {
  return "PROGDIR:";
}

#elif defined(MACOSX)

/* Defined elsewhere */

#else /* POSIX */
// cph - V.Aguilar (5/30/99) suggested return ~/.lxdoom/, creating
//  if non-existant
// cph 2006/07/23 - give prboom+ its own dir
// Mead rem extra slash 8/21/03

const char* I_DoomExeDir(void) {
  static const char doom2k_dir[] = {"/." PACKAGE_TARNAME};
  static char *base;

  if (!base) { // cache multiple requests
    char *home = getenv("HOME");
    size_t len = strlen(home);

    base = malloc(len + strlen(doom2k_dir) + 1);
    strcpy(base, home);
    // I've had trouble with trailing slashes before...
    if (base[len - 1] == '/')
      base[len - 1] = 0;

    strcat(base, doom2k_dir);
    mkdir(base, S_IRUSR | S_IWUSR | S_IXUSR); // Make sure it exists
  }

  return base;
}

const char* I_GetTempDir(void) {
  return "/tmp";
}

#endif

/*
 * HasTrailingSlash
 *
 * cphipps - simple test for trailing slash on dir names
 */

dboolean HasTrailingSlash(const char* dn)
{
  return ( (dn[strlen(dn)-1] == '/')
#if defined(_WIN32)
        || (dn[strlen(dn)-1] == '\\')
#endif
#if defined(AMIGA)
        || (dn[strlen(dn)-1] == ':')
#endif
          );
}

/*
 * I_FindFile
 *
 * proff_fs 2002-07-04 - moved to i_system
 *
 * cphipps 19/1999 - writen to unify the logic in FindIWADFile and the WAD
 *      autoloading code.
 * Searches the standard dirs for a named WAD file
 * The dirs are listed at the start of the function
 */

#ifndef MACOSX /* OSX defines its search paths elsewhere. */

char* I_FindFileInternal(const char* wfname, const char* ext) {
  // lookup table of directories to search
  static const struct {
    const char *dir; // directory
    const char *sub; // subdirectory
    const char *env; // environment variable
    const char *(*func)(void); // for I_DoomExeDir
  } search[] = {
    {NULL, NULL, NULL, I_DoomExeDir}, // config directory
    {NULL}, // current working directory
    {NULL, NULL, "DOOMWADDIR"}, // run-time $DOOMWADDIR
    {DOOMWADDIR}, // build-time configured DOOMWADDIR
    {NULL, "doom", "HOME"}, // ~/doom
    {NULL, NULL, "HOME"}, // ~
    {"/usr/local/share/games/doom"},
    {"/usr/share/games/doom"},
    {"/usr/local/share/doom"},
    {"/usr/share/doom"},
  };

  size_t i;
  buf_t buf;

  M_BufferInit(&buf);

  /*
  size_t pl;
  static char static_p[PATH_MAX];
  char * dynamic_p = NULL;
  char *p = (is_static ? static_p : dynamic_p);
  */

  if (!wfname)
    return NULL;

  /* Precalculate a length we will need in the loop */
  // pl = strlen(wfname) + (ext ? strlen(ext) : 0) + 4;

  for (i = 0; i < sizeof(search) / sizeof(*search); i++) {
    const char *d = NULL;
    const char *s = NULL;

    /*
     * Each entry in the switch sets d to the directory to look in, and
     * optionally s to a subdirectory of d
     */

    /* switch replaced with lookup table */
    if (search[i].env) {
      if (!(d = getenv(search[i].env)))
        continue;
    }
    else if (search[i].func) {
      d = search[i].func();
    }
    else {
      d = search[i].dir;
    }
    s = search[i].sub;

    M_BufferClear(&buf);

    if (d == NULL && s == NULL) {
      M_BufferWriteString(&buf, wfname, strlen(wfname));
    }
    else if (d != NULL && s != NULL) {
      if (!M_PathJoinBuf(&buf, d, s)) {
        lprintf(LO_WARN, " Error looking for %s: %s.\n",
          wfname, M_GetFileError()
        );
        continue;
      }

      M_BufferSeekBackward(&buf, 1);

      if (!M_PathJoinBuf(&buf, M_BufferGetData(&buf), wfname)) {
        lprintf(LO_WARN, " Error looking for %s: %s.\n",
          wfname, M_GetFileError()
        );
        continue;
      }

    }
    else if (d == NULL) {
      if (!M_PathJoinBuf(&buf, s, wfname)) {
        lprintf(LO_WARN, " Error looking for %s: %s.\n",
          wfname, M_GetFileError()
        );
        continue;
      }
    }
    else if (s == NULL) {
      if (!M_PathJoinBuf(&buf, d, wfname)) {
        lprintf(LO_WARN, " Error looking for %s: %s.\n",
          wfname, M_GetFileError()
        );
        continue;
      }
    }

    if (ext && !M_IsFile(M_BufferGetData(&buf))) {
      M_BufferSeekBackward(&buf, 1);
      M_BufferWriteString(&buf, ext, strlen(ext));
    }

    if (M_IsFile(M_BufferGetData(&buf))) {
      char *out = strdup(M_BufferGetData(&buf));

      M_BufferFree(&buf);
      lprintf(LO_INFO, " found %s\n", out);
      return out;
    }

    M_BufferClear(&buf);
  }

  M_BufferFree(&buf);
  return NULL;
}

char* I_FindFile(const char* wfname, const char* ext) {
  return I_FindFileInternal(wfname, ext);
}

const char* I_FindFile2(const char* wfname, const char* ext) {
  return (const char*)I_FindFileInternal(wfname, ext);
}

#endif

#endif // PRBOOM_SERVER

/* vi: set et ts=2 sw=2: */

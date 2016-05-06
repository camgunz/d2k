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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
typedef BOOL (WINAPI *SetAffinityFunc)(HANDLE hProcess, DWORD mask);
#endif

#include "TEXTSCREEN/txt_main.h"

#include "doomdef.h"
#include "doomstat.h"
#include "d_cfg.h"
#include "d_main.h"
#include "e6y.h"
#include "g_game.h"
#include "i_font.h"
#include "i_input.h"
#include "i_main.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_file.h"
#include "m_fixed.h"
#include "m_misc.h"
#include "m_random.h"
#include "r_fps.h"
#include "x_main.h"
#include "xam_main.h"
#include "xc_main.h"
#include "xcl_main.h"
#include "xd_cfg.h"
#include "xd_comp.h"
#include "xd_msg.h"
#include "xd_main.h"
#include "xg_game.h"
#include "xg_keys.h"
#include "xi_input.h"
#include "xi_main.h"
#include "xm_menu.h"
#include "xm_misc.h"
#include "xn_main.h"
#include "xp_user.h"
#include "xr_demo.h"
#include "xr_main.h"
#include "xs_main.h"
#include "xst_main.h"
#include "xv_main.h"

#include "icon.c"

#undef main

/* CPhipps - flags controlling ENDOOM behaviour */
enum {
  endoom_colours       = 1,
  endoom_nonasciichars = 2,
  endoom_droplastline  = 4
};

static ExeptionsList_t current_exception_index;

static int has_exited;

static int_64_t get_time_scale = 1 << 24;

ExeptionParam_t ExeptionsParams[EXEPTION_MAX + 1] =
{
  {NULL},
  {"gld_CreateScreenSizeFBO: Access violation in glFramebufferTexture2DEXT.\n\n"
    "Are you using ATI graphics? Try to update your drivers "
    "or change gl_compatibility variable in cfg to 1.\n"},
  {NULL}
};

int realtic_clock_rate = 100;
int (*I_GetTime)(void) = NULL;
int endoom_mode;

/*
 * Most of the following has been rewritten by Lee Killough
 *
 * I_GetTime
 * killough 4/13/98: Make clock rate adjustable by scale factor
 * cphipps - much made static
 */

static int get_time_scaled(void) {
  return (int)( (int_64_t) I_GetTime_RealTime() * get_time_scale >> 24);
}

static int get_time_fast_demo(void) {
  static int fasttic;
  return fasttic++;
}

static int get_time_error(void) {
  I_Error("get_time_error: GetTime() used before initialization");
  return 0;
}

/* killough 2/22/98: Add support for ENDBOOM, which is PC-specific
 *
 * this converts BIOS color codes to ANSI codes.
 * Its not pretty, but it does the job - rain
 * CPhipps - made static
 */
#if 0
inline static int convert(int color, int *bold)
{
  if (color > 7) {
    color -= 8;
    *bold = 1;
  }
  switch (color) {
  case 0:
    return 0;
  case 1:
    return 4;
  case 2:
    return 2;
  case 3:
    return 6;
  case 4:
    return 1;
  case 5:
    return 5;
  case 6:
    return 3;
  case 7:
    return 7;
  }
  return 0;
}
#endif

static void print_version(void) {
  char vbuf[200];

  D_Msg(MSG_INFO, "%s\n", I_GetVersionString(vbuf, 200));
}

//
// ENDOOM support using text mode emulation
//
static void end_doom(void) {
  int lump_eb, lump_ed, lump = -1;

  const unsigned char *endoom_data;
  unsigned char *screendata;

#ifndef _WIN32
  print_version();
#endif

  if (!showendoom || demorecording)
    return;

  /* CPhipps - ENDOOM/ENDBOOM selection */
  lump_eb = W_CheckNumForName("ENDBOOM");/* jff 4/1/98 sign our work    */
  lump_ed = W_CheckNumForName("ENDOOM"); /* CPhipps - also maybe ENDOOM */

  if (lump_eb == -1) {
    lump = lump_ed;
  }
  else if (lump_ed == -1) {
    lump = lump_eb;
  }
  else {
    /* Both ENDOOM and ENDBOOM are present */
#define LUMP_IS_NEW(num) (!((lumpinfo[num].source == source_iwad) || (lumpinfo[num].source == source_auto_load)))
    switch ((LUMP_IS_NEW(lump_ed) ? 1 : 0 ) |
      (LUMP_IS_NEW(lump_eb) ? 2 : 0)) {
    case 1:
      lump = lump_ed;
      break;
    case 2:
      lump = lump_eb;
      break;
    default:
      /* Both lumps have equal priority, both present */
      lump = (P_Random(pr_misc) & 1) ? lump_ed : lump_eb;
      break;
    }
  }

  if (lump != -1) {
    endoom_data = W_CacheLumpNum(lump);

    // Set up text mode screen
    TXT_Init();

    // Make sure the new window has the right title and icon
    I_SetWindowCaption();
    I_SetWindowIcon();

    // Write the data to the screen memory
    screendata = TXT_GetScreenData();
    memcpy(screendata, endoom_data, 4000);

    // Wait for a keypress
    while (true) {
      TXT_UpdateScreen();

      if (TXT_GetChar() > 0)
        break;

      TXT_Sleep(0);
    }

    // Shut down text mode screen
    TXT_Shutdown();
  }
}

/* I_EndDoom
 * Prints out ENDOOM or ENDBOOM, using some common sense to decide which.
 * cphipps - moved to l_main.c, made static
 */
#if 0
static void I_EndDoom2(void)
{
  int lump_eb, lump_ed, lump = -1;

  //e6y
  if (!showendoom)
  {
    D_Msg(MSG_INFO,
      "I_EndDoom: All following output is skipped because of "
      "\"Fast Exit\" option\n"
    );
    return;
  }

  /* CPhipps - ENDOOM/ENDBOOM selection */
  lump_eb = W_CheckNumForName("ENDBOOM");/* jff 4/1/98 sign our work    */
  lump_ed = W_CheckNumForName("ENDOOM"); /* CPhipps - also maybe ENDOOM */

  if (lump_eb == -1)
    lump = lump_ed;
  else if (lump_ed == -1)
    lump = lump_eb;
  else
  { /* Both ENDOOM and ENDBOOM are present */
#define LUMP_IS_NEW(num) (!((lumpinfo[num].source == source_iwad) || (lumpinfo[num].source == source_auto_load)))
    switch ((LUMP_IS_NEW(lump_ed) ? 1 : 0 ) |
      (LUMP_IS_NEW(lump_eb) ? 2 : 0)) {
    case 1:
      lump = lump_ed;
      break;
    case 2:
      lump = lump_eb;
      break;
    default:
      /* Both lumps have equal priority, both present */
      lump = (P_Random(pr_misc) & 1) ? lump_ed : lump_eb;
      break;
    }
  }

  if (lump != -1)
  {
    int i, l = W_LumpLength(lump) / 2;
    const char (*endoom)[2];
    endoom = W_CacheLumpNum(lump);

    /* cph - colour ENDOOM by rain */
#ifndef _WIN32
    if (endoom_mode & endoom_nonasciichars)
      /* switch to secondary charset, and set to cp437 (IBM charset) */
      printf("\e)K\016");
#endif /* _WIN32 */

    /* cph - optionally drop the last line, so everything fits on one screen */
    if (endoom_mode & endoom_droplastline)
      l -= 80;
    D_Msg(MSG_INFO, "\n");
    for (i=0; i<l; i++)
    {
#ifdef _WIN32
      I_ConTextAttr(endoom[i][1]);
#elif defined (DJGPP)
      textattr(endoom[i][1]);
#else
      if (endoom_mode & endoom_colours)
      {
        int oldbg = -1, oldcolor = -1, bold = 0, oldbold = -1, color = 0;

        if (!(i % 80))
        {
          /* reset color but not bold when we start a new line */
          oldbg = -1;
          oldcolor = -1;
          printf("\e[39m\e[49m\n");
        }
        /* foreground color */
        bold = 0;
        color = endoom[i][1] % 16;
        if (color != oldcolor)
        {
          oldcolor = color;
          color = convert(color, &bold);
          if (oldbold != bold)
          {
            oldbold = bold;
      printf("\e[%cm", bold + '0');
      if (!bold) oldbg = -1;
          }
          /* we buffer everything or output is horrendously slow */
          printf("\e[%dm", color + 30);
          bold = 0;
        }
        /* background color */
        color = endoom[i][1] / 16;
        if (color != oldbg)
        {
          oldbg = color;
          color = convert(color, &bold);
          printf("\e[%dm", color + 40);
        }
      }
#endif
      /* cph - portable ascii printout if requested */
      if (isascii(endoom[i][0]) || (endoom_mode & endoom_nonasciichars))
        D_Msg(MSG_INFO, "%c", endoom[i][0]);
      else /* Probably a box character, so do #'s */
        D_Msg(MSG_INFO, "#");
    }
#ifndef _WIN32
    D_Msg(MSG_INFO, "\b"); /* hack workaround for extra newline at bottom of screen */
    D_Msg(MSG_INFO, "\r");
    if (endoom_mode & endoom_nonasciichars)
      printf("%c",'\017'); /* restore primary charset */
#endif /* _WIN32 */
    W_UnlockLumpNum(lump);
  }
#ifndef _WIN32
  if (endoom_mode & endoom_colours)
    puts("\e[0m"); /* cph - reset colours */
  print_version();
#endif /* _WIN32 */
}
#endif

static void quit(void)
{
  if (!has_exited)
    has_exited=1;   /* Prevent infinitely recursive exits -- killough */

  if (has_exited == 1) {
    if (!demorecording)
      end_doom();

    if (demorecording)
      G_CheckDemoStatus();

    M_SaveDefaults();
    I_DemoExShutdown();
  }
}

#ifdef SECURE_UID
uid_t stored_euid = -1;
#endif

//
// Ability to use only the allowed CPUs
//

static void set_affinity_mask(void) {
  // Forcing single core only for "SDL MIDI Player"
  process_affinity_mask = 0;
  if (!strcasecmp(snd_midiplayer, midiplayers[midi_player_sdl]))
    process_affinity_mask = 1;

  // Set the process affinity mask so that all threads
  // run on the same processor.  This is a workaround for a bug in
  // SDL_mixer that causes occasional crashes.
  if (process_affinity_mask) {
    const char *errbuf = NULL;
#ifdef _WIN32
    HMODULE kernel32_dll;
    SetAffinityFunc SetAffinity = NULL;
    int ok = false;

    // Find the kernel interface DLL.
    kernel32_dll = LoadLibrary((LPCWSTR)"kernel32.dll");

    if (kernel32_dll) {
      // Find the SetProcessAffinityMask function.
      SetAffinity = (SetAffinityFunc)GetProcAddress(kernel32_dll, "SetProcessAffinityMask");

      // If the function was not found, we are on an old (Win9x) system
      // that doesn't have this function.  That's no problem, because
      // those systems don't support SMP anyway.

      if (SetAffinity)
        ok = SetAffinity(GetCurrentProcess(), process_affinity_mask);
    }

    if (!ok)
      errbuf = WINError();
#elif defined(HAVE_SCHED_SETAFFINITY)
    // POSIX version:
    cpu_set_t set;

    CPU_ZERO(&set);

    for(int i = 0; i < 16; i++)
      CPU_SET((process_affinity_mask>>i)&1, &set);

    if (sched_setaffinity(getpid(), sizeof(set), &set) == -1)
      errbuf = strerror(errno);
#else
    return;
#endif

    if (errbuf == NULL) {
      D_Msg(MSG_INFO, "I_SetAffinityMask: manual affinity mask is %d\n",
        process_affinity_mask
      );
    }
    else {
      D_Msg(MSG_ERROR,
        "I_SetAffinityMask: failed to set process affinity mask (%s)\n",
        errbuf
      );
    }
  }
}

static void initialize_messaging(const char *log_file) {
  if (log_file) {
    if (!D_MsgActivateWithFile(MSG_INFO, log_file))
      I_Error("Error opening %s", log_file);

#ifdef DEBUG
    if (!D_MsgActivateWithFile(MSG_DEBUG, log_file))
      I_Error("Error opening %s", log_file);
#endif

    if (!D_MsgActivateWithFile(MSG_WARN, log_file))
      I_Error("Error opening %s", log_file);

    if (!D_MsgActivateWithFile(MSG_ERROR, log_file))
      I_Error("Error opening %s", log_file);

    if (!D_MsgActivateWithFile(MSG_DEH, log_file))
      I_Error("Error opening %s", log_file);

    if (!D_MsgActivateWithFile(MSG_GAME, log_file))
      I_Error("Error opening %s", log_file);
  }
  else {
    D_MsgActivate(MSG_INFO);
#ifdef DEBUG
    D_MsgActivate(MSG_DEBUG);
#endif
    D_MsgActivate(MSG_INFO);
    D_MsgActivate(MSG_WARN);
    D_MsgActivate(MSG_ERROR);
    D_MsgActivate(MSG_DEH);
    D_MsgActivate(MSG_GAME);
  }
}

static void cleanup_pid_file(void) {
  int p;
  char *pid_file;

  puts("Cleaning up PID file");

  if ((p = M_CheckParm("-pid"))) {
    pid_file = myargv[p + 1];
  }
  else {
    pid_file = M_PathJoin(I_DoomExeDir(), DEFAULT_PID_FILE_NAME);

    if (pid_file) {
      I_Error("Error joining %s and %s: %s\n",
        I_DoomExeDir(),
        DEFAULT_PID_FILE_NAME,
        M_GetFileError()
      );
    }
  }

  if (!M_DeleteFile(pid_file)) {
    fprintf(stderr, "Error deleting PID file %s: %s\n",
      pid_file,
      M_GetFileError()
    );
  }
}

static void exit_gracefully(void) {
  for (msg_channel_e chan = MSG_MIN; chan <= MSG_MAX; chan++) {
    D_MsgDeactivate(chan);
  }
}

static void handle_sigint(int signum) {
  exit_gracefully();
}

static void handle_sigterm(int signum) {
  exit_gracefully();
}

#ifdef G_OS_UNIX
static void daemonize(void) {
  char *log_file;
  char *pid_file;
  char *pid_string;
  pid_t pid;
  pid_t sid;
  int infofd;
  int errorfd;
  int p;

  if ((p = M_CheckParm("-log"))) {
    log_file = myargv[p + 1];
  }
  else {
    log_file = M_PathJoin(I_DoomExeDir(), DEFAULT_LOG_FILE_NAME);

    if (!log_file) {
      I_Error("Error joining %s and %s: %s\n",
        I_DoomExeDir(),
        DEFAULT_LOG_FILE_NAME,
        M_GetFileError()
      );
    }
  }

  if ((p = M_CheckParm("-pid"))) {
    pid_file = myargv[p + 1];
  }
  else {
    pid_file = M_PathJoin(I_DoomExeDir(), DEFAULT_PID_FILE_NAME);

    if (!pid_file) {
      I_Error("Error joining %s and %s: %s\n",
        I_DoomExeDir(),
        DEFAULT_PID_FILE_NAME,
        M_GetFileError()
      );
    }
  }

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  pid = getpid();

  umask(0);

  for (int fd = 0; fd < 1024; fd++) {
    close(fd);
  }

  initialize_messaging(log_file);

  infofd = D_MsgGetFD(MSG_INFO);

  if (infofd < 0) {
    I_Error("Error getting file descriptor of info log");
  }

  if (dup2(infofd, STDOUT_FILENO) == -1) {
    I_Error("Error duplicating STDOUT_FILENO: %s", strerror(errno));
  }

  errorfd = D_MsgGetFD(MSG_ERROR);

  if (errorfd < 0) {
    I_Error("Error getting file descriptor of error log");
  }

  dup2(errorfd, STDERR_FILENO);

  if (dup2(errorfd, STDERR_FILENO) == -1) {
    I_Error("Error duplicating STDERR_FILENO: %s", strerror(errno));
  }

  sid = setsid();

  if (sid < 0) {
    I_Error("Error setting session ID: %s", strerror(errno));
  }

  pid_string = g_strdup_printf("%d\n", pid);

  if (!M_WriteFile(pid_file, pid_string, strlen(pid_string))) {
    I_Error("Error writing PID to a file: %s", M_GetFileError());
  }

  atexit(cleanup_pid_file);

  g_free(pid_string);

  if (chdir("/") < 0) {
    I_Error("Error changing directory to \"/\": %s", strerror(errno));
  }

  infofd = D_MsgGetFD(MSG_INFO);

  if (infofd < 0) {
    I_Error("Error getting info log file descriptor: %s", strerror(errno));
  }

  errorfd = D_MsgGetFD(MSG_ERROR);

  if (errorfd < 0) {
    I_Error("Error getting error log file descriptor: %s", strerror(errno));
  }
}
#endif

//
// Sets the priority class for the prboom-plus process
//

void I_Init(void) {
  /* killough 4/14/98: Adjustable speedup based on realtic_clock_rate */
  if (fastdemo) {
    I_GetTime = get_time_fast_demo;
  }
  else if (realtic_clock_rate != 100) {
    get_time_scale = ((int_64_t) realtic_clock_rate << 24) / 100;
    I_GetTime = get_time_scaled;
  }
  else {
    I_GetTime = I_GetTime_RealTime;
  }

  /* killough 2/21/98: avoid sound initialization if no sound & no music */
  if (!(nomusicparm && nosfxparm)) {
    I_InitSound();
  }

  R_InitInterpolation();
}

//e6y
void I_Init2(void) {
  if (fastdemo) {
    I_GetTime = get_time_fast_demo;
  }
  else if (realtic_clock_rate != 100) {
    get_time_scale = ((int_64_t) realtic_clock_rate << 24) / 100;
    I_GetTime = get_time_scaled;
  }
  else {
    I_GetTime = I_GetTime_RealTime;
  }
  R_InitInterpolation();
  force_singletics_to = gametic + BACKUPTICS;
}

void I_StartTic(void) {
  I_InputHandle();
}

void I_ExeptionBegin(ExeptionsList_t exception_index) {
  if (current_exception_index == EXEPTION_NONE)
    current_exception_index = exception_index;
  else
    I_Error("I_SignalStateSet: signal_state set!");
}

void I_ExeptionEnd(void) {
  current_exception_index = EXEPTION_NONE;
}

void I_ExeptionProcess(void) {
  if (current_exception_index > EXEPTION_NONE && current_exception_index < EXEPTION_MAX)
    I_Error("%s", ExeptionsParams[current_exception_index].error_message);
}

void I_SetWindowIcon(void) {
  static SDL_Surface *surface = NULL;

  // do it only once, because of crash in SDL_InitVideoMode in SDL 1.3
  if (!surface)
  {
    surface = SDL_CreateRGBSurfaceFrom(
      icon_data,
      icon_w,
      icon_h,
      32,
      icon_w * 4,
      0xff << 0,
      0xff << 8,
      0xff << 16,
      0xff << 24
    );
  }

  if (surface)
    SDL_WM_SetIcon(surface, NULL);
}

void I_SetWindowCaption(void) {
  size_t len = strlen(PACKAGE_NAME) + strlen(PACKAGE_VERSION) + 3;
  char *buf = calloc(len, sizeof(char));

  if (buf == NULL)
    I_Error("I_SetWindowCaption: calloc failed");

  snprintf(buf, len, "%s v%s", PACKAGE_NAME, PACKAGE_VERSION);

  SDL_WM_SetCaption(buf, NULL);

  free(buf);
}

/* I_SafeExit
 * This function is called instead of exit() by functions that might be called
 * during the exit process (i.e. after exit() has already been called)
 * Prevent infinitely recursive exits -- killough
 */

void I_SafeExit(int rc) {
  if (!has_exited) {  /* If it hasn't exited yet, exit now -- killough */
    if (rc)
      has_exited = 2;
    else
      has_exited = 1;

    exit(rc);
  }
}

void I_SetProcessPriority(void) {
  if (process_priority) {
    const char *errbuf = NULL;

#ifdef _WIN32
    DWORD dwPriorityClass = NORMAL_PRIORITY_CLASS;

    if (process_priority == 1)
      dwPriorityClass = HIGH_PRIORITY_CLASS;
    else if (process_priority == 2)
      dwPriorityClass = REALTIME_PRIORITY_CLASS;

    if (SetPriorityClass(GetCurrentProcess(), dwPriorityClass) == 0)
      errbuf = WINError();
#else
    return;
#endif

    if (errbuf == NULL) {
      D_Msg(MSG_INFO,
        "I_SetProcessPriority: priority for the process is %d\n",
        process_priority
      );
    }
    else {
      D_Msg(MSG_ERROR,
        "I_SetProcessPriority: failed to set priority for the process (%s)\n",
        errbuf
      );
    }
  }
}

#ifndef RUNNING_UNIT_TESTS
int main(int argc, char **argv) {
  int p;

#ifdef SECURE_UID
  /* First thing, revoke setuid status (if any) */
  stored_euid = geteuid();
  if (getuid() != stored_euid) {
    if (seteuid(getuid()) < 0)
      fprintf(stderr, "Failed to revoke setuid\n");
    else
      fprintf(stderr, "Revoked uid %d\n", stored_euid);
  }
#endif

  I_GetTime = get_time_error;

  myargc = argc;
  myargv = malloc(sizeof(myargv[0]) * myargc);
  memcpy(myargv, argv, sizeof(myargv[0]) * myargc);

  // e6y: Check for conflicts.
  // Conflicting command-line parameters could cause the engine to be confused
  // in some cases. Added checks to prevent this.
  // Example: glboom.exe -record mydemo -playdemo demoname
  ParamsMatchingCheck();

#if __MINGW32__
  _fmode = _O_BINARY;

  _setmode(_fileno(stdin),  _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
  _setmode(_fileno(stderr), _O_BINARY);
#endif

  X_Init(); /* CG 07/22/2014: Scripting */

  XAM_RegisterInterface();
  XCL_RegisterInterface();
  XC_RegisterInterface();
  XD_CompatibilityRegisterInterface();
  XD_ConfigRegisterInterface();
  XD_MsgRegisterInterface();
  XD_RegisterInterface();
  XG_GameRegisterInterface();
  XG_KeysRegisterInterface();
  XI_InputRegisterInterface();
  XI_RegisterInterface();
  XM_MenuRegisterInterface();
  XM_MiscRegisterInterface();
  XN_RegisterInterface();
  XP_UserRegisterInterface();
  XR_DemoRegisterInterface();
  XR_RegisterInterface();
  XST_RegisterInterface();
  XS_RegisterInterface();
  XV_RegisterInterface();

  X_ExposeInterfaces(NULL);

  D_ConfigInit();

  D_InitMessaging(); /* 05/09/14 CG: Enable messaging */

  if (M_CheckParm("-serve")) {
#ifdef G_OS_UNIX
    if (!M_CheckParm("-nodaemon"))
      daemonize();
#else
    if ((p = M_CheckParm("-log"))) {
      const char *log_file = myargv[p + 1];

      initialize_messaging(log_file);
    }
#endif
  }
  else if ((p = M_CheckParm("-log"))) {
    const char *log_file = myargv[p + 1];

    initialize_messaging(log_file);
  }
  else {
    initialize_messaging(NULL);
  }

  // e6y: was moved from D_DoomMainSetup
  // init subsystems
  D_Msg(MSG_INFO, "M_LoadDefaults: Load system defaults.\n");
  M_LoadDefaults(); // load before initing other systems

  /* Version info */
  D_Msg(MSG_INFO, "\n");
  print_version();

  /* cph - Z_Close must be done after quit, so we register it first. */
  atexit(Z_Close);
  /*
     killough 1/98:

     This fixes some problems with exit handling
     during abnormal situations.

     The old code called quit() to end program,
     while now quit() is installed as an exit
     handler and exit() is called to exit, either
     normally or abnormally. Seg faults are caught
     and the error handler is used, to prevent
     being left in graphics mode or having very
     loud SFX noise because the sound card is
     left in an unstable state.
  */

  Z_Init();                  /* 1/18/98 killough: start up memory stuff first */

  atexit(quit);
  /* CG: There's really no need to parachute these */
#if 0
#ifdef DEBUG
  signal(SIGSEGV, I_DebugSignalHandler);
  signal(SIGTERM, I_DebugSignalHandler);
  signal(SIGFPE,  I_DebugSignalHandler);
  signal(SIGILL,  I_DebugSignalHandler);
  /* killough 3/6/98: allow CTRL-BRK during init */
  signal(SIGINT,  I_DebugSignalHandler);
  signal(SIGABRT, I_DebugSignalHandler);
#else
  if (!M_CheckParm("-devparm"))
    signal(SIGSEGV, I_SignalHandler);

  signal(SIGTERM, I_SignalHandler);
  signal(SIGFPE,  I_SignalHandler);
  signal(SIGILL,  I_SignalHandler);
  signal(SIGINT,  I_SignalHandler);  /* killough 3/6/98: allow CTRL-BRK during init */
  signal(SIGABRT, I_SignalHandler);
#endif
#endif

  signal(SIGINT, handle_sigint);
  signal(SIGTERM, handle_sigterm);

  // Ability to use only the allowed CPUs
  set_affinity_mask();

  // Priority class for the prboom-plus process
  I_SetProcessPriority();

  /* cphipps - call to video specific startup code */
  I_PreInitGraphics();

  /* CG 07/11/2014: Enable Unicode in SDL */
  SDL_EnableUNICODE(true);

  /* CG 07/22/2014: Enable key repeating */
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  I_LoadCustomFonts();

  D_DoomMain();

  return EXIT_SUCCESS;
}
#endif

/* vi: set et ts=2 sw=2: */


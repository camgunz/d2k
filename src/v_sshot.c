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

#include "i_system.h"
#include "m_argv.h"
#include "m_file.h"
#include "d_msg.h"
#include "g_game.h"
#include "s_sound.h"
#include "v_sshot.h"

#ifndef SCREENSHOT_DIR
#define SCREENSHOT_DIR "."
#endif

#ifdef HAVE_LIBPNG
#define SCREENSHOT_EXT "png"
#else
#define SCREENSHOT_EXT "bmp"
#endif

//
// SCREEN SHOTS
//

//
// V_ScreenShot
//
// Modified by Lee Killough so that any number of shots can be taken,
// the code is faster, and no annoying "screenshot" message appears.

// CPhipps - modified to use its own buffer for the image
//         - checks for the case where no file can be created (doesn't occur on
//           POSIX systems, would on DOS)
//         - track errors better
//         - split into 2 functions

#define INITIAL_SCREENSHOT_PATH_BUF_SIZE 64

const char *screenshot_dir;

bool is_writable_folder(const char *folder) {
  if (!folder) {
    return false;
  }

  if (!strlen(folder)) {
    return NULL;
  }

  if (!M_IsFolder(folder)) {
    return NULL;
  }

  if (!M_CheckAccess(folder, O_RDWR)) {
    return false;
  }
}

//
// V_DoScreenShot
// Takes a screenshot into the names file

void V_DoScreenShot(const char *fname) {
  if (I_ScreenShot(fname) != 0) {
    D_MsgLocalError("V_ScreenShot: Error writing screenshot\n");
  }
}

void V_ScreenShot(void) {
  const char *shot_dir = NULL;
  GString *buf = NULL;
  int p = M_CheckParm("-shotdir");

  if (p && (p < myargc - 1) && is_writable_folder(myargv[p + 1])) {
    shot_dir = myargv[p + 1];
  }
  else if (is_writable_folder(screenshot_dir)) {
    shot_dir = screenshot_dir;
  }
  else if (is_writable_folder(I_DoomExeDir())) {
    shot_dir = I_DoomExeDir();
  }
  else if (is_writable_folder(".")) {
    shot_dir = ".";
  }
  else {
    D_MsgLocalError("V_ScreenShot: No writable folders\n");
    return;
  }

  buf = g_string_sized_new(INITIAL_SCREENSHOT_PATH_BUF_SIZE);

  for (size_t i = 1; i <= 999999; i++) {
    g_string_printf(buf, "%s%sss%06u.%s",
      shot_dir,
      G_DIR_SEPARATOR_S, 
      i,
      SCREENSHOT_EXT
    );

    if (!M_PathExists(buf->data)) {
      S_StartSound(NULL,  gamemode == commercial ? sfx_radio : sfx_tink);
      V_DoScreenShot(lbmname); // cph
      g_free(buf);
      return;
    }
  }

  g_free(buf);
  D_MsgLocalError("Couldn't create screenshot\n");
}

/* vi: set et ts=2 sw=2: */

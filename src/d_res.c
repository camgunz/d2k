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
#include "m_misc.h"
#include "m_swap.h"
#include "d_deh.h"
#include "d_main.h"
#include "d_res.h"
#include "g_game.h"
#include "g_save.h"
#include "w_wad.h"
#include "n_getwad.h"

static char *iwad_base = NULL;
static char *iwad_path = NULL;

/*
 * CG: Keep track of the specified resource and DEH/BEX files so we can
 *     potentially send them to a client on request, or save them in a
 *     savegame, or whatever.
 */
GPtrArray *resource_files = NULL;
GPtrArray *deh_files = NULL;

//jff 4/19/98 list of standard IWAD names
const char *const standard_iwads[]= {
  "doom2f.wad",
  "doom2.wad",
  "plutonia.wad",
  "tnt.wad",
  "doom.wad",
  "doom1.wad",
  "doomu.wad", /* CPhipps - alow doomu.wad */
  "freedoom2.wad", /* wart@kobold.org:  added freedoom for Fedora Extras */
  "freedoom1.wad",
  "freedm.wad",
  "hacx.wad",
  "chex.wad",
  "bfgdoom2.wad",
  "bfgdoom.wad",
};

//e6y static
const size_t nstandard_iwads = sizeof(standard_iwads) /
                               sizeof(*standard_iwads);

/* cph - MBF-like wad/deh/bex autoload code */
const char *wad_file_names[MAXLOADFILES];
const char *deh_file_names[MAXLOADFILES];

static void destroy_deh_file(gpointer data) {
  deh_file_t *df = (deh_file_t *)data;

  if (df->filename) {
    free(df->filename);
  }

  if (df->outfilename) {
    free(df->outfilename);
  }

  free(df);
}

static void destroy_wad_file(gpointer data) {
  wadfile_info_t *wf = (wadfile_info_t *)data;

  if (wf->handle > 0) {
    M_FDClose(wf->handle);
    wf->handle = 0;
  }

  free(wf->name);
  free(wf);
}

/*
 * find_iwad_file
 *
 * Search for one of the standard IWADs
 * CPhipps  - static, proper prototype
 *    - 12/1999 - rewritten to use I_FindFile
 */
static char* find_iwad_file(void) {
  int pi;

  if (CLIENT) {
    D_MsgLocalInfo("find_iwad_file: Looking for IWAD %s\n", D_GetIWAD());
    return I_FindFile(D_GetIWAD(), NULL);
  }

  pi = M_CheckParm("-iwad");

  if (pi && (++pi < myargc)) {
    D_MsgLocalInfo("find_iwad_file: Searching for IWAD %s\n", myargv[pi]);
    return I_FindFile(myargv[pi], ".wad");
  }

  for (size_t i = 0; i < nstandard_iwads; i++) {
    char *iwad = NULL;

    D_MsgLocalInfo("find_iwad_file: Searching for IWAD %s\n",
      standard_iwads[i]
    );

    iwad = I_FindFile(standard_iwads[i], ".wad");

    if (iwad) {
      return iwad;
    }
  }

  return NULL;
}

GPtrArray* D_ResNew(void) {
  return g_ptr_array_new_with_free_func(destroy_wad_file);
}

void D_ResClear(GPtrArray *resources) {
  g_ptr_array_remove_range(resources, 0, resources->len);
}

void D_ResFree(GPtrArray *resources) {
  D_ResClear(resources);
  g_ptr_array_free(resources, true);
}

void D_ResAdd(GPtrArray *resources, const char *file_name,
                                    wad_source_e source, 
                                    int handle) {
  wadfile_info_t *wf = malloc(sizeof(wadfile_info_t));

  if (!wf) {
    I_Error("D_ResAdd: Error allocating memory for new wadfile_info_t\n");
  }

  wf->name = strdup(file_name);
  wf->src = source; // Ty 08/29/98
  wf->handle = handle;
}

void D_ResourcesInit(void) {
  resource_files = D_ResNew();
  deh_files = g_ptr_array_new_with_free_func(destroy_deh_file);
}

void D_ResourcesSet(GPtrArray *wad_files) {
  size_t iwad_index = 0;
  bool iwad_found = false;
  size_t preloaded_wad_file_count = 0;
  GArray *preloaded = NULL;

  for (size_t i = 0; i < resource_files->len; i++) {
    wadfile_info_t *wf = &g_array_index(preloaded, wadfile_info_t, i);

    if (!(wf->src == source_pre) || (wf->src == source_auto_load)) {
      continue;
    }

    preloaded_wad_file_count++;
  }
  
  if (preloaded_wad_file_count) {
    preloaded = g_array_sized_new(
      false,
      false,
      sizeof(wadfile_info_t),
      preloaded_wad_file_count
    );

    for (size_t i = 0; i < resource_files->len; i++) {
      wadfile_info_t *wf = NULL;
      wadfile_info_t *rf = g_ptr_array_index(resource_files, i);

      if (!(rf->src == source_pre) || (rf->src == source_auto_load)) {
        continue;
      }

      wf = &g_array_index(preloaded, wadfile_info_t, i);

      wf->name = strdup(rf->name);
      wf->src = rf->src;
      wf->handle = rf->handle;
    }
  }

  D_ClearResourceFiles();

  for (iwad_index = 0; iwad_index < wad_files->len; iwad_index++) {
    wadfile_info_t *wf = g_ptr_array_index(wad_files, iwad_index);

    if (wf->src == source_iwad) {
      D_AddIWAD(I_FindFile(wf->name, ".wad"));
      iwad_found = true;
      break;
    }
  }

  if (!iwad_found) {
    I_Error("WadDataToWadFiles: IWAD not found");
  }

  if (preloaded_wad_file_count) {
    for (size_t i = 0; i < preloaded->len; i++) {
      wadfile_info_t *pf = &g_array_index(preloaded, wadfile_info_t, i);
      wadfile_info_t *wf = malloc(sizeof(wadfile_info_t));

      if (!wf) {
        I_Error("WadDataToWadFiles: Allocating WAD file info failed");
      }

      wf->name = strdup(pf->name);
      wf->src = wf->src;
      wf->handle = wf->handle;

      g_ptr_array_add(resource_files, wf);
    }

    g_array_free(preloaded, true);
  }

  for (size_t i = 0; i < wad_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(wad_files, i);

    if (wf->src == source_auto_load) {
      wadfile_info_t *nwf = malloc(sizeof(wadfile_info_t));

      if (!nwf) {
        I_Error("WadDataToWadFiles: Allocating WAD file info failed");
      }

      nwf->name = strdup(wf->name);
      nwf->src = wf->src;
      nwf->handle = wf->handle;

      g_ptr_array_add(resource_files, nwf);
    }
  }

  for (size_t i = 0; i < wad_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(wad_files, i);

    if (wf->src == source_iwad) {
      if (i != iwad_index) {
        D_AddResource(wf->name, source_pwad);
        modifiedgame = true;
      }
    }
    else if (wf->src == source_pwad) {
      const char *file = I_FindFile2(wf->name, ".wad");

      if (!file && N_GetWad(wf->name)) {
        file = I_FindFile2(wf->name, ".wad");
        if (file) {
          free(wf->name);
          wf->name = strdup(file);
        }
      }

      if (file) {
        D_AddResource(wf->name, source_pwad);
        modifiedgame = true;
      }
    }

    if (wf->src == source_deh) {
      D_AddDEH(wf->name, 0);
    }
  }

  for (size_t i = 0; i < wad_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(wad_files, i);

    if (wf->src == source_lmp || wf->src == source_net) {
      D_AddResource(wf->name, wf->src);
    }
  }
}

void D_AutoLoadResources(void) {
  for (size_t i = 0; i < MAXLOADFILES; i++) {
    const char *fname = wad_file_names[i];
    char *fpath;

    if (!(fname && *fname)) {
      continue;
    }

    // Filename is now stored as a zero terminated string
    fpath = I_FindFile(fname, ".wad");

    if (!fpath) {
      D_MsgLocalInfo("Failed to autoload %s\n", fname);
      continue;
    }

    D_AddResource(fpath, source_auto_load);
    free(fpath);
  }
}

void D_AutoLoadPatches(void) {
  for (size_t i = 0; i < MAXLOADFILES; i++) {
    const char *fname = deh_file_names[i];
    char *fpath;

    if (!(fname && *fname)) {
      continue;
    }

    // Filename is now stored as a zero terminated string
    fpath = I_FindFile(fname, ".bex");

    if (!fpath) {
      D_MsgLocalInfo("Failed to autoload %s\n", fname);
      continue;
    }

    D_AddDEH(fpath, 0);
    // this used to set modifiedgame here, but patches shouldn't
    free(fpath);
  }
}

//
// AddIWAD
//
void D_AddIWAD(const char *iwad) {
  size_t i;

  if (!(iwad && *iwad)) {
    return;
  }

  D_MsgLocalInfo("IWAD found: %s\n", iwad); //jff 4/20/98 print only if found
  D_CheckIWAD(iwad, &gamemode, &haswolflevels);

  /*
   * jff 8/23/98 set gamemission global appropriately in all cases
   * cphipps 12/1999 - no version output here, leave that to the caller
   */
  i = strlen(iwad);

  switch(gamemode) {
    case retail:
    case registered:
    case shareware:
      gamemission = doom;
      if (i >= 8 && !strnicmp(iwad + i - 8, "chex.wad", 8)) {
        gamemission = chex;
      }
    break;
    case commercial:
      gamemission = doom2;
      if (i >= 10 && !strnicmp(iwad + i - 10, "doom2f.wad", 10)) {
        language = french;
      }
      else if (i >= 7 && !strnicmp(iwad + i - 7, "tnt.wad", 7)) {
        gamemission = pack_tnt;
      }
      else if (i >= 12 && !strnicmp(iwad + i - 12, "plutonia.wad", 12)) {
        gamemission = pack_plut;
      }
      else if (i >= 8 && !strnicmp(iwad + i - 8, "hacx.wad", 8)) {
        gamemission = hacx;
      }
    break;
    default:
      gamemission = none;
    break;
  }

  if (gamemode == indetermined) {
    D_MsgLocalInfo("Unknown Game Version, may not work\n");
  }
}

void D_SetIWAD(const char *iwad) {
  if (iwad_base) {
    free(iwad_base);
  }

  iwad_base = M_Basename(iwad);

  if (!iwad_base) {
    I_Error(
      "D_SetIWAD: Error getting basename of %s: %s\n",
      iwad,
      M_GetFileError()
    );
  }

  if (iwad_path) {
    free(iwad_path);
  }

  iwad_path = strdup(iwad);
}

const char* D_GetIWAD(void) {
  return iwad_base;
}

const char* D_GetIWADPath(void) {
  return iwad_path;
}

//
// D_ClearIWAD
//
void D_ClearIWAD(void) {
  if (iwad_base) {
    free(iwad_base);
  }

  iwad_base = NULL;

  if (iwad_path) {
    free(iwad_path);
  }

  iwad_path = NULL;
}

void D_ClearResourceFiles(void) {
  if (resource_files) {
    D_ResClear(resource_files);
  }
}

//
// D_ClearDEHFiles
//
void D_ClearDEHFiles(void) {
  if (deh_files) {
    D_ResClear(deh_files);
  }
}

//
// CheckIWAD
//
// Verify a file is indeed tagged as an IWAD
// Scan its lumps for levelnames and return gamemode as indicated
// Detect missing wolf levels in DOOM II
//
// The filename to check is passed in iwadname, the gamemode detected is
// returned in gmode, hassec returns the presence of secret levels
//
// jff 4/19/98 Add routine to test IWAD for validity and determine
// the gamemode from it. Also note if DOOM II, whether secret levels exist
// CPhipps - const char* for iwadname, made static
//e6y static
void D_CheckIWAD(const char *iwadname, game_mode_e *gmode, bool *hassec) {
  int ud = 0;
  int rg = 0;
  int sw = 0;
  int cm = 0;
  int sc = 0;
  int hx = 0;
  int cq = 0;
  bool noiwad = false;
  FILE *fp;
  wadinfo_t header;
  size_t length;
  filelump_t *fileinfo;

  if (!M_CheckAccess(iwadname, R_OK)) { // error from access call
    I_Error("D_CheckIWAD: IWAD %s not readable", iwadname);
  }

  if (!(fp = M_FileOpen(iwadname, "rb"))) { // error from open call
    I_Error(
      "D_CheckIWAD: Can't open IWAD %s (%s)", iwadname, M_GetFileError()
    );
  }

  *gmode = indetermined;
  *hassec = false;

  if (!M_FileRead(fp, (char *)&header, 1, sizeof(header))) {
    I_Error("D_CheckIWAD: Error reading IWAD header from %s: %s",
      iwadname,
      M_GetFileError()
    );
  }

  // Identify IWAD correctly

  // read IWAD header

  // check for missing IWAD tag in header
  if (strncmp(header.identification, "IWAD", 4) != 0) {
    noiwad = true;
  }

  // read IWAD directory
  header.numlumps = LittleLong(header.numlumps);
  header.infotableofs = LittleLong(header.infotableofs);
  length = header.numlumps;
  fileinfo = malloc(length * sizeof(filelump_t));

  if (!fileinfo) {
    I_Error("D_CheckIWAD: Allocating fileinfo memory failed\n");
  }

  if (!M_FileSeek(fp, header.infotableofs, SEEK_SET)) {
    I_Error("D_CheckIWAD: Error seeking to info table in %s: %s\n",
      iwadname,
      M_GetFileError()
    );
  }

  if (!M_FileRead(fp, (char *)fileinfo, length, sizeof(filelump_t))) {
    I_Error("D_CheckIWAD: Error reading lump data from %s: %s\n",
      iwadname,
      M_GetFileError()
    );
  }

  M_FileClose(fp);

  // scan directory for levelname lumps
  while (length--) {
    filelump_t *fi = &fileinfo[length];

    if (fi->name[0] == 'E' && fi->name[2] == 'M' && fi->name[4] == 0) {
      if (fi->name[1] == '4') {
        ud++;
      }
      else if (fi->name[1] == '3') {
        rg++;
      }
      else if (fi->name[1] == '2') {
        rg++;
      }
      else if (fi->name[1] == '1') {
        sw++;
      }
    }
    else if (fi->name[0] == 'M' && fi->name[1] == 'A' && fi->name[2] == 'P' &&
                                                         fi->name[5] == 0) {
      cm++;
      if (fi->name[3] == '3') {
        if (fi->name[4] == '1' || fi->name[4] == '2') {
          sc++;
        }
      }
    }

    if (!strncmp(fi->name, "DMENUPIC", 8)) {
      bfgedition++;
    }

    if (!strncmp(fi->name, "HACX", 4)) {
      hx++;
    }

    if (!strncmp(fi->name, "W94_1", 5) || !strncmp(fi->name, "POSSH0M0", 8)) {
      cq++;
    }
  }

  free(fileinfo);

  if (noiwad && !bfgedition && cq < 2) {
    I_Error("D_CheckIWAD: IWAD tag %s not present", iwadname);
  }

  // Determine game mode from levels present
  // Must be a full set for whichever mode is present
  // Lack of wolf-3d levels also detected here

  if (cm >= 30 || (cm >= 20 && hx)) {
    *gmode = commercial;
    if (sc >= 2) {
      *hassec = true;
    }
    else {
      *hassec = false;
    }
  }
  else if (ud >= 9) {
    *gmode = retail;
  }
  else if (rg >= 18) {
    *gmode = registered;
  }
  else if (sw >= 9) {
    *gmode = shareware;
  }
}

//
// D_IdentifyVersion
//
// Set the location of the defaults file and the savegame root
// Locate and validate an IWAD file
// Determine gamemode from the IWAD
//
// supports IWADs with custom names. Also allows the -iwad parameter to
// specify which iwad is being searched for if several exist in one dir.
// The -iwad parm may specify:
//
// 1) a specific pathname, which must exist (.wad optional)
// 2) or a directory, which must contain a standard IWAD,
// 3) or a filename, which must be found in one of the standard places:
//   a) current dir,
//   b) exe dir
//   c) $DOOMWADDIR
//   d) or $HOME
//
// jff 4/19/98 rewritten to use a more advanced search algorithm

void D_IdentifyVersion(void) {
  int i;            //jff 3/24/98 index of args on commandline
  struct stat sbuf; //jff 3/24/98 used to test save path for existence
  char *iwad;

  // set save path to -save parm or current dir

  //jff 3/27/98 default to current dir
  //V.Aguilar (5/30/99): In LiNUX, default to $HOME/.lxdoom
  // CPhipps - use DOOMSAVEDIR if defined
  const char *p = getenv("DOOMSAVEDIR");

  if (!p) {
    p = I_DoomExeDir();
  }

  if (basesavegame) {
    free(basesavegame);
  }

  basesavegame = strdup(p);

  //jff 3/24/98 if -save present
  if ((i = M_CheckParm("-save")) && i < myargc - 1) {
    if (!stat(myargv[i + 1], &sbuf) && S_ISDIR(sbuf.st_mode)) { // and is a dir
      free(basesavegame);
      basesavegame = strdup(myargv[i + 1]); //jff 3/24/98 use that for savegame
      M_StrNormalizeSlashes(basesavegame);  //jff 9/22/98 fix c:\ not working
    }
    else {
      D_MsgLocalWarn("Error: -save path does not exist, using %s\n",
        basesavegame
      );
    }
  }

  // locate the IWAD and determine game mode from it
  iwad = find_iwad_file();

#if (defined(GL_DOOM) && defined(LEVELINFO_DEBUG))
  // proff 11/99: used for debugging
  FILE *f = M_OpenFile("levelinfo.txt", "w");

  if (f) {
    fprintf(f, "%s\n", iwad);
    fclose(f);
  }
#endif

  if (!iwad || !(*iwad)) {
    I_Error("IdentifyVersion: IWAD not found\n");
  }

  D_AddIWAD(iwad);
  D_SetIWAD(iwad);
  D_AddResource(iwad_path, source_iwad);
  free(iwad);
}

void D_AddResource(const char *path, wad_source_e source) {
  char *wad_ext_path;
  char *wad_path;
  char *gwa_ext_path;
  char *gwa_filepath;
  int len;
  wadfile_info_t *wadfile;
  wadfile_info_t *gwafile;

  wad_ext_path = M_AddDefaultExtension(path, "wad");

  for (size_t i = 0; i < resource_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(resource_files, i);

    if (strcmp(wad_ext_path, wf->name) == 0) {
      D_MsgLocalWarn("D_AddResource: Skipping %s (already added).\n", path);
      free(wad_ext_path);
      return;
    }
  }

  D_MsgLocalInfo("D_AddResource: Searching for %s...\n", wad_ext_path);

  wad_path = I_FindFile(wad_ext_path, NULL);

  if (!wad_path) {
    I_Error(" %s missing (original path: %s)\n", wad_ext_path, path);
  }

  wadfile = malloc(sizeof(wadfile_info_t));

  if (!wadfile) {
    I_Error("D_AddResource: Allocating WAD file info failed");
  }

  D_ResAdd(resource_files, wad_path, source, 0);
  free(wad_path);

  // No Rest For The Living
  len = strlen(wadfile->name);
  if (len >= 9 && !strnicmp(wadfile->name + len - 9, "nerve.wad", 9)) {
    if (bfgedition) {
      gamemission = pack_nerve;
    }
  }

  // proff: automatically try to add the gwa files
  // proff - moved from w_wad.c
  gwa_ext_path = M_SetFileExtension(wad_ext_path, "gwa");

  free(wad_ext_path);

  gwa_filepath = I_FindFile(gwa_ext_path, NULL);

  free(gwa_ext_path);

  if (gwa_filepath) {
    gwafile = malloc(sizeof(wadfile_info_t));

    if (!gwafile) {
      I_Error("D_AddResource: Allocating GWA file info failed");
    }

    D_ResAdd(resource_files, gwa_filepath, source, 0);
    free(gwa_filepath);
  }

  free(gwa_filepath);
}

//
// D_AddDEH
//
void D_AddDEH(const char *filename, int lumpnum) {
  char *deh_path;
  deh_file_t *dehfile;
  const char *deh_out = D_DEHOut();

  if ((!filename) && lumpnum == 0) {
    I_Error("D_AddDEH: No filename or lumpnum given\n");
  }

  if (!filename) {
    deh_path = NULL;
  }
  else {
    deh_path = I_FindFile(filename, NULL);

    if (!deh_path) {
      I_Error("D_AddDEH: Couldn't find %s\n", filename);
    }
  }

  for (unsigned int i = 0; i < deh_files->len; i++) {
    deh_file_t *stored_deh_file = g_ptr_array_index(deh_files, i);

    if (!filename) {
      if (lumpnum == stored_deh_file->lumpnum) {
        D_MsgLocalInfo("D_AddDEH: Skipping duplicate DeH/BEX (%d)\n", lumpnum);
        return;
      }
      continue;
    }

    if (!stored_deh_file->filename) {
      continue;
    }

    if (strcmp(deh_path, stored_deh_file->filename) == 0) {
      D_MsgLocalInfo("D_AddDEH: Skipping %s (already added).\n", deh_path);
      return;
    }
  }

  if (deh_path) {
    D_MsgLocalInfo("D_AddDEH: Adding %s.\n", deh_path);
  }

  dehfile = malloc(sizeof(deh_file_t));

  if (!dehfile) {
    I_Error("D_AddDEH: Error allocating DEH file info");
  }

  dehfile->filename = deh_path;
  if (deh_out) {
    dehfile->outfilename = strdup(deh_out);
  }
  else {
    dehfile->outfilename = NULL;
  }
  dehfile->lumpnum = lumpnum;

  g_ptr_array_add(deh_files, dehfile);
}

/* vi: set et ts=2 sw=2: */

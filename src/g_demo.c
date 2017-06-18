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

#include <pcreposix.h>

#include "i_system.h"
#include "m_argv.h"
#include "m_file.h"
#include "m_misc.h"
#include "m_swap.h"
#include "d_event.h"
#include "d_main.h"
#include "d_res.h"
#include "e6y.h"
#include "g_comp.h"
#include "g_demo.h"
#include "g_game.h"
#include "g_input.h"
#include "g_keys.h"
#include "g_opt.h"
#include "g_overflow.h"
#include "g_save.h"
#include "hu_stuff.h"
#include "mn_def.h"
#include "p_camera.h"
#include "p_checksum.h"
#include "p_mobj.h"
#include "pl_cmd.h"
#include "pl_main.h"
#include "r_defs.h"
#include "w_wad.h"

#include "n_getwad.h"

extern bool doSkip;

#define PWAD_SIGNATURE "PWAD"
#define DEMOEX_VERSION "2"

#define DEMOEX_VERSION_LUMPNAME "VERSION"
#define DEMOEX_PORTNAME_LUMPNAME "PORTNAME"
#define DEMOEX_PARAMS_LUMPNAME "CMDLINE"
#define DEMOEX_MLOOK_LUMPNAME "MLOOK"
#define DEMOEX_COMMENT_LUMPNAME "COMMENT"

#define DEMOEX_SEPARATOR "\n"

extern patchnum_t hu_font[HU_FONTSIZE];

typedef struct {
  const char name[9];
  short     *data;
  int        lump;
  size_t     maxtick;
  size_t     tick;
} mlooklump_t;

static int demolumpnum = -1;

/* cph - only used for playback */
static char *demobuffer;

/* check for overrun (missing DEMOMARKER) */
static size_t demolength;

/* cph - record straight to file */
static FILE *demofp;

static const char *defdemoname;

//e6y static 
static char *demo_p;
static char *demo_continue_p = NULL;

static int smooth_playing_turns[SMOOTH_PLAYING_MAXFACTOR];
static int64_t smooth_playing_sum;
static int smooth_playing_index;
static angle_t smooth_playing_angle;

bool  netdemo;
bool  demorecording;
bool  demoplayback;
bool  democontinue = false;
char* demo_continue_name;
int   demover;
bool  singledemo; // quit after playing a demo from cmdline
bool  timingdemo;    // if true, exit with report on completion
bool  fastdemo;      // if true, run at full speed -- killough
int   longtics;
int   demo_skiptics;
int   demo_playerscount;
int   demo_tics_count;
int   demo_curr_tic;
char  demo_len_st[80];
bool  demo_stoponnext;
bool  demo_stoponend;
bool  demo_warp;
int   demo_overwriteexisting;
// e6y
// it's required for demos recorded in "demo compatibility" mode by boom201
// for example
int demover;
const char *getwad_cmdline;
int demo_smoothturns = false;
int demo_smoothturnsfactor = 6;
int demo_patterns_count;
const char *demo_patterns_mask;
char **demo_patterns_list;
const char *demo_patterns_list_def[9];

// demo ex
int demo_extendedformat = -1;
int demo_extendedformat_default;
bool use_demoex_info = false;

char demoex_filename[PATH_MAX];
const char *demo_demoex_filename;

mlooklump_t mlook_lump = { DEMOEX_MLOOK_LUMPNAME, NULL, -2, 0, 0 };

static size_t add_string(char **str, const char *val) {
  size_t size;

  if ((!str) || (!val)) {
    return 0;
  }

  if (*str) {
    size = strlen(*str) + strlen(val) + 1;
    *str = realloc(*str, size);
    strcat(*str, val);
  }
  else {
    size = strlen(val) + 1;
    *str = malloc(size);
    strcpy(*str, val);
  }

  return size;
}

static int get_version(void) {
  int result = -1;

  int lump, ver;
  unsigned int size;
  const char *data;
  char str_ver[32];

  lump = W_CheckNumForName(DEMOEX_VERSION_LUMPNAME);
  if (lump != -1) {
    size = W_LumpLength(lump);
    if (size > 0) {
      size_t len = MIN(size, sizeof(str_ver) - 1);
      data = W_CacheLumpNum(lump);
      strncpy(str_ver, data, len);
      str_ver[len] = 0;

      if (sscanf(str_ver, "%d", &ver) == 1) {
        result = ver;
      }
    }
    W_UnlockLumpNum(lump);
  }

  return result;
}

static void get_params(char *pwad_p, GPtrArray *wad_files) {
  int lump;
  size_t size;
  char *str;
  const char *data;
  char **params;
  int i, p, paramscount;

  lump = W_CheckNumForName(DEMOEX_PARAMS_LUMPNAME);
  if (lump == -1) {
    return;
  }

  size = W_LumpLength(lump);
  if (size <= 0) {
    return;
  }

  str = calloc(size + 1, 1);
  if (!str) {
    return;
  }

  data = W_CacheLumpNum(lump);
  strncpy(str, data, size);

  M_ParseCmdLine(str, NULL, NULL, &paramscount, &i);

  params = malloc(paramscount * sizeof(char *) + i * sizeof(char) + 1);
  if (params) {
    struct {
      const char  *param;
      wad_source_e source;
    }

    files[] = {
      { "-iwad", source_iwad },
      { "-file", source_pwad },
      { "-deh", source_deh },
      { NULL }
    };

    M_ParseCmdLine(str,
      params,
      ((char *)params) + sizeof(char *) * paramscount,
      &paramscount,
      &i);

    if (!M_CheckParm("-iwad") && !M_CheckParm("-file")) {
      for (i = 0; files[i].param; i++) {
        p = M_CheckParmEx(files[i].param, params, paramscount);

        if (p < 0) {
          continue;
        }

        while (++p != paramscount && *params[p] != '-') {
          // something is wrong here
          char *filename = I_FindFile(params[p], ".wad");

          if (!filename) {
            filename = strdup(params[p]);
          }

          D_ResAdd(wad_files, filename, files[i].source, 0);
          free(filename);
        }
      }
    }

    if (!M_CheckParm("-complevel")) {
      p = M_CheckParmEx("-complevel", params, paramscount);
      if (p >= 0 && p < (int)paramscount - 1) {
        M_AddParam("-complevel");
        M_AddParam(params[p + 1]);
      }
    }

    // for recording or playback using "single-player coop" mode
    if (!M_CheckParm("-solo-net")) {
      p = M_CheckParmEx("-solo-net", params, paramscount);
      if (p >= 0) {
        M_AddParam("-solo-net");
      }
    }

    if (!M_CheckParm("-emulate")) {
      p = M_CheckParmEx("-emulate", params, paramscount);
      if (p >= 0 && p < (int)paramscount - 1) {
        M_AddParam("-emulate");
        M_AddParam(params[p + 1]);
      }
    }

    // for doom 1.2
    if (!M_CheckParm("-respawn")) {
      p = M_CheckParmEx("-respawn", params, paramscount);
      if (p >= 0) {
        M_AddParam("-respawn");
      }
    }

    // for doom 1.2
    if (!M_CheckParm("-fast")) {
      p = M_CheckParmEx("-fast", params, paramscount);
      if (p >= 0) {
        M_AddParam("-fast");
      }
    }

    // for doom 1.2
    if (!M_CheckParm("-nomonsters")) {
      p = M_CheckParmEx("-nomonsters", params, paramscount);
      if (p >= 0) {
        M_AddParam("-nomonsters");
      }
    }

    p = M_CheckParmEx("-spechit", params, paramscount);
    if (p >= 0 && p < (int)paramscount - 1) {
      spechit_baseaddr = atoi(params[p + 1]);
    }

    // overflows
    {
      overrun_list_t overflow;
      for (overflow = 0; overflow < OVERFLOW_MAX; overflow++) {
        int value;
        char *pstr, *mask;

        mask = malloc(strlen(overflow_cfgname[overflow]) + 16);
        if (mask) {
          sprintf(mask, "-set %s", overflow_cfgname[overflow]);
          pstr = strstr(str, mask);

          if (pstr) {
            strcat(mask, " = %d");
            if (sscanf(pstr, mask, &value) == 1) {
              overflows[overflow].footer = true;
              overflows[overflow].footer_emulate = value;
            }
          }
          free(mask);
        }
      }
    }

    free(params);
  }

  W_UnlockLumpNum(lump);
  free(str);
}

static void add_params(wadtbl_t *wadtbl) {
  int p;
  char buf[200];

  char **item;
  char *filename_p;
  char *fileext_p;
  char *files = NULL;
  char *iwad  = NULL;
  char *pwads = NULL;
  char *dehs  = NULL;

  // IWAD and PWADs
  for (unsigned int i = 0; i < resource_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(resource_files, i);

    filename_p = PathFindFileName(wf->name);
    fileext_p = filename_p + strlen(filename_p) - 1;

    while (fileext_p != filename_p && *(fileext_p - 1) != '.') {
      fileext_p--;
    }

    if (fileext_p == filename_p) {
      continue;
    }

    item = NULL;

    if (wf->src == source_iwad && !iwad && !strcasecmp(fileext_p, "wad")) {
      item = &iwad;
    }

    if (wf->src == source_pwad && !strcasecmp(fileext_p, "wad")) {
      item = &pwads;
    }

    if (item) {
      add_string(item, "\"");
      add_string(item, filename_p);
      add_string(item, "\" ");
    }
  }

  // dehs
  p = M_CheckParm("-deh");
  if (p) {
    while (++p != myargc && *myargv[p] != '-') {
      char *file = NULL;
      if ((file = I_FindFile(myargv[p], ".bex")) ||
        (file = I_FindFile(myargv[p], ".deh"))) {
        filename_p = PathFindFileName(file);
        add_string(&dehs, "\"");
        add_string(&dehs, filename_p);
        add_string(&dehs, "\" ");
        free(file);
      }
    }
  }

  if (iwad) {
    add_string(&files, "-iwad ");
    add_string(&files, iwad);
  }

  if (pwads) {
    add_string(&files, "-file ");
    add_string(&files, pwads);
  }

  if (dehs) {
    add_string(&files, "-deh ");
    add_string(&files, dehs);
  }

  // add complevel for formats which do not have it in header
  if (demo_compatibility) {
    sprintf(buf, "-complevel %d ", compatibility_level);
    add_string(&files, buf);
  }

  // for recording or playback using "single-player coop" mode
  if (M_CheckParm("-solo-net")) {
    sprintf(buf, "-solo-net ");
    add_string(&files, buf);
  }

  if ((p = M_CheckParm("-emulate")) && (p < myargc - 1)) {
    sprintf(buf, "-emulate %s", myargv[p + 1]);
    add_string(&files, buf);
  }

  // doom 1.2 does not store these params in header
  if (compatibility_level == doom_12_compatibility) {
    if (M_CheckParm("-respawn")) {
      sprintf(buf, "-respawn ");
      add_string(&files, buf);
    }
    if (M_CheckParm("-fast")) {
      sprintf(buf, "-fast ");
      add_string(&files, buf);
    }
    if (M_CheckParm("-nomonsters")) {
      sprintf(buf, "-nomonsters ");
      add_string(&files, buf);
    }
  }

  if (spechit_baseaddr != 0 && spechit_baseaddr != DEFAULT_SPECHIT_MAGIC) {
    sprintf(buf, "-spechit %d ", spechit_baseaddr);
    add_string(&files, buf);
  }

  // overflows
  {
    overrun_list_t overflow;
    for (overflow = 0; overflow < OVERFLOW_MAX; overflow++) {
      if (overflows[overflow].shit_happens) {
        sprintf(buf, "-set %s=%d ", overflow_cfgname[overflow],
          overflows[overflow].emulate);
        add_string(&files, buf);
      }
    }
  }

  if (files) {
    W_AddLump(wadtbl, DEMOEX_PARAMS_LUMPNAME, files, strlen(files));
  }
}

static void add_mouse_look_data(wadtbl_t *wadtbl) {
  int i = 0;

  if (!mlook_lump.data) {
    return;
  }

  // search for at least one tic with a nonzero pitch
  while (i < (int)mlook_lump.tick) {
    if (mlook_lump.data[i] != 0) {
      W_AddLump(wadtbl, mlook_lump.name,
        (char *)mlook_lump.data,
        mlook_lump.tick * sizeof(mlook_lump.data[0]));
      break;
    }
    i++;
  }
}

static inline bool should_smooth(player_t *player) {
  if (!demo_smoothturns) {
    return false;
  }

  if (!demoplayback) {
    return false;
  }

  if (player && !PL_IsDisplayPlayer(player)) {
    return false;
  }

  return true;
}

static void read_one_tick(ticcmd_t* cmd, char **data_p) {
  char at = 0; // e6y: for tasdoom demo format

  cmd->forwardmove = (signed char)(*(*data_p)++);
  cmd->sidemove = (signed char)(*(*data_p)++);

  if (!longtics) {
    cmd->angleturn = ((char)(at = *(*data_p)++)) << 8;
  }
  else {
    unsigned int lowbyte = (char)(*(*data_p)++);

    cmd->angleturn = (((signed int)(*(*data_p)++)) << 8) + lowbyte;
  }

  cmd->buttons = (unsigned char)(*(*data_p)++);
  
  // e6y: ability to play tasdoom demos directly
  if (compatibility_level == tasdoom_compatibility) {
    signed char tmp = cmd->forwardmove;

    cmd->forwardmove = cmd->sidemove;
    cmd->sidemove = (signed char)at;
    cmd->angleturn = ((unsigned char)cmd->buttons) << 8;
    cmd->buttons = (unsigned char)tmp;
  }
}

static int get_original_doom_compat_level(int ver) {
  int lev;
  int i = M_CheckParm("-complevel");

  if (i && (i + 1 < myargc)) {
    lev = atoi(myargv[i + 1]);

    if (lev >= 0) {
      return lev;
    }
  }

  if (ver == 110) {
    return tasdoom_compatibility;
  }

  if (ver < 107) {
    return doom_1666_compatibility;
  }

  if (gamemode == retail) {
    return ultdoom_compatibility;
  }

  if (gamemission == pack_tnt || gamemission == pack_plut) {
    return finaldoom_compatibility;
  }

  return doom2_19_compatibility;
}

static char* get_demo_footer(const char *filename,
                             char **footer,
                             size_t *size) {
  char *buffer = NULL;
  size_t file_size = 0;
  char *p = NULL;
  char *result = NULL;

  if (!M_ReadFile(filename, &buffer, &file_size)) {
    return NULL;
  }

  // skip demo header
  p = G_DemoReadHeaderEx(buffer, file_size, RDH_SKIP_HEADER);

  // skip demo data
  while (p < buffer + file_size && *p != DEMOMARKER) {
    p += bytes_per_tic;
  }

  if (*p == DEMOMARKER) {
    p++; // skip DEMOMARKER

    // seach for the "PWAD" signature after ENDDEMOMARKER
    while (p - buffer + sizeof(wadinfo_t) < file_size) {
      if (!memcmp(p, PWAD_SIGNATURE, strlen(PWAD_SIGNATURE))) {
        // got it!
        // the demo has an additional information itself
        int demoex_size = file_size - (p - buffer);

        result = buffer;

        if (footer) {
          *footer = p;
        }

        if (size) {
          *size = demoex_size;
        }

        break;
      }
      p++;
    }
  }

  return result;
}

void set_demo_footer(const char *filename, wadtbl_t *wadtbl) {
  FILE *hfile = NULL;
  char *buffer = NULL;
  char *demoex_p = NULL;
  size_t size = 0;
  GString *new_file_name = g_string_sized_new(64);
  ptrdiff_t demosize = 0;
  size_t headersize = 0;
  size_t datasize = 0;
  size_t lumpssize = 0;

  // char newfilename[PATH_MAX];

  buffer = get_demo_footer(filename, &demoex_p, &size);

  if (!buffer) {
    D_MsgLocalError("set_demo_footer: Error opening original demo file: %s\n",
      M_GetFileError()
    );
    return;
  }

  g_string_append(new_file_name, filename);
  g_string_truncate(new_file_name, new_file_name->len - 5);
  g_string_append(new_file_name, ".out");

  hfile = M_FileOpen(new_file_name->str, "wb");

  g_string_free(new_file_name, true);
  if (!hfile) {
    D_MsgLocalError("set_demo_footer: Error opening new demo file: %s\n",
      M_GetFileError()
    );
    free(buffer);
    return;
  }

  demosize = (demoex_p - buffer);
  headersize = sizeof(wadtbl->header);
  datasize = wadtbl->datasize;
  lumpssize = wadtbl->header.numlumps * sizeof(wadtbl->lumps[0]);

  if (demosize < 0) {
    D_MsgLocalError("set_demo_footer: Pointer arithmetic problem\n");
    M_FileClose(hfile);
    free(buffer);
  }

  // write pwad header, all data and lookup table to the end of a demo
  if (!M_FileWrite(hfile, buffer, demosize, sizeof(char))) {
    D_MsgLocalError("set_demo_footer: Error writing demo size: %s\n",
      M_GetFileError()
    );
  }
  else if (!M_FileWrite(hfile, (char *)&wadtbl->header, headersize,
                                                        sizeof(char))) {
    D_MsgLocalError("set_demo_footer: Error writing WAD table header: %s\n",
      M_GetFileError()
    );
  }
  else if (!M_FileWrite(hfile, (char *)wadtbl->data, datasize, sizeof(char))) {
    D_MsgLocalError("set_demo_footer: Error writing WAD table data: %s\n",
      M_GetFileError()
    );
  }
  else if (!M_FileWrite(hfile, (char *)wadtbl->lumps, lumpssize,
                                                      sizeof(char))) {
    D_MsgLocalError("set_demo_footer: Error writing WAD table lumps: %s\n",
      M_GetFileError()
    );
  }

  M_FileClose(hfile);
  free(buffer);
}

static bool check_wad_buf_integrity(const char *buffer, size_t size) {
  size_t length;
  wadinfo_t *header;
  filelump_t *fileinfo;

  if (!buffer) {
    return false;
  }

  if (size <= sizeof(*header)) {
    return false;
  }

  header = (wadinfo_t *)buffer;

  if (!(strncmp(header->identification, "IWAD", 4) == 0 ||
        strncmp(header->identification, "PWAD", 4) == 0)) {
    return false;
  }

  header->numlumps = LittleLong(header->numlumps);
  header->infotableofs = LittleLong(header->infotableofs);
  length = header->numlumps * sizeof(filelump_t);

  if (header->infotableofs + length > size) {
    return false;
  }

  fileinfo = (filelump_t *)(buffer + header->infotableofs);

  for (size_t i = 0; i < header->numlumps; i++, fileinfo++) {
    if (fileinfo->filepos < 0 ||
        fileinfo->filepos > header->infotableofs ||
        fileinfo->filepos + fileinfo->size > header->infotableofs) {
      return false;
    }
  }

  return true;
}

static void change_demo_extended_format(void) {
  if (demo_extendedformat == -1) {
    demo_extendedformat = demo_extendedformat_default;
  }

  use_demoex_info = demo_extendedformat || M_CheckParm("-auto");
}

static bool read_demo_footer(const char *filename) {
  bool result = false;
  char *buffer = NULL;
  char *demoex_p = NULL;
  size_t size;

  change_demo_extended_format();

  if (!use_demoex_info) {
    return false;
  }

  demoex_filename[0] = 0;

  if (demo_demoex_filename && *demo_demoex_filename) {
    strncpy(demoex_filename, demo_demoex_filename, PATH_MAX);
  }
  else {
    char *tmp_path = NULL;
    const char *template_format = "%sd2k-demoex-XXXXXX";
    const char *tmp_dir = I_GetTempDir();

    if (tmp_dir && *tmp_dir != '\0') {
      tmp_path = malloc(strlen(tmp_dir) + 2);
      strcpy(tmp_path, tmp_dir);

      if (!HasTrailingSlash(tmp_dir)) {
        strcat(tmp_path, "/");
      }

      snprintf(
        demoex_filename,
        sizeof(demoex_filename),
        template_format,
        tmp_path
      );

      mktemp(demoex_filename);

      free(tmp_path);
    }
  }

  if (!demoex_filename[0]) {
    D_MsgLocalError("read_demo_footer: failed to create demoex temp file");
  }
  else {
    char *temp = M_AddDefaultExtension(demoex_filename, "wad");

    if (strlen(temp) >= PATH_MAX) {
      I_Error("read_demo_footer: No room in demoex_filename for extension");
    }

    strcpy(demoex_filename, temp);

    free(temp);

    buffer = get_demo_footer(filename, &demoex_p, &size);
    if (buffer) {
      // the demo has an additional information itself
      GPtrArray *wads;

      if (!check_wad_buf_integrity((const char *)demoex_p, size)) {
        D_MsgLocalError("read_demo_footer: demo footer is corrupted\n");
      }
      else if (!M_WriteFile(demoex_filename, (const char *)demoex_p, size)) {
        // write an additional info from a demo to demoex.wad
        D_MsgLocalError(
          "read_demo_footer: failed to create demoex temp file %s\n",
          demoex_filename
        );
      }
      else {
        // add demoex.wad to the wads list
        D_AddResource(demoex_filename, source_auto_load);

        // cache demoex.wad for immediately getting its data with
        // W_CacheLumpName
        W_Init();

        wads = D_ResNew();

        // enumerate and save all auto-loaded files and demo for future use
        for (size_t i = 0; i < resource_files->len; i++) {
          wadfile_info_t *wf = g_ptr_array_index(resource_files, i);

          if (wf->src == source_auto_load || wf->src == source_pre ||
                                             wf->src == source_lmp) {
            D_ResAdd(wads, wf->name, wf->src, 0);
          }
        }

        // get needed wads and dehs from demoex.wad
        // restore all critical params like -spechit x
        get_params(buffer, wads);

        // replace old wadfiles with the new ones
        if (wads->len) {
          for (size_t i = 0; i < wads->len; i++) {
            wadfile_info_t *wf = g_ptr_array_index(wads, i);

            if (wf->src == source_iwad) {
              W_ReleaseAllWads();
              D_ResourcesSet(wads);
              result = true;
              break;
            }
          }
        }
        D_ResFree(wads);
      }
      free(buffer);
    }
    else {
      demoex_filename[0] = 0;
    }
  }

  return result;
}

static size_t parse_demo_pattern(const char *str, GPtrArray *resources,
                                                  char **missed,
                                                  bool trytodownload) {
  size_t processed = 0;
  char *pStr = strdup(str);
  char *pToken = pStr;

  D_ResClear(resources);

  if (missed) {
    *missed = NULL;
  }

  for (; (pToken = strtok(pToken, "|")); pToken = NULL) {
    char *token = NULL;

    processed++;

    if (trytodownload && !I_FindFile2(pToken, ".wad")) {
      N_GetWad(pToken);
    }

#ifdef _MSC_VER
    token = malloc(PATH_MAX);
    if (GetFullPath(pToken, ".wad", token, PATH_MAX))
#else  /* ifdef _MSC_VER */
    if ((token = I_FindFile(pToken, ".wad")))
#endif /* ifdef _MSC_VER */
    {
      size_t len = strlen(token);

      if (pToken == pStr) {
        D_ResAdd(resources, token, source_iwad, 0);
      }
      else if (!strcasecmp(&token[len - 4], ".wad")) {
        D_ResAdd(resources, token, source_pwad, 0);
      }
      else if (!strcasecmp(&token[len - 4], ".deh") ||
               !strcasecmp(&token[len - 4], ".bex")) {
        D_AddDEH(token, 0);
      }
      else {
        I_Error("parse_demo_pattern: Unknown resource type for %s\n", token);
      }
    }
    else if (missed) {
      int len = (*missed ? strlen(*missed) : 0);

      *missed = realloc(*missed, len + strlen(pToken) + 100);
      sprintf(*missed + len, " %s not found\n", pToken);
    }
  }

  free(pStr);

  return processed;
}

static size_t demo_name_to_wad_data(const char *demoname,
                                    GPtrArray *resources,
                                    patterndata_t *patterndata) {
  size_t numwadfiles_required = 0;
  size_t maxlen = 0;
  char *pattern;
  char *demofilename = PathFindFileName(demoname);

  for (size_t i = 0; i < demo_patterns_count; i++) {
    if (strlen(demo_patterns_list[i]) > maxlen) {
      maxlen = strlen(demo_patterns_list[i]);
    }
  }

  pattern = malloc(maxlen + sizeof(char));
  for (size_t i = 0; i < demo_patterns_count; i++) {
    int result;
    regex_t preg;
    regmatch_t pmatch[4];
    char errbuf[256];
    char *buf = demo_patterns_list[i];

    regcomp(&preg, "(.*?)\\/(.*)\\/(.+)", REG_ICASE);
    result = regexec(&preg, buf, 4, &pmatch[0], REG_NOTBOL);
    regerror(result, &preg, errbuf, sizeof(errbuf));
    regfree(&preg);

    if (result != 0) {
      D_MsgLocalWarn(
        "Incorrect format of the <%s%zu = \"%s\"> config entry\n",
        demo_patterns_mask,
        i,
        buf
      );
    }
    else {
      regmatch_t demo_match[2];
      int len = pmatch[2].rm_eo - pmatch[2].rm_so;

      strncpy(pattern, buf + pmatch[2].rm_so, len);
      pattern[len] = '\0';
      result = regcomp(&preg, pattern, REG_ICASE);
      if (result != 0) {
        regerror(result, &preg, errbuf, sizeof(errbuf));
        D_MsgLocalWarn(
          "Incorrect regular expressions in the <%s%zu = \"%s\"> config "
          "entry - %s\n",
          demo_patterns_mask,
          i,
          buf,
          errbuf
        );
      }
      else {
        result = regexec(&preg, demofilename, 1, &demo_match[0], 0);
        if (result == 0 && demo_match[0].rm_so == 0 &&
            demo_match[0].rm_eo == (int)strlen(demofilename)) {
          int count = parse_demo_pattern(
            buf + pmatch[3].rm_so,
            resources,
            (patterndata ? &patterndata->missed : NULL),
            true
          );

          if (count < 0) {
            I_Error(
              "demo_name_to_wad_data: Demo pattern contained a negative WAD "
              "count\n"
            );
          }

          numwadfiles_required = (size_t)count;

          D_ResAdd(resources, demoname, source_lmp, 0);

          if (patterndata) {
            len = MIN(
              pmatch[1].rm_eo - pmatch[1].rm_so,
              sizeof(patterndata->pattern_name) - 1
            );
            strncpy(patterndata->pattern_name, buf, len);
            patterndata->pattern_name[len] = '\0';

            patterndata->pattern_num = i;
          }

          break;
        }
      }
      regfree(&preg);
    }
  }
  free(pattern);

  return numwadfiles_required;
}

static bool check_for_overrun(char *start_p, char *current_p,
                                             ptrdiff_t maxsize,
                                             ptrdiff_t size,
                                             bool failonerror) {
  ptrdiff_t pos = current_p - start_p;

  if (pos + size > maxsize) {
    if (failonerror) {
      I_Error("check_for_overrun: Overrun detected\n");
    }

    return true;
  }

  return false;
}

void G_BeginRecording(void) {
  int i;
  unsigned char game_options[GAME_OPTION_SIZE];
  char *demostart, *demo_p;
  size_t bytes_written;

  demostart = demo_p = malloc(1000);
  longtics = 0;

  /* cph - 3 demo record formats supported: MBF+, BOOM, and Doom v1.9 */
  if (mbf_features) {
    unsigned char v = 0;

    switch(compatibility_level) {
      case mbf_compatibility:
        v = 203;
      break; // e6y: Bug in MBF compatibility mode fixed
      case prboom_2_compatibility:
        v = 210;
      break;
      case prboom_3_compatibility:
        v = 211;
      break;
      case prboom_4_compatibility:
        v = 212;
      break;
      case prboom_5_compatibility:
        v = 213;
      break;
      case prboom_6_compatibility:
			  v = 214; 
			  longtics = 1;
      break;
      default:
        I_Error("G_BeginRecording: PrBoom compatibility level unrecognised");
    }

    *demo_p++ = v; /* Write version code into demo */

    // signature
    *demo_p++ = 0x1d;
    *demo_p++ = 'M';
    *demo_p++ = 'B';
    *demo_p++ = 'F';
    *demo_p++ = 0xe6;
    *demo_p++ = '\0';

    /* killough 2/22/98: save compatibility flag in new demos
     * cph - FIXME? MBF demos will always be not in compat. mode */
    *demo_p++ = 0;

    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = P_GetConsolePlayer()->id;

    G_WriteOptions(game_options); // killough 3/1/98: Save game options
    for (i = 0; i < GAME_OPTION_SIZE; i++) {
      *demo_p++ = game_options[i];
    }

    for (i = 0; i < VANILLA_MAXPLAYERS; i++) {
      if (P_PlayersLookup(i + 1)) {
        *demo_p++ = 1;
      }
      else {
        *demo_p++ = 0;
      }
    }

    // killough 2/28/98:
    // We always store at least MIN_MAXPLAYERS bytes in demo, to
    // support enhancements later w/o losing demo compatibility

    for (; i < MIN_MAXPLAYERS; i++) {
      *demo_p++ = 0;
    }
  }
  /*
   * [FIXME] e6y
   * else if (compatibility_level >= boom_compatibility_compatibility)
   */
  else if (compatibility_level > boom_compatibility_compatibility) {
    unsigned char v = 0;
    unsigned char c = 0; /* Nominally, version and compatibility bits */

    switch (compatibility_level) {
      case boom_compatibility_compatibility:
        v = 202;
        c = 1;
      break;
      case boom_201_compatibility:
        v = 201;
        c = 0;
      break;
      case boom_202_compatibility:
        v = 202;
        c = 0;
      break;
      default:
        I_Error("G_BeginRecording: Boom compatibility level unrecognised");
    }

    *demo_p++ = v;

    // signature
    *demo_p++ = 0x1d;
    *demo_p++ = 'B';
    *demo_p++ = 'o';
    *demo_p++ = 'o';
    *demo_p++ = 'm';
    *demo_p++ = 0xe6;

    /* CPhipps - save compatibility level in demos */
    *demo_p++ = c;

    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = P_GetConsolePlayer()->id;

    G_WriteOptions(game_options); // killough 3/1/98: Save game options
    for (i = 0; i < GAME_OPTION_SIZE; i++) {
      *demo_p++ = game_options[i];
    }

    for (i = 0; i < VANILLA_MAXPLAYERS; i++) {
      if (P_PlayersLookup(i + 1)) {
        *demo_p++ = 1;
      }
      else {
        *demo_p++ = 0;
      }
    }

    // killough 2/28/98:
    // We always store at least MIN_MAXPLAYERS bytes in demo, to
    // support enhancements later w/o losing demo compatibility

    for (; i < MIN_MAXPLAYERS; i++) {
      *demo_p++ = 0;
    }
  }
  else { // cph - write old v1.9 demos (might even sync)
    unsigned char v = 109;

    longtics = M_CheckParm("-longtics");

    if (longtics) {
      v = 111;
    }
    else {
      switch (compatibility_level) {
        case doom_1666_compatibility:
          v = 106;
        break;
        case tasdoom_compatibility:
          v = 110;
        break;
      }
    }
    *demo_p++ = v;
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = deathmatch;
    *demo_p++ = respawnparm;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = P_GetConsolePlayer()->id;

    for (i = 0; i < VANILLA_MAXPLAYERS; i++) {
      if (P_PlayersLookup(i + 1)) {
        *demo_p++ = 1;
      }
      else {
        *demo_p++ = 0;
      }
    }
  }

  bytes_written = fwrite(demostart, 1, demo_p - demostart, demofp);
  
  if (bytes_written != (size_t)(demo_p - demostart)) {
    I_Error("G_BeginRecording: Error writing demo header");
  }

  free(demostart);
}

//
// G_PlayDemo
//

void G_DeferedPlayDemo(const char *name) {
  defdemoname = name;
  G_SetGameAction(ga_playdemo);
}

char* G_DemoReadHeader(char *demo_p, size_t size) {
  return G_DemoReadHeaderEx(demo_p, size, 0);
}

char* G_DemoReadHeaderEx(char *demo_p, size_t size, unsigned int params) {
  skill_e skill;
  int i;
  int episode;
  int map;
  unsigned char game_options[GAME_OPTION_SIZE];

  // e6y
  // The local variable should be used instead of demobuffer,
  // because demobuffer can be uninitialized
  char *header_p = demo_p;
  bool failonerror = (params & RDH_SAFE);

  basetic = gametic;  // killough 9/29/98

  // killough 2/22/98, 2/28/98: autodetect old demos and act accordingly.
  // Old demos turn on demo_compatibility => compatibility; new demos load
  // compatibility flag, and other flags as well, as a part of the demo.

  //e6y: check for overrun
  if (check_for_overrun(header_p, demo_p, size, 1, failonerror)) {
    return NULL;
  }

  demover = *demo_p++;
  longtics = 0;

  // e6y
  // Handling of unrecognized demo formats
  // Versions up to 1.2 use a 7-byte header - first byte is a skill level.
  // Versions after 1.2 use a 13-byte header - first byte is a demoversion.
  // BOOM's demoversion starts from 200
  if (!((demover >=   0  && demover <=   4) ||
        (demover >= 104  && demover <= 111) ||
        (demover >= 200  && demover <= 214))) {
    I_Error("G_DemoReadHeader: Unknown demo format %d.", demover);
  }

  if (demover < 200) {   // Autodetect old demos
    if (demover >= 111) {
      longtics = 1;
    }

    // killough 3/2/98: force these variables to be 0 in demo_compatibility

    variable_friction = 0;
    weapon_recoil = 0;
    allow_pushers = 0;
    monster_infighting = 1;           // killough 7/19/98
#ifdef DOGS
    dogs = 0;                         // killough 7/19/98
    dog_jumping = 0;                  // killough 10/98
#endif
    monster_backing = 0;              // killough 9/8/98
    monster_avoid_hazards = 0;        // killough 9/9/98
    monster_friction = 0;             // killough 10/98
    help_friends = 0;                 // killough 9/9/98
    monkeys = 0;

    // killough 3/6/98: rearrange to fix savegame bugs (moved fastparm,
    // respawnparm, nomonsters flags to G_LoadOptions()/G_SaveOptions())

    skill = demover;

    if (demover >= 100) {                 // For demos from versions >= 1.4
      unsigned char consoleplayer;

      //e6y: check for overrun
      if (check_for_overrun(header_p, demo_p, size, 8, failonerror)) {
        return NULL;
      }

      compatibility_level = get_original_doom_compat_level(demover);
      skill = *demo_p++;
      episode = *demo_p++;
      map = *demo_p++;
      deathmatch = *demo_p++;
      respawnparm = *demo_p++;
      fastparm = *demo_p++;
      nomonsters = *demo_p++;
      consoleplayer = *demo_p++;

      P_SetConsolePlayer(P_PlayersGetNewWithID(consoleplayer));
    }
    else {
      //e6y: check for overrun
      if (check_for_overrun(header_p, demo_p, size, 2, failonerror)) {
        return NULL;
      }

      compatibility_level = doom_12_compatibility;
      episode = *demo_p++;
      map = *demo_p++;
      deathmatch = 0;
      respawnparm = 0;
      fastparm = 0;
      nomonsters = 0;
      
      // e6y
      // Ability to force -nomonsters and -respawn for playback of 1.2 demos.
      // Demos recorded with Doom.exe 1.2 did not contain any information
      // about whether these parameters had been used. In order to play them
      // back, you should add them to the command-line for playback.
      // There is no more desynch on mesh.lmp @ mesh.wad
      // prboom -iwad doom.wad -file mesh.wad -playdemo mesh.lmp -nomonsters
      // http://www.doomworld.com/idgames/index.php?id=13976
      respawnparm = M_CheckParm("-respawn");
      fastparm = M_CheckParm("-fast");
      nomonsters = M_CheckParm("-nomonsters");

      // e6y: detection of more unsupported demo formats
      if (*(header_p + size - 1) == DEMOMARKER) {
        // file size test;
        // DOOM_old and HERETIC don't use maps>9;
        // 2 at 4,6 means playerclass=mage -> not DOOM_old or HERETIC;
        if ((size >= 8 && (size - 8) % 4 != 0) ||
            (map > 9) ||
            (size >= 6 && (*(header_p + 4) == 2 || *(header_p + 6) == 2))) {
          I_Error("Unrecognised demo format.");
        }
      }

    }

    G_Compatibility();
  }
  else {  // new versions of demos
    unsigned char consoleplayer;

    demo_p += 6;               // skip signature;

    switch (demover) {
      case 200: /* BOOM */
      case 201:
        //e6y: check for overrun
        if (check_for_overrun(header_p, demo_p, size, 1, failonerror)) {
          return NULL;
        }

        if (!*demo_p++) {
          compatibility_level = boom_201_compatibility;
        }
        else {
          compatibility_level = boom_compatibility_compatibility;
        }
      break;
      case 202:
        //e6y: check for overrun
        if (check_for_overrun(header_p, demo_p, size, 1, failonerror)) {
          return NULL;
        }

        if (!*demo_p++) {
          compatibility_level = boom_202_compatibility;
        }
        else {
          compatibility_level = boom_compatibility_compatibility;
        }
      break;
      case 203:
      /* LxDoom or MBF - determine from signature
       * cph - load compatibility level */
      switch (*(header_p + 2)) {
        case 'B': /* LxDoom */
          /* cph - DEMOSYNC - LxDoom demos recorded in compatibility modes
           * support dropped */
          compatibility_level = lxdoom_1_compatibility;
          break;
        case 'M':
          compatibility_level = mbf_compatibility;
          demo_p++;
          break;
      }
      break;
      case 210:
        compatibility_level = prboom_2_compatibility;
        demo_p++;
      break;
      case 211:
        compatibility_level = prboom_3_compatibility;
        demo_p++;
      break;
      case 212:
        compatibility_level = prboom_4_compatibility;
        demo_p++;
      break;
      case 213:
        compatibility_level = prboom_5_compatibility;
        demo_p++;
      break;
      case 214:
        compatibility_level = prboom_6_compatibility;
        longtics = 1;
        demo_p++;
      break;
    }
    //e6y: check for overrun
    if (check_for_overrun(header_p, demo_p, size, 5, failonerror)) {
      return NULL;
    }

    skill = *demo_p++;
    episode = *demo_p++;
    map = *demo_p++;
    deathmatch = *demo_p++;
    consoleplayer = *demo_p++;

    P_SetConsolePlayer(P_PlayersGetNewWithID(consoleplayer));

    //e6y: check for overrun
    if (check_for_overrun(header_p, demo_p, size, GAME_OPTION_SIZE,
                                                  failonerror)) {
      return NULL;
    }

    for (i = 0; i < GAME_OPTION_SIZE; i++) {
      game_options[i] = *demo_p++;
    }

    G_ReadOptions(game_options);  // killough 3/1/98: Read game options

    if (demover == 200) { // killough 6/3/98: partially fix v2.00 demos
      demo_p += 256 - GAME_OPTION_SIZE;
    }
  }

  D_MsgLocalInfo("G_DoPlayDemo: playing demo with %s compatibility\n",
    comp_lev_str[compatibility_level]
  );

  //e6y
  // only 4 players can exist in old demos
  if (demo_compatibility || demover < 200) {
    //e6y: check for overrun
    if (check_for_overrun(header_p, demo_p, size, 4, failonerror)) {
      return NULL;
    }

    for (i = 0; i < VANILLA_MAXPLAYERS; i++) {
      bool in_game = *demo_p++;

      if (in_game) {
        P_PlayersGetNewWithID(i + 1);
      }
    }
  }
  else {
    //e6y: check for overrun
    if (check_for_overrun(header_p, demo_p, size, VANILLA_MAXPLAYERS,
                                                  failonerror)) {
      return NULL;
    }

    for (i = 0; i < VANILLA_MAXPLAYERS; i++) {
      bool in_game = *demo_p++;

      if (in_game) {
        P_PlayersGetNewWithID(i + 1);
      }
    }

    demo_p += MIN_MAXPLAYERS - VANILLA_MAXPLAYERS;
  }

  if (P_PlayersLookup(2)) {
    netgame = true;
    netdemo = true;
  }

  if (!(params & RDH_SKIP_HEADER)) {
    if (gameaction != ga_loadgame) { /* killough 12/98: support -loadgame */
      G_InitNew(skill, episode, map);
    }
  }

  PLAYERS_FOR_EACH(iter) {
    iter.player->cheats = 0; // killough 4/24/98
  }

  // e6y
  // additional params
  {
    char *p = demo_p;

    if (longtics) {
      bytes_per_tic = 5;
    }
    else {
      bytes_per_tic = 4;
    }

    demo_playerscount = 0;
    demo_tics_count = 0;
    demo_curr_tic = 0;
    strcpy(demo_len_st, "-");

    demo_playerscount = P_PlayersGetCount();

    if (demo_playerscount > 0 && demolength > 0) {
      do {
        demo_tics_count++;
        p += bytes_per_tic;
      } while ((p < demobuffer + demolength) && (*p != DEMOMARKER));

      demo_tics_count /= demo_playerscount;

      sprintf(demo_len_st, "\x1b\x35/%d:%02d", 
        demo_tics_count / TICRATE / 60, 
        (demo_tics_count % (60 * TICRATE)) / TICRATE
      );
    }
  }

  return demo_p;
}

/*
 * G_CheckDemoStatus
 *
 * Called after a death or level completion to allow demos to be cleaned up
 * Returns true if a new demo loop action will take place
 */
bool G_CheckDemoStatus(void) {
  //e6y
  if (doSkip && (demo_stoponend || demo_stoponnext)) {
    G_SkipDemoStop();
  }

  P_ChecksumFinal();

  if (demorecording) {
    demorecording = false;
    fputc(DEMOMARKER, demofp);
    
    //e6y
    G_DemoWriteFooter(demofp);

    D_MsgLocalInfo("G_CheckDemoStatus: Demo recorded\n");

    return false;  // killough
  }

  if (timingdemo) {
    int endtime = I_GetTime_RealTime ();
    // killough -- added fps information and made it work for longer demos:
    unsigned realtics = endtime-starttime;

    MN_SaveDefaults();

    I_Error("Timed %u gametics in %u realtics = %-.1f frames per second",
      (unsigned) gametic, realtics,
      (unsigned) gametic * (double) TICRATE / realtics
    );
  }

  if (demoplayback) {
    if (singledemo) {
      exit(0);  // killough
    }

    if (demolumpnum != -1) {
      // cph - unlock the demo lump
      W_UnlockLumpNum(demolumpnum);
      demolumpnum = -1;
    }
    G_ReloadDefaults();    // killough 3/1/98
    netgame = false;       // killough 3/29/98
    deathmatch = false;
    D_AdvanceDemo();
    return true;
  }

  return false;
}

//e6y
void G_CheckDemoContinue(void) {
  if (democontinue) {
    if (G_DemoLoad(defdemoname, &demobuffer, &demolength, &demolumpnum)) {
      demo_continue_p = G_DemoReadHeaderEx(demobuffer, demolength, RDH_SAFE);

      singledemo = true;
      autostart = true;
      G_RecordDemo(demo_continue_name);
      G_BeginRecording();
      usergame = true;
    }
  }
}

//
// DEMO RECORDING
//

//
// G_RecordDemo
//

void G_RecordDemo(const char* name) {
  char *demoname = M_AddDefaultExtension(name, "lmp"); // 1/18/98 killough

  usergame = false;
  demorecording = true;
  
  /* cph - Record demos straight to file
  * If file already exists, try to continue existing demo
  */

  demofp = NULL;
  if (access(demoname, F_OK) || democontinue ||
      (demo_compatibility && demo_overwriteexisting)) {
    if (strlen(demoname) > 4
        && !strcasecmp(demoname + strlen(demoname) - 4, ".wad")) {
      I_Error(
        "G_RecordDemo: Cowardly refusing to record over what appears to be a "
        "WAD. (%s)",
        demoname
      );
    }

    demofp = fopen(demoname, "wb");
  }
  else {
    if (demo_compatibility && !demo_overwriteexisting) {
      I_Error("G_RecordDemo: file %s already exists", name);
    }

    demofp = fopen(demoname, "rb+");
    if (demofp) {
      int slot = -1;
      char *pos;
      char buf[200];
      size_t len;

      //e6y: save all data which can be changed by G_DemoReadHeader
      G_SaveRestoreGameOptions(true);

      /* Read the demo header for options etc */
      len = M_FileRead(demofp, buf, 1, sizeof(buf));
      pos = G_DemoReadHeader(buf, len);
      if (pos) {
        int rc;
        int bytes_per_tic;
        
        if (longtics) {
          bytes_per_tic = 5;
        }
        else {
          bytes_per_tic = 4;
        }

        M_FileSeek(demofp, pos - buf, SEEK_SET);

        /* Now read the demo to find the last save slot */
        do {
          char buf2[5];
          unsigned char buttons;

          rc = M_FileRead(demofp, buf2, 1, bytes_per_tic);

          if (buf2[0] == DEMOMARKER || rc < bytes_per_tic - 1) {
            break;
          }

          buttons = (unsigned char)buf2[bytes_per_tic - 1];

          if (buttons & BT_SPECIAL) {
            if ((buttons & BT_SPECIALMASK) == BTS_SAVEGAME) {
              slot = (buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
            }
          }
        } while (rc == bytes_per_tic);

        if (slot != -1) {
          /* Return to the last save position, and load the relevant savegame */
          M_FileSeek(demofp, -rc, SEEK_CUR);
          G_LoadGame(slot, false);
          autostart = false;
          return;
        }
      }

      //demo cannot be continued
      M_FileClose(demofp);
      if (demo_overwriteexisting) {
        //restoration of all data which could be changed by G_DemoReadHeader
        G_SaveRestoreGameOptions(false);
        demofp = M_FileOpen(demoname, "wb");
      }
      else {
        I_Error("G_RecordDemo: No save in demo, can't continue");
      }
    }
  }

  if (!demofp) {
    I_Error("G_RecordDemo: failed to open %s", name);
  }

  free(demoname);
}

const char* G_DemoGetDemoPlaybackArg(void) {
  int p;

  if ((p = M_CheckParm("-playdemo")) && (p < myargc - 1)) {
    return myargv[p + 1];
  }

  if ((p = M_CheckParm("-timedemo")) && (p < myargc - 1)) {
    return myargv[p + 1];
  }

  if ((p = M_CheckParm("-fastdemo")) && (p < myargc - 1)) {
    return myargv[p + 1];
  }

  return NULL;
}

const char* G_DemoGetDemoContinueArg(void) {
  int p;

  if ((p = M_CheckParm("-recordfromto")) &&
      (p < myargc - 2) &&
      I_FindFile(myargv[p + 1], ".lmp")) {
    return myargv[p + 1];
  }

  return NULL;
}

const char* G_DemoGetDemoArg(void) {
  const char *demo_playback_arg = G_DemoGetDemoPlaybackArg();

  if (!demo_playback_arg) {
    return G_DemoGetDemoContinueArg();
  }

  return demo_playback_arg;
}

bool G_DemoIsPlayback(void) {
  const char *demo_playback_arg = G_DemoGetDemoPlaybackArg();

  if (!demo_playback_arg) {
    return false;
  }

  return true;
}

bool G_DemoIsContinue(void) {
  const char *demo_continue_arg = G_DemoGetDemoContinueArg();

  if (!demo_continue_arg) {
    return false;
  }

  return true;
}

bool G_DemoLoad(const char *name, char **buffer, size_t *length, int *lump) {
  char basename[9];
  int  num = -1;
  int  intlen = 0;

  M_ExtractFileBase(name, basename);
  basename[8] = 0;

  // check ns_demos namespace first, then ns_global
  num = (W_CheckNumForName)(basename, ns_demos);

  if (num < 0) {
    num = W_CheckNumForName(basename);
  }

  if (num < 0) {
    // Allow for demos not loaded as lumps
    bool success;
    char *filename = I_FindFile(name, ".lmp");

    if (!filename) {
      return false;
    }

    success = M_ReadFile(filename, buffer, length);
    free(filename);

    return success;
  }

  *buffer = (char *)W_CacheLumpNum(num);
  intlen = W_LumpLength(num);

  if (*lump) {
    *lump = num;
  }

  if (intlen > 0) {
    if (*length) {
      *length = (size_t)intlen;
    }
    return true;
  }

  return false;
}

void G_DemoSmoothPlayingReset(player_t *player) {
  if (!should_smooth(player)) {
    return;
  }

  if (!player) {
    player = P_GetDisplayPlayer();
  }

  if (!player->mo) {
    return;
  }

  smooth_playing_angle = player->mo->angle;
  memset(
    smooth_playing_turns,
    0,
    sizeof(smooth_playing_turns[0]) * SMOOTH_PLAYING_MAXFACTOR
  );
  smooth_playing_sum = 0;
  smooth_playing_index = 0;
}

void G_DemoSmoothPlayingAdd(int delta) {
  if (!should_smooth(NULL)) {
    return;
  }

  smooth_playing_sum -= smooth_playing_turns[smooth_playing_index];
  smooth_playing_turns[smooth_playing_index] = delta;
  smooth_playing_index = (smooth_playing_index + 1) % (demo_smoothturnsfactor);
  smooth_playing_sum += delta;
  smooth_playing_angle += (int)(smooth_playing_sum / (demo_smoothturnsfactor));
}

angle_t G_DemoSmoothPlayingGet(player_t *player) {
  if (should_smooth(player)) {
    return smooth_playing_angle;
  }

  return player->mo->angle;
}

angle_t G_DemoReadMouseLook(void) {
  angle_t pitch;

  if (!use_demoex_info || !(demoplayback || democontinue)) {
    return 0;
  }

  // mlook data must be initialised here
  if (mlook_lump.lump == -2) {
    if (get_version() < 2) { // unsupported format
      mlook_lump.lump = -1;
    }
    else {
      mlook_lump.lump = W_CheckNumForName(mlook_lump.name);

      if (mlook_lump.lump != -1) {
        unsigned char *data = (unsigned char *)W_CacheLumpName(
          mlook_lump.name
        );
        int size = W_LumpLength(mlook_lump.lump);

        mlook_lump.maxtick = size / sizeof(mlook_lump.data[0]);
        mlook_lump.data = malloc(size);
        memcpy(mlook_lump.data, data, size);
      }
    }
  }

  pitch = 0;

  if (mlook_lump.data &&
      mlook_lump.tick < mlook_lump.maxtick &&
      P_GetConsolePlayer() == P_GetDisplayPlayer() &&
      walkcamera.mode == camera_mode_disabled) {
    pitch = mlook_lump.data[mlook_lump.tick];
  }
  mlook_lump.tick++;

  return pitch << 16;
}

void G_DemoWriteMouseLook(angle_t pitch) {
  if (!use_demoex_info || !demorecording) {
    return;
  }

  if (mlook_lump.tick >= mlook_lump.maxtick) {
    int ticks = mlook_lump.maxtick;
    mlook_lump.maxtick = (mlook_lump.maxtick ? mlook_lump.maxtick * 2 : 8192);
    if (mlook_lump.tick >= mlook_lump.maxtick) {
      mlook_lump.maxtick = mlook_lump.tick * 2;
    }
    mlook_lump.data =
      realloc(mlook_lump.data, mlook_lump.maxtick * sizeof(mlook_lump.data[0]));
    memset(mlook_lump.data + ticks, 0,
      (mlook_lump.maxtick - ticks) * sizeof(mlook_lump.data[0]));
  }

  mlook_lump.data[mlook_lump.tick] = (short)(pitch >> 16);
  mlook_lump.tick++;
}

void G_DemoShutdown(void) {
  W_ReleaseAllWads();

  if (demoex_filename[0] && !(demo_demoex_filename && *demo_demoex_filename)) {
    D_MsgLocalDebug("G_DemoShutdown: removing %s\n", demoex_filename);
    if (unlink(demoex_filename) != 0) {
      D_MsgLocalError("G_DemoShutdown: %s\n", strerror(errno));
    }
  }
}

void G_DemoWriteFooter(FILE *file) {
  wadtbl_t demoex;

  if (!use_demoex_info) {
    return;
  }

  // init PWAD header
  W_InitPWADTable(&demoex);

  //
  // write all the data
  //

  // separators for eye-friendly looking
  W_AddLump(
    &demoex,
    NULL,
    DEMOEX_SEPARATOR,
    strlen(DEMOEX_SEPARATOR)
  );

  W_AddLump(
    &demoex,
    NULL,
    DEMOEX_SEPARATOR,
    strlen(DEMOEX_SEPARATOR)
  );

  // process format version
  W_AddLump(
    &demoex,
    DEMOEX_VERSION_LUMPNAME,
    DEMOEX_VERSION,
    strlen(DEMOEX_VERSION)
  );

  W_AddLump(
    &demoex,
    NULL,
    DEMOEX_SEPARATOR,
    strlen(DEMOEX_SEPARATOR)
  );

  // process mlook
  add_mouse_look_data(&demoex);

  W_AddLump(
    &demoex,
    NULL,
    DEMOEX_SEPARATOR,
    strlen(DEMOEX_SEPARATOR)
  );

  // process port name
  W_AddLump(
    &demoex,
    DEMOEX_PORTNAME_LUMPNAME,
    PACKAGE_NAME " " PACKAGE_VERSION,
    strlen(PACKAGE_NAME " " PACKAGE_VERSION)
  );

  W_AddLump(
    &demoex,
    NULL,
    DEMOEX_SEPARATOR,
    strlen(DEMOEX_SEPARATOR)
  );

  // process iwad, pwads, dehs and critical for demos params like -spechit, etc
  add_params(&demoex);

  W_AddLump(
    &demoex,
    NULL,
    DEMOEX_SEPARATOR,
    strlen(DEMOEX_SEPARATOR)
  );

  // write pwad header, all data and lookup table to the end of a demo
  if (!M_FileWrite(file, (char *)&demoex.header, 1, sizeof(demoex.header))) {
    D_MsgLocalError("G_WriteDemoFooter: Error writing header: %s\n",
      M_GetFileError()
    );
  }
  else if (!M_FileWrite(file, demoex.data, demoex.datasize, sizeof(char))) {
    D_MsgLocalError("G_WriteDemoFooter: Error writing size: %s\n",
      M_GetFileError()
    );
  }
  else if (!M_FileWrite(file, (char *)demoex.lumps, demoex.header.numlumps,
                                                    sizeof(demoex.lumps[0]))) {
    D_MsgLocalError("G_WriteDemoFooter: Error writing lumps: %s\n",
      M_GetFileError()
    );
  }
}

bool G_DemoCheckAutoDemo(void) {
  GPtrArray *resources = D_ResNew();
  bool result = false;

  if (!M_CheckParm("-auto")) {
    return false;
  }

  for (size_t i = 0; i < resource_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(resource_files, i);

    if (wf->src == source_lmp) {
      size_t numwadfiles_required;

      patterndata_t patterndata;
      memset(&patterndata, 0, sizeof(patterndata));

      numwadfiles_required = demo_name_to_wad_data(
        wf->name,
        resources,
        &patterndata
      );

      if (resources->len) {
        result = true;

        if (numwadfiles_required + 1 != resources->len && patterndata.missed) {
          D_MsgLocalWarn(
            "DataAutoload: pattern #%i is used\n"
            "%s not all required files are found, may not work\n",
            patterndata.pattern_num, patterndata.missed
          );
        }
        else {
          D_MsgLocalWarn("DataAutoload: pattern #%i is used\n",
            patterndata.pattern_num
          );
        }
        D_ResourcesSet(resources);
      }

      free(patterndata.missed);
      D_ResFree(resources);

      break;
    }
  }

  return result;
}

bool G_DemoCheckExDemo(void) {
  const char *demo_arg = NULL;
  char *filename = NULL;
  char *demoname = NULL;
  bool result = false;

  change_demo_extended_format();

  demo_arg = G_DemoGetDemoArg();

  if (!demo_arg) {
    return false;
  }

  filename = M_AddDefaultExtension(demo_arg, "lmp");
  demoname = I_FindFile(filename, NULL);

  free(filename);

  if (!demoname) {
    return false;
  }

  result = read_demo_footer(demoname);
  free(demoname);

  return result;
}

// e6y: Check for overrun
void G_DoPlayDemo(void) {
  if (G_DemoLoad(defdemoname, &demobuffer, &demolength, &demolumpnum)) {
    demo_p = G_DemoReadHeaderEx(demobuffer, demolength, RDH_SAFE);

    G_SetGameAction(ga_nothing);
    usergame = false;

    demoplayback = true;
    G_DemoSmoothPlayingReset(NULL); // e6y
  }
  else {
    // e6y
    // Do not exit if corresponding demo lump is not found.
    // It makes sense for Plutonia and TNT IWADs, which have no DEMO4 lump,
    // but DEMO4 should be in a demo cycle as real Plutonia and TNT have.
    //
    // Plutonia/Tnt executables exit with "W_GetNumForName: DEMO4 not found"
    // message after playing of DEMO3, because DEMO4 is not present
    // in the corresponding IWADs.
    usergame = false;
    D_StartTitle();               // Start the title screen
    G_SetGameState(GS_DEMOSCREEN);// And set the game state accordingly
  }
}

void G_DemoReadTiccmd(ticcmd_t *cmd) {
  demo_curr_tic++;

  if (*demo_p == DEMOMARKER) {
    G_CheckDemoStatus();      // end of demo data stream
  }
  else if (demoplayback && demo_p + bytes_per_tic > demobuffer + demolength) {
    D_MsgLocalWarn("G_DemoReadTiccmd: missing DEMOMARKER\n");
    G_CheckDemoStatus();
  }
  else {
    read_one_tick(cmd, &demo_p);
  }
}

void G_DemoReadContinueTiccmd(ticcmd_t* cmd) {
  if (!demo_continue_p) {
    return;
  }

  if (gametic <= demo_tics_count && 
    demo_continue_p + bytes_per_tic <= demobuffer + demolength &&
    *demo_continue_p != DEMOMARKER) {
    read_one_tick(cmd, &demo_continue_p);
  }

  if (gametic >= demo_tics_count ||
    demo_continue_p > demobuffer + demolength ||
    gamekeydown[key_demo_jointogame] || joybuttons[joybuse]) {
    demo_continue_p = NULL;
    democontinue = false;
  }
}

/*
 * Demo limits removed -- killough
 * cph - record straight to file
 */
void G_DemoWriteTiccmd(ticcmd_t *cmd) {
  char buf[5];
  char *p = buf;

  if (compatibility_level == tasdoom_compatibility) {
    *p++ = cmd->buttons;
    *p++ = cmd->forwardmove;
    *p++ = cmd->sidemove;
    *p++ = (cmd->angleturn + 128) >> 8;
  }
  else {
    *p++ = cmd->forwardmove;
    *p++ = cmd->sidemove;

    if (!longtics) {
      *p++ = (cmd->angleturn + 128) >> 8;
    }
    else {
      signed short a = cmd->angleturn;

      *p++ = a & 0xff;
      *p++ = (a >> 8) & 0xff;
    }
    *p++ = cmd->buttons;
  } //e6y

  if (fwrite(buf, p-buf, 1, demofp) != 1)
    I_Error("G_DemoWriteTiccmd: error writing demo");

  /* cph - alias demo_p to it so we can read it back */
  demo_p = buf;
  G_DemoReadTiccmd(cmd);         // make SURE it is exactly the same
}
/* vi: set et ts=2 sw=2: */

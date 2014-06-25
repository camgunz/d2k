/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 2001 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Transparent access to data in WADs using mmap
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include "doomstat.h"

#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif

#include "w_wad.h"
#include "z_zone.h"
#include "lprintf.h"
#include "i_system.h"
#include "m_file.h"

#include "e6y.h"//e6y

static struct {
  void *cache;
#ifdef TIMEDIAG
  int locktic;
#endif
  int locks;
} *cachelump;

#ifdef HEAPDUMP
void W_PrintLump(FILE *fp, void *p) {
  for (int i = 0; i < numlumps; i++) {
    if (cachelump[i].cache == p) {
      fprintf(fp, " %8.8s %6u %2d %6d",
        lumpinfo[i].name, W_LumpLength(i), cachelump[i].locks,
        gametic - cachelump[i].locktic
      );
      return;
    }
  }
  fprintf(fp, " not found");
}
#endif

#ifdef TIMEDIAG
static void W_ReportLocks(void) {
  lprintf(LO_DEBUG, "W_ReportLocks:\nLump     Size   Locks  Tics\n");
  if (cachelump) {
    for (int i = 0; i < numlumps; i++) {
      if (cachelump[i].locks > 0) {
        lprintf(LO_DEBUG, "%8.8s %6u %2d   %6d\n", lumpinfo[i].name,
        W_LumpLength(i), cachelump[i].locks, gametic - cachelump[i].locktic);
      }
    }
  }
}
#endif

#ifdef _WIN32

#include "wchar.h"

typedef struct {
  HANDLE hnd;
  OFSTRUCT fileinfo;
  HANDLE hnd_map;
  void   *data;
} mmap_info_t;

mmap_info_t *mapped_wad;

void W_DoneCache(void) {
  size_t wadfile_count = M_CBufGetObjectCount(&resource_files_buf);

  if (cachelump) {
    free(cachelump);
    cachelump = NULL;
  }

  if (!mapped_wad)
    return;

  for (size_t i = 0; i < wadfile_count; i++) {
    if (mapped_wad[i].data) {
      UnmapViewOfFile(mapped_wad[i].data);
      mapped_wad[i].data = NULL;
    }
    if (mapped_wad[i].hnd_map) {
      CloseHandle(mapped_wad[i].hnd_map);
      mapped_wad[i].hnd_map = NULL;
    }

    if (mapped_wad[i].hnd) {
      CloseHandle(mapped_wad[i].hnd);
      mapped_wad[i].hnd = NULL;
    }
  }

  free(mapped_wad);

  mapped_wad = NULL;
}

void W_InitCache(void) {
  size_t wadfile_count = M_CBufGetObjectCount(&resource_files_buf);

  // set up caching
  cachelump = calloc(numlumps, sizeof(*cachelump));
  if (!cachelump)
    I_Error("W_InitCache: Couldn't allocate lumpcache");

#ifdef TIMEDIAG
  atexit(W_ReportLocks);
#endif

  mapped_wad = calloc(wadfile_count, sizeof(mmap_info_t));
  memset(mapped_wad, 0, sizeof(mmap_info_t) * wadfile_count);

  for (int i = 0; i < numlumps; i++) {
    int wad_index = (int)(lumpinfo[i].wadfile);
    wadfile_info_t *wadfile;
    cachelump[i].locks = -1;

    if (lumpinfo[i].wadfile == -1)
      continue;

    wadfile = M_CBufGet(&resource_files_buf, wad_index);

#ifdef RANGECHECK
    if ((wad_index < 0) || ((size_t)wad_index >= wadfile_count))
      I_Error("W_InitCache: wad_index out of range");
#endif

    if (mapped_wad[wad_index].data)
      continue;

    lprintf(LO_INFO, "Mapping %s (%d/%d)\n",
      wadfile->name, wad_index, wadfile_count
    );

    wchar_t *local_path = (wchar_t *)M_LocalizePath(wadfile->name);

    if (!local_path) {
      I_Error(
        "W_InitCache: Error localizing path %s: %s",
        wadfile->name, M_GetFileError()
      );
    }

    mapped_wad[wad_index].hnd = CreateFile(
      local_path,
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL,
      OPEN_EXISTING,
      0,
      NULL
    );
    if (mapped_wad[wad_index].hnd == INVALID_HANDLE_VALUE) {
      I_Error(
        "W_InitCache: CreateFile for memory mapping %S failed (LastError %lu)",
        local_path, GetLastError()
      );
    }

    mapped_wad[wad_index].hnd_map = CreateFileMapping(
      mapped_wad[wad_index].hnd, NULL, PAGE_READONLY, 0, 0, NULL
    );
    if (mapped_wad[wad_index].hnd_map == NULL) {
      I_Error(
        "W_InitCache: CreateFileMapping for memory mapping %S failed "
        "(LastError %lu)",
        local_path, GetLastError()
      );
    }

    mapped_wad[wad_index].data = MapViewOfFile(
      mapped_wad[wad_index].hnd_map, FILE_MAP_READ, 0, 0, 0
    );
    if (mapped_wad[wad_index].data == NULL) {
      I_Error(
        "W_InitCache: MapViewOfFile for memory mapping %S failed "
        "(LastError %lu)",
        local_path, GetLastError()
      );
    }

    lprintf(LO_INFO, "W_InitCache: Mapped %s.\n", wadfile->name);

    free(local_path);
  }
}

const void* W_CacheLumpNum(int lump) {
  size_t wadfile_count = M_CBufGetObjectCount(&resource_files_buf);
  int wad_index = (int)(lumpinfo[lump].wadfile);

#ifdef RANGECHECK
  if ((wad_index < 0)||((size_t)wad_index >= wadfile_count))
    I_Error("W_CacheLumpNum: wad_index out of range");

  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error ("W_CacheLumpNum: %i >= numlumps", lump);
#endif

  if (lumpinfo[lump].wadfile == -1)
    return NULL;

  return (void *)(
    (unsigned char *)mapped_wad[wad_index].data + lumpinfo[lump].position
  );
}

#else

void **mapped_wad;

void W_InitCache(void) {
  int maxfd = 0;
  cachelump = calloc(numlumps, sizeof(*cachelump)); // set up caching

  lprintf(LO_INFO, "W_InitCache\n");

  if (!cachelump)
    I_Error("W_InitCache: Couldn't allocate lumpcache");

#ifdef TIMEDIAG
  atexit(W_ReportLocks);
#endif

  for (int i = 0; i < numlumps; i++) {
    wadfile_info_t *wf = NULL;

    if (lumpinfo[i].wadfile == -1)
      continue;

    wf = M_CBufGet(&resource_files_buf, lumpinfo[i].wadfile);

    if (wf == NULL) {
      I_Error(
        "W_InitCache: Lump %d has invalid wadfile index (%d)",
        i, lumpinfo[i].wadfile
      );
    }

    maxfd = MAX(maxfd, lumpinfo[i].wadfile);
  }

  if (maxfd <= 0)
    I_Error("W_InitCache: No WADs loaded");

  mapped_wad = calloc(maxfd + 1, sizeof(void *));

  for (int i = 0; i < numlumps; i++) {
    int fd;
    void *map;
    wadfile_info_t *wf = NULL;

    cachelump[i].locks = -1;

    if (lumpinfo[i].wadfile == -1)
      continue;

    wf = M_CBufGet(&resource_files_buf, lumpinfo[i].wadfile);

    if (wf == NULL) {
      I_Error(
        "W_InitCache: Lump %d has invalid wadfile index (%d)",
        i, lumpinfo[i].wadfile
      );
    }

    fd = wf->handle;

    if (mapped_wad[lumpinfo[i].wadfile])
      continue;

    map = mmap(NULL, M_FDLength(wf->handle), PROT_READ, MAP_SHARED, fd, 0);

    if (map == MAP_FAILED)
      I_Error("W_InitCache: failed to mmap [%s]", wf->name);

    mapped_wad[lumpinfo[i].wadfile] = map;
    lprintf(LO_INFO, " Mapped %s\n", wf->name);
  }
}

void W_DoneCache(void) {
  for (int i = 0; i < numlumps; i++) {
    int fd;
    wadfile_info_t *wf;

    if (lumpinfo[i].wadfile == -1)
      continue;

    wf = M_CBufGet(&resource_files_buf, lumpinfo[i].wadfile);

    if (wf == NULL)
      continue;

    fd = wf->handle;

    if (!mapped_wad[lumpinfo[i].wadfile])
      continue;

    if (munmap(mapped_wad[lumpinfo[i].wadfile], M_FDLength(fd))) 
      I_Error("W_DoneCache: failed to munmap");

    mapped_wad[lumpinfo[i].wadfile] = NULL;
  }

  free(mapped_wad);
  mapped_wad = NULL;
}

const void* W_CacheLumpNum(int lump) {
  lumpinfo_t *l;

#ifdef RANGECHECK
  if ((unsigned)lump >= (unsigned)numlumps)
    I_Error("W_CacheLumpNum: %i >= numlumps", lump);
#endif
  l = &lumpinfo[lump];

  if (l->wadfile == -1)
    return NULL;

  return (const void *)(
    ((const byte *)(mapped_wad[l->wadfile])) + l->position
  );
}
#endif

/*
 * W_LockLumpNum
 *
 * This copies the lump into a malloced memory region and returns its address
 * instead of returning a pointer into the memory mapped area
 *
 */
const void* W_LockLumpNum(int lump) {
  size_t len = W_LumpLength(lump);
  const void *data = W_CacheLumpNum(lump);

  if (!cachelump[lump].cache) { // read the lump in
    Z_Malloc(len, PU_CACHE, &cachelump[lump].cache);
    memcpy(cachelump[lump].cache, data, len);
  }

  /* cph - if wasn't locked but now is, tell z_zone to hold it */
  if (cachelump[lump].locks <= 0) {
    Z_ChangeTag(cachelump[lump].cache, PU_STATIC);
#ifdef TIMEDIAG
    cachelump[lump].locktic = gametic;
#endif
    cachelump[lump].locks = 1; // reset lock counter
  }
  else { // increment lock counter
    cachelump[lump].locks += 1;
  }

#ifdef SIMPLECHECKS
  if (!((cachelump[lump].locks + 1) & 0xf)) {
    lprintf(LO_DEBUG, "W_CacheLumpNum: High lock on %8s (%d)\n",
      lumpinfo[lump].name, cachelump[lump].locks
    );
  }
#endif

  return cachelump[lump].cache;
}

void W_UnlockLumpNum(int lump) {
  if (cachelump[lump].locks == -1)
    return; // this lump is memory mapped

#ifdef SIMPLECHECKS
  if (cachelump[lump].locks == 0) {
    lprintf(LO_DEBUG, "W_UnlockLumpNum: Excess unlocks on %8s\n",
      lumpinfo[lump].name
    );
  }
#endif
  cachelump[lump].locks -= 1;
  /*
   * cph - Note: must only tell z_zone to make purgeable if currently locked,
   * else it might already have been purged
   */
  if (cachelump[lump].locks == 0)
    Z_ChangeTag(cachelump[lump].cache, PU_CACHE);
}

/* vi: set et ts=2 sw=2: */


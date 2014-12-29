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

#ifdef __GNUG__
#pragma implementation "w_wad.h"
#endif

#include "doomstat.h"
#include "d_main.h"
#include "d_net.h"
#include "doomtype.h"
#include "e6y.h"
#include "i_system.h"
#include "lprintf.h"
#include "m_file.h"
#include "r_demo.h"
#include "r_main.h"
#include "w_wad.h"

#include "n_net.h"
#include "n_main.h"

//
// GLOBALS
//

// Location of each lump on disk.
lumpinfo_t *lumpinfo;
int        numlumps;         // killough

//
// LUMP BASED ROUTINES.
//

//
// W_AddFile
// All files are optional, but at least one file must be
//  found (PWAD, if all required lumps are present).
// Files with a .wad extension are wadlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
//
// Reload hack removed by Lee Killough
// CPhipps - source is an enum
//
// proff - changed using pointer to wadfile_info_t
// killough 1/31/98: static, const
static void W_AddFile(size_t wadfile_index) {
  wadfile_info_t *wadfile = g_ptr_array_index(resource_files, wadfile_index);
  wadinfo_t       header;
  lumpinfo_t     *lump_p;
  unsigned        i;
  int             length;
  int             startlump;
  filelump_t     *fileinfo, *fileinfo2free = NULL; //killough
  filelump_t      singleinfo;
  int             flags = 0;

  if (wadfile == NULL)
    I_Error("W_AddFile: Invalid wadfile index %zu\n", wadfile_index);

  // open the file and add to directory

  wadfile->handle = M_Open(wadfile->name, O_RDONLY | _O_BINARY, S_IRUSR);

  if (wadfile->handle == -1 && N_GetWad(wadfile->name)) // CPhipps
    wadfile->handle = M_Open(wadfile->name, O_RDONLY | _O_BINARY, S_IRUSR);

  if (wadfile->handle == -1 &&
      strlen(wadfile->name) > 4 &&
      wadfile->src == source_pwad &&
      !strcasecmp(wadfile->name + strlen(wadfile->name) - 4 , ".wad") &&
      N_GetWad(wadfile->name)) {
    wadfile->handle = M_Open(wadfile->name, O_RDONLY | _O_BINARY, S_IRUSR);
  }

  if (wadfile->handle == -1) {
    if (strlen(wadfile->name) <= 4 ||      // add error check -- killough
        (strcasecmp(wadfile->name + strlen(wadfile->name) - 4, ".lmp") &&
         strcasecmp(wadfile->name + strlen(wadfile->name) - 4, ".gwa"))) {
      I_Error("W_AddFile: couldn't open %s", wadfile->name);
    }
    return;
  }

  //jff 8/3/98 use logical output routine
  lprintf(LO_INFO, " adding %s\n", wadfile->name);
  startlump = numlumps;

  // mark lumps from internal resource
  if (wadfile->src == source_auto_load) {
    int len = strlen(PACKAGE_TARNAME ".wad");
    int len_file = strlen(wadfile->name);

    if (len_file >= len) {
      if (!strcasecmp(wadfile->name + len_file - len, PACKAGE_TARNAME ".wad")) {
        flags = LUMP_PRBOOM;
      }
    }
  }

  if (strlen(wadfile->name) <=4 ||
	    (strcasecmp(wadfile->name + strlen(wadfile->name) - 4,".wad") &&
	     strcasecmp(wadfile->name + strlen(wadfile->name) - 4,".gwa"))) {
      // single lump file
      fileinfo = &singleinfo;
      singleinfo.filepos = 0;
      singleinfo.size = LittleLong(M_FDLength(wadfile->handle));
      M_ExtractFileBase(wadfile->name, singleinfo.name);
      numlumps++;
  }
  else {
    // WAD file
    M_Read(wadfile->handle, &header, sizeof(header));
    if (strncmp(header.identification,"IWAD",4) &&
        strncmp(header.identification,"PWAD",4)) {
      I_Error("W_AddFile: Wad file %s doesn't have IWAD or PWAD id",
        wadfile->name
      );
    }
    header.numlumps = LittleLong(header.numlumps);
    header.infotableofs = LittleLong(header.infotableofs);
    length = header.numlumps * sizeof(filelump_t);
    fileinfo2free = fileinfo = malloc(length);    // killough
    M_Seek(wadfile->handle, header.infotableofs, SEEK_SET),
    M_Read(wadfile->handle, fileinfo, length);
    numlumps += header.numlumps;
  }

  // Fill in lumpinfo
  lumpinfo = realloc(lumpinfo, numlumps * sizeof(lumpinfo_t));

  lump_p = &lumpinfo[startlump];

  for (i = startlump; (int)i < numlumps; i++, lump_p++, fileinfo++) {
    lump_p->flags = flags;
    lump_p->wadfile = wadfile_index;              //  killough 4/25/98
    lump_p->position = LittleLong(fileinfo->filepos);
    lump_p->size = LittleLong(fileinfo->size);

    // Modifications to place command-line-added demo lumps
    // into a separate "ns_demos" namespace so that they cannot
    // conflict with other lump names
    if (wadfile->src == source_lmp)
      lump_p->li_namespace = ns_demos;
    else
      lump_p->li_namespace = ns_global;              // killough 4/17/98

    strncpy (lump_p->name, fileinfo->name, 8);
    lump_p->source = wadfile->src;                    // Ty 08/29/98
    // IWAD file used as recource PWAD must not override TEXTURE1 or PNAMES
    if (wadfile->src != source_iwad &&
        !strncmp(header.identification,"IWAD",4) &&
        (!strnicmp(fileinfo->name,"TEXTURE1", 8) ||
         !strnicmp(fileinfo->name,"PNAMES", 6))) {
      strncpy(lump_p->name, "-IGNORE-", 8);
    }
  }

  free(fileinfo2free);      // killough
}

// jff 1/23/98 Create routines to reorder the master directory
// putting all flats into one marked block, and all sprites into another.
// This will allow loading of sprites and flats from a PWAD with no
// other changes to code, particularly fast hashes of the lumps.
//
// killough 1/24/98 modified routines to be a little faster and smaller

static int IsMarker(const char *marker, const char *name) {
  // doubled first character test for single-character prefixes only
  // FF_* is valid alias for F_*, but HI_* should not allow HHI_*
  return !strncasecmp(name, marker, 8) ||
    (marker[1] == '_' && *name == *marker && !strncasecmp(name+1, marker, 7));
}

// killough 4/17/98: add namespace tags

static int W_CoalesceMarkedResource(const char *start_marker,
                                    const char *end_marker,
                                    li_namespace_e li_namespace) {
  int result = 0;
  lumpinfo_t *marked = malloc(sizeof(*marked) * numlumps);
  size_t i, num_marked = 0, num_unmarked = 0;
  int is_marked = 0, mark_end = 0;
  lumpinfo_t *lump = lumpinfo;

  for (i = numlumps; i--; lump++) {
    if (IsMarker(start_marker, lump->name)) {     // start marker found
      // If this is the first start marker, add start marker to marked lumps
      if (!num_marked) {
        strncpy(marked->name, start_marker, 8);
        marked->size = 0;  // killough 3/20/98: force size to be 0
        marked->li_namespace = ns_global;        // killough 4/17/98
        marked->wadfile = -1;
        num_marked = 1;
      }
      is_marked = 1;                            // start marking lumps
    }
    else if (IsMarker(end_marker, lump->name)) {     // end marker found
      mark_end = 1;                           // add end marker below
      is_marked = 0;                          // stop marking lumps
    }
    else if (is_marked || lump->li_namespace == li_namespace) {
      // if we are marking lumps,
      // move lump to marked list
      // sf: check for namespace already set

      // sf 26/10/99:
      // ignore sprite lumps smaller than 8 bytes (the smallest possible)
      // in size -- this was used by some dmadds wads
      // as an 'empty' graphics resource
      if(li_namespace != ns_sprites || lump->size > 8) {
        marked[num_marked] = *lump;
        marked[num_marked++].li_namespace = li_namespace;  // killough 4/17/98
        result++;
      }
    }
    else {
      // else move down THIS list
      memmove(lumpinfo + num_unmarked, lump, sizeof(lumpinfo_t));
      num_unmarked++;
    }
  }

  // Append marked list to end of unmarked list
  memmove(lumpinfo + num_unmarked, marked, num_marked * sizeof(*marked));

  free(marked);                                   // free marked list

  numlumps = num_unmarked + num_marked;           // new total number of lumps

  if (mark_end) {                                 // add end marker
    lumpinfo[numlumps].size = 0;  // killough 3/20/98: force size to be 0
    lumpinfo[numlumps].wadfile = -1;
    lumpinfo[numlumps].li_namespace = ns_global;   // killough 4/17/98
    strncpy(lumpinfo[numlumps++].name, end_marker, 8);
  }

  return result;
}

// Hash function used for lump names.
// Must be mod'ed with table size.
// Can be used for any 8-character names.
// by Lee Killough

unsigned W_LumpNameHash(const char *s)
{
  unsigned hash;
  (void) ((hash =        toupper(s[0]), s[1]) &&
          (hash = hash*3+toupper(s[1]), s[2]) &&
          (hash = hash*2+toupper(s[2]), s[3]) &&
          (hash = hash*2+toupper(s[3]), s[4]) &&
          (hash = hash*2+toupper(s[4]), s[5]) &&
          (hash = hash*2+toupper(s[5]), s[6]) &&
          (hash = hash*2+toupper(s[6]),
           hash = hash*2+toupper(s[7]))
         );
  return hash;
}

//
// W_CheckNumForName
// Returns -1 if name not found.
//
// Rewritten by Lee Killough to use hash table for performance. Significantly
// cuts down on time -- increases Doom performance over 300%. This is the
// single most important optimization of the original Doom sources, because
// lump name lookup is used so often, and the original Doom used a sequential
// search. For large wads with > 1000 lumps this meant an average of over
// 500 were probed during every search. Now the average is under 2 probes per
// search. There is no significant benefit to packing the names into longwords
// with this new hashing algorithm, because the work to do the packing is
// just as much work as simply doing the string comparisons with the new
// algorithm, which minimizes the expected number of comparisons to under 2.
//
// killough 4/17/98: add namespace parameter to prevent collisions
// between different resources such as flats, sprites, colormaps
//

// W_FindNumFromName, an iterative version of W_CheckNumForName
// returns list of lump numbers for a given name (latest first)
//
int (W_FindNumFromName)(const char *name, int li_namespace, int i) {
  // Hash function maps the name to one of possibly numlump chains.
  // It has been tuned so that the average chain length never exceeds 2.

  // proff 2001/09/07 - check numlumps==0, this happens when called before WAD loaded
  if (numlumps == 0) {
    i = -1;
  }
  else {
    if (i < 0)
      i = lumpinfo[W_LumpNameHash(name) % (unsigned) numlumps].index;
    else
      i = lumpinfo[i].next;

    // We search along the chain until end, looking for case-insensitive
    // matches which also match a namespace tag. Separate hash tables are
    // not used for each namespace, because the performance benefit is not
    // worth the overhead, considering namespace collisions are rare in
    // Doom wads.

    while (i >= 0 && (strncasecmp(lumpinfo[i].name, name, 8) ||
                      lumpinfo[i].li_namespace != li_namespace)) {
      i = lumpinfo[i].next;
    }
  }

  // Return the matching lump, or -1 if none found.

  return i;
}

//
// killough 1/31/98: Initialize lump hash table
//

void W_HashLumps(void) {
  int i;

  for (i = 0; i < numlumps; i++)
    lumpinfo[i].index = -1;                     // mark slots empty

  // Insert nodes to the beginning of each chain, in first-to-last
  // lump order, so that the last lump of a given name appears first
  // in any chain, observing pwad ordering rules. killough

  for (i=0; i<numlumps; i++) {
    // hash function:
    int j = W_LumpNameHash(lumpinfo[i].name) % (unsigned) numlumps;

    lumpinfo[i].next = lumpinfo[j].index;     // Prepend to list
    lumpinfo[j].index = i;
  }
}

// End of lump hashing -- killough 1/31/98



// W_GetNumForName
// Calls W_CheckNumForName, but bombs out if not found.
//
int W_GetNumForName(const char* name) {   // killough -- const added
  int i = W_CheckNumForName(name);

  if (i == -1)
    I_Error("W_GetNumForName: %.8s not found", name);

  return i;
}

// e6y
// W_SafeGetNumForName
// Calls W_CheckNumForName, and returns (-1) if any error happens
// Makes sense for doom.wad v1.2 for skip of some absent sounds
int W_SafeGetNumForName(const char *name) {
  int i = W_CheckNumForName (name);

  if (i == -1)
    lprintf(LO_DEBUG, "W_GetNumForName: %.8s not found\n", name);

  return i;
}

const lumpinfo_t* W_GetLumpInfoByNum(int lump) {
  if (lump < 0 || lump >= numlumps)
    I_Error("W_GetLumpInfoByNum: lump num %d out of range", lump);

  return &lumpinfo[lump];
}

// W_CheckNumForNameInternal
// checks only internal resource
//
int W_CheckNumForNameInternal(const char *name) {
  int p;

  for (p = -1; (p = W_ListNumFromName(name, p)) >= 0; ) {
    if (lumpinfo[p].flags == LUMP_PRBOOM)
      return p;
  }

  return -1;
}

// W_ListNumFromName
// calls W_FindNumFromName and returns the lumps in ascending order
//
int W_ListNumFromName(const char *name, int lump) {
  int i, next;

  for (i = -1; (next = W_FindNumFromName(name, i)) >= 0; i = next) {
    if (next == lump)
      break;
  }

  return i;
}

// W_Init
// Loads each of the files in the wadfiles array.
// All files are optional, but at least one file
//  must be found.
// Files with a .wad extension are idlink files
//  with multiple lumps.
// Other files are single lumps with the base filename
//  for the lump name.
// Lump names can appear multiple times.
// The name searcher looks backwards, so a later file
//  does override all earlier ones.
//
// CPhipps - modified to use the new wadfiles array
//

void W_Init(void) {
  // CPhipps - start with nothing

  numlumps = 0;
  lumpinfo = NULL;

  // CPhipps - new wadfiles array used
  // open all the files, load headers, and count lumps
  for (unsigned int i = 0; i < resource_files->len; i++)
    W_AddFile(i);

  if (!numlumps)
    I_Error("W_Init: No files found");

  //jff 1/23/98
  // get all the sprites and flats into one marked block each
  // killough 1/24/98: change interface to use M_START/M_END explicitly
  // killough 4/17/98: Add namespace tags to each entry
  // killough 4/4/98: add colormap markers
  W_CoalesceMarkedResource("S_START", "S_END", ns_sprites);
  W_CoalesceMarkedResource("F_START", "F_END", ns_flats);
  W_CoalesceMarkedResource("C_START", "C_END", ns_colormaps);
  W_CoalesceMarkedResource("B_START", "B_END", ns_prboom);
  r_have_internal_hires = (
    0 < W_CoalesceMarkedResource("HI_START", "HI_END", ns_hires)
  );

  // killough 1/31/98: initialize lump hash table
  W_HashLumps();

  /* cph 2001/07/07 - separated cache setup */
  W_InitCache();

  V_FreePlaypal();
}

void W_ReleaseAllWads(void) {
  W_DoneCache();
  D_ClearResourceFiles();

  numlumps = 0;
  free(lumpinfo);
  lumpinfo = NULL;

  V_FreePlaypal();
}

//
// W_LumpLength
// Returns the buffer size needed to load the given lump.
//
int W_LumpLength (int lump) {
  if (lump >= numlumps)
    I_Error("W_LumpLength: %i >= numlumps", lump);

  return lumpinfo[lump].size;
}

//
// W_ReadLump
// Loads the lump into the given buffer,
//  which must be >= W_LumpLength().
//

void W_ReadLump(int lump, void *dest) {
  wadfile_info_t *wadfile;

#ifdef RANGECHECK
  if (lump >= numlumps)
    I_Error ("W_ReadLump: %i >= numlumps", lump);
#endif

  lumpinfo_t *l = lumpinfo + lump;

  if (l->wadfile == -1)
    return;

  wadfile = g_ptr_array_index(resource_files, l->wadfile);

  if (wadfile == NULL)
    return;

  if (!M_Seek(wadfile->handle, l->position, SEEK_SET))
    I_Error("W_ReadLump: seek failed: %s.\n", M_GetFileError());

  if (!M_Read(wadfile->handle, dest, l->size))
    I_Error("W_ReadLump: read failed: %s\n", M_GetFileError());
}

/* vi: set et ts=2 sw=2: */


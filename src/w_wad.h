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


#ifndef W_WAD_H__
#define W_WAD_H__

#ifdef __GNUG__
#pragma interface
#endif

// e6y: lump flags
#define LUMP_STATIC 0x00000001 /* assigned gltexture should be static */
#define LUMP_CM2RGB 0x00000002 /* for fake colormap for hires patches */
#define LUMP_PRBOOM 0x00000004 /* from internal resource */

//
// TYPES
//

typedef enum {
  ns_global = 0,
  ns_sprites,
  ns_flats,
  ns_colormaps,
  ns_prboom,
  ns_demos,
  ns_hires //e6y
} li_namespace_e; // haleyjd 05/21/02: renamed from "namespace"

typedef struct {
  char identification[4];                  // Should be "IWAD" or "PWAD".
  int  numlumps;
  int  infotableofs;
} wadinfo_t;

typedef struct {
  int  filepos;
  int  size;
  char name[8];
} filelump_t;

// CPhipps - changed wad init
// We _must_ have the wadfiles[] the same as those actually loaded, so there 
// is no point having these separate entities. This belongs here.
typedef struct {
  char         *name;
  wad_source_e  src;
  int           handle;
} wadfile_info_t;

typedef struct wadtbl_s {
  wadinfo_t header;
  filelump_t *lumps;
  char* data;
  int datasize;
} wadtbl_t;

typedef struct {
  // WARNING: order of some fields important (see info.c).

  char name[9];
  int size;

  // killough 1/31/98: hash table fields, used for ultra-fast hash table lookup
  int index;
  int next;

  // killough 4/17/98: namespace tags, to prevent conflicts between resources
  li_namespace_e li_namespace; // haleyjd 05/21/02: renamed from "namespace"

  int wadfile;
  int position;
  wad_source_e source;
  int flags; //e6y
} lumpinfo_t;

extern GPtrArray *resource_files;
extern GPtrArray *deh_files;

extern lumpinfo_t *lumpinfo;
extern int         numlumps;

void W_Init(void); // CPhipps - uses the above array
void W_ReleaseAllWads(void); // Proff - Added for iwad switching
void W_InitCache(void);
void W_DoneCache(void);

int               (W_FindNumFromName)(const char *name, int ns, int lump);
int               W_CheckNumForNameInternal(const char *name);
int               W_ListNumFromName(const char *name, int lump);
int               W_GetNumForName (const char* name);
const lumpinfo_t* W_GetLumpInfoByNum(int lump);
int               W_SafeGetNumForName (const char* name); //e6y
int               W_LumpLength (int lump);
void              W_ReadLump (int lump, void *dest);

// CPhipps - modified for 'new' lump locking
const void* W_CacheLumpNum (int lump);
const void* W_LockLumpNum(int lump);
void        W_UnlockLumpNum(int lump);

unsigned W_LumpNameHash(const char *s);         // killough 1/31/98
void W_HashLumps(void);                         // cph 2001/07/07 - made public

void W_AddLump(wadtbl_t *wadtbl, const char *name, const unsigned char *data,
                                                   size_t size);

void W_InitPWADTable(wadtbl_t *wadtbl);
void W_FreePWADTable(wadtbl_t *wadtbl);

// killough 4/17/98: if W_CheckNumForName() called with only
// one argument, pass ns_global as the default namespace
#define W_FindNumFromName(name, lump) (W_FindNumFromName)(name, ns_global, lump)
#define W_CheckNumForName(name) (W_CheckNumForName)(name, ns_global)
// CPhipps - convenience macros
//#define W_CacheLumpNum(num) (W_CacheLumpNum)((num),1)
#define W_CacheLumpName(name) W_CacheLumpNum (W_GetNumForName(name))

//#define W_UnlockLumpNum(num) (W_UnlockLumpNum)((num),1)
#define W_UnlockLumpName(name) W_UnlockLumpNum (W_GetNumForName(name))

static inline int (W_CheckNumForName)(const char *name, int ns) {
  return (W_FindNumFromName)(name, ns, -1);
}

#endif

/* vi: set et ts=2 sw=2: */

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


#ifndef P_MAP_H__
#define P_MAP_H__

struct mapthing_s;
typedef struct mapthing_s mapthing_t;

struct mobj_s;
typedef struct mobj_s mobj_t;

struct player_s;
typedef struct player_s player_t;

#define NO_TOPTEXTURES        0x00000001
#define NO_BOTTOMTEXTURES     0x00000002
#define SECTOR_IS_CLOSED      0x00000004
#define NULL_SECTOR           0x00000008

#define USERANGE     (64 * FRACUNIT)
#define MELEERANGE   (64 * FRACUNIT)
#define MISSILERANGE (32 * 64 * FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes the spider demon
// is larger, but we do not have any moving sectors nearby

#define MAXRADIUS    (32 * FRACUNIT)

//
// Move clipping aid for LineDefs.
//
typedef enum {
  ST_HORIZONTAL,
  ST_VERTICAL,
  ST_POSITIVE,
  ST_NEGATIVE
} slopetype_e;

//
// Your plain vanilla vertex.
// Note: transformed values not buffered locally,
// like some DOOM-alikes ("wt", "WebView") do.
//
typedef struct vertex_s {
  fixed_t x;
  fixed_t y;
  angle_t viewangle;   // e6y: precalculated angle for clipping
  int     angletime;   // e6y: recalculation time for view angle 
} vertex_t;

// Each sector has a degenmobj_t in its center for sound origin purposes.
typedef struct {
  thinker_t thinker;  // not used for anything
  fixed_t   x;
  fixed_t   y;
  fixed_t   z;
  uint32_t  id;
} degenmobj_t;

//
// The SECTORS record, at runtime.
// Stores things/mobjs.
//

typedef struct sector_s {
  int iSectorID;         // proff 04/05/2000: needed for OpenGL and used in
                         //                   debugmode by the HUD to draw
                         //                   sectornum
  unsigned int flags;    // e6y: instead of .no_toptextures and
                         //      .no_bottomtextures
  fixed_t floorheight;
  fixed_t ceilingheight;
  int nexttag,firsttag;  // killough 1/30/98: improves searches for tags.
  int soundtraversed;    // 0 = untraversed, 1,2 = sndlines-1
  mobj_t *soundtarget;   // thing that made a sound (or null)
  int blockbox[4];       // mapblock bounding box for height changes
  int bbox[4];           // bounding box in map units
  degenmobj_t soundorg;  // origin for any sounds played by the sector
  int validcount;        // if == validcount, already checked
  mobj_t *thinglist;     // list of mobjs in sector

  /*
   * killough 8/28/98: friction is a sector property, not an mobj property.
   * these fields used to be in mobj_t, but presented performance problems
   * when processed as mobj properties. Fix is to make them sector properties.
   */
  int friction;
  int movefactor;

  // thinker_t for reversable actions
  void *floordata;    // jff 2/22/98 make thinkers on
  void *ceilingdata;  // floors, ceilings, lighting,
  void *lightingdata; // independent of one another

  // jff 2/26/98 lockout machinery for stairbuilding
  int stairlock;   // -2 on first locked -1 after thinker done 0 normally
  int prevsec;     // -1 or number of sector for previous step
  int nextsec;     // -1 or number of next step sector

  // killough 3/7/98: support flat heights drawn at another sector's heights
  int heightsec;    // other sector, or -1 if no other sector

  int bottommap, midmap, topmap; // killough 4/4/98: dynamic colormaps

  // list of mobjs that are at least partially in the sector
  // thinglist is a subset of touching_thinglist
  struct msecnode_s *touching_thinglist;               // phares 3/14/98

  int linecount;
  struct line_s **lines;

  // killough 10/98: support skies coming from sidedefs. Allows scrolling
  // skies and other effects. No "level info" kind of lump is needed,
  // because you can use an arbitrary number of skies per level with this
  // method. This field only applies when skyflatnum is used for floorpic
  // or ceilingpic, because the rest of Doom needs to know which is sky
  // and which isn't, etc.

  int sky;

  // killough 3/7/98: floor and ceiling texture offsets
  fixed_t floor_xoffs;
  fixed_t floor_yoffs;
  fixed_t ceiling_xoffs;
  fixed_t ceiling_yoffs;

  // killough 4/11/98: support for lightlevels coming from another sector
  int floorlightsec;
  int ceilinglightsec;

  short floorpic;
  short ceilingpic;
  short lightlevel;
  short special;
  short oldspecial;      //jff 2/16/98 remembers if sector WAS secret (automap)
  short tag;

  //e6y
  int INTERP_SectorFloor;
  int INTERP_SectorCeiling;
  int INTERP_FloorPanning;
  int INTERP_CeilingPanning;
#ifdef GL_DOOM
  int fakegroup[2];
#endif
} sector_t;

//
// The SideDef.
//

typedef struct side_s {
  fixed_t textureoffset; // add this to the calculated texture column
  fixed_t rowoffset;     // add this to the calculated texture top
  short toptexture;      // Texture indices. We do not maintain names here.
  short bottomtexture;
  short midtexture;
  sector_t* sector;      // Sector the SideDef is facing.

  // killough 4/4/98, 4/11/98: highest referencing special linedef's type,
  // or lump number of special effect. Allows texture names to be overloaded
  // for other functions.

  int special;
  int INTERP_WallPanning;
#ifdef GL_DOOM
  int skybox_index;
#endif
} side_t;

typedef struct line_s {
  int iLineID;           // proff 04/05/2000: needed for OpenGL
  vertex_t *v1, *v2;     // Vertices, from v1 to v2.
  fixed_t dx, dy;        // Precalculated v2 - v1 for side checking.
#ifdef GL_DOOM
  float texel_length;
#endif
  unsigned short flags;  // Animation related.
  short special;
  short tag;
  unsigned short sidenum[2]; // Visual appearance: SideDefs.
  fixed_t bbox[4];       // A bounding box, for the linedef's extent
  slopetype_e slopetype; // To aid move clipping.
  sector_t *frontsector; // Front and back sector.
  sector_t *backsector;
  int validcount;        // if == validcount, already checked
  void *specialdata;     // thinker_t for reversable actions
  int tranlump;          // killough 4/11/98: translucency filter, -1 == none
  int firsttag,nexttag;  // killough 4/17/98: improves searches for tags.
  int r_validcount;      // cph: if == gametic, r_flags already done
  enum {                 // cph:
    RF_TOP_TILE = 1,     // Upper texture needs tiling
    RF_MID_TILE = 2,     // Mid texture needs tiling
    RF_BOT_TILE = 4,     // Lower texture needs tiling
    RF_IGNORE   = 8,     // Renderer can skip this line
    RF_CLOSED   =16,     // Line blocks view
    RF_ISOLATED =32,     // Isolated line
  } r_flags;
  degenmobj_t soundorg;  // sound origin for switches/buttons
} line_t;

// phares 3/14/98
//
// Sector list node showing all sectors an object appears in.
//
// There are two threads that flow through these nodes. The first thread
// starts at touching_thinglist in a sector_t and flows through the m_snext
// links to find all mobjs that are entirely or partially in the sector.
// The second thread starts at touching_sectorlist in an mobj_t and flows
// through the m_tnext links to find all sectors a thing touches. This is
// useful when applying friction or push effects to sectors. These effects
// can be done as thinkers that act upon all objects touching their sectors.
// As an mobj moves through the world, these nodes are created and
// destroyed, with the links changed appropriately.
//
// For the links, NULL means top or end of list.

typedef struct msecnode_s {
  struct sector_s   *m_sector; // a sector containing this object
  struct mobj_s     *m_thing;  // this object
  struct msecnode_s *m_tprev;  // prev msecnode_t for this thing
  struct msecnode_s *m_tnext;  // next msecnode_t for this thing
  struct msecnode_s *m_sprev;  // prev msecnode_t for this sector
  struct msecnode_s *m_snext;  // next msecnode_t for this sector
  bool               visited;  // killough 4/4/98, 4/7/98: used in search
                               //                          algorithms
} msecnode_t;

//
// The LineSeg.
//
typedef struct seg_s {
  vertex_t *v1;
  vertex_t *v2;
  fixed_t   offset;
  angle_t   angle;
  side_t   *sidedef;
  line_t   *linedef;
  bool      miniseg;     // figgi -- needed for glnodes
  sector_t *frontsector; // Sector references.
  sector_t *backsector;  // Could be retrieved from linedef, too
                         // (but that would be slower -- killough)
                         // backsector is NULL for one sided lines
} seg_t;

typedef struct ssline_s {
  seg_t *seg;
  line_t *linedef;
  fixed_t x1, y1;
  fixed_t x2, y2;
  fixed_t bbox[4];
} ssline_t;

//
// A SubSector.
// References a Sector.
// Basically, this is a list of LineSegs,
//  indicating the visible walls that define
//  (all or some) sides of a convex BSP leaf.
//

typedef struct subsector_s {
  sector_t *sector;
  int numlines;  // e6y: support for extended nodes
  int firstline; // 'int' instead of 'short'
} subsector_t;


//
// BSP node.
//
typedef struct node_s {
  fixed_t x;  // Partition line
  fixed_t y;  // |
  fixed_t dx; // |
  fixed_t dy; // V
  fixed_t bbox[2][4]; // Bounding box for each child.
  //unsigned short children[2];    // If NF_SUBSECTOR its a subsector.
  int children[2];    // If NF_SUBSECTOR its a subsector.
} node_t;

/* for fast sight rejection -  cph - const* */
extern const unsigned char *rejectmatrix;

/* killough 3/1/98: change blockmap from "short" to "long" offsets: */
extern int     *blockmaplump; /* offsets in blockmap are from here */
extern int     *blockmap;
extern int      bmapwidth;
extern int      bmapheight;   /* in mapblocks */
extern fixed_t  bmaporgx;
extern fixed_t  bmaporgy;     /* origin of block map */

// MAES: extensions to support 512x512 blockmaps.
extern int blockmapxneg;
extern int blockmapyneg;

extern mapthing_t *deathmatchstarts;     // killough
extern size_t      num_deathmatchstarts; // killough
extern size_t      num_playerstarts;

extern mapthing_t *deathmatch_p;

// Player spawn spots.
extern mapthing_t *playerstarts;

extern int          numvertexes;
extern vertex_t    *vertexes;

extern int          numsegs;
extern seg_t       *segs;

extern int          numsectors;
extern sector_t    *sectors;

extern int          numsubsectors;
extern subsector_t *subsectors;

extern int          numnodes;
extern node_t      *nodes;

extern int          numlines;
extern line_t      *lines;

extern int          numsides;
extern side_t      *sides;

extern int         *sslines_indexes;
extern ssline_t    *sslines;

extern unsigned char *map_subsectors;

// If "floatok" true, move would be ok if within "tmfloorz - tmceilingz".
extern bool floatok;
extern bool felldown;   // killough 11/98: indicates object pushed off ledge
extern fixed_t tmfloorz;
extern fixed_t tmceilingz;
extern line_t *ceilingline;
extern line_t *floorline;      // killough 8/23/98
extern mobj_t *linetarget;     // who got hit (or NULL)
extern mobj_t *crosshair_target;
extern msecnode_t *sector_list;                             // phares 3/16/98
extern fixed_t tmbbox[4];         // phares 3/20/98
extern line_t *blockline;   // killough 8/11/98

extern int forceOldBsp;

// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.

extern int skyflatnum;

// killough 3/15/98: add fourth argument to P_TryMove
bool P_TryMove(mobj_t *thing, fixed_t x, fixed_t y, int dropoff);

// killough 8/9/98: extra argument for telefragging
bool P_TeleportMove(mobj_t *thing, fixed_t x, fixed_t y,bool boss);
bool P_StompSpawnPointBlockers(mobj_t *thing);
void P_SlideMove(mobj_t *mo);
bool P_CheckSight(mobj_t *t1, mobj_t *t2);
void P_UseLines(player_t *player);

typedef bool (*CrossSubsectorFunc)(int num);
extern CrossSubsectorFunc P_CrossSubsector;
bool P_CrossSubsector_Doom(int num);
bool P_CrossSubsector_Boom(int num);
bool P_CrossSubsector_PrBoom(int num);

// killough 8/2/98: add 'mask' argument to prevent friends autoaiming at others
fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance,
                                                   uint64_t mask);

void    P_LineAttack(mobj_t *t1, angle_t angle, fixed_t distance,
                                                fixed_t slope,
                                                int damage );
void    P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage);
bool    P_CheckPosition(mobj_t *thing, fixed_t x, fixed_t y);

//jff 3/19/98 P_CheckSector(): new routine to replace P_ChangeSector()
bool P_ChangeSector(sector_t *sector, bool crunch);
bool P_CheckSector(sector_t *sector, bool crunch);
void P_DelSeclist(msecnode_t *node);                     // phares 3/16/98
void P_FreeSecNodeList(void);                            // sf
void P_CreateSecNodeList(mobj_t *thing, fixed_t x, fixed_t y); // phares 3/14/98
bool Check_Sides(mobj_t *, int, int);                    // phares

int  P_GetMoveFactor(mobj_t *mo, int *friction);   // killough 8/28/98
int  P_GetFriction(const mobj_t *mo, int *factor); // killough 8/28/98
void P_ApplyTorque(mobj_t *mo);                    // killough 9/12/98

/* cphipps 2004/08/30 */
void P_MapStart(const char *caller);
void P_MapEnd(void);

void P_Init(void);

#endif

/* vi: set et ts=2 sw=2: */

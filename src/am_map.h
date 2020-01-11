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


#ifndef AMMAP_H__
#define AMMAP_H__

enum automapmode_e {
  am_active = 1,  // currently shown
  am_overlay= 2,  // covers the screen, i.e. not overlay mode
  am_rotate = 4,  // rotates to the player facing direction
  am_follow = 8,  // keep the player centred
  am_grid   =16,  // show grid
};
extern enum automapmode_e automapmode; // Mode that the automap is in

typedef struct map_point_s
{
  float x, y;
  unsigned char r, g, b, a;
} PACKEDATTR map_point_t;

typedef struct map_line_s
{
  map_point_t point[2];
} PACKEDATTR map_line_t;

extern GArray *map_lines;

#define MAPBITS 12
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// Used by ST StatusBar stuff.
#define AM_MSGHEADER (('a' << 24) + ('m' << 16))
#define AM_MSGENTERED (AM_MSGHEADER | ('e' << 8))
#define AM_MSGEXITED (AM_MSGHEADER | ('x' << 8))

// Called by main loop.
void AM_Ticker(void);

// Called by main loop,
// called instead of view drawer if automap active.
void AM_Drawer(void);

// Called to force the automap to quit
// if the level is completed while it is up.
void AM_Stop(void);

// killough 2/22/98: for saving automap information in savegame:

void AM_Start(void);

//jff 4/16/98 make externally available

void AM_clearMarks(void);

void AM_setMarkParams(int num);

void AM_SetResolution(void);

typedef struct
{
 fixed_t x,y;
 float fx,fy;
} mpoint_t;

typedef struct
{
 fixed_t x, y;
 fixed_t w, h;

 char label[16];
 int widths[16];
} markpoint_t;

extern markpoint_t *markpoints;
extern int markpointnum, markpointnum_max;

// end changes -- killough 2/22/98

// killough 5/2/98: moved from m_misc.c

//jff 1/7/98 automap colors added
extern int mapcolor_back;     // map background
extern int mapcolor_grid;     // grid lines color
extern int mapcolor_wall;     // normal 1s wall color
extern int mapcolor_fchg;     // line at floor height change color
extern int mapcolor_cchg;     // line at ceiling height change color
extern int mapcolor_clsd;     // line at sector with floor=ceiling color
extern int mapcolor_rkey;     // red key color
extern int mapcolor_bkey;     // blue key color
extern int mapcolor_ykey;     // yellow key color
extern int mapcolor_rdor;     // red door color (diff from keys to allow option)
extern int mapcolor_bdor;     // blue door color (of enabling one not other)
extern int mapcolor_ydor;     // yellow door color
extern int mapcolor_tele;     // teleporter line color
extern int mapcolor_secr;     // secret sector boundary color
//jff 4/23/98
extern int mapcolor_exit;     // exit line
extern int mapcolor_unsn;     // computer map unseen line color
extern int mapcolor_flat;     // line with no floor/ceiling changes
extern int mapcolor_sprt;     // general sprite color
extern int mapcolor_item;     // item sprite color
extern int mapcolor_enemy;    // enemy sprite color
extern int mapcolor_frnd;     // friendly sprite color
extern int mapcolor_hair;     // crosshair color
extern int mapcolor_sngl;     // single player arrow color

// colors for players in multiplayer
extern int vanilla_mapplayer_colors[VANILLA_MAXPLAYERS];

extern int mapcolor_me;       // consoleplayer's chosen colour
//jff 3/9/98
extern int map_secret_after;  // secrets do not appear til after bagged

extern int map_always_updates;
extern int map_grid_size;
extern int map_scroll_speed;
extern int map_wheel_zoom;
extern int map_use_multisamling;

extern int map_textured;
extern int map_textured_trans;
extern int map_textured_overlay_trans;
extern int map_lines_overlay_trans;
extern int map_overlay_pos_x;
extern int map_overlay_pos_y;
extern int map_overlay_pos_width;
extern int map_overlay_pos_height;
extern int map_type;
void M_ChangeMapTextured(void);
void M_ChangeMapMultisamling(void);

typedef struct am_frame_s
{
  fixed_t centerx, centery;
  fixed_t sin, cos;

  float centerx_f, centery_f;
  float sin_f, cos_f;

  fixed_t bbox[4];

  int precise;
} am_frame_t;
extern am_frame_t am_frame;

typedef enum
{
  map_things_appearance_classic,
  map_things_appearance_scaled,
#if defined(HAVE_LIBSDL_IMAGE) && defined(GL_DOOM)
  map_things_appearance_icon,
#endif
  
  map_things_appearance_max
} map_things_appearance_t;
extern int map_things_appearance;
extern const char *map_things_appearance_list[];

#endif

/* vi: set et ts=2 sw=2: */


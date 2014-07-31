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


#ifndef HU_STUFF_H__
#define HU_STUFF_H__

#include "d_event.h"

/*
 * Globally visible constants.
 */

/*
#define HU_FONT "boombox2 8"
#define HU_FONT "snap 8"
#define HU_FONT "Monkirta Pursuit NC 12"
*/

#define HU_FONT         "Monkirta Pursuit NC 12"
#define HU_FONT_FILE    "Monkirta Pursuit NC.ttf"
#define HU_UNIFONT      "unifont 12"
#define HU_UNIFONT_FILE "unifont-7.0.03.ttf"

#define HU_FONTSTART    '!'     /* the first font characters */
#define HU_FONTEND      (0x7f) /*jff 2/16/98 '_' the last font characters */

/* Calculate # of glyphs in font. */
#define HU_FONTSIZE     (HU_FONTEND - HU_FONTSTART + 1)

#define HU_BROADCAST    5

#define HU_MSGX         0
#define HU_MSGY         0
#define HU_MSGWIDTH     64      /* in characters */
#define HU_MSGHEIGHT    1       /* in lines */

#define HU_MSGTIMEOUT   (4*TICRATE)

#define HU_CROSSHAIRS	4
extern const char *crosshair_nam[HU_CROSSHAIRS];
extern const char *crosshair_str[HU_CROSSHAIRS];

extern dboolean chat_on;

/*
 * Heads up text
 */
void HU_Init(void);
void HU_LoadHUDDefs(void);
void HU_Start(void);

bool HU_ChatActive(void);
void HU_DeactivateChat(void);

dboolean HU_Responder(event_t* ev);

void  HU_Ticker(void);
void  HU_Drawer(void);
char  HU_dequeueChatChar(void);
void  HU_Erase(void);
void  HU_MoveHud(int force); // jff 3/9/98 avoid glitch in HUD display
void  HU_NextHud(void);
void* HU_GetRenderContext(void);

/* killough 5/2/98: moved from m_misc.c: */

/* jff 2/16/98 hud supported automap colors added */
extern int hudcolor_titl;   /* color range of automap level title   */
extern int hudcolor_xyco;   /* color range of new coords on automap */
extern int hudcolor_mapstat_title;
extern int hudcolor_mapstat_value;
extern int hudcolor_mapstat_time;
/* jff 2/16/98 hud text colors, controls added */
extern int hudcolor_mesg;   /* color range of scrolling messages    */
extern int hudcolor_chat;   /* color range of chat lines            */
/* jff 2/26/98 hud message list color and background enable */
extern int hudcolor_list;   /* color of list of past messages                  */
extern int hud_list_bgon;   /* solid window background for list of messages    */
extern int hud_msg_lines;   /* number of message lines in window up to 16      */
/* jff 2/23/98 hud is currently displayed */
extern int hud_displayed;   /* hud is displayed */
/* jff 2/18/98 hud/status control */
extern int hud_num;
extern int huds_count;

typedef struct custom_message_s
{
  int ticks;
  int cm;
  int sfx;
  const char *msg;
} custom_message_t;

typedef struct message_thinker_s
{
  thinker_t thinker;
  int plr;
  int delay;
  custom_message_t msg;
} message_thinker_t;

typedef struct crosshair_s
{
  int lump;
  int w, h, flags;
  int target_x, target_y, target_z, target_sprite;
  float target_screen_x, target_screen_y;
} crosshair_t;
extern crosshair_t crosshair;

int SetCustomMessage(int plr, const char *msg, int delay, int ticks, int cm, int sfx);

#endif

/* vi: set et ts=2 sw=2: */


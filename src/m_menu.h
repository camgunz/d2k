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


#ifndef M_MENU_H__
#define M_MENU_H__

#include "d_event.h"

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.

typedef struct menuitem_s {
  short status; // 0 = no cursor here, 1 = ok, 2 = arrows ok
  char  name[10];

  // choice = menu item #.
  // if status = 2,
  //   choice=0:leftarrow,1:rightarrow
  void  (*routine)(int choice);
  char  alphaKey; // hotkey in menu
  const char *alttext;
} menuitem_t;

typedef struct menu_s {
  short           numitems;     // # of menu items
  struct menu_s*  prevMenu;     // previous menu
  menuitem_t*     menuitems;    // menu items
  void            (*routine)(); // draw routine
  short           x;
  short           y;            // x,y of menu
  short           lastOn;       // last item user was on in menu
} menu_t;

enum {
  sfx_vol,
  sfx_empty1,
  music_vol,
  sfx_empty2,
  sound_end
} sound_e;

extern short itemOn;
extern menu_t HelpDef;
extern menu_t SoundDef;
extern menu_t SetupDef;
extern int screenSize;

//jff 4/18/98
extern bool inhelpscreens;
extern bool BorderNeedRefresh;

void M_SaveGame(int choice);
void M_LoadGame(int choice);
void M_QuickSave(void);
void M_EndGame(int choice);
void M_ChangeMessages(int choice);
void M_QuickLoad(void);
void M_QuitDOOM(int choice);
void M_SizeDisplay(int choice);
void M_SetupNextMenu(menu_t *menudef);

void M_SetCurrentMenu(menu_t *new_menu);

bool M_Responder (event_t *ev);

// Called by main loop,
// only used for menu (skull cursor) animation.

void M_Ticker (void);

// Called by main loop,
// draws the menus directly into the screen buffer.

void M_Drawer (void);

// Called by D_DoomMain,
// loads the config file.

void M_Init (void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.

void M_StartControlPanel (void);

void M_ForcedLoadGame(const char *msg); // killough 5/15/98: forced loadgames

void M_Trans(void);          // killough 11/98: reset translucency

void M_ResetMenu(void);      // killough 11/98: reset main menu ordering

void M_DrawCredits(void);    // killough 11/98

/* killough 8/15/98: warn about changes not being committed until next game */
#define warn_about_changes(x) (warning_about_changes=(x), \
             print_warning_about_changes = 2)

extern int warning_about_changes, print_warning_about_changes;

extern bool menu_background;

/****************************
 *
 *  The following #defines are for the m_flags field of each item on every
 *  Setup Screen. They can be OR'ed together where appropriate
 */

#define S_HILITE     0x1 // Cursor is sitting on this item
#define S_SELECT     0x2 // We're changing this item
#define S_TITLE      0x4 // Title item
#define S_YESNO      0x8 // Yes or No item
#define S_CRITEM    0x10 // Message color
#define S_COLOR     0x20 // Automap color
#define S_CHAT      0x40 // Chat String
#define S_RESET     0x80 // Reset to Defaults Button
#define S_PREV     0x100 // Previous menu exists
#define S_NEXT     0x200 // Next menu exists
#define S_KEY      0x400 // Key Binding
#define S_WEAP     0x800 // Weapon #
#define S_NUM     0x1000 // Numerical item
#define S_SKIP    0x2000 // Cursor can't land here
#define S_KEEP    0x4000 // Don't swap key out
#define S_END     0x8000 // Last item in list (dummy)
#define S_LEVWARN 0x10000// killough 8/30/98: Always warn about pending change
#define S_PRGWARN 0x20000// killough 10/98: Warn about change until next run
#define S_BADVAL  0x40000// killough 10/98: Warn about bad value
#define S_FILE    0x80000// killough 10/98: Filenames
#define S_LEFTJUST 0x100000 // killough 10/98: items which are left-justified
#define S_CREDIT  0x200000  // killough 10/98: credit
#define S_BADVID  0x400000  // killough 12/98: video mode change error
#define S_CHOICE  0x800000  // this item has several values

//e6y
#define S_DISABLE  0x1000000

/* S_SHOWDESC  = the set of items whose description should be displayed
 * S_SHOWSET   = the set of items whose setting should be displayed
 * S_STRING    = the set of items whose settings are strings -- killough 10/98:
 * S_HASDEFPTR = the set of items whose var field points to default array
 */

#define S_SHOWDESC (S_TITLE|S_YESNO|S_CRITEM|S_COLOR|S_CHAT|S_RESET|S_PREV|S_NEXT|S_KEY|S_WEAP|S_NUM|S_FILE|S_CREDIT|S_CHOICE)

#define S_SHOWSET  (S_YESNO|S_CRITEM|S_COLOR|S_CHAT|S_KEY|S_WEAP|S_NUM|S_FILE|S_CHOICE)

#define S_STRING (S_CHAT|S_FILE)

#define S_HASDEFPTR (S_STRING|S_YESNO|S_NUM|S_WEAP|S_COLOR|S_CRITEM|S_CHOICE)

/****************************
 *
 * The setup_group enum is used to show which 'groups' keys fall into so
 * that you can bind a key differently in each 'group'.
 */

typedef enum {
  m_null,       // Has no meaning; not applicable
  m_scrn,       // A key can not be assigned to more than one action
  m_map,        // in the same group. A key can be assigned to one
  m_menu,       // action in one group, and another action in another.
} setup_group;

/****************************
 *
 * phares 4/17/98:
 * State definition for each item.
 * This is the definition of the structure for each setup item. Not all
 * fields are used by all items.
 *
 * A setup screen is defined by an array of these items specific to
 * that screen.
 *
 * killough 11/98:
 *
 * Restructured to allow simpler table entries,
 * and to Xref with defaults[] array in m_misc.c.
 * Moved from m_menu.c to m_menu.h so that m_misc.c can use it.
 */

typedef struct setup_menu_s
{
  const char  *m_text;  /* text to display */
  int         m_flags;  /* phares 4/17/98: flag bits S_* (defined above) */
  setup_group m_group;  /* Group */
  short       m_x;      /* screen x position (left is 0) */
  short       m_y;      /* screen y position (top is 0) */

  union  /* killough 11/98: The first field is a union of several types */
  {
    const void          *var;   /* generic variable */
    int                 *m_key; /* key value, or 0 if not shown */
    const char          *name;  /* name */
    struct default_s    *def;   /* default[] table entry */
    struct setup_menu_s *menu;  /* next or prev menu */
  } var;

  int         *m_mouse; /* mouse button value, or 0 if not shown */
  int         *m_joy;   /* joystick button value, or 0 if not shown */
  void (*action)(void); /* killough 10/98: function to call after changing */
  const char **selectstrings; /* list of strings for choice value */
} setup_menu_t;

#endif

/* vi: set et ts=2 sw=2: */


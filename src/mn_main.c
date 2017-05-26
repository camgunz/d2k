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

#include "dstrings.h"
#include "d_event.h"
#include "am_map.h"
#include "d_deh.h"
#include "d_main.h"
#include "d_mouse.h"
#include "e6y.h"//e6y
#ifdef _WIN32
#include "e6y_launcher.h"
#endif
#include "p_setup.h"
#include "pl_main.h"
#include "pl_msg.h"
#include "g_game.h"
#include "g_keys.h"
#include "w_wad.h"
#include "r_demo.h"
#include "r_fps.h"
#include "r_defs.h"
#include "r_patch.h"
#include "r_data.h"
#include "r_main.h"
#include "i_input.h"
#include "i_main.h"
#include "sounds.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_video.h"
#include "m_misc.h"
#include "n_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "st_stuff.h"
#include "v_video.h"
#include "m_menu.h"
#include "xd_main.h"

#include "hu_lib.h"
#include "hu_stuff.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#define SAVESTRINGSIZE  24
#define SKULLXOFF  -32
#define LINEHEIGHT  16
#define LOADGRAPHIC_Y 8

// I'm using a scale of 100 since I don't know what's normal -- killough.
#define MOUSE_SENS_MAX 100


// Establish the message colors to be used
#define CR_TITLE  CR_GOLD
#define CR_SET    CR_GREEN
#define CR_ITEM   CR_RED
#define CR_HILITE CR_ORANGE
#define CR_SELECT CR_GRAY

//e6y
#define CR_DISABLE CR_GRAY

// Data used by the Automap color selection code
#define CHIP_SIZE 7 // size of color block for colored items

#define COLORPALXORIG ((320 - 16*(CHIP_SIZE+1))/2)
#define COLORPALYORIG ((200 - 16*(CHIP_SIZE+1))/2)

#define PAL_BLACK   0
#define PAL_WHITE   4

// Data used by the Chat String editing code
#define CHAT_STRING_BFR_SIZE 128

// chat strings must fit in this screen space
// killough 10/98: reduced, for more general uses
#define MAXCHATWIDTH         272

#define MAXGATHER 5
#define VERIFYBOXXORG 66
#define VERIFYBOXYORG 88

#define KB_X  160
#define KB_PREV  57
#define KB_NEXT 310
#define KB_Y   31

// phares 4/16/98:
// X,Y position of reset button. This is the same for every screen, and is
// only defined once here.
#define X_BUTTON 301
#define Y_BUTTON   3

#define WP_X 203
#define WP_Y  33

#define SB_X 203
#define SB_Y  31

//e6y
#define ADVHUD_X 284
#define AU_X    275
#define AU_Y     31
#define AU_PREV KB_PREV
#define AU_NEXT KB_NEXT

#define E_X 250
#define E_Y  31

#define G_X 226
#define GF_X 76
#define G_Y 23
#define G_X2 284

#define C_X  284
#define C_Y  32
#define COMP_SPC 12
#define C_NEXTPREV 131

#define M_X 230
#define M_Y  39

#define CS_X 20
#define CS_Y (31+8)

#define KT_X1 283
#define KT_X2 172
#define KT_X3  87

#define KT_Y1   2
#define KT_Y2 118
#define KT_Y3 102

#define SPACEWIDTH 4

#define CR_S 9
#define CR_X 20
#define CR_X2 50
#define CR_Y 32
#define CR_SH 9

enum {
  newgame = 0,
  loadgame,
  savegame,
  options,
  readthis,
  quitdoom,
  main_end
} main_e;

enum
{
  rdthsempty1,
  read1_end
} read_e;

enum
{
  rdthsempty2,
  read2_end
} read_e2;

enum               // killough 10/98
{
  helpempty,
  help_end
} help_e;


enum
{
  ep1,
  ep2,
  ep3,
  ep4,
  ep_end
} episodes_e;

enum
{
  killthings,
  toorough,
  hurtme,
  violence,
  nightmare,
  newg_end
} newgame_e;

enum
{
  load1,
  load2,
  load3,
  load4,
  load5,
  load6,
  load7, //jff 3/15/98 extend number of slots
  load8,
  load_end
} load_e;

enum
{
  general, // killough 10/98
  // killough 4/6/98: move setup to be a sub-menu of OPTIONs
  setup,                                                    // phares 3/21/98
  endgame,
  messages,
  /*    detail, obsolete -- killough */
  scrnsize,
  option_empty1,
  mousesens,
  /* option_empty2, submenu now -- killough */
  soundvol,
  opt_end
} options_e;

enum
{
  mouse_horiz,
  mouse_empty1,
  mouse_vert,
  mouse_empty2,

//e6y
  mouse_mlook,
  mouse_empty3,
  mouse_accel,
  mouse_empty4,

  mouse_end
} mouse_e;

// The setup_e enum is used to provide a unique number for each group of Setup

enum
{
  set_compat,
  set_key_bindings,
  set_weapons,
  set_statbar,
  set_automap,
  set_enemy,
  set_messages,
  set_chatstrings,
  set_setup_end
} setup_e;


// the generic_setup_e enum mimics the 'Big Font' menu structures, but
// means nothing to the Setup Menus.

enum
{
  generic_setupempty1,
  generic_setup_end
} generic_setup_e;

enum {           // killough 10/98: enum for y-offset info
  weap_recoil,
  weap_bobbing,
  weap_bfg,
  weap_stub1,
  weap_pref1,
  weap_pref2,
  weap_pref3,
  weap_pref4,
  weap_pref5,
  weap_pref6,
  weap_pref7,
  weap_pref8,
  weap_pref9,
  weap_stub2,
  weap_toggle,
  weap_toggle2,
};

enum {
  enem_infighting,

  enem_remember = 1,

  enem_backing,
  enem_monkeys,
  enem_avoid_hazards,
  enem_friction,
  enem_help_friends,

#ifdef DOGS
  enem_helpers,
#endif

  enem_distfriend,

#ifdef DOGS
  enem_dog_jumping,
#endif

  enem_end
};

enum
{
  compat_telefrag,
  compat_dropoff,
  compat_falloff,
  compat_staylift,
  compat_doorstuck,
  compat_pursuit,
  compat_vile,
  compat_pain,
  compat_skull,
  compat_blazing,
  compat_doorlight = 0,
  compat_god,
  compat_infcheat,
  compat_zombie,
  compat_skymap,
  compat_stairs,
  compat_floors,
  compat_moveblock,
  compat_model,
  compat_zerotags,
  compat_666 = 0,
  compat_soul,
  compat_maskedanim,
  compat_sound,
  //e6y
  compat_plussettings,
  compat_ouchface,
  compat_maxhealth,
  compat_translucency,
};

// killough 11/98: enumerated

enum {
  mess_color_play,
  mess_timer,
  mess_color_chat,
  mess_chat_timer,
  mess_color_review,
  mess_timed,
  mess_hud_timer,
  mess_lines,
  mess_scrollup,
  mess_background,
};

enum {
  prog,
  prog_stub,
  prog_stub1,
  prog_stub2,
  adcr
};

enum {
  cr_prog=0,
  cr_adcr=2,
};

extern bool       message_dontfuckwithme;
extern patchnum_t hu_font[HU_FONTSIZE];

//
// defaulted values
//

int showMessages;    // Show messages has default, 0 = off, 1 = on

int hide_setup=1; // killough 5/15/98

// Blocky mode, has default, 0 = high, 1 = normal
//int     detailLevel;    obsolete -- killough
int screenblocks;    // has default

int screenSize;      // temp for screenblocks (0-9)

int quickSaveSlot;   // -1 = no quicksave slot picked!

int messageToPrint;  // 1 = message to be printed

// CPhipps - static const
static const char* messageString; // ...and here is the message string!

// message x & y
int     messx;
int     messy;
int     messageLastMenuActive;

bool messageNeedsInput; // timed message = no input from user

void (*messageRoutine)(int response);

/* killough 8/15/98: when changes are allowed to sync-critical variables */
static int allow_changes(void)
{
 return !(demoplayback || demorecording || netgame);
}

static void MN_UpdateCurrent(default_t* def)
{
  /* cph - requires rewrite of m_misc.c */
  if (def->current) {
    if (allow_changes())  /* killough 8/15/98 */
  *def->current = *def->location.pi;
    else if (*def->current != *def->location.pi)
  warn_about_changes(S_LEVWARN); /* killough 8/15/98 */
  }
}

/* cphipps - MN_DrawBackground renamed and moved to v_video.c */

static void MN_DrawBackground(const char *flat, int scrn)
{
  if (menu_background)
    V_DrawBackground(flat, scrn);
}

// we are going to be entering a savegame string

int warning_about_changes;
int print_warning_about_changes;
int menu_background = 1; // do Boom fullscreen menus have backgrounds?

bool saveStringEnter;
int saveSlot;        // which slot to save in
int saveCharIndex;   // which char we're editing
// old save description before edit
char saveOldString[SAVESTRINGSIZE];

bool inhelpscreens; // indicates we are in or just left a help screen

bool BorderNeedRefresh;

enum menuactive_e menuactive;    // The menus are up

char savegamestrings[10][SAVESTRINGSIZE];

//
// MENU TYPEDEFS
//

short itemOn;           // menu item skull is on (for Big Font menus)
short skullAnimCounter; // skull animation counter
short whichSkull;       // which skull to draw (he blinks)

// graphic name of skulls

const char skullName[2][/*8*/9] = {"M_SKULL1","M_SKULL2"};

menu_t* currentMenu; // current menudef

// phares 3/30/98
// externs added for setup menus

int mapcolor_me;    // cph

extern int map_point_coordinates; // killough 10/98
extern int map_level_stat;

extern char* chat_macros[];  // chat macros
extern const char* shiftxform;
extern default_t defaults[];
extern int numdefaults;

// end of externs added for setup menus

//
// PROTOTYPES
//
void MN_NewGame(int choice);
void MN_Episode(int choice);
void MN_ChooseSkill(int choice);
void MN_LoadGame(int choice);
void MN_SaveGame(int choice);
void MN_Options(int choice);
void MN_EndGame(int choice);
void MN_ReadThis(int choice);
void MN_ReadThis2(int choice);
void MN_QuitDOOM(int choice);

void MN_ChangeMessages(int choice);
void MN_ChangeSensitivity(int choice);
void MN_SfxVol(int choice);
void MN_MusicVol(int choice);
/* void MN_ChangeDetail(int choice);  unused -- killough */
void MN_SizeDisplay(int choice);
void MN_StartGame(int choice);
void MN_Sound(int choice);

void MN_DrawMouse(void);

void MN_FinishReadThis(int choice);
void MN_FinishHelp(int choice);            // killough 10/98
void MN_LoadSelect(int choice);
void MN_SaveSelect(int choice);
void MN_ReadSaveStrings(void);
void MN_QuickSave(void);
void MN_QuickLoad(void);

void MN_DrawMainMenu(void);
void MN_DrawReadThis1(void);
void MN_DrawReadThis2(void);
void MN_DrawNewGame(void);
void MN_DrawEpisode(void);
void MN_DrawOptions(void);
void MN_DrawSound(void);
void MN_DrawLoad(void);
void MN_DrawSave(void);
void MN_DrawSetup(void);                                     // phares 3/21/98
void MN_DrawHelp (void);                                     // phares 5/04/98

void MN_DrawSaveLoadBorder(int x,int y);
void MN_SetupNextMenu(menu_t *menudef);
void MN_DrawThermo(int x,int y,int thermWidth,int thermDot);
void MN_DrawEmptyCell(menu_t *menu,int item);
void MN_DrawSelCell(menu_t *menu,int item);
void MN_WriteText(int x, int y, const char *string, int cm);
int  MN_StringWidth(const char *string);
int  MN_StringHeight(const char *string);
void MN_DrawTitle(int x, int y, const char *patch, int cm,
                 const char *alttext, int altcm);
void MN_StartMessage(const char *string, void *routine, bool input);
void MN_StopMessage(void);
void MN_ClearMenus (void);

// phares 3/30/98
// prototypes added to support Setup Menus and Extended HELP screens

int  MN_GetKeyString(int, int);
void MN_Setup(int choice);
void MN_KeyBindings(int choice);
void MN_Weapons(int);
void MN_StatusBar(int);
void MN_Automap(int);
void MN_Enemy(int);
void MN_Messages(int);
void MN_ChatStrings(int);
void MN_InitExtendedHelp(void);
void MN_ExtHelpNextScreen(int);
void MN_ExtHelp(int);
static int MN_GetPixelWidth(const char*);
void MN_DrawKeybnd(void);
void MN_DrawWeapons(void);
static void MN_DrawMenuString(int,int,int);
static void MN_DrawStringCentered(int,int,int,const char*);
void MN_DrawStatusHUD(void);
void MN_DrawExtHelp(void);
void MN_DrawAutoMap(void);
void MN_DrawEnemy(void);
void MN_DrawMessages(void);
void MN_DrawChatStrings(void);
void MN_Compat(int);       // killough 10/98
void MN_ChangeDemoSmoothTurns(void);
void MN_ChangeTextureParams(void);
void MN_General(int);      // killough 10/98
void MN_DrawCompat(void);  // killough 10/98
void MN_DrawGeneral(void); // killough 10/98
void MN_ChangeFullScreen(void);
void MN_ChangeVideoMode(void);
void MN_ChangeUseGLSurface(void);
void MN_ChangeApplyPalette(void);

menu_t NewDef;                                              // phares 5/04/98

// end of prototypes added to support Setup Menus and Extended HELP screens

/////////////////////////////////////////////////////////////////////////////
//
// DOOM MENUS
//

/////////////////////////////
//
// MAIN MENU
//

// main_e provides numerical values for which Big Font screen you're on

//
// MainMenu is the definition of what the main menu Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_NGAME") that should be displayed,
// the program which takes over when an item is selected, and the hotkey
// associated with the item.
//

menuitem_t MainMenu[]=
{
  {1,"M_NGAME", MN_NewGame, 'n'},
  {1,"M_OPTION",MN_Options, 'o'},
  {1,"M_LOADG", MN_LoadGame,'l'},
  {1,"M_SAVEG", MN_SaveGame,'s'},
  // Another hickup with Special edition.
  {1,"M_RDTHIS",MN_ReadThis,'r'},
  {1,"M_QUITG", MN_QuitDOOM,'q'}
};

menu_t MainDef =
{
  main_end,        // number of menu items
  NULL,            // previous menu screen
  MainMenu,        // table that defines menu items
  MN_DrawMainMenu, // drawing routine
  97,64,           // initial cursor position
  0                // last menu item the user was on
};

//
// MN_DrawMainMenu
//

void MN_DrawMainMenu(void)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(94, 2, 0, "M_DOOM", CR_DEFAULT, VPT_STRETCH);
}

/////////////////////////////
//
// Read This! MENU 1 & 2
//

// There are no menu items on the Read This! screens, so read_e just
// provides a placeholder to maintain structure.

// The definitions of the Read This! screens

menuitem_t ReadMenu1[] =
{
  {1,"",MN_ReadThis2,0}
};

menuitem_t ReadMenu2[]=
{
  {1,"",MN_FinishReadThis,0}
};

menuitem_t HelpMenu[]=    // killough 10/98
{
  {1,"",MN_FinishHelp,0}
};

menu_t ReadDef1 =
{
  read1_end,
  &MainDef,
  ReadMenu1,
  MN_DrawReadThis1,
  330,175,
  //280,185,              // killough 2/21/98: fix help screens
  0
};

menu_t ReadDef2 =
{
  read2_end,
  &ReadDef1,
  ReadMenu2,
  MN_DrawReadThis2,
  330,175,
  0
};

menu_t HelpDef =           // killough 10/98
{
  help_end,
  &HelpDef,
  HelpMenu,
  MN_DrawHelp,
  330,175,
  0
};

//
// MN_ReadThis
//

void MN_ReadThis(int choice)
{
  MN_SetupNextMenu(&ReadDef1);
}

void MN_ReadThis2(int choice)
{
  MN_SetupNextMenu(&ReadDef2);
}

void MN_FinishReadThis(int choice)
{
  MN_SetupNextMenu(&MainDef);
}

void MN_FinishHelp(int choice)        // killough 10/98
{
  MN_SetupNextMenu(&MainDef);
}

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
// killough 10/98: updated with new screens

void MN_DrawReadThis1(void)
{
  inhelpscreens = true;
  if (gamemode == shareware)
  {
    V_DrawNamePatch(0, 0, 0, "HELP2", CR_DEFAULT, VPT_STRETCH);
    // e6y: wide-res
    V_FillBorder(-1, 0);
  }
  else
    MN_DrawCredits();
}

//
// Read This Menus - optional second page.
//
// killough 10/98: updated with new screens

void MN_DrawReadThis2(void)
{
  inhelpscreens = true;
  if (gamemode == shareware)
    MN_DrawCredits();
  else
  {
    V_DrawNamePatch(0, 0, 0, "CREDIT", CR_DEFAULT, VPT_STRETCH);
    // e6y: wide-res
    V_FillBorder(-1, 0);
  }
}

/////////////////////////////
//
// EPISODE SELECT
//

//
// episodes_e provides numbers for the episode menu items. The default is
// 4, to accomodate Ultimate Doom. If the user is running anything else,
// this is accounted for in the code.
//

// The definitions of the Episodes menu

menuitem_t EpisodeMenu[]=
{
  {1,"M_EPI1", MN_Episode,'k'},
  {1,"M_EPI2", MN_Episode,'t'},
  {1,"M_EPI3", MN_Episode,'i'},
  {1,"M_EPI4", MN_Episode,'t'}
};

menu_t EpiDef =
{
  ep_end,         // # of menu items
  &MainDef,       // previous menu
  EpisodeMenu,    // menuitem_t ->
  MN_DrawEpisode, // drawing routine ->
  48,63,          // x,y
  ep1             // lastOn
};

//
//    MN_Episode
//
int epi;

void MN_DrawEpisode(void) {
  // CPhipps - patch drawing updated
  V_DrawNamePatch(54, 38, 0, "M_EPISOD", CR_DEFAULT, VPT_STRETCH);
}

void MN_Episode(int choice) {
  if ((gamemode == shareware) && choice) {
    MN_StartMessage(s_SWSTRING, NULL, false); // Ty 03/27/98 - externalized
    MN_SetupNextMenu(&ReadDef1);
    return;
  }

  // Yet another hack...
  if ((gamemode == registered) && (choice > 2)) {
    D_Msg(MSG_WARN, "MN_Episode: 4th episode requires Ultimate DOOM\n");
    choice = 0;
  }

  epi = choice;
  MN_SetupNextMenu(&NewDef);
}

/////////////////////////////
//
// NEW GAME
//

// numerical values for the New Game menu items

// The definitions of the New Game menu

menuitem_t NewGameMenu[]=
{
  {1,"M_JKILL", MN_ChooseSkill, 'i'},
  {1,"M_ROUGH", MN_ChooseSkill, 'h'},
  {1,"M_HURT",  MN_ChooseSkill, 'h'},
  {1,"M_ULTRA", MN_ChooseSkill, 'u'},
  {1,"M_NMARE", MN_ChooseSkill, 'n'}
};

menu_t NewDef =
{
  newg_end,       // # of menu items
  &EpiDef,        // previous menu
  NewGameMenu,    // menuitem_t ->
  MN_DrawNewGame, // drawing routine ->
  48,63,          // x,y
  hurtme          // lastOn
};

//
// MN_NewGame
//

void MN_DrawNewGame(void)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(96, 14, 0, "M_NEWG", CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch(54, 38, 0, "M_SKILL",CR_DEFAULT, VPT_STRETCH);
}

void MN_SetCurrentMenu(menu_t *new_menu) {
  currentMenu = new_menu;
}

/* cph - make `New Game' restart the level in a netgame */
static void MN_RestartLevelResponse(int ch)
{
  if (ch != 'y')
    return;

  if (demorecording)
    exit(0);

  currentMenu->lastOn = itemOn;
  MN_ClearMenus();
  G_RestartLevel();
}

void MN_NewGame(int choice)
{
  if (netgame && !demoplayback) {
    if (compatibility_level < lxdoom_1_compatibility)
      MN_StartMessage(s_NEWGAME,NULL,false); // Ty 03/27/98 - externalized
    else // CPhipps - query restarting the level
      MN_StartMessage(s_RESTARTLEVEL,MN_RestartLevelResponse,true);
    return;
  }

  if (demorecording) {  /* killough 5/26/98: exclude during demo recordings */
    MN_StartMessage("you can't start a new game\n"
       "while recording a demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
  }

  // Chex Quest disabled the episode select screen, as did Doom II.
  if (gamemode == commercial || gamemission == chex)
    MN_SetupNextMenu(&NewDef);
  else
    MN_SetupNextMenu(&EpiDef);
}

// CPhipps - static
static void MN_VerifyNightmare(int ch)
{
  if (ch != 'y')
    return;

  G_DeferedInitNew(nightmare,epi+1,1);
  MN_ClearMenus ();
}

void MN_ChooseSkill(int choice)
{
  if (choice == nightmare)
    {   // Ty 03/27/98 - externalized
      MN_StartMessage(s_NIGHTMARE,MN_VerifyNightmare,true);
      return;
    }

  G_DeferedInitNew(choice,epi+1,1);
  MN_ClearMenus ();
}

/////////////////////////////
//
// LOAD GAME MENU
//

// numerical values for the Load Game slots

// The definitions of the Load Game screen

menuitem_t LoadMenue[]=
{
  {1,"", MN_LoadSelect,'1'},
  {1,"", MN_LoadSelect,'2'},
  {1,"", MN_LoadSelect,'3'},
  {1,"", MN_LoadSelect,'4'},
  {1,"", MN_LoadSelect,'5'},
  {1,"", MN_LoadSelect,'6'},
  {1,"", MN_LoadSelect,'7'}, //jff 3/15/98 extend number of slots
  {1,"", MN_LoadSelect,'8'},
};

menu_t LoadDef =
{
  load_end,
  &MainDef,
  LoadMenue,
  MN_DrawLoad,
  80,34, //jff 3/15/98 move menu up
  0
};

//
// MN_LoadGame & Cie.
//

void MN_DrawLoad(void)
{
  int i;

  //jff 3/15/98 use symbolic load position
  // CPhipps - patch drawing updated
  V_DrawNamePatch(72 ,LOADGRAPHIC_Y, 0, "M_LOADG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0 ; i < load_end ; i++) {
    MN_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    MN_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i], CR_DEFAULT);
  }
}

//
// Draw border for the savegame description
//

void MN_DrawSaveLoadBorder(int x,int y)
{
  int i;

  V_DrawNamePatch(x-8, y+7, 0, "M_LSLEFT", CR_DEFAULT, VPT_STRETCH);

  for (i = 0 ; i < 24 ; i++)
    {
      V_DrawNamePatch(x, y+7, 0, "M_LSCNTR", CR_DEFAULT, VPT_STRETCH);
      x += 8;
    }

  V_DrawNamePatch(x, y+7, 0, "M_LSRGHT", CR_DEFAULT, VPT_STRETCH);
}

//
// User wants to load this game
//

void MN_LoadSelect(int choice)
{
  // CPhipps - Modified so savegame filename is worked out only internal
  //  to g_game.c, this only passes the slot.

  G_LoadGame(choice, false); // killough 3/16/98, 5/15/98: add slot, cmd

  MN_ClearMenus();
}

//
// killough 5/15/98: add forced loadgames
//

static char *forced_loadgame_message;

static void MN_VerifyForcedLoadGame(int ch)
{
  if (ch=='y')
    G_ForcedLoadGame();
  free(forced_loadgame_message);    // free the message strdup()'ed below
  MN_ClearMenus();
}

void MN_ForcedLoadGame(const char *msg)
{
  forced_loadgame_message = strdup(msg); // free()'d above
  MN_StartMessage(forced_loadgame_message, MN_VerifyForcedLoadGame, true);
}

//
// Selected from DOOM menu
//

void MN_LoadGame (int choice)
{
  /* killough 5/26/98: exclude during demo recordings
   * cph - unless a new demo */
  if (demorecording && (compatibility_level < prboom_2_compatibility))
    {
    MN_StartMessage("you can't load a game\n"
       "while recording an old demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
    }

  MN_SetupNextMenu(&LoadDef);
  MN_ReadSaveStrings();
}

/////////////////////////////
//
// SAVE GAME MENU
//

// The definitions of the Save Game screen

menuitem_t SaveMenu[]=
{
  {1,"", MN_SaveSelect,'1'},
  {1,"", MN_SaveSelect,'2'},
  {1,"", MN_SaveSelect,'3'},
  {1,"", MN_SaveSelect,'4'},
  {1,"", MN_SaveSelect,'5'},
  {1,"", MN_SaveSelect,'6'},
  {1,"", MN_SaveSelect,'7'}, //jff 3/15/98 extend number of slots
  {1,"", MN_SaveSelect,'8'},
};

menu_t SaveDef =
{
  load_end, // same number of slots as the Load Game screen
  &MainDef,
  SaveMenu,
  MN_DrawSave,
  80,34, //jff 3/15/98 move menu up
  0
};

//
// MN_ReadSaveStrings
//  read the strings from the savegame files
//
void MN_ReadSaveStrings(void) {
  char *name; // killough 3/22/98
  int len;
  pbuf_t savebuffer;
  buf_t save_game_string;
  bool load_succeeded;

  M_PBufInit(&savebuffer);
  M_BufferInitWithCapacity(&save_game_string, SAVESTRINGSIZE);

  /* CG: FIXME: Reading the entire save for just the first 24 bytes sucks */

  for (int i = 0; i < load_end; i++) {
    /* killough 3/22/98
     * cph - add not-demoplayback parameter */
    M_PBufClear(&savebuffer);
    M_BufferClear(&save_game_string);
    memset(&savegamestrings[i][0], 0, SAVESTRINGSIZE);

    len = G_SaveGameName(NULL, 0, i, false);
    name = malloc(len + 1);
    G_SaveGameName(name, len + 1, i, false);
    load_succeeded = M_PBufSetFile(&savebuffer, name);
    free(name);

    if (!load_succeeded) {  // Ty 03/27/98 - externalized:
      strcpy(&savegamestrings[i][0], s_EMPTYSTRING);
      LoadMenue[i].status = 0;
      continue;
    }

    M_PBufSeek(&savebuffer, 0);
    M_PBufReadBytes(&savebuffer, &save_game_string);
    memcpy(
      &savegamestrings[i][0],
      M_BufferGetData(&save_game_string),
      M_BufferGetSize(&save_game_string)
    );
    LoadMenue[i].status = 1;
  }

  M_PBufFree(&savebuffer);
  M_BufferFree(&save_game_string);
}

//
//  MN_SaveGame & Cie.
//
void MN_DrawSave(void)
{
  int i;

  //jff 3/15/98 use symbolic load position
  // CPhipps - patch drawing updated
  V_DrawNamePatch(72, LOADGRAPHIC_Y, 0, "M_SAVEG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0; i < load_end; i++)
  {
    MN_DrawSaveLoadBorder(LoadDef.x, LoadDef.y + LINEHEIGHT * i);
    MN_WriteText(
      LoadDef.x, LoadDef.y + LINEHEIGHT * i, savegamestrings[i], CR_DEFAULT
    );
  }

  if (saveStringEnter)
  {
    i = MN_StringWidth(savegamestrings[saveSlot]);
    MN_WriteText(
      LoadDef.x + i, LoadDef.y + LINEHEIGHT * saveSlot, "_", CR_DEFAULT
    );
  }
}

//
// MN_Responder calls this when user is finished
//
static void MN_DoSave(int slot)
{
  G_SaveGame(slot,savegamestrings[slot]);
  MN_ClearMenus();

  // PICK QUICKSAVE SLOT YET?
  if (quickSaveSlot == -2)
    quickSaveSlot = slot;
}

//
// User wants to save. Start string input for MN_Responder
//
void MN_SaveSelect(int choice)
{
  // we are going to be intercepting all chars
  saveStringEnter = true;

  saveSlot = choice;
  strcpy(saveOldString,savegamestrings[choice]);
  if (!strcmp(savegamestrings[choice], s_EMPTYSTRING)) // Ty 03/27/98 - externalized
    savegamestrings[choice][0] = 0;
  saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void MN_SaveGame(int choice)
{
  // killough 10/6/98: allow savegames during single-player demo playback
  if (!usergame && (!demoplayback || netgame))
  {
    MN_StartMessage(s_SAVEDEAD,NULL,false); // Ty 03/27/98 - externalized
    return;
  }

  if (gamestate != GS_LEVEL)
    return;

  MN_SetupNextMenu(&SaveDef);
  MN_ReadSaveStrings();
}

/////////////////////////////
//
// OPTIONS MENU
//

// numerical values for the Options menu items

// The definitions of the Options menu

menuitem_t OptionsMenu[]=
{
  // killough 4/6/98: move setup to be a sub-menu of OPTIONs
  {1,"M_GENERL", MN_General, 'g', "GENERAL"},      // killough 10/98
  {1,"M_SETUP",  MN_Setup,   's', "SETUP"},        // phares 3/21/98
  {1,"M_ENDGAM", MN_EndGame,'e',  "END GAME"},
  {1,"M_MESSG",  MN_ChangeMessages,'m', "MESSAGES:"},
  /*    {1,"M_DETAIL",  MN_ChangeDetail,'g'},  unused -- killough */
  {2,"M_SCRNSZ", MN_SizeDisplay,'s', "SCREEN SIZE"},
  {-1,"",0},
  {1,"M_MSENS",  MN_ChangeSensitivity,'m', "MOUSE SENSITIVITY"},
  /* {-1,"",0},  replaced with submenu -- killough */
  {1,"M_SVOL",   MN_Sound,'s', "SOUND VOLUME"},
};

menu_t OptionsDef =
{
  opt_end,
  &MainDef,
  OptionsMenu,
  MN_DrawOptions,
  60,37,
  0
};

//
// MN_Options
//
char detailNames[2][9] = {"M_GDHIGH","M_GDLOW"};
char msgNames[2][9]  = {"M_MSGOFF","M_MSGON"};


void MN_DrawOptions(void)
{
  // CPhipps - patch drawing updated
  // proff/nicolas 09/20/98 -- changed for hi-res
  V_DrawNamePatch(108, 15, 0, "M_OPTTTL", CR_DEFAULT, VPT_STRETCH);

  if ((W_CheckNumForName("M_GENERL") < 0) || (W_CheckNumForName("M_SETUP") < 0))
  {
    MN_WriteText(OptionsDef.x + HU_StringWidth("MESSAGES: "),
      OptionsDef.y+8-(HU_StringHeight("ONOFF")/2)+LINEHEIGHT*messages,
      showMessages ? "ON" : "OFF", CR_DEFAULT);
  }
  else
  {
    V_DrawNamePatch(OptionsDef.x + 120, OptionsDef.y+LINEHEIGHT*messages, 0,
      msgNames[showMessages], CR_DEFAULT, VPT_STRETCH);
  }

  MN_DrawThermo(OptionsDef.x,OptionsDef.y+LINEHEIGHT*(scrnsize+1),
   9,screenSize);
}

void MN_Options(int choice)
{
  MN_SetupNextMenu(&OptionsDef);
}

/////////////////////////////
//
// MN_QuitDOOM
//
int quitsounds[8] =
{
  sfx_pldeth,
  sfx_dmpain,
  sfx_popain,
  sfx_slop,
  sfx_telept,
  sfx_posit1,
  sfx_posit3,
  sfx_sgtatk
};

int quitsounds2[8] =
{
  sfx_vilact,
  sfx_getpow,
  sfx_boscub,
  sfx_slop,
  sfx_skeswg,
  sfx_kntdth,
  sfx_bspact,
  sfx_sgtatk
};

static void MN_QuitResponse(int ch)
{
  if (ch != 'y')
    return;
  
  //e6y: Optional removal of a quit sound
  if ((!netgame && showendoom) // killough 12/98
      && !nosfxparm && snd_card) // avoid delay if no sound card
  {
    int i;

    if (gamemode == commercial)
      S_StartSound(NULL,quitsounds2[(gametic>>2)&7]);
    else
      S_StartSound(NULL,quitsounds[(gametic>>2)&7]);

    // wait till all sounds stopped or 3 seconds are over
    i = 30;
    while (i>0) {
      I_Sleep(100); // CPhipps - don't thrash cpu in this loop
      if (!I_AnySoundStillPlaying())
        break;
      i--;
    }
  }
  //e6y: I_SafeExit instead of exit - prevent recursive exits
  I_SafeExit(0); // killough
}

void MN_QuitDOOM(int choice)
{
  static char endstring[160];

  // We pick index 0 which is language sensitive,
  // or one at random, between 1 and maximum number.
  // Ty 03/27/98 - externalized DOSY as a string s_DOSY that's in the sprintf
  if (language != english)
    sprintf(endstring,"%s\n\n%s",s_DOSY, endmsg[0] );
  else         // killough 1/18/98: fix endgame message calculation:
    sprintf(endstring,"%s\n\n%s", endmsg[gametic%(NUM_QUITMESSAGES-1)+1], s_DOSY);

  MN_StartMessage(endstring,MN_QuitResponse,true);
}

/////////////////////////////
//
// SOUND VOLUME MENU
//

// numerical values for the Sound Volume menu items
// The 'empty' slots are where the sliding scales appear.

// The definitions of the Sound Volume menu

menuitem_t SoundMenu[]=
{
  {2,"M_SFXVOL",MN_SfxVol,'s'},
  {-1,"",0},
  {2,"M_MUSVOL",MN_MusicVol,'m'},
  {-1,"",0}
};

menu_t SoundDef =
{
  sound_end,
  &OptionsDef,
  SoundMenu,
  MN_DrawSound,
  80,64,
  0
};

//
// Change Sfx & Music volumes
//

void MN_DrawSound(void)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(60, 38, 0, "M_SVOL", CR_DEFAULT, VPT_STRETCH);

  MN_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),16,snd_SfxVolume);

  MN_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),16,snd_MusicVolume);
}

void MN_Sound(int choice)
{
  MN_SetupNextMenu(&SoundDef);
}

void MN_SfxVol(int choice)
{
  switch(choice)
    {
    case 0:
      if (snd_SfxVolume)
        snd_SfxVolume--;
      break;
    case 1:
      if (snd_SfxVolume < 15)
        snd_SfxVolume++;
      break;
    }

  S_SetSfxVolume(snd_SfxVolume /* *8 */);
}

void MN_MusicVol(int choice)
{
  switch(choice)
    {
    case 0:
      if (snd_MusicVolume)
        snd_MusicVolume--;
      break;
    case 1:
      if (snd_MusicVolume < 15)
        snd_MusicVolume++;
      break;
    }

  S_SetMusicVolume(snd_MusicVolume /* *8 */);
}

/////////////////////////////
//
// MOUSE SENSITIVITY MENU -- killough
//

// numerical values for the Mouse Sensitivity menu items
// The 'empty' slots are where the sliding scales appear.

// The definitions of the Mouse Sensitivity menu

menuitem_t MouseMenu[]=
{
  {2,"M_HORSEN",D_MouseHoriz,'h', "HORIZONTAL"},
  {-1,"",0},
  {2,"M_VERSEN",D_MouseVert,'v', "VERTICAL"},
  {-1,"",0}

  //e6y
  ,
  {2,"M_LOKSEN",D_MouseMLook,'l', "MOUSE LOOK"},
  {-1,"",0},
  {2,"M_ACCEL",D_MouseAccel,'a', "ACCELERATION"},
  {-1,"",0}
};

menu_t MouseDef =
{
  mouse_end,
  &OptionsDef,
  MouseMenu,
  MN_DrawMouse,
  60,37,//e6y
  0
};


//
// Change Mouse Sensitivities -- killough
//

void MN_DrawMouse(void)
{
  int mhmx,mvmx; /* jff 4/3/98 clamp drawn position    99max mead */

  // CPhipps - patch drawing updated
  V_DrawNamePatch(60, 15, 0, "M_MSENS", CR_DEFAULT, VPT_STRETCH);//e6y

  //jff 4/3/98 clamp horizontal sensitivity display
  mhmx = mouseSensitivity_horiz>99? 99 : mouseSensitivity_horiz; /*mead*/
  MN_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_horiz+1),100,mhmx);
  //jff 4/3/98 clamp vertical sensitivity display
  mvmx = mouseSensitivity_vert>99? 99 : mouseSensitivity_vert; /*mead*/
  MN_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_vert+1),100,mvmx);

  //e6y
  {
    int mpmx;
    mpmx = mouseSensitivity_mlook>99? 99 : mouseSensitivity_mlook;
    MN_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_mlook+1),100,mpmx);
    mpmx = mouse_acceleration>99? 99 : mouse_acceleration;
    MN_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_accel+1),100,mpmx);
  }
}

void MN_ChangeSensitivity(int choice)
{
  MN_SetupNextMenu(&MouseDef);      // killough

  //  switch(choice)
  //      {
  //    case 0:
  //      if (mouseSensitivity)
  //        mouseSensitivity--;
  //      break;
  //    case 1:
  //      if (mouseSensitivity < 9)
  //        mouseSensitivity++;
  //      break;
  //      }
}

/////////////////////////////
//
//    MN_QuickSave
//

char tempstring[80];

static void MN_QuickSaveResponse(int ch)
{
  if (ch == 'y')  {
    MN_DoSave(quickSaveSlot);
    S_StartSound(NULL, sfx_swtchx);
  }
}

void MN_QuickSave(void)
{
  if (!usergame && (!demoplayback || netgame)) { /* killough 10/98 */
    S_StartSound(NULL, sfx_oof);
    return;
  }

  if (gamestate != GS_LEVEL)
    return;

  if (quickSaveSlot < 0) {
    MN_StartControlPanel();
    MN_ReadSaveStrings();
    MN_SetupNextMenu(&SaveDef);
    quickSaveSlot = -2; // means to pick a slot now
    return;
  }
  sprintf(tempstring,s_QSPROMPT,savegamestrings[quickSaveSlot]); // Ty 03/27/98 - externalized
  MN_StartMessage(tempstring,MN_QuickSaveResponse,true);
}

/////////////////////////////
//
// MN_QuickLoad
//

static void MN_QuickLoadResponse(int ch)
{
  if (ch == 'y') {
    MN_LoadSelect(quickSaveSlot);
    S_StartSound(NULL, sfx_swtchx);
  }
}

void MN_QuickLoad(void)
{
  // cph - removed restriction against quickload in a netgame

  if (demorecording) {  // killough 5/26/98: exclude during demo recordings
    MN_StartMessage("you can't quickload\n"
       "while recording a demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
  }

  if (quickSaveSlot < 0) {
    MN_StartMessage(s_QSAVESPOT,NULL,false); // Ty 03/27/98 - externalized
    return;
  }
  sprintf(tempstring,s_QLPROMPT,savegamestrings[quickSaveSlot]); // Ty 03/27/98 - externalized
  MN_StartMessage(tempstring,MN_QuickLoadResponse,true);
}

/////////////////////////////
//
// MN_EndGame
//

static void MN_EndGameResponse(int ch)
{
  if (ch != 'y')
    return;

  // killough 5/26/98: make endgame quit if recording or playing back demo
  if (demorecording || singledemo)
    G_CheckDemoStatus();

  currentMenu->lastOn = itemOn;
  MN_ClearMenus();
  D_StartTitle();
}

void MN_EndGame(int choice) {
  // Ty 03/27/98 - externalized
  if (netgame)
    MN_StartMessage(s_NETEND, NULL, false);
  else
    MN_StartMessage(s_ENDGAME, MN_EndGameResponse, true);
}

/////////////////////////////
//
// Toggle messages on/off
//

void MN_ChangeMessages(int choice) {
  choice = 0; // suppress warning: unused parameter `int choice'
  showMessages = 1 - showMessages;

  if (!showMessages) {
    PL_Write(P_GetConsolePlayer(), s_MSGOFF); // Ty 03/27/98 - externalized
  }
  else {
    PL_Write(P_GetConsolePlayer(), s_MSGON); // Ty 03/27/98 - externalized
  }

  message_dontfuckwithme = true;
}

/////////////////////////////
//
// CHANGE DISPLAY SIZE
//
// jff 2/23/98 restored to pre-HUD state
// hud_active controlled soley by F5=key_detail (key_hud)
// hud_displayed is toggled by + or = in fullscreen
// hud_displayed is cleared by -

void MN_SizeDisplay(int choice)
{
  switch(choice) {
  case 0:
    if (screenSize > 0) {
      screenblocks--;
      screenSize--;
      hud_displayed = 0;
    }
    break;
  case 1:
    if (screenSize < 8) {
      screenblocks++;
      screenSize++;
    }
    else
      hud_displayed = !hud_displayed;
    break;
  }
  R_SetViewSize (screenblocks /*, detailLevel obsolete -- killough */);
}

//
// End of Original Menus
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// SETUP MENU (phares)
//
// We've added a set of Setup Screens from which you can configure a number
// of variables w/o having to restart the game. There are 7 screens:
//
//    Key Bindings
//    Weapons
//    Status Bar / HUD
//    Automap
//    Enemies
//    Messages
//    Chat Strings
//
// killough 10/98: added Compatibility and General menus
//

/////////////////////////////
//
// booleans for setup screens
// these tell you what state the setup screens are in, and whether any of
// the overlay screens (automap colors, reset button message) should be
// displayed

bool setup_active      = false; // in one of the setup screens
bool set_keybnd_active = false; // in key binding setup screens
bool set_weapon_active = false; // in weapons setup screen
bool set_status_active = false; // in status bar/hud setup screen
bool set_auto_active   = false; // in automap setup screen
bool set_enemy_active  = false; // in enemies setup screen
bool set_mess_active   = false; // in messages setup screen
bool set_chat_active   = false; // in chat string setup screen
bool setup_select      = false; // changing an item
bool setup_gather      = false; // gathering keys for value
bool colorbox_active   = false; // color palette being shown
bool default_verify    = false; // verify reset defaults decision
bool set_general_active = false;
bool set_compat_active = false;

/////////////////////////////
//
// set_menu_itemon is an index that starts at zero, and tells you which
// item on the current screen the cursor is sitting on.
//
// current_setup_menu is a pointer to the current setup menu table.

static int set_menu_itemon; // which setup item is selected?   // phares 3/98
setup_menu_t* current_setup_menu; // points to current setup menu table

/////////////////////////////
//
// The menu_buffer is used to construct strings for display on the screen.

static char menu_buffer[64];

/////////////////////////////
//
// Screens.
//

int setup_screen; // the current setup screen. takes values from setup_e

/////////////////////////////
//
// SetupMenu is the definition of what the main Setup Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_KEYBND") that should be displayed,
// the program which takes over when an item is selected, and the hotkey
// associated with the item.

menuitem_t SetupMenu[]=
{
  {1,"M_COMPAT",MN_Compat,     'p', "DOOM COMPATIBILITY"},
  {1,"M_KEYBND",MN_KeyBindings,'k', "KEY BINDINGS"},
  {1,"M_WEAP"  ,MN_Weapons,    'w', "WEAPONS"},
  {1,"M_STAT"  ,MN_StatusBar,  's', "STATUS BAR / HUD"},
  {1,"M_AUTO"  ,MN_Automap,    'a', "AUTOMAP"},
  {1,"M_ENEM"  ,MN_Enemy,      'e', "ENEMIES"},
  {1,"M_MESS"  ,MN_Messages,   'm', "MESSAGES"},
  {1,"M_CHAT"  ,MN_ChatStrings,'c', "CHAT STRINGS"},
};

/////////////////////////////
//
// MN_DoNothing does just that: nothing. Just a placeholder.

static void MN_DoNothing(int choice)
{
}

/////////////////////////////
//
// Items needed to satisfy the 'Big Font' menu structures:
//

// Generic_Setup is a do-nothing definition that the mainstream Menu code
// can understand, while the Setup Menu code is working. Another placeholder.

menuitem_t Generic_Setup[] =
{
  {1,"",MN_DoNothing,0}
};

/////////////////////////////
//
// SetupDef is the menu definition that the mainstream Menu code understands.
// This is used by MN_Setup (below) to define what is drawn and what is done
// with the main Setup screen.

menu_t  SetupDef =
{
  set_setup_end, // number of Setup Menu items (Key Bindings, etc.)
  &OptionsDef,   // menu to return to when BACKSPACE is hit on this menu
  SetupMenu,     // definition of items to show on the Setup Screen
  MN_DrawSetup,  // program that draws the Setup Screen
  59,37,         // x,y position of the skull (modified when the skull is
                 // drawn). The skull is parked on the upper-left corner
                 // of the Setup screens, since it isn't needed as a cursor
  0              // last item the user was on for this menu
};

/////////////////////////////
//
// Here are the definitions of the individual Setup Menu screens. They
// follow the format of the 'Big Font' menu structures. See the comments
// for SetupDef (above) to help understand what each of these says.

menu_t KeybndDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawKeybnd,
  34,5,      // skull drawn here
  0
};

menu_t WeaponDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawWeapons,
  34,5,      // skull drawn here
  0
};

menu_t StatusHUDDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawStatusHUD,
  34,5,      // skull drawn here
  0
};

menu_t AutoMapDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawAutoMap,
  34,5,      // skull drawn here
  0
};

menu_t EnemyDef =                                           // phares 4/08/98
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawEnemy,
  34,5,      // skull drawn here
  0
};

menu_t MessageDef =                                         // phares 4/08/98
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawMessages,
  34,5,      // skull drawn here
  0
};

menu_t ChatStrDef =                                         // phares 4/10/98
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawChatStrings,
  34,5,      // skull drawn here
  0
};

menu_t GeneralDef =                                           // killough 10/98
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  MN_DrawGeneral,
  34,5,      // skull drawn here
  0
};

menu_t CompatDef =                                           // killough 10/98
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  MN_DrawCompat,
  34,5,      // skull drawn here
  0
};

/////////////////////////////
//
// Draws the Title for the main Setup screen

void MN_DrawSetup(void)
{
  // CPhipps - patch drawing updated
  HU_DrawTitle(124, 15, "M_SETUP", CR_DEFAULT, "SETUP", CR_GOLD);
}

/////////////////////////////
//
// Uses the SetupDef structure to draw the menu items for the main
// Setup screen

void MN_Setup(int choice)
{
  MN_SetupNextMenu(&SetupDef);
}

/////////////////////////////
//
// Data that's used by the Setup screen code
//

int   chat_index;
char* chat_string_buffer; // points to new chat strings while editing

/////////////////////////////
//
// phares 4/17/98:
// Added 'Reset to Defaults' Button to Setup Menu screens
// This is a small button that sits in the upper-right-hand corner of
// the first screen for each group. It blinks when selected, thus the
// two patches, which it toggles back and forth.

char ResetButtonName[2][8] = {"M_BUTT1","M_BUTT2"};

/////////////////////////////
//
// phares 4/18/98:
// Consolidate Item drawing code
//
// MN_DrawItem draws the description of the provided item (the left-hand
// part). A different color is used for the text depending on whether the
// item is selected or not, or whether it's about to change.

// CPhipps - static, hanging else removed, const parameter
static void MN_DrawItem(const setup_menu_t *s) {
  int x = s->m_x;
  int y = s->m_y;
  int flags = s->m_flags;

  if (flags & S_RESET) {

    // This item is the reset button
    // Draw the 'off' version if this isn't the current menu item
    // Draw the blinking version in tune with the blinking skull otherwise

    // proff/nicolas 09/20/98 -- changed for hi-res
    // CPhipps - Patch drawing updated, reformatted

    V_DrawNamePatch(
      x,
      y,
      0,
      ResetButtonName[(flags & (S_HILITE|S_SELECT)) ? whichSkull : 0],
      CR_DEFAULT,
      VPT_STRETCH
    );
  }
  else { // Draw the item string
    char *p;
    char *t;
    int w = 0;
    int color =
      flags & S_DISABLE ? CR_DISABLE : //e6y
      flags & S_SELECT ? CR_SELECT :
      flags & S_HILITE ? CR_HILITE :
      flags & (S_TITLE|S_NEXT|S_PREV) ? CR_TITLE : CR_ITEM; // killough 10/98

    /*
     * killough 10/98:
     * Enhance to support multiline text separated by newlines.
     * This supports multiline items on horizontally-crowded menus.
     */

    for (p = t = strdup(s->m_text); (p = strtok(p,"\n")); y += 8, p = NULL) {
      /* killough 10/98: support left-justification: */
      strcpy(menu_buffer,p);

      if (!(flags & S_LEFTJUST)) {
        w = MN_GetPixelWidth(menu_buffer) + 4;
      }

      MN_DrawMenuString(x - w, y ,color);
    }

    free(t);
  }
}

// If a number item is being changed, allow up to N keystrokes to 'gather'
// the value. Gather_count tells you how many you have so far. The legality
// of what is gathered is determined by the low/high settings for the item.

int  gather_count;
char gather_buffer[MAXGATHER+1];  // killough 10/98: make input character-based

/////////////////////////////
//
// phares 4/18/98:
// Consolidate Item Setting drawing code
//
// MN_DrawSetting draws the setting of the provided item (the right-hand
// part. It determines the text color based on whether the item is
// selected or being changed. Then, depending on the type of item, it
// displays the appropriate setting value: yes/no, a key binding, a number,
// a paint chip, etc.

static void MN_DrawSetting(const setup_menu_t* s)
{
  int x = s->m_x, y = s->m_y, flags = s->m_flags, color;

  // Determine color of the text. This may or may not be used later,
  // depending on whether the item is a text string or not.

  color =
    flags & S_DISABLE ? CR_DISABLE : //e6y
    flags & S_SELECT ? CR_SELECT : flags & S_HILITE ? CR_HILITE : CR_SET;

  // Is the item a YES/NO item?

  if (flags & S_YESNO) {
    strcpy(menu_buffer,*s->var.def->location.pi ? "YES" : "NO");
    MN_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a simple number?

  if (flags & S_NUM) {
    // killough 10/98: We must draw differently for items being gathered.
    if (flags & (S_HILITE|S_SELECT) && setup_gather) {
      gather_buffer[gather_count] = 0;
      strcpy(menu_buffer, gather_buffer);
    }
    else
      sprintf(menu_buffer,"%d",*s->var.def->location.pi);
    MN_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a key binding?

  if (flags & S_KEY) { // Key Binding
    int *key = s->var.m_key;

    // Draw the key bound to the action

    if (key) {
      MN_GetKeyString(*key, 0); // string to display
      if (key == &key_up ||
          key == &key_down ||
          key == &key_speed ||
          key == &key_fire ||
          key == &key_strafe ||
          key == &key_strafeleft ||
          key == &key_straferight ||
          key == &key_use) {
        if (s->m_mouse && *s->m_mouse != -1)
          sprintf(menu_buffer + strlen(menu_buffer), "/MB%d", *s->m_mouse + 1);
        if (s->m_joy)
          sprintf(menu_buffer + strlen(menu_buffer), "/JSB%d", *s->m_joy + 1);
      }
      MN_DrawMenuString(x, y, color);
    }
    return;
  }

  // Is the item a weapon number?
  // OR, Is the item a colored text string from the Automap?
  //
  // killough 10/98: removed special code, since the rest of the engine
  // already takes care of it, and this code prevented the user from setting
  // their overall weapons preferences while playing Doom 1.
  //
  // killough 11/98: consolidated weapons code with color range code

  if (flags & (S_WEAP|S_CRITEM)) // weapon number or color range
    {
      sprintf(menu_buffer,"%d", *s->var.def->location.pi);
      MN_DrawMenuString(x,y, flags & S_CRITEM ? *s->var.def->location.pi : color);
      return;
    }

  // Is the item a paint chip?

  if (flags & S_COLOR) // Automap paint chip
    {
      int ch;

      ch = *s->var.def->location.pi;
      // proff 12/6/98: Drawing of colorchips completly changed for hi-res, it now uses a patch
      // draw the paint chip
      // e6y: wide-res
      {
        int xx = x, yy = y - 1, ww = 8, hh = 8;
        V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
        V_FillRect(0, xx, yy, ww, hh, PAL_BLACK);
        xx = x + 1, yy = y, ww = 6, hh = 6;
        V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
        V_FillRect(0, xx, yy, ww, hh, (unsigned char)ch);
      }

      if (!ch) { // don't show this item in automap mode
        V_DrawNamePatch(x+1,y,0,"M_PALNO", CR_DEFAULT, VPT_STRETCH);
      }
      return;
    }

  // Is the item a chat string?
  // killough 10/98: or a filename?

  if (flags & S_STRING) {
    /* cph - cast to char* as it's really a Z_Strdup'd string (see m_misc.h) */
    union { const char **c; char **s; } u; // type punning via unions
    char *text;

    u.c = s->var.def->location.ppsz;
    text = *(u.s);

    // Are we editing this string? If so, display a cursor under
    // the correct character.

    if (setup_select && (s->m_flags & (S_HILITE|S_SELECT))) {
      int cursor_start, char_width;
      char c[2];

      // If the string is too wide for the screen, trim it back,
      // one char at a time until it fits. This should only occur
      // while you're editing the string.

      while (MN_GetPixelWidth(text) >= MAXCHATWIDTH) {
  int len = strlen(text);
  text[--len] = 0;
  if (chat_index > len)
    chat_index--;
      }

      // Find the distance from the beginning of the string to
      // where the cursor should be drawn, plus the width of
      // the char the cursor is under..

      *c = text[chat_index]; // hold temporarily
      c[1] = 0;
      char_width = MN_GetPixelWidth(c);
      if (char_width == 1)
  char_width = 7; // default for end of line
      text[chat_index] = 0; // NULL to get cursor position
      cursor_start = MN_GetPixelWidth(text);
      text[chat_index] = *c; // replace stored char

      // Now draw the cursor
      // proff 12/6/98: Drawing of cursor changed for hi-res
      // e6y: wide-res
      {
        int xx = (x+cursor_start-1), yy = y, ww = char_width, hh = 9;
        V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
        V_FillRect(0, xx, yy, ww, hh, PAL_WHITE);
      }
    }

    // Draw the setting for the item

    strcpy(menu_buffer,text);
    MN_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a selection of choices?

  if (flags & S_CHOICE) {
    if (s->var.def->type == def_int) {
      if (s->selectstrings == NULL) {
        sprintf(menu_buffer,"%d",*s->var.def->location.pi);
      } else {
        strcpy(menu_buffer,s->selectstrings[*s->var.def->location.pi]);
      }
    }

    if (s->var.def->type == def_str) {
      sprintf(menu_buffer,"%s", *s->var.def->location.ppsz);
    }

    MN_DrawMenuString(x,y,color);
    return;
  }
}

/////////////////////////////
//
// MN_DrawScreenItems takes the data for each menu item and gives it to
// the drawing routines above.

// CPhipps - static, const parameter, formatting
static void MN_DrawScreenItems(const setup_menu_t* src)
{
  if (print_warning_about_changes > 0) { /* killough 8/15/98: print warning */
  //e6y
    if (warning_about_changes & S_CANT_GL_ARB_MULTITEXTURE) {
  strcpy(menu_buffer, "Extension GL_ARB_multitexture not found");
  MN_DrawMenuString(30,176,CR_RED);
  } else
    if (warning_about_changes & S_CANT_GL_ARB_MULTISAMPLEFACTOR) {
  strcpy(menu_buffer, "Mast be even number like 0-none, 2, 4, 6");
  MN_DrawMenuString(30,176,CR_RED);
  } else

    if (warning_about_changes & S_BADVAL) {
  strcpy(menu_buffer, "Value out of Range");
  MN_DrawMenuString(100,176,CR_RED);
    } else if (warning_about_changes & S_PRGWARN) {
        strcpy(menu_buffer, "Warning: Program must be restarted to see changes");
  MN_DrawMenuString(3, 176, CR_RED);
    } else if (warning_about_changes & S_BADVID) {
        strcpy(menu_buffer, "Video mode not supported");
  MN_DrawMenuString(80,176,CR_RED);
    } else {
  strcpy(menu_buffer, "Warning: Changes are pending until next game");
        MN_DrawMenuString(18,184,CR_RED);
    }
  }

  while (!(src->m_flags & S_END)) {

    // See if we're to draw the item description (left-hand part)

    if (src->m_flags & S_SHOWDESC)
      MN_DrawItem(src);

    // See if we're to draw the setting (right-hand part)

    if (src->m_flags & S_SHOWSET)
      MN_DrawSetting(src);
    src++;
  }
}

/////////////////////////////
//
// Data used to draw the "are you sure?" dialogue box when resetting
// to defaults.

// And the routine to draw it.

static void MN_DrawDefVerify(void)
{
  // proff 12/6/98: Drawing of verify box changed for hi-res, it now uses a patch
  V_DrawNamePatch(VERIFYBOXXORG,VERIFYBOXYORG,0,"M_VBOX",CR_DEFAULT,VPT_STRETCH);
  // The blinking messages is keyed off of the blinking of the
  // cursor skull.

  if (whichSkull) { // blink the text
    strcpy(menu_buffer,"Reset to defaults? (Y or N)");
    MN_DrawMenuString(VERIFYBOXXORG+8,VERIFYBOXYORG+8,CR_RED);
  }
}


/////////////////////////////
//
// phares 4/18/98:
// MN_DrawInstructions writes the instruction text just below the screen title
//
// cph 2006/08/06 - go back to the Boom version, and then clean up by using
// MN_DrawStringCentered (much better than all those magic 'x' valies!)

static void MN_DrawInstructions(void)
{
  int flags = current_setup_menu[set_menu_itemon].m_flags;

  // There are different instruction messages depending on whether you
  // are changing an item or just sitting on it.

  if (setup_select) {
    switch (flags & (S_KEY | S_YESNO | S_WEAP | S_NUM | S_COLOR | S_CRITEM | S_CHAT | S_RESET | S_FILE | S_CHOICE)) {
      case S_KEY:
        // See if a joystick or mouse button setting is allowed for
        // this item.
        if (current_setup_menu[set_menu_itemon].m_mouse || current_setup_menu[set_menu_itemon].m_joy)
          MN_DrawStringCentered(160, 20, CR_SELECT, "Press key or button for this action");
        else
          MN_DrawStringCentered(160, 20, CR_SELECT, "Press key for this action");
        break;

    case S_YESNO:
      MN_DrawStringCentered(160, 20, CR_SELECT, "Press ENTER key to toggle");
      break;
    case S_WEAP:
      MN_DrawStringCentered(160, 20, CR_SELECT, "Enter weapon number");
      break;
    case S_NUM:
      MN_DrawStringCentered(160, 20, CR_SELECT, "Enter value. Press ENTER when finished.");
      break;
    case S_COLOR:
      MN_DrawStringCentered(160, 20, CR_SELECT, "Select color and press enter");
      break;
    case S_CRITEM:
      MN_DrawStringCentered(160, 20, CR_SELECT, "Enter value");
      break;
    case S_CHAT:
      MN_DrawStringCentered(160, 20, CR_SELECT, "Type/edit chat string and Press ENTER");
      break;
    case S_FILE:
      MN_DrawStringCentered(160, 20, CR_SELECT, "Type/edit filename and Press ENTER");
      break;
    case S_CHOICE: 
      MN_DrawStringCentered(160, 20, CR_SELECT, "Press left or right to choose");
      break;
    case S_RESET:
      break;
#ifdef SIMPLECHECKS
    default:
      D_Msg(MSG_WARN, "Unrecognized menu item type %d", flags);
#endif
    }
  } else {
    if (flags & S_RESET)
      MN_DrawStringCentered(160, 20, CR_HILITE, "Press ENTER key to reset to defaults");
    else
      MN_DrawStringCentered(160, 20, CR_HILITE, "Press Enter to Change");
  }
}


/////////////////////////////
//
// The Key Binding Screen tables.

// Definitions of the (in this case) four key binding screens.

setup_menu_t keys_settings1[];
setup_menu_t keys_settings2[];
setup_menu_t keys_settings3[];
setup_menu_t keys_settings4[];
setup_menu_t keys_settings5[];
setup_menu_t keys_settings6[];
setup_menu_t keys_settings7[];

// The table which gets you from one screen table to the next.

setup_menu_t* keys_settings[] =
{
  keys_settings1,
  keys_settings2,
  keys_settings3,
  keys_settings4,
  keys_settings5,
  keys_settings6,
  keys_settings7,
  NULL
};

int mult_screens_index; // the index of the current screen in a set

// Here's an example from this first screen, with explanations.
//
//  {
//  "STRAFE",      // The description of the item ('strafe' key)
//  S_KEY,         // This is a key binding item
//  m_scrn,        // It belongs to the m_scrn group. Its key cannot be
//                 // bound to two items in this group.
//  KB_X,          // The X offset of the start of the right-hand side
//  KB_Y+ 8*8,     // The Y offset of the start of the right-hand side.
//                 // Always given in multiples off a baseline.
//  &key_strafe,   // The variable that holds the key value bound to this
//                    OR a string that holds the config variable name.
//                    OR a pointer to another setup_menu
//  &mousebstrafe, // The variable that holds the mouse button bound to
                   // this. If zero, no mouse button can be bound here.
//  &joybstrafe,   // The variable that holds the joystick button bound to
                   // this. If zero, no mouse button can be bound here.
//  }

// The first Key Binding screen table.
// Note that the Y values are ascending. If you need to add something to
// this table, (well, this one's not a good example, because it's full)
// you need to make sure the Y values still make sense so everything gets
// displayed.
//
// Note also that the first screen of each set has a line for the reset
// button. If there is more than one screen in a set, the others don't get
// the reset button.
//
// Note also that this screen has a "NEXT ->" line. This acts like an
// item, in that 'activating' it moves you along to the next screen. If
// there's a "<- PREV" item on a screen, it behaves similarly, moving you
// to the previous screen. If you leave these off, you can't move from
// screen to screen.

setup_menu_t keys_settings1[] =  // Key Binding screen strings
{
  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FORWARD"     ,S_KEY       ,m_scrn,KB_X,KB_Y+1*8,{&key_up},&mousebforward},
  {"BACKWARD"    ,S_KEY       ,m_scrn,KB_X,KB_Y+2*8,{&key_down},&mousebbackward},
  {"TURN LEFT"   ,S_KEY       ,m_scrn,KB_X,KB_Y+3*8,{&key_left}},
  {"TURN RIGHT"  ,S_KEY       ,m_scrn,KB_X,KB_Y+4*8,{&key_right}},
  {"RUN"         ,S_KEY       ,m_scrn,KB_X,KB_Y+5*8,{&key_speed},0,&joybspeed},
  {"STRAFE LEFT" ,S_KEY       ,m_scrn,KB_X,KB_Y+6*8,{&key_strafeleft},0,&joybstrafeleft},
  {"STRAFE RIGHT",S_KEY       ,m_scrn,KB_X,KB_Y+7*8,{&key_straferight},0,&joybstraferight},
  {"STRAFE"      ,S_KEY       ,m_scrn,KB_X,KB_Y+8*8,{&key_strafe},&mousebstrafe,&joybstrafe},
  {"AUTORUN"     ,S_KEY       ,m_scrn,KB_X,KB_Y+9*8,{&key_autorun}},
  {"180 TURN"    ,S_KEY       ,m_scrn,KB_X,KB_Y+10*8,{&key_reverse}},
  {"USE"         ,S_KEY       ,m_scrn,KB_X,KB_Y+11*8,{&key_use},&mousebuse,&joybuse},
  {"JUMP/FLY UP" ,S_KEY       ,m_scrn,KB_X,KB_Y+12*8,{&key_flyup}},
  {"FLY DOWN"    ,S_KEY       ,m_scrn,KB_X,KB_Y+13*8,{&key_flydown}},
  {"MOUSE LOOK"  ,S_KEY       ,m_scrn,KB_X,KB_Y+17*8,{&key_mlook}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings2}},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

setup_menu_t keys_settings2[] =  // Key Binding screen strings
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},

  // phares 4/13/98:
  // key_help and key_escape can no longer be rebound. This keeps the
  // player from getting themselves in a bind where they can't remember how
  // to get to the menus, and can't remember how to get to the help screen
  // to give them a clue as to how to get to the menus. :)

  // Also, the keys assigned to these functions cannot be bound to other
  // functions. Introduce an S_KEEP flag to show that you cannot swap this
  // key with other keys in the same 'group'. (m_scrn, etc.)

  {"HELP"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{&key_help}},
  {"MENU"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{&key_menu_toggle}},
  // killough 10/98: hotkey for entering setup menu:
  {"SETUP"       ,S_KEY       ,m_scrn,KB_X,KB_Y+ 1*8,{&key_setup}},
  {"PAUSE"       ,S_KEY       ,m_scrn,KB_X,KB_Y+ 2*8,{&key_pause}},
  {"AUTOMAP"     ,S_KEY       ,m_scrn,KB_X,KB_Y+ 3*8,{&key_map}},
  {"VOLUME"      ,S_KEY       ,m_scrn,KB_X,KB_Y+ 4*8,{&key_soundvolume}},
  {"HUD"         ,S_KEY       ,m_scrn,KB_X,KB_Y+ 5*8,{&key_hud}},
  {"MESSAGES"    ,S_KEY       ,m_scrn,KB_X,KB_Y+ 6*8,{&key_messages}},
  {"GAMMA FIX"   ,S_KEY       ,m_scrn,KB_X,KB_Y+ 7*8,{&key_gamma}},
  {"SPY"         ,S_KEY       ,m_scrn,KB_X,KB_Y+ 8*8,{&key_spy}},
  {"LARGER VIEW" ,S_KEY       ,m_scrn,KB_X,KB_Y+ 9*8,{&key_zoomin}},
  {"SMALLER VIEW",S_KEY       ,m_scrn,KB_X,KB_Y+10*8,{&key_zoomout}},
  {"SCREENSHOT"  ,S_KEY       ,m_scrn,KB_X,KB_Y+11*8,{&key_screenshot}},
  {"GAME"        ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+12*8},
  {"SAVE"        ,S_KEY       ,m_scrn,KB_X,KB_Y+13*8,{&key_savegame}},
  {"LOAD"        ,S_KEY       ,m_scrn,KB_X,KB_Y+14*8,{&key_loadgame}},
  {"QUICKSAVE"   ,S_KEY       ,m_scrn,KB_X,KB_Y+15*8,{&key_quicksave}},
  {"QUICKLOAD"   ,S_KEY       ,m_scrn,KB_X,KB_Y+16*8,{&key_quickload}},
  {"END GAME"    ,S_KEY       ,m_scrn,KB_X,KB_Y+17*8,{&key_endgame}},
  {"QUIT"        ,S_KEY       ,m_scrn,KB_X,KB_Y+18*8,{&key_quit}},
  {"<- PREV", S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings1}},
  {"NEXT ->", S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings3}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings3[] =  // Key Binding screen strings
{
  {"WEAPONS" ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FIST"    ,S_KEY       ,m_scrn,KB_X,KB_Y+ 1*8,{&key_weapon1}},
  {"PISTOL"  ,S_KEY       ,m_scrn,KB_X,KB_Y+ 2*8,{&key_weapon2}},
  {"SHOTGUN" ,S_KEY       ,m_scrn,KB_X,KB_Y+ 3*8,{&key_weapon3}},
  {"CHAINGUN",S_KEY       ,m_scrn,KB_X,KB_Y+ 4*8,{&key_weapon4}},
  {"ROCKET"  ,S_KEY       ,m_scrn,KB_X,KB_Y+ 5*8,{&key_weapon5}},
  {"PLASMA"  ,S_KEY       ,m_scrn,KB_X,KB_Y+ 6*8,{&key_weapon6}},
  {"BFG",     S_KEY       ,m_scrn,KB_X,KB_Y+ 7*8,{&key_weapon7}},
  {"CHAINSAW",S_KEY       ,m_scrn,KB_X,KB_Y+ 8*8,{&key_weapon8}},
  {"SSG"     ,S_KEY       ,m_scrn,KB_X,KB_Y+ 9*8,{&key_weapon9}},
  {"NEXT"    ,S_KEY       ,m_scrn,KB_X,KB_Y+11*8,{&key_nextweapon}},
  {"PREVIOUS",S_KEY       ,m_scrn,KB_X,KB_Y+12*8,{&key_prevweapon}},
  {"BEST"    ,S_KEY       ,m_scrn,KB_X,KB_Y+13*8,{&key_weapontoggle}},
  {"FIRE"    ,S_KEY       ,m_scrn,KB_X,KB_Y+15*8,{&key_fire},&mousebfire,&joybfire},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings2}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings4}},

  // Final entry

  {0,S_SKIP|S_END,m_null}

};

setup_menu_t keys_settings4[] =  // Key Binding screen strings
{
  {"AUTOMAP"    ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FOLLOW"     ,S_KEY       ,m_map ,KB_X,KB_Y+ 1*8,{&key_map_follow}},
  {"ZOOM IN"    ,S_KEY       ,m_map ,KB_X,KB_Y+ 2*8,{&key_map_zoomin}},
  {"ZOOM OUT"   ,S_KEY       ,m_map ,KB_X,KB_Y+ 3*8,{&key_map_zoomout}},
  {"SHIFT UP"   ,S_KEY       ,m_map ,KB_X,KB_Y+ 4*8,{&key_map_up}},
  {"SHIFT DOWN" ,S_KEY       ,m_map ,KB_X,KB_Y+ 5*8,{&key_map_down}},
  {"SHIFT LEFT" ,S_KEY       ,m_map ,KB_X,KB_Y+ 6*8,{&key_map_left}},
  {"SHIFT RIGHT",S_KEY       ,m_map ,KB_X,KB_Y+ 7*8,{&key_map_right}},
  {"MARK PLACE" ,S_KEY       ,m_map ,KB_X,KB_Y+ 8*8,{&key_map_mark}},
  {"CLEAR MARKS",S_KEY       ,m_map ,KB_X,KB_Y+ 9*8,{&key_map_clear}},
  {"FULL/ZOOM"  ,S_KEY       ,m_map ,KB_X,KB_Y+10*8,{&key_map_gobig}},
  {"GRID"       ,S_KEY       ,m_map ,KB_X,KB_Y+11*8,{&key_map_grid}},
#ifdef GL_DOOM
  {"TEXTURED"   ,S_KEY       ,m_map ,KB_X,KB_Y+12*8,{&key_map_textured}},
#endif

  {"<- PREV" ,S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings3}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings5}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings5[] =  // Key Binding screen strings
{
  {"CHATTING"   ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+0*8},
  {"BEGIN CHAT" ,S_KEY       ,m_scrn,KB_X,KB_Y+1*8,{&key_chat}},
  {"PLAYER 1"   ,S_KEY       ,m_scrn,KB_X,KB_Y+2*8,{&destination_key_green}},
  {"PLAYER 2"   ,S_KEY       ,m_scrn,KB_X,KB_Y+3*8,{&destination_key_indigo}},
  {"PLAYER 3"   ,S_KEY       ,m_scrn,KB_X,KB_Y+4*8,{&destination_key_brown}},
  {"PLAYER 4"   ,S_KEY       ,m_scrn,KB_X,KB_Y+5*8,{&destination_key_red}},
  {"BACKSPACE"  ,S_KEY       ,m_scrn,KB_X,KB_Y+6*8,{&key_backspace}},
  {"ENTER"      ,S_KEY       ,m_scrn,KB_X,KB_Y+7*8,{&key_enter}},

  {"<- PREV" ,S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings4}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings6}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

//e6y
setup_menu_t keys_settings6[] =  // Key Binding screen strings
{
  {"GAME SPEED"           ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"SPEED UP"             ,S_KEY     ,m_scrn,KB_X,KB_Y+ 1*8,{&key_speed_up}},
  {"SPEED DOWN"           ,S_KEY     ,m_scrn,KB_X,KB_Y+ 2*8,{&key_speed_down}},
  {"RESET TO DEFAULT"     ,S_KEY     ,m_scrn,KB_X,KB_Y+ 3*8,{&key_speed_default}},
  {"STEP OF CHANGE (0-AUTO)" ,S_NUM  ,m_null,KB_X,KB_Y+ 4*8, {"speed_step"}},
  {"DEMOS"                ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+5*8},
  {"START/STOP SKIPPING"  ,S_KEY     ,m_scrn,KB_X,KB_Y+ 6*8,{&key_demo_skip}},
  {"END LEVEL"            ,S_KEY     ,m_scrn,KB_X,KB_Y+ 7*8,{&key_demo_endlevel}},
  {"CAMERA MODE"          ,S_KEY     ,m_scrn,KB_X,KB_Y+ 8*8,{&key_walkcamera}},
  {"JOIN"                 ,S_KEY     ,m_scrn,KB_X,KB_Y+ 9*8,{&key_demo_jointogame}},
  {"MISC"                 ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+10*8},
  {"RESTART CURRENT MAP"  ,S_KEY     ,m_scrn,KB_X,KB_Y+ 11*8,{&key_level_restart}},
  {"NEXT LEVEL"           ,S_KEY     ,m_scrn,KB_X,KB_Y+ 12*8,{&key_nextlevel}},
#ifdef GL_DOOM
  {"Show Alive Monsters"  ,S_KEY     ,m_scrn,KB_X,KB_Y+13*8,{&key_showalive}},
#endif

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings5}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings7}},
  // Final entry
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings7[] =
{
  {"MENUS"       ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+0*8},
  {"NEXT ITEM"   ,S_KEY       ,m_menu,KB_X,KB_Y+1*8,{&key_menu_down}},
  {"PREV ITEM"   ,S_KEY       ,m_menu,KB_X,KB_Y+2*8,{&key_menu_up}},
  {"LEFT"        ,S_KEY       ,m_menu,KB_X,KB_Y+3*8,{&key_menu_left}},
  {"RIGHT"       ,S_KEY       ,m_menu,KB_X,KB_Y+4*8,{&key_menu_right}},
  {"BACKSPACE"   ,S_KEY       ,m_menu,KB_X,KB_Y+5*8,{&key_menu_backspace}},
  {"SELECT ITEM" ,S_KEY       ,m_menu,KB_X,KB_Y+6*8,{&key_menu_enter}},
  {"EXIT"        ,S_KEY       ,m_menu,KB_X,KB_Y+7*8,{&key_menu_escape}},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings6}},
  // Final entry
  {0,S_SKIP|S_END,m_null}
};

// Setting up for the Key Binding screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_KeyBindings(int choice)
{
  MN_SetupNextMenu(&KeybndDef);

  setup_active = true;
  setup_screen = ss_keys;
  set_keybnd_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = keys_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the Key Bindings Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawKeybnd(void)
{
  menuactive = mnact_full;

  // Set up the Key Binding screen

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  HU_DrawTitle(84, 2, "M_KEYBND", CR_DEFAULT, "KEY BINDINGS", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    MN_DrawDefVerify();
}

/////////////////////////////
//
// The Weapon Screen tables.

// There's only one weapon settings screen (for now). But since we're
// trying to fit a common description for screens, it gets a setup_menu_t,
// which only has one screen definition in it.
//
// Note that this screen has no PREV or NEXT items, since there are no
// neighboring screens.

setup_menu_t weap_settings1[];

setup_menu_t* weap_settings[] =
{
  weap_settings1,
  NULL
};

setup_menu_t weap_settings1[] =  // Weapons Settings screen
{
  {"ENABLE RECOIL", S_YESNO,m_null,WP_X, WP_Y+ weap_recoil*8, {"weapon_recoil"}},
  {"ENABLE BOBBING",S_YESNO,m_null,WP_X, WP_Y+weap_bobbing*8, {"player_bobbing"}},

  {"1ST CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref1*8, {"weapon_choice_1"}},
  {"2nd CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref2*8, {"weapon_choice_2"}},
  {"3rd CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref3*8, {"weapon_choice_3"}},
  {"4th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref4*8, {"weapon_choice_4"}},
  {"5th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref5*8, {"weapon_choice_5"}},
  {"6th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref6*8, {"weapon_choice_6"}},
  {"7th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref7*8, {"weapon_choice_7"}},
  {"8th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref8*8, {"weapon_choice_8"}},
  {"9th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref9*8, {"weapon_choice_9"}},

  {"Enable Fist/Chainsaw\n& SG/SSG toggle", S_YESNO, m_null, WP_X,
   WP_Y+ weap_toggle*8, {"doom_weapon_toggles"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

// Setting up for the Weapons screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_Weapons(int choice)
{
  MN_SetupNextMenu(&WeaponDef);

  setup_active = true;
  setup_screen = ss_weap;
  set_weapon_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = weap_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}


// The drawing part of the Weapons Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawWeapons(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  HU_DrawTitle(109, 2, "M_WEAP", CR_DEFAULT, "WEAPONS", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    MN_DrawDefVerify();
}

/////////////////////////////
//
// The Status Bar / HUD tables.

// Screen table definitions

setup_menu_t stat_settings1[];
//e6y
setup_menu_t stat_settings2[];

setup_menu_t* stat_settings[] =
{
  stat_settings1,
  //e6y
  stat_settings2,
  NULL
};

setup_menu_t stat_settings1[] =  // Status Bar and HUD Settings screen
{
  {"STATUS BAR"        ,S_SKIP|S_TITLE,m_null,SB_X,SB_Y+ 1*8 },

  {"USE RED NUMBERS"   ,S_YESNO, m_null,SB_X,SB_Y+ 2*8, {"sts_always_red"}},
  {"GRAY %"            ,S_YESNO, m_null,SB_X,SB_Y+ 3*8, {"sts_pct_always_gray"}},
  {"SINGLE KEY DISPLAY",S_YESNO, m_null,SB_X,SB_Y+ 4*8, {"sts_traditional_keys"}},

  {"HEADS-UP DISPLAY"  ,S_SKIP|S_TITLE,m_null,SB_X,SB_Y+ 6*8},

  {"HEALTH LOW/OK"     ,S_NUM       ,m_null,SB_X,SB_Y+ 7*8, {"health_red"}},
  {"HEALTH OK/GOOD"    ,S_NUM       ,m_null,SB_X,SB_Y+ 8*8, {"health_yellow"}},
  {"HEALTH GOOD/EXTRA" ,S_NUM       ,m_null,SB_X,SB_Y+ 9*8, {"health_green"}},
  {"ARMOR LOW/OK"      ,S_NUM       ,m_null,SB_X,SB_Y+10*8, {"armor_red"}},
  {"ARMOR OK/GOOD"     ,S_NUM       ,m_null,SB_X,SB_Y+11*8, {"armor_yellow"}},
  {"ARMOR GOOD/EXTRA"  ,S_NUM       ,m_null,SB_X,SB_Y+12*8, {"armor_green"}},
  {"AMMO LOW/OK"       ,S_NUM       ,m_null,SB_X,SB_Y+13*8, {"ammo_red"}},
  {"AMMO OK/GOOD"      ,S_NUM       ,m_null,SB_X,SB_Y+14*8, {"ammo_yellow"}},
  {"BACKPACK CHANGES THRESHOLDS",S_CHOICE,m_null,SB_X,SB_Y+15*8, 
   {"ammo_colour_behaviour"},0,0,NULL,ammo_colour_behaviour_list},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  //e6y
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,SB_Y+20*8, {stat_settings2}},
  // Final entry
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t stat_settings2[] =
{
  {"ADVANCED HUD SETTINGS"       ,S_SKIP|S_TITLE,m_null,ADVHUD_X,SB_Y+1*8},
  {"SECRET AREAS"                ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 2*8, {"hudadd_secretarea"}},
  {"SMART TOTALS"                ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 3*8, {"hudadd_smarttotals"}},
  {"SHOW GAMESPEED"              ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 4*8, {"hudadd_gamespeed"}},
  {"SHOW LEVELTIME"              ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 5*8, {"hudadd_leveltime"}},
  {"SHOW DEMOTIME"               ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 6*8, {"hudadd_demotime"}},
  {"SHOW PROGRESS BAR DURING DEMO PLAYBACK" ,S_YESNO     ,m_null,ADVHUD_X,SB_Y+ 7*8, {"hudadd_demoprogressbar"}},

  {"CROSSHAIR SETTINGS"            ,S_SKIP|S_TITLE,m_null,ADVHUD_X,SB_Y+9*8},
  {"ENABLE CROSSHAIR"              ,S_CHOICE   ,m_null,ADVHUD_X,SB_Y+10*8, {"hudadd_crosshair"}, 0, 0, 0, crosshair_str},
  {"SCALE CROSSHAIR"               ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+11*8, {"hudadd_crosshair_scale"}},
  {"CHANGE CROSSHAIR COLOR BY PLAYER HEALTH" ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+12*8, {"hudadd_crosshair_health"}},
  {"CHANGE CROSSHAIR COLOR ON TARGET"        ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+13*8, {"hudadd_crosshair_target"}},
  {"LOCK CROSSHAIR ON TARGET"                ,S_YESNO    ,m_null,ADVHUD_X,SB_Y+14*8, {"hudadd_crosshair_lock_target"}},
  {"DEFAULT CROSSHAIR COLOR"                 ,S_CRITEM   ,m_null,ADVHUD_X,SB_Y+15*8, {"hudadd_crosshair_color"}},
  {"TARGET CROSSHAIR COLOR"                  ,S_CRITEM   ,m_null,ADVHUD_X,SB_Y+16*8, {"hudadd_crosshair_target_color"}},

  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},
  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,SB_Y+20*8, {stat_settings1}},
  {0,S_SKIP|S_END,m_null}
};

// Setting up for the Status Bar / HUD screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_StatusBar(int choice)
{
  MN_SetupNextMenu(&StatusHUDDef);

  setup_active = true;
  setup_screen = ss_stat;
  set_status_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = stat_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}


// The drawing part of the Status Bar / HUD Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawStatusHUD(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  HU_DrawTitle(59, 2, "M_STAT", CR_DEFAULT, "STATUS BAR / HUD", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    MN_DrawDefVerify();
}


/////////////////////////////
//
// The Automap tables.

setup_menu_t auto_settings1[];
setup_menu_t auto_settings2[];
setup_menu_t auto_settings3[];

setup_menu_t* auto_settings[] =
{
  auto_settings1,
  auto_settings2,
  auto_settings3,
  NULL
};

setup_menu_t auto_settings1[] =  // 1st AutoMap Settings screen
{
  {"Show Kills/Secrts/Items statistics",      S_YESNO,m_null,AU_X,AU_Y+0*8, {"map_level_stat"}},
  {"Show coordinates of automap pointer",     S_YESNO,m_null,AU_X,AU_Y+1*8, {"map_point_coord"}},  // killough 10/98
  {"Show Secrets only after entering",        S_YESNO,m_null,AU_X,AU_Y+2*8, {"map_secret_after"}},
  {"Update unexplored parts in automap mode", S_YESNO,m_null,AU_X,AU_Y+3*8, {"map_always_updates"}},
  {"Grid cell size (8..256)",                 S_NUM,  m_null,AU_X,AU_Y+4*8, {"map_grid_size"}},
  {"Scroll / Zoom speed  (1..32)",            S_NUM,  m_null,AU_X,AU_Y+5*8, {"map_scroll_speed"}},
  {"Use mouse wheel for zooming",             S_YESNO,m_null,AU_X,AU_Y+6*8, {"map_wheel_zoom"}},
  {"Apply multisampling",                     S_YESNO,m_null,AU_X,AU_Y+7*8, {"map_use_multisamling"}, 0, 0, MN_ChangeMapMultisamling},
#ifdef GL_DOOM
  {"Enable textured display",                 S_YESNO,m_null,AU_X,AU_Y+8*8, {"map_textured"}, 0, 0, MN_ChangeMapTextured},
  {"Things appearance",                       S_CHOICE,m_null,AU_X,AU_Y+9*8, {"map_things_appearance"}, 0, 0, NULL, map_things_appearance_list},
  {"Translucency percentage",                 S_SKIP|S_TITLE,m_null,AU_X,AU_Y+10*8},
  {"Textured automap",                        S_NUM,  m_null,AU_X,AU_Y+11*8, {"map_textured_trans"}},
  {"Textured automap in overlay mode",        S_NUM,  m_null,AU_X,AU_Y+12*8, {"map_textured_overlay_trans"}},
  {"Lines in overlay mode",                   S_NUM,  m_null,AU_X,AU_Y+13*8, {"map_lines_overlay_trans"}},
#endif

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,AU_NEXT,AU_Y+20*8, {auto_settings2}},

  // Final entry
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t auto_settings2[] =  // 2st AutoMap Settings screen
{
  {"background", S_COLOR, m_null, AU_X, AU_Y, {"mapcolor_back"}},
  {"grid lines", S_COLOR, m_null, AU_X, AU_Y + 1*8, {"mapcolor_grid"}},
  {"normal 1s wall", S_COLOR, m_null,AU_X,AU_Y+ 2*8, {"mapcolor_wall"}},
  {"line at floor height change", S_COLOR, m_null, AU_X, AU_Y+ 3*8, {"mapcolor_fchg"}},
  {"line at ceiling height change"      ,S_COLOR,m_null,AU_X,AU_Y+ 4*8, {"mapcolor_cchg"}},
  {"line at sector with floor = ceiling",S_COLOR,m_null,AU_X,AU_Y+ 5*8, {"mapcolor_clsd"}},
  {"red key"                            ,S_COLOR,m_null,AU_X,AU_Y+ 6*8, {"mapcolor_rkey"}},
  {"blue key"                           ,S_COLOR,m_null,AU_X,AU_Y+ 7*8, {"mapcolor_bkey"}},
  {"yellow key"                         ,S_COLOR,m_null,AU_X,AU_Y+ 8*8, {"mapcolor_ykey"}},
  {"red door"                           ,S_COLOR,m_null,AU_X,AU_Y+ 9*8, {"mapcolor_rdor"}},
  {"blue door"                          ,S_COLOR,m_null,AU_X,AU_Y+10*8, {"mapcolor_bdor"}},
  {"yellow door"                        ,S_COLOR,m_null,AU_X,AU_Y+11*8, {"mapcolor_ydor"}},

  {"AUTOMAP LEVEL TITLE COLOR"      ,S_CRITEM,m_null,AU_X,AU_Y+12*8, {"hudcolor_titl"}},
  {"AUTOMAP COORDINATES COLOR"      ,S_CRITEM,m_null,AU_X,AU_Y+13*8, {"hudcolor_xyco"}},

  {"Automap Statistics Titles Color",S_CRITEM,m_null,AU_X,AU_Y+14*8, {"hudcolor_mapstat_title"}},
  {"Automap Statistics Values Color",S_CRITEM,m_null,AU_X,AU_Y+15*8, {"hudcolor_mapstat_value"}},
  {"Automap Statistics Time Color"  ,S_CRITEM,m_null,AU_X,AU_Y+16*8, {"hudcolor_mapstat_time"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"<- PREV",S_SKIP|S_PREV,m_null,AU_PREV,AU_Y+20*8, {auto_settings1}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,AU_NEXT,AU_Y+20*8, {auto_settings3}},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

setup_menu_t auto_settings3[] =  // 3nd AutoMap Settings screen
{
  {"teleporter line"                ,S_COLOR ,m_null,AU_X,AU_Y, {"mapcolor_tele"}},
  {"secret sector boundary"         ,S_COLOR ,m_null,AU_X,AU_Y+ 1*8, {"mapcolor_secr"}},
  //jff 4/23/98 add exit line to automap
  {"exit line"                      ,S_COLOR ,m_null,AU_X,AU_Y+ 2*8, {"mapcolor_exit"}},
  {"computer map unseen line"       ,S_COLOR ,m_null,AU_X,AU_Y+ 3*8, {"mapcolor_unsn"}},
  {"line w/no floor/ceiling changes",S_COLOR ,m_null,AU_X,AU_Y+ 4*8, {"mapcolor_flat"}},
  {"general sprite"                 ,S_COLOR ,m_null,AU_X,AU_Y+ 5*8, {"mapcolor_sprt"}},
  {"countable enemy sprite"         ,S_COLOR ,m_null,AU_X,AU_Y+ 6*8, {"mapcolor_enemy"}},      // cph 2006/06/30
  {"countable item sprite"          ,S_COLOR ,m_null,AU_X,AU_Y+ 7*8, {"mapcolor_item"}},       // mead 3/4/2003
  {"crosshair"                      ,S_COLOR ,m_null,AU_X,AU_Y+ 8*8, {"mapcolor_hair"}},
  {"single player arrow"            ,S_COLOR ,m_null,AU_X,AU_Y+ 9*8, {"mapcolor_sngl"}},
  {"your colour in multiplayer"     ,S_COLOR ,m_null,AU_X,AU_Y+10*8, {"mapcolor_me"}},

  {"friends"                        ,S_COLOR ,m_null,AU_X,AU_Y+12*8, {"mapcolor_frnd"}},        // killough 8/8/98

  {"<- PREV",S_SKIP|S_PREV,m_null,AU_PREV,AU_Y+20*8, {auto_settings2}},

  // Final entry

  {0,S_SKIP|S_END,m_null}

};


// Setting up for the Automap screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_Automap(int choice)
{
  MN_SetupNextMenu(&AutoMapDef);

  setup_active = true;
  setup_screen = ss_auto;
  set_auto_active = true;
  setup_select = false;
  colorbox_active = false;
  default_verify = false;
  setup_gather = false;
  set_menu_itemon = 0;
  mult_screens_index = 0;
  current_setup_menu = auto_settings[0];
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// Data used by the color palette that is displayed for the player to
// select colors.

int color_palette_x; // X position of the cursor on the color palette
int color_palette_y; // Y position of the cursor on the color palette
unsigned char palette_background[16*(CHIP_SIZE+1)+8];

// MN_DrawColPal() draws the color palette when the user needs to select a
// color.

// phares 4/1/98: now uses a single lump for the palette instead of
// building the image out of individual paint chips.

static void MN_DrawColPal(void)
{
  int cpx, cpy;

  // Draw a background, border, and paint chips

  // proff/nicolas 09/20/98 -- changed for hi-res
  // CPhipps - patch drawing updated
  V_DrawNamePatch(COLORPALXORIG-5, COLORPALYORIG-5, 0, "M_COLORS", CR_DEFAULT, VPT_STRETCH);

  // Draw the cursor around the paint chip
  // (cpx,cpy) is the upper left-hand corner of the paint chip

  cpx = COLORPALXORIG+color_palette_x*(CHIP_SIZE+1)-1;
  cpy = COLORPALYORIG+color_palette_y*(CHIP_SIZE+1)-1;
  // proff 12/6/98: Drawing of colorchips completly changed for hi-res, it now uses a patch
  V_DrawNamePatch(cpx,cpy,0,"M_PALSEL",CR_DEFAULT,VPT_STRETCH); // PROFF_GL_FIX
}

// The drawing part of the Automap Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawAutoMap(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // CPhipps - patch drawing updated
  HU_DrawTitle(109, 2, "M_AUTO", CR_DEFAULT, "AUTOMAP", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If a color is being selected, need to show color paint chips

  if (colorbox_active)
    MN_DrawColPal();

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  else if (default_verify)
    MN_DrawDefVerify();
}


/////////////////////////////
//
// The Enemies table.

setup_menu_t enem_settings1[];

setup_menu_t* enem_settings[] =
{
  enem_settings1,
  NULL
};

setup_menu_t enem_settings1[] =  // Enemy Settings screen
{
  // killough 7/19/98
  {"Monster Infighting When Provoked",S_YESNO,m_null,E_X,E_Y+ enem_infighting*8, {"monster_infighting"}},

  {"Remember Previous Enemy",S_YESNO,m_null,E_X,E_Y+ enem_remember*8, {"monsters_remember"}},

  // killough 9/8/98
  {"Monster Backing Out",S_YESNO,m_null,E_X,E_Y+ enem_backing*8, {"monster_backing"}},

  {"Climb Steep Stairs", S_YESNO,m_null,E_X,E_Y+enem_monkeys*8, {"monkeys"}},

  // killough 9/9/98
  {"Intelligently Avoid Hazards",S_YESNO,m_null,E_X,E_Y+ enem_avoid_hazards*8, {"monster_avoid_hazards"}},

  // killough 10/98
  {"Affected by Friction",S_YESNO,m_null,E_X,E_Y+ enem_friction*8, {"monster_friction"}},

  {"Rescue Dying Friends",S_YESNO,m_null,E_X,E_Y+ enem_help_friends*8, {"help_friends"}},

#ifdef DOGS
  // killough 7/19/98
  {"Number Of Single-Player Helper Dogs",S_NUM|S_LEVWARN,m_null,E_X,E_Y+ enem_helpers*8, {"player_helpers"}},

  // killough 8/8/98
  {"Distance Friends Stay Away",S_NUM,m_null,E_X,E_Y+ enem_distfriend*8, {"friend_distance"}},

  {"Allow dogs to jump down",S_YESNO,m_null,E_X,E_Y+ enem_dog_jumping*8, {"dog_jumping"}},
#endif

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

/////////////////////////////

// Setting up for the Enemies screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_Enemy(int choice)
{
  MN_SetupNextMenu(&EnemyDef);

  setup_active = true;
  setup_screen = ss_enem;
  set_enemy_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = enem_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the Enemies Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawEnemy(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  HU_DrawTitle(114, 2, "M_ENEM", CR_DEFAULT, "ENEMIES", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    MN_DrawDefVerify();
}


/////////////////////////////
//
// The General table.
// killough 10/10/98

extern int usejoystick, usemouse, default_mus_card, default_snd_card;
extern int detect_voices, realtic_clock_rate, tran_filter_pct;

setup_menu_t gen_settings1[], gen_settings2[], gen_settings3[];
setup_menu_t gen_settings4[], gen_settings5[], gen_settings6[];
setup_menu_t gen_settings7[], gen_settings8[];

setup_menu_t* gen_settings[] =
{
  gen_settings1,
  gen_settings2,
  gen_settings3,
  gen_settings4,
  gen_settings5,
  gen_settings6,
  gen_settings7,
  gen_settings8,
  NULL
};

/*
static const char *videomodes[] = {
  "8bit","15bit","16bit", "32bit", "OpenGL", NULL};
*/

static const char *videomodes[] = {"32bit", "OpenGL", NULL};

static const char *gltexformats[] = {
  "GL_RGBA","GL_RGB5_A1", "GL_RGBA4", NULL};

setup_menu_t gen_settings1[] = { // General Settings screen1

  {"Video",                          S_SKIP|S_TITLE,     m_null, G_X, G_Y+ 1*8},
  {"Video mode",                     S_CHOICE,           m_null, G_X, G_Y+ 2*8, {"videomode"}, 0, 0, MN_ChangeVideoMode, videomodes},
  {"Screen Resolution",              S_CHOICE,           m_null, G_X, G_Y+ 3*8, {"screen_resolution"}, 0, 0, MN_ChangeVideoMode, screen_resolutions_list},
  {"Aspect Ratio",                   S_CHOICE,           m_null, G_X, G_Y+ 4*8, {"render_aspect"}, 0, 0, MN_ChangeAspectRatio, render_aspects_list},
  {"Fullscreen Video mode",          S_YESNO,            m_null, G_X, G_Y+ 5*8, {"use_fullscreen"}, 0, 0, MN_ChangeFullScreen},
  {"Status Bar and Menu Appearance", S_CHOICE,           m_null, G_X, G_Y+ 6*8, {"render_stretch_hud"}, 0, 0, MN_ChangeStretch, render_stretch_list},
#ifdef GL_DOOM
  {"Use GL surface for software mode",S_YESNO,           m_null, G_X, G_Y+ 7*8, {"use_gl_surface"}, 0, 0, MN_ChangeUseGLSurface},
  {"Vertical Sync",                  S_YESNO|S_PRGWARN,  m_null, G_X, G_Y+ 8*8, {"gl_vsync"}},
#endif
  
  {"Enable Translucency",            S_YESNO,            m_null, G_X, G_Y+10*8, {"translucency"}, 0, 0, MN_Trans},
  {"Translucency filter percentage", S_NUM,              m_null, G_X, G_Y+11*8, {"tran_filter_pct"}, 0, 0, MN_Trans},
  {"Uncapped Framerate",             S_YESNO,            m_null, G_X, G_Y+12*8, {"uncapped_framerate"}, 0, 0, MN_ChangeUncappedFrameRate},

  {"Sound & Music",                  S_SKIP|S_TITLE,     m_null, G_X, G_Y+14*8},
  {"Number of Sound Channels",       S_NUM|S_PRGWARN,    m_null, G_X, G_Y+15*8, {"snd_channels"}},
  {"Enable v1.1 Pitch Effects",      S_YESNO,            m_null, G_X, G_Y+16*8, {"pitched_sounds"}},
//#ifdef HAVE_LIBSDL_MIXER
  {"PC Speaker emulation",           S_YESNO|S_PRGWARN,  m_null, G_X, G_Y+17*8, {"snd_pcspeaker"}},
//#endif
  {"Preferred MIDI player",          S_CHOICE|S_PRGWARN, m_null, G_X, G_Y+18*8, {"snd_midiplayer"}, 0, 0, MN_ChangeMIDIPlayer, midiplayers},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings2}},
  {0,S_SKIP|S_END,m_null}
};

static const char *gen_skillstrings[] = {
  // Dummy first option because defaultskill is 1-based
  "", "ITYTD", "HNTR", "HMP", "UV", "NM", NULL
};

static const char *gen_compstrings[] =
{
  "Default",
  "Doom v1.2",
  "Doom v1.666",
  "Doom/2 v1.9",
  "Ultimate Doom",
  "Final Doom",
  "DosDoom",
  "TASDoom",
  "Boom's vanilla",
  "Boom v2.01",
  "Boom",
  "LxDoom",
  "MBF",
  "PrBoom 2.03b",
  "PrBoom 2.1.x",
  "PrBoom 2.2.x",
  "PrBoom 2.3.x",
  "PrBoom 2.4.0",
  "Latest PrBoom+",
  NULL
};

setup_menu_t gen_settings2[] = { // General Settings screen2

  {"Input Devices",                    S_SKIP|S_TITLE, m_null, G_X, G_Y+ 1*8},
  {"Enable Mouse",                     S_YESNO, m_null, G_X, G_Y+ 2*8, {"use_mouse"}},
  {"Enable Joystick",                  S_YESNO, m_null, G_X, G_Y+ 3*8, {"use_joystick"}},

  {"Files Preloaded at Game Startup",  S_SKIP|S_TITLE, m_null, G_X, G_Y + 5*8},
  {"WAD # 1",                          S_FILE, m_null, GF_X, G_Y+ 6*8, {"wadfile_1"}}, 
  {"WAD #2",                           S_FILE, m_null, GF_X, G_Y+ 7*8, {"wadfile_2"}},
  {"DEH/BEX # 1",                      S_FILE, m_null, GF_X, G_Y+ 8*8, {"dehfile_1"}},
  {"DEH/BEX #2",                       S_FILE, m_null, GF_X, G_Y+ 9*8, {"dehfile_2"}},

  {"Miscellaneous",                    S_SKIP|S_TITLE,  m_null, G_X, G_Y+11*8},
  {"Maximum number of player corpses", S_NUM|S_PRGWARN, m_null, G_X, G_Y+12*8, {"max_player_corpse"}},
  {"Game speed, percentage of normal", S_NUM|S_PRGWARN, m_null, G_X, G_Y+13*8, {"realtic_clock_rate"}},
  {"Default skill level",              S_CHOICE,        m_null, G_X, G_Y+14*8, {"default_skill"}, 0, 0, NULL, gen_skillstrings},
  {"Default compatibility level",      S_CHOICE,        m_null, G_X, G_Y+15*8, {"default_compatibility_level"}, 0, 0, NULL, &gen_compstrings[1]},
  {"Show ENDOOM screen",               S_YESNO,         m_null, G_X, G_Y+16*8, {"showendoom"}},
  {"Fullscreen menu background",       S_YESNO, m_null, G_X, G_Y + 17*8, {"menu_background"}},
#ifdef USE_WINDOWS_LAUNCHER
  {"Use In-Game Launcher",             S_CHOICE,        m_null, G_X, G_Y+ 18*8, {"launcher_enable"}, 0, 0, NULL, launcher_enable_states},
#endif


  {"<- PREV",S_SKIP|S_PREV, m_null, KB_PREV, KB_Y+20*8, {gen_settings1}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings3}},
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t gen_settings3[] = { // General Settings screen2
  {"Demos",                       S_SKIP|S_TITLE, m_null, G_X, G_Y+ 1*8},
  {"Use Extended Format",         S_YESNO|S_PRGWARN, m_null,G_X,G_Y+ 2*8, {"demo_extendedformat"}, 0, 0, MN_ChangeDemoExtendedFormat},
  {"Overwrite Existing",          S_YESNO, m_null, G_X, G_Y+ 3*8, {"demo_overwriteexisting"}},
  {"Smooth Demo Playback",        S_YESNO, m_null, G_X, G_Y+ 4*8, {"demo_smoothturns"}, 0, 0, MN_ChangeDemoSmoothTurns},
  {"Smooth Demo Playback Factor", S_NUM,   m_null, G_X, G_Y+ 5*8, {"demo_smoothturnsfactor"}, 0, 0, MN_ChangeDemoSmoothTurns},

  {"Movements",                   S_SKIP|S_TITLE,m_null,G_X, G_Y+7*8},
  {"Permanent Strafe50",          S_YESNO, m_null, G_X, G_Y+ 8*8, {"movement_strafe50"}, 0, 0, MN_ChangeSpeed},
  {"Strafe50 On Turns",           S_YESNO, m_null, G_X, G_Y+ 9*8, {"movement_strafe50onturns"}, 0, 0, MN_ChangeSpeed},

  {"Mouse",                       S_SKIP|S_TITLE,m_null, G_X, G_Y+11*8},
  {"Dbl-Click As Use",            S_YESNO, m_null, G_X, G_Y+12*8, {"mouse_doubleclick_as_use"}},

  {"Enable Mouselook",            S_YESNO, m_null, G_X, G_Y+13*8, {"movement_mouselook"}, 0, 0, MN_ChangeMouseLook},
  {"Invert Mouse",                S_YESNO, m_null, G_X, G_Y+14*8, {"movement_mouseinvert"}, 0, 0, MN_ChangeMouseInvert},
  {"Max View Pitch",              S_NUM,   m_null, G_X, G_Y+15*8, {"movement_maxviewpitch"}, 0, 0, MN_ChangeMaxViewPitch},

  {"<- PREV",S_SKIP|S_PREV, m_null,KB_PREV, KB_Y+20*8, {gen_settings2}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings4}},
  {0,S_SKIP|S_END,m_null}
};

static const char *renderfilters[] = {"none", "point", "linear", "rounded"};
static const char *edgetypes[] = {"jagged", "sloped"};

setup_menu_t gen_settings4[] = { // General Settings screen3
  {"Display Options",          S_SKIP|S_TITLE, m_null, G_X, G_Y+ 1*8},
  {"Filter for walls",           S_CHOICE, m_null, G_X, G_Y+ 2*8, {"filter_wall"}, 0, 0, NULL, renderfilters},
  {"Filter for floors/ceilings", S_CHOICE, m_null, G_X, G_Y+ 3*8, {"filter_floor"}, 0, 0, NULL, renderfilters},
  {"Filter for sprites",         S_CHOICE, m_null, G_X, G_Y+ 4*8, {"filter_sprite"}, 0, 0, NULL, renderfilters},
  {"Filter for patches",         S_CHOICE, m_null, G_X, G_Y+ 5*8, {"filter_patch"}, 0, 0, NULL, renderfilters},
  {"Filter for lighting",        S_CHOICE, m_null, G_X, G_Y+ 6*8, {"filter_z"}, 0, 0, NULL, renderfilters},

  {"Drawing of sprite edges",    S_CHOICE, m_null, G_X, G_Y+ 8*8, {"sprite_edges"}, 0, 0, NULL, edgetypes},
  {"Drawing of patch edges",     S_CHOICE, m_null, G_X, G_Y+ 9*8, {"patch_edges"}, 0, 0, NULL, edgetypes},
  {"Flashing HOM indicator",     S_YESNO,  m_null, G_X, G_Y+10*8, {"flashing_hom"}},

  // prboom-plus 
  {"Rendering quality",          S_CHOICE, m_null, G_X, G_Y+12*8, {"render_precise"}, 0, 0, MN_ChangeRenderPrecise, render_precises},
  {"Wipe Screen Effect",         S_YESNO,  m_null, G_X, G_Y+13*8, {"render_wipescreen"}},
  {"Change Palette On Pain",     S_YESNO,  m_null, G_X, G_Y+15*8, {"palette_ondamage"}, 0, 0, MN_ChangeApplyPalette},
  {"Change Palette On Bonus",    S_YESNO,  m_null, G_X, G_Y+16*8, {"palette_onbonus"}, 0, 0, MN_ChangeApplyPalette},
  {"Change Palette On Powers",   S_YESNO,  m_null, G_X, G_Y+17*8, {"palette_onpowers"}, 0, 0, MN_ChangeApplyPalette},

  {"<- PREV",S_SKIP|S_PREV, m_null, KB_PREV, KB_Y+20*8, {gen_settings3}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings5}},
  // Final entry

  {0,S_SKIP|S_END,m_null}
};

setup_menu_t gen_settings5[] = { // General Settings screen3
  {"Software Options",               S_SKIP|S_TITLE, m_null, G_X, G_Y+1*8},
  {"Screen Multiple Factor (1-None)", S_NUM,m_null,G_X,G_Y+2*8, {"render_screen_multiply"}, 0, 0, MN_ChangeScreenMultipleFactor},
  {"Interlaced Scanning",       S_YESNO,  m_null, G_X, G_Y+3*8, {"render_interlaced_scanning"}, 0, 0, MN_ChangeInterlacedScanning},
#ifdef GL_DOOM
  {"OpenGL Options",             S_SKIP|S_TITLE,m_null,G_X,G_Y+5*8},
  {"Multisampling (0-None)",    S_NUM|S_PRGWARN|S_CANT_GL_ARB_MULTISAMPLEFACTOR,m_null,G_X,G_Y+6*8, {"render_multisampling"}, 0, 0, MN_ChangeMultiSample},
  {"Field Of View",             S_NUM,    m_null, G_X, G_Y+ 7*8, {"render_fov"}, 0, 0, MN_ChangeFOV},
  {"Sector Light Mode",         S_CHOICE, m_null, G_X, G_Y+ 8*8, {"gl_lightmode"}, 0, 0, MN_ChangeLightMode, gl_lightmodes},
  {"Allow Fog",                 S_YESNO,  m_null, G_X, G_Y+ 9*8, {"gl_fog"}, 0, 0, MN_ChangeAllowFog},
  {"Simple Shadows",            S_YESNO,  m_null, G_X, G_Y+10*8, {"gl_shadows"}},

  {"Paper Items",               S_YESNO,  m_null, G_X, G_Y+12*8, {"render_paperitems"}},
  {"Smooth sprite edges",       S_YESNO,  m_null, G_X, G_Y+13*8, {"gl_sprite_blend"}},
  {"Adjust Sprite Clipping",    S_CHOICE, m_null, G_X, G_Y+14*8, {"gl_spriteclip"}, 0, 0, MN_ChangeSpriteClip, gl_spriteclipmodes},
  {"Item out of Floor offset",  S_NUM,    m_null, G_X, G_Y+15*8, {"gl_sprite_offset"}, 0, 0, MN_ChangeSpriteClip},
  {"Health Bar Above Monsters", S_YESNO,  m_null, G_X, G_Y+16*8, {"health_bar"}},
#endif

  {"<- PREV",S_SKIP|S_PREV, m_null,KB_PREV, KB_Y+20*8, {gen_settings4}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings6}},
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t gen_settings6[] =
{
  {"EMULATION"                         ,S_SKIP|S_TITLE,m_null,G_X2,G_Y+ 1*8},
  {"WARN ON SPECHITS OVERFLOW"         ,S_YESNO     ,m_null,G_X2,G_Y+ 2*8, {"overrun_spechit_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 3*8, {"overrun_spechit_emulate"}},
  {"WARN ON REJECT OVERFLOW"           ,S_YESNO     ,m_null,G_X2,G_Y+ 4*8, {"overrun_reject_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 5*8, {"overrun_reject_emulate"}},
  {"WARN ON INTERCEPTS OVERFLOW"       ,S_YESNO     ,m_null,G_X2,G_Y+ 6*8, {"overrun_intercept_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 7*8, {"overrun_intercept_emulate"}},
  {"WARN ON PLAYERINGAME OVERFLOW"     ,S_YESNO     ,m_null,G_X2,G_Y+ 8*8, {"overrun_playeringame_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+ 9*8, {"overrun_playeringame_emulate"}},
  {"WARN ON DONUT OVERFLOW"            ,S_YESNO     ,m_null,G_X2,G_Y+10*8, {"overrun_donut_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+11*8, {"overrun_donut_emulate"}},
  {"WARN ON MISSEDBACKSIDE OVERFLOW"   ,S_YESNO     ,m_null,G_X2,G_Y+12*8, {"overrun_missedbackside_warn"}},
  {"TRY TO EMULATE IT"                 ,S_YESNO     ,m_null,G_X2,G_Y+13*8, {"overrun_missedbackside_emulate"}},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {gen_settings5}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings7}},
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t gen_settings7[] =
{
  {"COMPATIBILITY WITH COMMON MAPPING ERRORS" ,S_SKIP|S_TITLE,m_null,G_X2,G_Y+1*8},
  {"LINEDEFS W/O TAGS APPLY LOCALLY"   ,S_YESNO     ,m_null,G_X2,G_Y+2*8, {"comperr_zerotag"}},
  {"USE PASSES THRU ALL SPECIAL LINES" ,S_YESNO     ,m_null,G_X2,G_Y+3*8, {"comperr_passuse"}},
  {"WALK UNDER SOLID HANGING BODIES"   ,S_YESNO     ,m_null,G_X2,G_Y+4*8, {"comperr_hangsolid"}},
  {"FIX CLIPPING PROBLEMS IN LARGE LEVELS" ,S_YESNO ,m_null,G_X2,G_Y+5*8, {"comperr_blockmap"}},
  {"ALLOW JUMP"                        ,S_YESNO     ,m_null,G_X2,G_Y+6*8, {"comperr_allowjump"}},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {gen_settings6}},
#ifdef GL_DOOM
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {gen_settings8}},
#endif
  {0,S_SKIP|S_END,m_null}
};

static const char *gltexfilters[] = {
  "None", "Linear", "Nearest Mipmap", "Linear Mipmap", "Bilinear", "Trilinear", NULL};

static const char *gltexfilters_anisotropics[] = 
  {"Off", "2x", "4x", "8x", "16x", NULL};

setup_menu_t gen_settings8[] = { // General Settings screen4
#ifdef GL_DOOM
  {"Texture Options",  S_SKIP|S_TITLE,m_null,G_X,G_Y+ 1*8},
  {"Texture Filter Mode",        S_CHOICE, m_null, G_X, G_Y+2 *8, {"gl_texture_filter"}, 0, 0, MN_ChangeTextureParams, gltexfilters},
  {"Spritre Filter Mode",        S_CHOICE, m_null, G_X, G_Y+3 *8, {"gl_sprite_filter"}, 0, 0, MN_ChangeTextureParams, gltexfilters},
  {"Patch Filter Mode",          S_CHOICE, m_null, G_X, G_Y+4 *8, {"gl_patch_filter"}, 0, 0, MN_ChangeTextureParams, gltexfilters},
  {"Anisotropic filter",         S_CHOICE, m_null, G_X, G_Y+5 *8, {"gl_texture_filter_anisotropic"}, 0, 0, MN_ChangeTextureParams, gltexfilters_anisotropics},
  {"Texture format",             S_CHOICE, m_null, G_X, G_Y+6 *8, {"gl_tex_format_string"}, 0, 0, MN_ChangeTextureParams, gltexformats},

  {"Enable Colormaps",           S_YESNO, m_null, G_X,G_Y+ 8*8, {"gl_boom_colormaps"}, 0, 0, MN_ChangeAllowBoomColormaps},
  {"Enable Internal Hi-Res",     S_YESNO, m_null, G_X,G_Y+ 9*8, {"gl_texture_internal_hires"}, 0, 0, MN_ChangeTextureUseHires},
  {"Enable External Hi-Res",     S_YESNO, m_null, G_X,G_Y+10*8, {"gl_texture_external_hires"}, 0, 0, MN_ChangeTextureUseHires},
  {"Override PWAD's graphics with Hi-Res" ,S_YESNO|S_PRGWARN,m_null,G_X,G_Y+11*8, {"gl_hires_override_pwads"}, 0, 0, MN_ChangeTextureUseHires},

  {"Enable High Quality Resize", S_YESNO,  m_null, G_X, G_Y+13*8, {"gl_texture_hqresize"}, 0, 0, MN_ChangeTextureHQResize},
  {"Resize textures",            S_CHOICE, m_null, G_X, G_Y+14*8, {"gl_texture_hqresize_textures"}, 0, 0, MN_ChangeTextureHQResize, gl_hqresizemodes},
  {"Resize sprites",             S_CHOICE, m_null, G_X, G_Y+15*8, {"gl_texture_hqresize_sprites"}, 0, 0, MN_ChangeTextureHQResize, gl_hqresizemodes},
  {"Resize patches",             S_CHOICE, m_null, G_X, G_Y+16*8, {"gl_texture_hqresize_patches"}, 0, 0, MN_ChangeTextureHQResize, gl_hqresizemodes},

  {"Allow Detail Textures",      S_YESNO,  m_null, G_X, G_Y+18*8, {"gl_allow_detail_textures"}, 0, 0, MN_ChangeUseDetail},
  {"Blend Animations",           S_YESNO,  m_null, G_X, G_Y+19*8, {"gl_blend_animations"}},
#endif //GL_DOOM

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {gen_settings7}},
  {0,S_SKIP|S_END,m_null}
};

void MN_Trans(void) // To reset translucency after setting it in menu
{
  general_translucency = default_translucency; //e6y: Fix for "translucency won't change until you restart the engine"

  if (general_translucency)
    R_InitTranMap(0);
}

// To (un)set fullscreen video after menu changes
void MN_ChangeFullScreen(void)
{
  V_ToggleFullscreen();
}

void MN_ChangeVideoMode(void)
{
  V_ChangeScreenResolution();
}

void MN_ChangeUseGLSurface(void)
{
  V_ChangeScreenResolution();
}

void MN_ChangeDemoSmoothTurns(void)
{
  R_SmoothPlaying_Reset(NULL);
}

void MN_ChangeTextureParams(void)
{
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    gld_InitTextureParams();
    gld_FlushTextures();
  }
#endif
}

// Setting up for the General screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_General(int choice)
{
  MN_SetupNextMenu(&GeneralDef);

  setup_active = true;
  setup_screen = ss_gen;
  set_general_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = gen_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the General Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawGeneral(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  HU_DrawTitle(114, 2, "M_GENERL", CR_DEFAULT, "GENERAL", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    MN_DrawDefVerify();
}

/////////////////////////////
//
// The Compatibility table.
// killough 10/10/98

setup_menu_t comp_settings1[], comp_settings2[], comp_settings3[];
setup_menu_t comp_settings3[];//e6y

setup_menu_t* comp_settings[] =
{
  comp_settings1,
  comp_settings2,
  comp_settings3,
  NULL
};

setup_menu_t comp_settings1[] =  // Compatibility Settings screen #1
{
  {"Any monster can telefrag on MAP30", S_YESNO, m_null, C_X,
   C_Y + compat_telefrag * COMP_SPC, {"comp_telefrag"}},

  {"Some objects never hang over tall ledges", S_YESNO, m_null, C_X,
   C_Y + compat_dropoff * COMP_SPC, {"comp_dropoff"}},

  {"Objects don't fall under their own weight", S_YESNO, m_null, C_X,
   C_Y + compat_falloff * COMP_SPC, {"comp_falloff"}},

  {"Monsters randomly walk off of moving lifts", S_YESNO, m_null, C_X,
   C_Y + compat_staylift * COMP_SPC, {"comp_staylift"}},

  {"Monsters get stuck on doortracks", S_YESNO, m_null, C_X,
   C_Y + compat_doorstuck * COMP_SPC, {"comp_doorstuck"}},

  {"Monsters don't give up pursuit of targets", S_YESNO, m_null, C_X,
   C_Y + compat_pursuit * COMP_SPC, {"comp_pursuit"}},

  {"Arch-Vile resurrects invincible ghosts", S_YESNO, m_null, C_X,
   C_Y + compat_vile * COMP_SPC, {"comp_vile"}},

  {"Pain Elementals limited to 21 lost souls", S_YESNO, m_null, C_X,
   C_Y + compat_pain * COMP_SPC, {"comp_pain"}},

  {"Lost souls get stuck behind walls", S_YESNO, m_null, C_X,
   C_Y + compat_skull * COMP_SPC, {"comp_skull"}},

  {"Blazing doors make double closing sounds", S_YESNO, m_null, C_X,
   C_Y + compat_blazing * COMP_SPC, {"comp_blazing"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT, m_null, KB_NEXT, C_Y+C_NEXTPREV, {comp_settings2}},

  // Final entry
  {0,S_SKIP|S_END,m_null}
};

setup_menu_t comp_settings2[] =  // Compatibility Settings screen #2
{
  {"Tagged doors don't trigger special lighting", S_YESNO, m_null, C_X,
   C_Y + compat_doorlight * COMP_SPC, {"comp_doorlight"}},

  {"God mode isn't absolute", S_YESNO, m_null, C_X,
   C_Y + compat_god * COMP_SPC, {"comp_god"}},

  {"Powerup cheats are not infinite duration", S_YESNO, m_null, C_X,
   C_Y + compat_infcheat * COMP_SPC, {"comp_infcheat"}},

  {"Dead players can exit levels", S_YESNO, m_null, C_X,
   C_Y + compat_zombie * COMP_SPC, {"comp_zombie"}},

  {"Sky is unaffected by invulnerability", S_YESNO, m_null, C_X,
   C_Y + compat_skymap * COMP_SPC, {"comp_skymap"}},

  {"Use exactly Doom's stairbuilding method", S_YESNO, m_null, C_X,
   C_Y + compat_stairs * COMP_SPC, {"comp_stairs"}},

  {"Use exactly Doom's floor motion behavior", S_YESNO, m_null, C_X,
   C_Y + compat_floors * COMP_SPC, {"comp_floors"}},

  {"Use exactly Doom's movement clipping code", S_YESNO, m_null, C_X,
   C_Y + compat_moveblock * COMP_SPC, {"comp_moveblock"}},

  {"Use exactly Doom's linedef trigger model", S_YESNO, m_null, C_X,
   C_Y + compat_model * COMP_SPC, {"comp_model"}},

  {"Linedef effects work with sector tag = 0", S_YESNO, m_null, C_X,
   C_Y + compat_zerotags * COMP_SPC, {"comp_zerotags"}},

  {"NEXT ->",S_SKIP|S_NEXT, m_null, KB_NEXT, C_Y+C_NEXTPREV, {comp_settings3}},//e6y
  {"<- PREV", S_SKIP|S_PREV, m_null, KB_PREV, C_Y+C_NEXTPREV,{comp_settings1}},

  {"NEXT ->",S_SKIP|S_NEXT, m_null, KB_NEXT, C_Y+C_NEXTPREV, {comp_settings3}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

//e6y
setup_menu_t comp_settings3[] =  // Compatibility Settings screen #3
{
  {"Emulate pre-Ultimate BossDeath behaviour", S_YESNO, m_null, C_X,
   C_Y + compat_666 * COMP_SPC, {"comp_666"}},

  {"Lost souls don't bounce off flat surfaces", S_YESNO, m_null, C_X,
   C_Y + compat_soul * COMP_SPC, {"comp_soul"}},

  {"2S middle textures do not animate", S_YESNO, m_null, C_X,
   C_Y + compat_maskedanim * COMP_SPC, {"comp_maskedanim"}},

  //e6y
  {"Retain quirks in Doom's sound code", S_YESNO, m_null, C_X,
   C_Y + compat_sound * COMP_SPC, {"comp_sound"}},
  {"PrBoom-Plus Settings", S_SKIP|S_TITLE,m_null,C_X,
   C_Y + compat_plussettings * COMP_SPC},
  {"Use Doom's buggy \"Ouch\" face code", S_YESNO, m_null, C_X,
   C_Y + compat_ouchface * COMP_SPC, {"comp_ouchface"}},
  {"Max Health in DEH applies only to potions", S_YESNO, m_null, C_X,
   C_Y + compat_maxhealth * COMP_SPC, {"comp_maxhealth"}},
  {"No predefined translucency for some things", S_YESNO, m_null, C_X,
   C_Y + compat_translucency * COMP_SPC, {"comp_translucency"}},
  {"<- PREV", S_SKIP|S_PREV, m_null, KB_PREV, C_Y+C_NEXTPREV,{comp_settings2}},
  {0,S_SKIP|S_END,m_null}
};

// Setting up for the Compatibility screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_Compat(int choice)
{
  MN_SetupNextMenu(&CompatDef);

  setup_active = true;
  setup_screen = ss_comp;
  set_general_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = comp_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the Compatibility Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawCompat(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  HU_DrawTitle(52, 2, "M_COMPAT", CR_DEFAULT, "DOOM COMPATIBILITY", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    MN_DrawDefVerify();
}

/////////////////////////////
//
// The Messages table.

setup_menu_t mess_settings1[];

setup_menu_t* mess_settings[] =
{
  mess_settings1,
  NULL
};

setup_menu_t mess_settings1[] =  // Messages screen
{
  {"Message Color During Play", S_CRITEM, m_null, M_X,
   M_Y + mess_color_play*8, {"hudcolor_mesg"}},

#if 0
  {"Message Duration During Play (ms)", S_NUM, m_null, M_X,
   M_Y  + mess_timer*8, {"message_timer"}},
#endif

  {"Chat Message Color", S_CRITEM, m_null, M_X,
   M_Y + mess_color_chat*8, {"hudcolor_chat"}},

#if 0
  {"Chat Message Duration (ms)", S_NUM, m_null, M_X,
   M_Y  + mess_chat_timer*8, {"chat_msg_timer"}},
#endif

  {"Message Review Color", S_CRITEM, m_null, M_X,
   M_Y + mess_color_review*8, {"hudcolor_list"}},

#if 0
  {"Message Listing Review is Temporary",  S_YESNO,  m_null,  M_X,
   M_Y + mess_timed*8, {"hud_msg_timed"}},

  {"Message Review Duration (ms)", S_NUM, m_null, M_X,
   M_Y  + mess_hud_timer*8, {"hud_msg_timer"}},
#endif

  {"Number of Review Message Lines", S_NUM, m_null,  M_X,
   M_Y + mess_lines*8, {"hud_msg_lines"}},

#if 0
  {"Message Listing Scrolls Upwards",  S_YESNO,  m_null,  M_X,
   M_Y + mess_scrollup*8, {"hud_msg_scrollup"}},
#endif

  {"Message Background",  S_YESNO,  m_null,  M_X,
   M_Y + mess_background*8, {"hud_list_bgon"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};


// Setting up for the Messages screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_Messages(int choice)
{
  MN_SetupNextMenu(&MessageDef);

  setup_active = true;
  setup_screen = ss_mess;
  set_mess_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = mess_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}


// The drawing part of the Messages Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawMessages(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // CPhipps - patch drawing updated
  HU_DrawTitle(103, 2, "M_MESS", CR_DEFAULT, "MESSAGES", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);
  if (default_verify)
    MN_DrawDefVerify();
}


/////////////////////////////
//
// The Chat Strings table.

setup_menu_t chat_settings1[];

setup_menu_t* chat_settings[] =
{
  chat_settings1,
  NULL
};

setup_menu_t chat_settings1[] =  // Chat Strings screen
{
  {"1",S_CHAT,m_null,CS_X,CS_Y+ 1*8, {"chatmacro1"}},
  {"2",S_CHAT,m_null,CS_X,CS_Y+ 2*8, {"chatmacro2"}},
  {"3",S_CHAT,m_null,CS_X,CS_Y+ 3*8, {"chatmacro3"}},
  {"4",S_CHAT,m_null,CS_X,CS_Y+ 4*8, {"chatmacro4"}},
  {"5",S_CHAT,m_null,CS_X,CS_Y+ 5*8, {"chatmacro5"}},
  {"6",S_CHAT,m_null,CS_X,CS_Y+ 6*8, {"chatmacro6"}},
  {"7",S_CHAT,m_null,CS_X,CS_Y+ 7*8, {"chatmacro7"}},
  {"8",S_CHAT,m_null,CS_X,CS_Y+ 8*8, {"chatmacro8"}},
  {"9",S_CHAT,m_null,CS_X,CS_Y+ 9*8, {"chatmacro9"}},
  {"0",S_CHAT,m_null,CS_X,CS_Y+10*8, {"chatmacro0"}},

  // Button for resetting to defaults
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  // Final entry
  {0,S_SKIP|S_END,m_null}

};

// Setting up for the Chat Strings screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void MN_ChatStrings(int choice)
{
  MN_SetupNextMenu(&ChatStrDef);
  setup_active = true;
  setup_screen = ss_chat;
  set_chat_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = chat_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

// The drawing part of the Chat Strings Setup initialization. Draw the
// background, title, instruction line, and items.

void MN_DrawChatStrings(void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0); // Draw background

  // CPhipps - patch drawing updated
  HU_DrawTitle(83, 2, "M_CHAT", CR_DEFAULT, "CHAT STRINGS", CR_GOLD);
  MN_DrawInstructions();
  MN_DrawScreenItems(current_setup_menu);

  // If the Reset Button has been selected, an "Are you sure?" message
  // is overlayed across everything else.

  if (default_verify)
    MN_DrawDefVerify();
}

/////////////////////////////
//
// General routines used by the Setup screens.
//

// phares 4/17/98:
// MN_SelectDone() gets called when you have finished entering your
// Setup Menu item change.

static void MN_SelectDone(setup_menu_t* ptr)
{
  ptr->m_flags &= ~S_SELECT;
  ptr->m_flags |= S_HILITE;
  S_StartSound(NULL, sfx_itemup);
  setup_select = false;
  colorbox_active = false;
  if (print_warning_about_changes)     // killough 8/15/98
    print_warning_about_changes--;
}

// phares 4/21/98:
// Array of setup screens used by MN_ResetDefaults()

static setup_menu_t **setup_screens[] =
{
  keys_settings,
  weap_settings,
  stat_settings,
  auto_settings,
  enem_settings,
  mess_settings,
  chat_settings,
  gen_settings,      // killough 10/98
  comp_settings,
};

// phares 4/19/98:
// MN_ResetDefaults() resets all values for a setup screen to default values
//
// killough 10/98: rewritten to fix bugs and warn about pending changes

static void MN_ResetDefaults(void)
{
  int i; //e6y

  default_t *dp;
  int warn = 0;

  // Look through the defaults table and reset every variable that
  // belongs to the group we're interested in.
  //
  // killough: However, only reset variables whose field in the
  // current setup screen is the same as in the defaults table.
  // i.e. only reset variables really in the current setup screen.

  // e6y
  // Fixed crash while trying to read data past array end
  // All previous versions of prboom worked only by a lucky accident
  // old code: for (dp = defaults; dp->name; dp++)
  for (i = 0; i < numdefaults ; i++)
  {
    dp = &defaults[i];

    if (dp->setupscreen == setup_screen)
      {
  setup_menu_t **l, *p;
  for (l = setup_screens[setup_screen-1]; *l; l++)
    for (p = *l; !(p->m_flags & S_END); p++)
      if (p->m_flags & S_HASDEFPTR ? p->var.def == dp :
    p->var.m_key == dp->location.pi ||
    p->m_mouse == dp->location.pi ||
    p->m_joy == dp->location.pi)
        {
    if (IS_STRING(*dp))
    {
      union { const char **c; char **s; } u; // type punning via unions

      u.c = dp->location.ppsz;
      free(*(u.s));
      *(u.c) = strdup(dp->defaultvalue.psz);
    }
    else
      *dp->location.pi = dp->defaultvalue.i;

#if 0
    if (p->m_flags & (S_LEVWARN | S_PRGWARN))
      warn |= p->m_flags & (S_LEVWARN | S_PRGWARN);
    else
      if (dp->current)
        if (allow_changes())
          *dp->current = *dp->location.pi;
        else
          warn |= S_LEVWARN;
#endif
    if (p->action)
      p->action();

    goto end;
        }
      end:;
      }
  }

  if (warn)
    warn_about_changes(warn);
}

//
// MN_InitDefaults()
//
// killough 11/98:
//
// This function converts all setup menu entries consisting of cfg
// variable names, into pointers to the corresponding default[]
// array entry. var.name becomes converted to var.def.
//

static void MN_InitDefaults(void)
{
  setup_menu_t *const *p, *t;
  default_t *dp;
  int i;
  for (i = 0; i < ss_max-1; i++)
  {
    for (p = setup_screens[i]; *p; p++)
    {
      for (t = *p; !(t->m_flags & S_END); t++)
      {
        if (t->m_flags & S_HASDEFPTR)
        {
          if (!(dp = MN_LookupDefault(t->var.name)))
            I_Error("MN_InitDefaults: Couldn't find config variable %s", t->var.name);
          else
            (t->var.def = dp)->setup_menu = t;
        }
      }
    }
  }
}

//
// End of Setup Screens.
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// Start of Extended HELP screens               // phares 3/30/98
//
// The wad designer can define a set of extended HELP screens for their own
// information display. These screens should be 320x200 graphic lumps
// defined in a separate wad. They should be named "HELP01" through "HELP99".
// "HELP01" is shown after the regular BOOM Dynamic HELP screen, and ENTER
// and BACKSPACE keys move the player through the HELP set.
//
// Rather than define a set of menu definitions for each of the possible
// HELP screens, one definition is used, and is altered on the fly
// depending on what HELPnn lumps the game finds.

// phares 3/30/98:
// Extended Help Screen variables

int extended_help_count;   // number of user-defined help screens found
int extended_help_index;   // index of current extended help screen

menuitem_t ExtHelpMenu[] =
{
  {1,"",MN_ExtHelpNextScreen,0}
};

menu_t ExtHelpDef =
{
  1,             // # of menu items
  &ReadDef1,     // previous menu
  ExtHelpMenu,   // menuitem_t ->
  MN_DrawExtHelp, // drawing routine ->
  330,181,       // x,y
  0              // lastOn
};

// MN_ExtHelpNextScreen establishes the number of the next HELP screen in
// the series.

void MN_ExtHelpNextScreen(int choice)
{
  choice = 0;
  if (++extended_help_index > extended_help_count)
    {

      // when finished with extended help screens, return to Main Menu

      extended_help_index = 1;
      MN_SetupNextMenu(&MainDef);
    }
}

// phares 3/30/98:
// Routine to look for HELPnn screens and create a menu
// definition structure that defines extended help screens.

void MN_InitExtendedHelp(void)

{
  int index,i;
  char namebfr[] = { "HELPnn"} ;

  extended_help_count = 0;
  for (index = 1 ; index < 100 ; index++) {
    namebfr[4] = index/10 + 0x30;
    namebfr[5] = index%10 + 0x30;
    i = W_CheckNumForName(namebfr);
    if (i == -1) {
      if (extended_help_count) {
        if (gamemode == commercial) {
          ExtHelpDef.prevMenu  = &ReadDef1; /* previous menu */
          ReadMenu1[0].routine = MN_ExtHelp;
  } else {
          ExtHelpDef.prevMenu  = &ReadDef2; /* previous menu */
          ReadMenu2[0].routine = MN_ExtHelp;
  }
      }
      return;
    }
    extended_help_count++;
  }

}

// Initialization for the extended HELP screens.

void MN_ExtHelp(int choice)
{
  choice = 0;
  extended_help_index = 1; // Start with first extended help screen
  MN_SetupNextMenu(&ExtHelpDef);
}

// Initialize the drawing part of the extended HELP screens.

void MN_DrawExtHelp(void)
{
  char namebfr[10] = { "HELPnn" }; // CPhipps - make it local & writable

  inhelpscreens = true;              // killough 5/1/98
  namebfr[4] = extended_help_index/10 + 0x30;
  namebfr[5] = extended_help_index%10 + 0x30;
  // CPhipps - patch drawing updated
  V_DrawNamePatch(0, 0, 0, namebfr, CR_DEFAULT, VPT_STRETCH);
}

//
// End of Extended HELP screens               // phares 3/30/98
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// Dynamic HELP screen                     // phares 3/2/98
//
// Rather than providing the static HELP screens from DOOM and its versions,
// BOOM provides the player with a dynamic HELP screen that displays the
// current settings of major key bindings.
//
// The Dynamic HELP screen is defined in a manner similar to that used for
// the Setup Screens above.
//
// MN_GetKeyString finds the correct string to represent the key binding
// for the current item being drawn.

int MN_GetKeyString(int c, int offset) {
  const char *s = I_GetKeyString(c);

  strcpy(&menu_buffer[offset], s); // string to display

  // CG: TODO:
  // The '=', ',', and '.' keys originally meant the shifted
  // versions of those keys, but w/o having to shift them in
  // the game. Any actions that are mapped to these keys will
  // still mean their shifted versions. Could be changed later
  // if someone can come up with a better way to deal with them.

  return offset + strlen(s);
}

//
// The Dynamic HELP screen table.

setup_menu_t helpstrings[] =  // HELP screen strings
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y1},
  {"HELP"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 1*8,{&key_help}},
  {"MENU"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 2*8,{&key_menu_toggle}},
  {"SETUP"       ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 3*8,{&key_setup}},
  {"PAUSE"       ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 4*8,{&key_pause}},
  {"AUTOMAP"     ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 5*8,{&key_map}},
  {"SOUND VOLUME",S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 6*8,{&key_soundvolume}},
  {"HUD"         ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 7*8,{&key_hud}},
  {"MESSAGES"    ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 8*8,{&key_messages}},
  {"GAMMA FIX"   ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 9*8,{&key_gamma}},
  {"SPY"         ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+10*8,{&key_spy}},
  {"LARGER VIEW" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+11*8,{&key_zoomin}},
  {"SMALLER VIEW",S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+12*8,{&key_zoomout}},
  {"SCREENSHOT"  ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+13*8,{&key_screenshot}},

  {"AUTOMAP"     ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y2},
  {"FOLLOW MODE" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 1*8,{&key_map_follow}},
  {"ZOOM IN"     ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 2*8,{&key_map_zoomin}},
  {"ZOOM OUT"    ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 3*8,{&key_map_zoomout}},
  {"MARK PLACE"  ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 4*8,{&key_map_mark}},
  {"CLEAR MARKS" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 5*8,{&key_map_clear}},
  {"FULL/ZOOM"   ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 6*8,{&key_map_gobig}},
  {"GRID"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 7*8,{&key_map_grid}},

  {"WEAPONS"     ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y1},
  {"FIST"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 1*8,{&key_weapon1}},
  {"PISTOL"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 2*8,{&key_weapon2}},
  {"SHOTGUN"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 3*8,{&key_weapon3}},
  {"CHAINGUN"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 4*8,{&key_weapon4}},
  {"ROCKET"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 5*8,{&key_weapon5}},
  {"PLASMA"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 6*8,{&key_weapon6}},
  {"BFG 9000"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 7*8,{&key_weapon7}},
  {"CHAINSAW"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 8*8,{&key_weapon8}},
  {"SSG"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 9*8,{&key_weapon9}},
  {"BEST"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+10*8,{&key_weapontoggle}},
  {"FIRE"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+11*8,{&key_fire},&mousebfire,&joybfire},

  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y3},
  {"FORWARD"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 1*8,{&key_up},&mousebforward},
  {"BACKWARD"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 2*8,{&key_down},&mousebbackward},
  {"TURN LEFT"   ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 3*8,{&key_left}},
  {"TURN RIGHT"  ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 4*8,{&key_right}},
  {"RUN"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 5*8,{&key_speed},0,&joybspeed},
  {"STRAFE LEFT" ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 6*8,{&key_strafeleft},0,&joybstrafeleft},
  {"STRAFE RIGHT",S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 7*8,{&key_straferight},0,&joybstraferight},
  {"STRAFE"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 8*8,{&key_strafe},&mousebstrafe,&joybstrafe},
  {"AUTORUN"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 9*8,{&key_autorun}},
  {"180 TURN"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+10*8,{&key_reverse}},
  {"USE"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+11*8,{&key_use},&mousebuse,&joybuse},

  {"GAME"        ,S_SKIP|S_TITLE,m_null,KT_X2,KT_Y1},
  {"SAVE"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 1*8,{&key_savegame}},
  {"LOAD"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 2*8,{&key_loadgame}},
  {"QUICKSAVE"   ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 3*8,{&key_quicksave}},
  {"END GAME"    ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 4*8,{&key_endgame}},
  {"QUICKLOAD"   ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 5*8,{&key_quickload}},
  {"QUIT"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 6*8,{&key_quit}},

  // Final entry

  {0,S_SKIP|S_END,m_null}
};

// MN_DrawMenuString() draws the string in menu_buffer[]

static void MN_DrawMenuString(int cx, int cy, int color) {
  HU_DrawString(cx, cy, color, menu_buffer);
}

//
// MN_DrawHelp
//
// This displays the help screen

void MN_DrawHelp (void)
{
  menuactive = mnact_full;

  MN_DrawBackground("FLOOR4_6", 0);

  MN_DrawScreenItems(helpstrings);
}

//
// End of Dynamic HELP screen                // phares 3/2/98
//
////////////////////////////////////////////////////////////////////////////

setup_menu_t cred_settings[]={

  {"Programmers",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*prog + CR_SH*cr_prog},
  {"Florian 'Proff' Schulze",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+1) + CR_SH*cr_prog},
  {"Colin Phipps",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+2) + CR_SH*cr_prog},
  {"Neil Stevens",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+3) + CR_SH*cr_prog},
  {"Andrey Budko",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+4) + CR_SH*cr_prog},

  {"Additional Credit To",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*adcr + CR_SH*cr_adcr},
  {"id Software for DOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+1)+CR_SH*cr_adcr},
  {"TeamTNT for BOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+2)+CR_SH*cr_adcr},
  {"Lee Killough for MBF",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+3)+CR_SH*cr_adcr},
  {"The DOSDoom-Team for DOSDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+4)+CR_SH*cr_adcr},
  {"Randy Heit for ZDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+5)+CR_SH*cr_adcr},
  {"Michael 'Kodak' Ryssen for DOOMGL",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+6)+CR_SH*cr_adcr},
  {"Jess Haas for lSDLDoom",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+7) + CR_SH*cr_adcr},
  {"all others who helped (see AUTHORS file)",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+8)+CR_SH*cr_adcr},

  {0,S_SKIP|S_END,m_null}
};

void MN_DrawCredits(void)     // killough 10/98: credit screen
{
  inhelpscreens = true;
  // Use V_DrawBackground here deliberately to force drawing a background
  V_DrawBackground(gamemode==shareware ? "CEIL5_1" : "MFLR8_4", 0);
  HU_DrawTitle(81, 9, "PRBOOM", CR_GOLD, PACKAGE_NAME " v" PACKAGE_VERSION, CR_GOLD);
  MN_DrawScreenItems(cred_settings);
}

static int MN_IndexInChoices(const char *str, const char **choices) {
  int i = 0;

  while (*choices != NULL) {
    if (!strcmp(str, *choices))
      return i;
    i++;
    choices++;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// MN_Responder
//
// Examines incoming keystrokes and button pushes and determines some
// action based on the state of the system.
//

bool MN_Responder(event_t* ev) {
  static int joywait   = 0;
  static int mousewait = 0;

  int  ch;
  int  i;
  bool joy_moved = false;
  bool joy_moved_up = false;
  bool joy_moved_down = false;
  bool joy_moved_left = false;
  bool joy_moved_right = false;

  ch = -1; // will be changed to a legit char if we're going to use it here

  // Process joystick input

  if (ev->type == ev_joystick_movement && joywait < I_GetTime()) {
    if (ev->jstype == ev_joystick_axis) {
      if (ev->value != 0 && (ev->key == 0 && ev->key == 1))
        joy_moved = true;

      if (ev->key == 0) {
        if (ev->value < 0)
          joy_moved_left = true;
        if (ev->value > 0)
          joy_moved_right = true;
      }
      else if (ev->key == 1) {
        if (ev->value < 0)
          joy_moved_down = true;
        if (ev->value > 0)
          joy_moved_up = true;
      }
    }
    else if (ev->jstype == ev_joystick_ball) {
      if (ev->xmove != 0 || ev->ymove != 0)
        joy_moved = true;

      if (ev->xmove < 0)
        joy_moved_left = true;
      if (ev->xmove > 0)
        joy_moved_right = true;
      if (ev->ymove < 0)
        joy_moved_down = true;
      if (ev->ymove > 0)
        joy_moved_up = true;
    }
    else if (ev->jstype == ev_joystick_hat) {
      if (ev->value != SDL_HAT_CENTERED)
        joy_moved = true;

      if (ev->value == SDL_HAT_UP) {
        joy_moved_up = true;
      }
      else if (ev->value == SDL_HAT_RIGHT) {
        joy_moved_right = true;
      }
      else if (ev->value == SDL_HAT_DOWN) {
        joy_moved_down = true;
      }
      else if (ev->value == SDL_HAT_LEFT) {
        joy_moved_left = true;
      }
      else if (ev->value == SDL_HAT_RIGHTUP) {
        joy_moved_right = true;
        joy_moved_up = true;
      }
      else if (ev->value == SDL_HAT_RIGHTDOWN) {
        joy_moved_right = true;
        joy_moved_down = true;
      }
      else if (ev->value == SDL_HAT_LEFTUP) {
        joy_moved_left = true;
        joy_moved_up = true;
      }
      else if (ev->value == SDL_HAT_LEFTDOWN) {
        joy_moved_left = true;
        joy_moved_down = true;
      }
    }
  }

  if (joy_moved) {
    if (joy_moved_up) {
      ch = key_menu_up;                                // phares 3/7/98
      joywait = I_GetTime() + 5;
    }
    else if (joy_moved_down) {
      ch = key_menu_down;                              // phares 3/7/98
      joywait = I_GetTime() + 5;
    }

    if (joy_moved_left) {
      ch = key_menu_left;                              // phares 3/7/98
      joywait = I_GetTime() + 2;
    }
    else if (joy_moved_right) {
      ch = key_menu_right;                             // phares 3/7/98
      joywait = I_GetTime() + 2;
    }

    if (ev->key & 1) {
      ch = key_menu_enter;                             // phares 3/7/98
      joywait = I_GetTime() + 5;
    }

    if (ev->key & 2) {
      ch = key_menu_backspace;                         // phares 3/7/98
      joywait = I_GetTime() + 5;
    }

    // phares 4/4/98:
    // Handle joystick buttons 3 and 4, and allow them to pass down
    // to where key binding can eat them.

    if (setup_active && set_keybnd_active) {
      if (ev->key & 4) {
        ch = 0; // meaningless, just to get you past the check for -1
        joywait = I_GetTime() + 5;
      }
      if (ev->key & 8) {
        ch = 0; // meaningless, just to get you past the check for -1
        joywait = I_GetTime() + 5;
      }
    }

  }
  else if (ev->type == ev_mouse && mousewait < I_GetTime()) {
    // Process mouse input

    if (ev->key & 1) {
      ch = key_menu_enter;                           // phares 3/7/98
      mousewait = I_GetTime() + 15;
    }

    if (ev->key & 2) {
      ch = key_menu_backspace;                       // phares 3/7/98
      mousewait = I_GetTime() + 15;
    }

    // phares 4/4/98:
    // Handle mouse button 3, and allow it to pass down
    // to where key binding can eat it.

    if (setup_active && set_keybnd_active) {
      if (ev->key & 4 || ev->key & 8 || ev->key & 16) {
        ch = 0; // meaningless, just to get you past the check for -1
        mousewait = I_GetTime() + 15;
      }
    }
  }
  else if (ev->type == ev_key && ev->pressed) {
    ch = ev->key;
  }

  if (ch == -1)
    return false; // we can't use the event here

  // Save Game string input

  if (saveStringEnter) {
    if (ch == key_menu_backspace) {                          // phares 3/7/98
      if (saveCharIndex > 0) {
        saveCharIndex--;
        savegamestrings[saveSlot][saveCharIndex] = 0;
      }
    }
    else if (ch == key_menu_escape) {                  // phares 3/7/98
      saveStringEnter = false;
      strcpy(&savegamestrings[saveSlot][0],saveOldString);
    }
    else if (ch == key_menu_enter) {                   // phares 3/7/98
      saveStringEnter = false;
      if (savegamestrings[saveSlot][0])
        MN_DoSave(saveSlot);
    }
    else {
      ch = toupper(ch);

      if (ch >= 32 && ch <= 127 &&
          saveCharIndex < SAVESTRINGSIZE-1 &&
          HU_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE - 2) * 8) {
        savegamestrings[saveSlot][saveCharIndex++] = ch;
        savegamestrings[saveSlot][saveCharIndex] = 0;
      }
    }
    return true;
  }

  // Take care of any messages that need input

  if (messageToPrint) {
    if (messageNeedsInput == true && !(ch == ' ' ||
                                       ch == 'n' ||
                                       ch == 'y' ||
                                       ch == key_escape)) { // phares
      return false;
    }

    menuactive = messageLastMenuActive;
    messageToPrint = 0;
    if (messageRoutine)
      messageRoutine(ch);

    menuactive = mnact_inactive;
    S_StartSound(NULL, sfx_swtchx);
    return true;
  }

  // If there is no active menu displayed...

  // Pop-up Main menu?

  if (!menuactive)
  {
    if (ch == key_menu_toggle)
    {
      MN_StartControlPanel ();
      S_StartSound(NULL, sfx_swtchn);
      return true;
    }
    return false;
  }

  // phares 3/26/98 - 4/11/98:
  // Setup screen key processing

  if (setup_active)
  {
    setup_menu_t* ptr1= current_setup_menu + set_menu_itemon;
    setup_menu_t* ptr2 = NULL;

    // phares 4/19/98:
    // Catch the response to the 'reset to default?' verification
    // screen

    if (default_verify)
    {
      if (toupper(ch) == 'Y') {
        MN_ResetDefaults();
        default_verify = false;
        MN_SelectDone(ptr1);
      }
      else if (toupper(ch) == 'N') {
        default_verify = false;
        MN_SelectDone(ptr1);
      }
      return true;
    }

    // Common processing for some items

    if (setup_select)
    { // changing an entry
      if (ch == key_menu_escape) // Exit key = no change
      {
        MN_SelectDone(ptr1);                           // phares 4/17/98
        setup_gather = false;   // finished gathering keys, if any
        return true;
      }

      if (ptr1->m_flags & S_YESNO) // yes or no setting?
      {
        if (ch == key_menu_enter)
        {
          *ptr1->var.def->location.pi = !*ptr1->var.def->location.pi; // killough 8/15/98

          // phares 4/14/98:
          // If not in demoplayback, demorecording, or netgame,
          // and there's a second variable in var2, set that
          // as well

          // killough 8/15/98: add warning messages

          if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN)) {
            warn_about_changes(ptr1->m_flags &    // killough 10/98
             (S_LEVWARN | S_PRGWARN));
          }
          else {
            MN_UpdateCurrent(ptr1->var.def);
          }

          if (ptr1->action)      // killough 10/98
            ptr1->action();
          
          //e6y
#ifdef GL_DOOM
          {
            extern bool gl_arb_multitexture;

            if ((ptr1->m_flags&S_CANT_GL_ARB_MULTITEXTURE) && !gl_arb_multitexture)
              warn_about_changes(ptr1->m_flags & S_CANT_GL_ARB_MULTITEXTURE);
          }
#endif
        }
        MN_SelectDone(ptr1);                           // phares 4/17/98
        return true;
      }

      if (ptr1->m_flags & S_CRITEM)
      {
        if (ch != key_menu_enter)
        {
          ch -= 0x30; // out of ascii

          if (ch < 0 || ch > 9)
            return true; // ignore

          *ptr1->var.def->location.pi = ch;
        }

        if (ptr1->action)      // killough 10/98
          ptr1->action();

        MN_SelectDone(ptr1);                      // phares 4/17/98
        return true;
      }

      if (ptr1->m_flags & S_NUM) // number?
      {
        if (setup_gather)
        { // gathering keys for a value?
          /* killough 10/98: Allow negatives, and use a more
           * friendly input method (e.g. don't clear value early,
           * allow backspace, and return to original value if bad
           * value is entered).
           */
          if (ch == key_menu_enter) {
            if (gather_count)
            {     // Any input?
              int value;

              gather_buffer[gather_count] = 0;
              value = atoi(gather_buffer);  // Integer value

              //e6y
              if ((ptr1->m_flags&S_CANT_GL_ARB_MULTISAMPLEFACTOR) && value%2!=0)
                warn_about_changes(ptr1->m_flags & S_CANT_GL_ARB_MULTISAMPLEFACTOR);
              else if ((ptr1->var.def->minvalue != UL &&
                   value < ptr1->var.def->minvalue) ||
                  (ptr1->var.def->maxvalue != UL &&
                   value > ptr1->var.def->maxvalue)) {
                warn_about_changes(S_BADVAL);
              }
              else {
                *ptr1->var.def->location.pi = value;

                /* killough 8/9/98: fix numeric vars
                 * killough 8/15/98: add warning message
                 */
                if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN)) {
                  warn_about_changes(ptr1->m_flags &
                   (S_LEVWARN | S_PRGWARN));
                }
                else {
                  MN_UpdateCurrent(ptr1->var.def);
                }

                if (ptr1->action)      // killough 10/98
                  ptr1->action();
              }
            }
            MN_SelectDone(ptr1);     // phares 4/17/98
            setup_gather = false; // finished gathering keys
            return true;
          }

          if (ch == key_menu_backspace && gather_count) {
            gather_count--;
            return true;
          }

          if (gather_count >= MAXGATHER)
            return true;

          if (!isdigit(ch) && ch != '-')
            return true; // ignore

          /* killough 10/98: character-based numerical input */
          gather_buffer[gather_count++] = ch;
        }
        return true;
      }

      if (ptr1->m_flags & S_CHOICE) // selection of choices?
      {
        if (ch == key_menu_left)
        {
          if (ptr1->var.def->type == def_int) {
            int value = *ptr1->var.def->location.pi;
          
            value = value - 1;

            if ((ptr1->var.def->minvalue != UL &&
                 value < ptr1->var.def->minvalue)) {
              value = ptr1->var.def->minvalue;
            }

            if ((ptr1->var.def->maxvalue != UL &&
                 value > ptr1->var.def->maxvalue)) {
              value = ptr1->var.def->maxvalue;
            }

            if (*ptr1->var.def->location.pi != value)
              S_StartSound(NULL, sfx_pstop);

            *ptr1->var.def->location.pi = value;
          }
          if (ptr1->var.def->type == def_str) {
            int old_value, value;

            old_value = MN_IndexInChoices(
              *ptr1->var.def->location.ppsz, ptr1->selectstrings
            );

            if (strcmp(ptr1->m_text, "Screen Resolution") == 0) {
              for (int a = 0; ptr1->selectstrings[a]; a++) {
                printf("Resolution %d: %s\n", a, ptr1->selectstrings[a]);
              }
              puts("");
            }

            value = old_value - 1;

            if (value < 0)
              value = 0;

            if (old_value != value)
              S_StartSound(NULL, sfx_pstop);

            *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
          }
        }
        if (ch == key_menu_right)
        {
          if (ptr1->var.def->type == def_int)
          {
            int value = *ptr1->var.def->location.pi;
          
            value = value + 1;

            if ((ptr1->var.def->minvalue != UL &&
                 value < ptr1->var.def->minvalue)) {
              value = ptr1->var.def->minvalue;
            }

            if ((ptr1->var.def->maxvalue != UL &&
                 value > ptr1->var.def->maxvalue)) {
              value = ptr1->var.def->maxvalue;
            }

            if (*ptr1->var.def->location.pi != value)
              S_StartSound(NULL, sfx_pstop);

            *ptr1->var.def->location.pi = value;
          }
          if (ptr1->var.def->type == def_str)
          {
            int old_value, value;

            old_value = MN_IndexInChoices(
              *ptr1->var.def->location.ppsz, ptr1->selectstrings
            );

            value = old_value + 1;

            if (ptr1->selectstrings[value] == NULL)
              value = old_value;

            if (old_value != value)
              S_StartSound(NULL, sfx_pstop);

            *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
          }
        }

        if (ch == key_menu_enter)
        {
          // phares 4/14/98:
          // If not in demoplayback, demorecording, or netgame,
          // and there's a second variable in var2, set that
          // as well

          // killough 8/15/98: add warning messages

          if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN)) {
            warn_about_changes(ptr1->m_flags &    // killough 10/98
             (S_LEVWARN | S_PRGWARN));
          }
          else {
            MN_UpdateCurrent(ptr1->var.def);
          }

          if (ptr1->action)      // killough 10/98
            ptr1->action();

          MN_SelectDone(ptr1);                           // phares 4/17/98
        }
        return true;
      }
    }

    // Key Bindings

    if (set_keybnd_active) // on a key binding setup screen
    {
      if (setup_select)    // incoming key or button gets bound
      {
        if (ev->type == ev_joystick)
        {
          int oldbutton;
          setup_group group;
          bool search = true;

          if (!ptr1->m_joy)
            return true; // not a legal action here (yet)

          // see if the button is already bound elsewhere. if so, you
          // have to swap bindings so the action where it's currently
          // bound doesn't go dead. Since there is more than one
          // keybinding screen, you have to search all of them for
          // any duplicates. You're only interested in the items
          // that belong to the same group as the one you're changing.

          oldbutton = *ptr1->m_joy;
          group  = ptr1->m_group;

          if (ev->key & 1)
            ch = 0;
          else if (ev->key & 2)
            ch = 1;
          else if (ev->key & 4)
            ch = 2;
          else if (ev->key & 8)
            ch = 3;
          else
            return true;

          for (i = 0; keys_settings[i] && search; i++) {
            for (ptr2 = keys_settings[i]; !(ptr2->m_flags & S_END); ptr2++) {
              if (ptr2->m_group == group && ptr1 != ptr2) {
                if (ptr2->m_flags & S_KEY && ptr2->m_joy) {
                  if (*ptr2->m_joy == ch)
                  {
                    *ptr2->m_joy = oldbutton;
                    search = false;
                    break;
                  }
                }
              }
            }
          }
          *ptr1->m_joy = ch;
        }
        else if (ev->type == ev_mouse)
        {
          int i, oldbutton;
          setup_group group;
          bool search = true;

          if (!ptr1->m_mouse)
            return true; // not a legal action here (yet)

          // see if the button is already bound elsewhere. if so, you
          // have to swap bindings so the action where it's currently
          // bound doesn't go dead. Since there is more than one
          // keybinding screen, you have to search all of them for
          // any duplicates. You're only interested in the items
          // that belong to the same group as the one you're changing.

          oldbutton = *ptr1->m_mouse;
          group  = ptr1->m_group;

          if (ev->key & 1)
            ch = 0;
          else if (ev->key & 2)
            ch = 1;
          else if (ev->key & 4)
            ch = 2;
          else if (ev->key & 8)
            ch = 3;
          else if (ev->key & 16)
            ch = 4;
          else
            return true;

          for (i = 0; keys_settings[i] && search; i++) {
            for (ptr2 = keys_settings[i]; !(ptr2->m_flags & S_END); ptr2++) {
              if (ptr2->m_group == group && ptr1 != ptr2) {
                if (ptr2->m_flags & S_KEY && ptr2->m_mouse) {
                  if (*ptr2->m_mouse == ch)
                  {
                    *ptr2->m_mouse = oldbutton;
                    search = false;
                    break;
                  }
                }
              }
            }
          }
          *ptr1->m_mouse = ch;
        }
        else  // keyboard key
        {
          int i,oldkey;
          setup_group group;
          bool search = true;

          // see if 'ch' is already bound elsewhere. if so, you have
          // to swap bindings so the action where it's currently
          // bound doesn't go dead. Since there is more than one
          // keybinding screen, you have to search all of them for
          // any duplicates. You're only interested in the items
          // that belong to the same group as the one you're changing.

          // if you find that you're trying to swap with an action
          // that has S_KEEP set, you can't bind ch; it's already
          // bound to that S_KEEP action, and that action has to
          // keep that key.

          oldkey = *ptr1->var.m_key;
          group  = ptr1->m_group;
          for (i = 0; keys_settings[i] && search; i++) {
            for (ptr2 = keys_settings[i]; !(ptr2->m_flags & S_END); ptr2++) {
              if (ptr2->m_flags & (S_KEY|S_KEEP) &&
                  ptr2->m_group == group &&
                  ptr1 != ptr2) {
                if (*ptr2->var.m_key == ch)
                {
                  if (ptr2->m_flags & S_KEEP)
                    return true; // can't have it!
                  *ptr2->var.m_key = oldkey;
                  search = false;
                  break;
                }
              }
            }
          }
          *ptr1->var.m_key = ch;
        }

        MN_SelectDone(ptr1);       // phares 4/17/98
        return true;
      }
    }

    // Weapons

    if (set_weapon_active) // on the weapons setup screen
    {
      if (setup_select) // changing an entry
      {
        if (ch != key_menu_enter)
        {
          ch -= '0'; // out of ascii
          if (ch < 1 || ch > 9)
            return true; // ignore

          // Plasma and BFG don't exist in shareware
          // killough 10/98: allow it anyway, since this
          // isn't the game itself, just setting preferences

          // see if 'ch' is already assigned elsewhere. if so,
          // you have to swap assignments.

          // killough 11/98: simplified

          for (i = 0; (ptr2 = weap_settings[i]); i++) {
            for (; !(ptr2->m_flags & S_END); ptr2++) {
              if (ptr2->m_flags & S_WEAP &&
                  *ptr2->var.def->location.pi == ch && ptr1 != ptr2) {
                *ptr2->var.def->location.pi = *ptr1->var.def->location.pi;
                goto end;
              }
            }
          }
          end:
          *ptr1->var.def->location.pi = ch;
        }

        MN_SelectDone(ptr1);       // phares 4/17/98
        return true;
      }
    }

    // Automap

    if (set_auto_active) // on the automap setup screen
    {
      if (setup_select) // incoming key
      {
        if (ch == key_menu_down)
        {
          if (++color_palette_y == 16)
            color_palette_y = 0;
          S_StartSound(NULL, sfx_itemup);
          return true;
        }

        if (ch == key_menu_up)
        {
          if (--color_palette_y < 0)
            color_palette_y = 15;
          S_StartSound(NULL, sfx_itemup);
          return true;
        }

        if (ch == key_menu_left)
        {
          if (--color_palette_x < 0)
            color_palette_x = 15;
          S_StartSound(NULL, sfx_itemup);
          return true;
        }

        if (ch == key_menu_right)
        {
          if (++color_palette_x == 16)
            color_palette_x = 0;
          S_StartSound(NULL, sfx_itemup);
          return true;
        }

        if (ch == key_menu_enter)
        {
          *ptr1->var.def->location.pi = color_palette_x + 16*color_palette_y;
          MN_SelectDone(ptr1);                         // phares 4/17/98
          colorbox_active = false;
          return true;
        }
      }
    }

    // killough 10/98: consolidate handling into one place:
    if (setup_select &&
        set_enemy_active | set_general_active | set_chat_active |
        set_mess_active | set_status_active | set_compat_active)
    {
      if (ptr1->m_flags & S_STRING) // creating/editing a string?
      {
        if (ch == key_menu_backspace) // backspace and DEL
        {
          if (chat_string_buffer[chat_index] == 0)
          {
            if (chat_index > 0)
              chat_string_buffer[--chat_index] = 0;
          }
          // shift the remainder of the text one char left
          else {
            strcpy(
              &chat_string_buffer[chat_index],
              &chat_string_buffer[chat_index+1]
            );
          }
        }
        else if (ch == key_menu_left) // move cursor left
        {
          if (chat_index > 0)
            chat_index--;
        }
        else if (ch == key_menu_right) // move cursor right
        {
          if (chat_string_buffer[chat_index] != 0)
            chat_index++;
        }
        else if ((ch == key_menu_enter) ||
                 (ch == key_menu_escape))
        {
          *ptr1->var.def->location.ppsz = chat_string_buffer;
          MN_SelectDone(ptr1);   // phares 4/17/98
        }

        // Adding a char to the text. Has to be a printable
        // char, and you can't overrun the buffer. If the
        // chat string gets larger than what the screen can hold,
        // it is dealt with when the string is drawn (above).

        else if ((ch >= 32) && (ch <= 126)) {
          if ((chat_index+1) < CHAT_STRING_BFR_SIZE)
          {
            if (key_states.shiftdown)
              ch = shiftxform[ch];

            if (chat_string_buffer[chat_index] == 0)
            {
              chat_string_buffer[chat_index++] = ch;
              chat_string_buffer[chat_index] = 0;
            }
            else {
              chat_string_buffer[chat_index++] = ch;
            }
          }
        }
        return true;
      }

      MN_SelectDone(ptr1);       // phares 4/17/98
      return true;
    }

    // Not changing any items on the Setup screens. See if we're
    // navigating the Setup menus or selecting an item to change.

    if (ch == key_menu_down)
    {
      ptr1->m_flags &= ~S_HILITE;     // phares 4/17/98
      do {
        if (ptr1->m_flags & S_END)
        {
          set_menu_itemon = 0;
          ptr1 = current_setup_menu;
        }
        else
        {
          set_menu_itemon++;
          ptr1++;
        }
      } while (ptr1->m_flags & S_SKIP);
      MN_SelectDone(ptr1);         // phares 4/17/98
      return true;
    }

    if (ch == key_menu_up)
    {
      ptr1->m_flags &= ~S_HILITE;     // phares 4/17/98
      do
      {
        if (set_menu_itemon == 0) {
          do {
            set_menu_itemon++;
          } while(!((current_setup_menu + set_menu_itemon)->m_flags & S_END));
        }
        set_menu_itemon--;
      } while((current_setup_menu + set_menu_itemon)->m_flags & S_SKIP);
      MN_SelectDone(current_setup_menu + set_menu_itemon);         // phares 4/17/98
      return true;
    }

    if (ch == key_menu_enter)
    {
      int flags = ptr1->m_flags;

      // You've selected an item to change. Highlight it, post a new
      // message about what to do, and get ready to process the
      // change.
      //
      // killough 10/98: use friendlier char-based input buffer

      if (flags & S_NUM)
      {
        setup_gather = true;
        print_warning_about_changes = 0;
        gather_count = 0;
      }
      else if (flags & S_COLOR)
      {
        int color = *ptr1->var.def->location.pi;

        if (color < 0 || color > 255) // range check the value
          color = 0;        // 'no show' if invalid

        color_palette_x = *ptr1->var.def->location.pi & 15;
        color_palette_y = *ptr1->var.def->location.pi >> 4;
        colorbox_active = true;
      }
      else if (flags & S_STRING)
      {
        // copy chat string into working buffer; trim if needed.
        // free the old chat string memory and replace it with
        // the (possibly larger) new memory for editing purposes
        //
        // killough 10/98: fix bugs, simplify

        chat_string_buffer = malloc(CHAT_STRING_BFR_SIZE);
        strncpy(chat_string_buffer,
          *ptr1->var.def->location.ppsz, CHAT_STRING_BFR_SIZE);

        // guarantee null delimiter
        chat_string_buffer[CHAT_STRING_BFR_SIZE-1] = 0;

        // set chat table pointer to working buffer
        // and free old string's memory.
        {
          union { const char **c; char **s; } u; // type punning via unions

          u.c = ptr1->var.def->location.ppsz;
          free(*(u.s));
          *(u.c) = chat_string_buffer;
        }
        chat_index = 0; // current cursor position in chat_string_buffer
      }
      else if (flags & S_RESET) {
        default_verify = true;
      }

      ptr1->m_flags |= S_SELECT;
      setup_select = true;
      S_StartSound(NULL, sfx_itemup);
      return true;
    }

    if ((ch == key_menu_escape) || (ch == key_menu_backspace))
    {
      if (ch == key_menu_escape)
      {
        // Clear all menus
        MN_ClearMenus();
      }
      else if (currentMenu->prevMenu) // key_menu_backspace = return to Setup Menu
      {
        currentMenu = currentMenu->prevMenu;
        itemOn = currentMenu->lastOn;
        S_StartSound(NULL, sfx_swtchn);
      }
      ptr1->m_flags &= ~(S_HILITE|S_SELECT);// phares 4/19/98
      setup_active = false;
      set_keybnd_active = false;
      set_weapon_active = false;
      set_status_active = false;
      set_auto_active = false;
      set_enemy_active = false;
      set_mess_active = false;
      set_chat_active = false;
      colorbox_active = false;
      default_verify = false;       // phares 4/19/98
      set_general_active = false;    // killough 10/98
      set_compat_active = false;    // killough 10/98
      HU_Start();    // catch any message changes // phares 4/19/98
      S_StartSound(NULL, sfx_swtchx);
      return true;
    }

    // Some setup screens may have multiple screens.
    // When there are multiple screens, m_prev and m_next items need to
    // be placed on the appropriate screen tables so the user can
    // move among the screens using the left and right arrow keys.
    // The m_var1 field contains a pointer to the appropriate screen
    // to move to.

    if (ch == key_menu_left)
    {
      ptr2 = ptr1;
      do
      {
        ptr2++;
        if (ptr2->m_flags & S_PREV)
        {
          ptr1->m_flags &= ~S_HILITE;
          mult_screens_index--;
          current_setup_menu = ptr2->var.menu;
          set_menu_itemon = 0;
          print_warning_about_changes = 0; // killough 10/98

          while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);

          current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
          S_StartSound(NULL, sfx_pstop);  // killough 10/98
          return true;
        }
      } while (!(ptr2->m_flags & S_END));
    }

    if (ch == key_menu_right)
    {
      ptr2 = ptr1;
      do {
        ptr2++;
        if (ptr2->m_flags & S_NEXT)
        {
          ptr1->m_flags &= ~S_HILITE;
          mult_screens_index++;
          current_setup_menu = ptr2->var.menu;
          set_menu_itemon = 0;
          print_warning_about_changes = 0; // killough 10/98

          while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);

          current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
          S_StartSound(NULL, sfx_pstop);  // killough 10/98
          return true;
        }
      } while (!(ptr2->m_flags & S_END));
    }
  } // End of Setup Screen processing

  // From here on, these navigation keys are used on the BIG FONT menus
  // like the Main Menu.

  if (ch == key_menu_down)                             // phares 3/7/98
  {
    do
    {
      if (itemOn+1 > currentMenu->numitems-1)
        itemOn = 0;
      else
        itemOn++;
      S_StartSound(NULL, sfx_pstop);
    } while(currentMenu->menuitems[itemOn].status==-1);
    return true;
  }

  if (ch == key_menu_up)                               // phares 3/7/98
  {
    do
    {
      if (!itemOn)
        itemOn = currentMenu->numitems-1;
      else
        itemOn--;
      S_StartSound(NULL, sfx_pstop);
    } while(currentMenu->menuitems[itemOn].status==-1);
    return true;
  }

  if (ch == key_menu_left)                             // phares 3/7/98
  {
    if (currentMenu->menuitems[itemOn].routine &&
        currentMenu->menuitems[itemOn].status == 2)
    {
      S_StartSound(NULL, sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(0);
    }
    return true;
  }

  if (ch == key_menu_right)                            // phares 3/7/98
  {
    if (currentMenu->menuitems[itemOn].routine &&
        currentMenu->menuitems[itemOn].status == 2)
    {
      S_StartSound(NULL, sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(1);
    }
    return true;
  }

  if (ch == key_menu_enter)                            // phares 3/7/98
  {
    if (currentMenu->menuitems[itemOn].routine &&
        currentMenu->menuitems[itemOn].status)
    {
      currentMenu->lastOn = itemOn;
      if (currentMenu->menuitems[itemOn].status == 2)
      {
        currentMenu->menuitems[itemOn].routine(1);   // right arrow
        S_StartSound(NULL, sfx_stnmov);
      }
      else
      {
        currentMenu->menuitems[itemOn].routine(itemOn);
        S_StartSound(NULL, sfx_pistol);
      }
    }
    //jff 3/24/98 remember last skill selected
    // killough 10/98 moved to skill-specific functions
    return true;
  }

  if (ch == key_menu_escape)                           // phares 3/7/98
  {
    currentMenu->lastOn = itemOn;
    MN_ClearMenus();
    S_StartSound(NULL, sfx_swtchx);
    return true;
  }

  if (ch == key_menu_backspace)                        // phares 3/7/98
  {
    currentMenu->lastOn = itemOn;

    // phares 3/30/98:
    // add checks to see if you're in the extended help screens
    // if so, stay with the same menu definition, but bump the
    // index back one. if the index bumps back far enough ( == 0)
    // then you can return to the Read_Thisn menu definitions

    if (currentMenu->prevMenu)
    {
      if (currentMenu == &ExtHelpDef)
      {
        if (--extended_help_index == 0)
        {
          currentMenu = currentMenu->prevMenu;
          extended_help_index = 1; // reset
        }
      }
      else {
        currentMenu = currentMenu->prevMenu;
      }
      itemOn = currentMenu->lastOn;
      S_StartSound(NULL, sfx_swtchn);
    }
    return true;
  }
  else
  {
    for (i = itemOn + 1; i < currentMenu->numitems; i++) {
      if (currentMenu->menuitems[i].alphaKey == ch)
      {
        itemOn = i;
        S_StartSound(NULL, sfx_pstop);
        return true;
      }
    }
    for (i = 0; i <= itemOn; i++) {
      if (currentMenu->menuitems[i].alphaKey == ch)
      {
        itemOn = i;
        S_StartSound(NULL, sfx_pstop);
        return true;
      }
    }
  }
  return false;
}

//
// End of MN_Responder
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// General Routines
//
// This displays the Main menu and gets the menu screens rolling.
// Plus a variety of routines that control the Big Font menu display.
// Plus some initialization for game-dependant situations.

void MN_StartControlPanel (void)
{
  // intro might call this repeatedly

  if (menuactive)
    return;

  //jff 3/24/98 make default skill menu choice follow -skill or defaultskill
  //from command line or config file
  //
  // killough 10/98:
  // Fix to make "always floating" with menu selections, and to always follow
  // defaultskill, instead of -skill.

  NewDef.lastOn = defaultskill - 1;

  // e6y
  // We need to remove the fourth episode for pre-ultimate complevels.
  // It is located here instead of MN_Init() because of TNTCOMP cheat.
  EpiDef.numitems = ep_end;
  if (gamemode != commercial
      && (compatibility_level < ultdoom_compatibility
          || W_SafeGetNumForName(EpiDef.menuitems[ep4].name) == -1))
  {
    EpiDef.numitems--;
  }

  default_verify = 0;                  // killough 10/98
  menuactive = mnact_float;
  currentMenu = &MainDef;         // JDC
  itemOn = currentMenu->lastOn;   // JDC
  print_warning_about_changes = 0;   // killough 11/98
}

//
// MN_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
// killough 9/29/98: Significantly reformatted source
//

void MN_Drawer (void)
{
  inhelpscreens = false;

  // Horiz. & Vertically center string and print it.
  // killough 9/29/98: simplified code, removed 40-character width limit
  if (messageToPrint)
    {
      /* cph - strdup string to writable memory */
      char *ms = strdup(messageString);
      char *p = ms;

      int y = 100 - HU_StringHeight(messageString)/2;
      while (*p)
      {
        char *string = p, c;
        while ((c = *p) && *p != '\n')
          p++;
        *p = 0;
        HU_WriteText(160 - HU_StringWidth(string)/2, y, string, CR_DEFAULT);
        y += hu_font[0].height;
        if ((*p = c))
          p++;
      }
      free(ms);
    }
  else
    if (menuactive)
      {
  int x,y,max,i;
  int lumps_missing = 0;

  menuactive = mnact_float; // Boom-style menu drawers will set mnact_full

  if (currentMenu->routine)
    currentMenu->routine();     // call Draw routine

  // DRAW MENU

  x = currentMenu->x;
  y = currentMenu->y;
  max = currentMenu->numitems;

  for (i = 0; i < max; i++)
    if (currentMenu->menuitems[i].name[0])
      if (W_CheckNumForName(currentMenu->menuitems[i].name) < 0)
        lumps_missing++;

  if (lumps_missing == 0)
    for (i=0;i<max;i++)
    {
      if (currentMenu->menuitems[i].name[0])
        V_DrawNamePatch(x,y,0,currentMenu->menuitems[i].name,
            CR_DEFAULT, VPT_STRETCH);
      y += LINEHEIGHT;
    }
  else
    for (i = 0; i < max; i++)
    {
      const char *alttext = currentMenu->menuitems[i].alttext;
      if (alttext)
        HU_WriteText(x, y+8-(HU_StringHeight(alttext)/2), alttext, CR_DEFAULT);
      y += LINEHEIGHT;
    }

  // DRAW SKULL

  // CPhipps - patch drawing updated
  V_DrawNamePatch(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,0,
      skullName[whichSkull], CR_DEFAULT, VPT_STRETCH);
      }
}

//
// MN_ClearMenus
//
// Called when leaving the menu screens for the real world

void MN_ClearMenus (void)
{
  menuactive = mnact_inactive;
  print_warning_about_changes = 0;     // killough 8/15/98
  default_verify = 0;                  // killough 10/98

  BorderNeedRefresh = true;

  // if (!netgame && usergame && paused)
  //     sendpause = true;
}

//
// MN_SetupNextMenu
//
void MN_SetupNextMenu(menu_t *menudef)
{
  currentMenu = menudef;
  itemOn = currentMenu->lastOn;

  BorderNeedRefresh = true;
}

/////////////////////////////
//
// MN_Ticker
//
void MN_Ticker (void)
{
  if (--skullAnimCounter <= 0)
    {
      whichSkull ^= 1;
      skullAnimCounter = 8;
    }
}

/////////////////////////////
//
// Message Routines
//

void MN_StartMessage(const char *string, void *routine, bool input) {
  messageLastMenuActive = menuactive;
  messageToPrint = 1;
  messageString = string;
  messageRoutine = routine;
  messageNeedsInput = input;
  menuactive = mnact_float;

  return;
}

void MN_StopMessage(void) {
  menuactive = messageLastMenuActive;
  messageToPrint = 0;
}

/////////////////////////////
//
// Thermometer Routines
//

//
// MN_DrawThermo draws the thermometer graphic for Mouse Sensitivity,
// Sound Volume, etc.
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
//
void MN_DrawThermo(int x, int y, int thermWidth, int thermDot) {
  int xx;
  int i;
  /*
   * Modification By Barry Mead to allow the Thermometer to have vastly
   * larger ranges. (the thermWidth parameter can now have a value as
   * large as 200.      Modified 1-9-2000  Originally I used it to make
   * the sensitivity range for the mouse better. It could however also
   * be used to improve the dynamic range of music and sound affect
   * volume controls for example.
   */
  int horizScaler; //Used to allow more thermo range for mouse sensitivity.
  thermWidth = (thermWidth > 200) ? 200 : thermWidth; //Clamp to 200 max
  horizScaler = (thermWidth > 23) ? (200 / thermWidth) : 8; //Dynamic range
  xx = x;
  V_DrawNamePatch(xx, y, 0, "M_THERML", CR_DEFAULT, VPT_STRETCH);
  xx += 8;
  for (i=0;i<thermWidth;i++)
    {
    V_DrawNamePatch(xx, y, 0, "M_THERMM", CR_DEFAULT, VPT_STRETCH);
    xx += horizScaler;
    }

  xx += (8 - horizScaler);  /* make the right end look even */

  V_DrawNamePatch(xx, y, 0, "M_THERMR", CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch((x+8)+thermDot*horizScaler,y,0,"M_THERMO",CR_DEFAULT,VPT_STRETCH);
}

//
// Draw an empty cell in the thermometer
//

void MN_DrawEmptyCell (menu_t* menu,int item)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(menu->x - 10, menu->y+item*LINEHEIGHT - 1, 0,
      "M_CELL1", CR_DEFAULT, VPT_STRETCH);
}

//
// Draw a full cell in the thermometer
//

void MN_DrawSelCell (menu_t* menu,int item)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(menu->x - 10, menu->y+item*LINEHEIGHT - 1, 0,
      "M_CELL2", CR_DEFAULT, VPT_STRETCH);
}

/////////////////////////////
//
// Initialization Routines to take care of one-time setup
//

// phares 4/08/98:
// MN_InitHelpScreen() clears the weapons from the HELP
// screen that don't exist in this version of the game.

void MN_InitHelpScreen(void)
{
  setup_menu_t* src;

  src = helpstrings;
  while (!(src->m_flags & S_END)) {

    if ((strncmp(src->m_text,"PLASMA",6) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"BFG",3) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"SSG",3) == 0) && (gamemode != commercial))
      src->m_flags = S_SKIP; // Don't show setting or item
    src++;
  }
}

//
// MN_Init
//
void MN_Init(void)
{
  MN_InitDefaults();                // killough 11/98
  currentMenu = &MainDef;
  menuactive = mnact_inactive;
  itemOn = currentMenu->lastOn;
  whichSkull = 0;
  skullAnimCounter = 10;
  screenSize = screenblocks - 3;
  messageToPrint = 0;
  messageString = NULL;
  messageLastMenuActive = menuactive;
  quickSaveSlot = -1;

  // Here we could catch other version dependencies,
  //  like HELP1/2, and four episodes.

  switch(gamemode)
    {
    case commercial:
      // This is used because DOOM 2 had only one HELP
      //  page. I use CREDIT as second page now, but
      //  kept this hack for educational purposes.
      MainMenu[readthis] = MainMenu[quitdoom];
      MainDef.numitems--;
      MainDef.y += 8;
      NewDef.prevMenu = &MainDef;
      ReadDef1.routine = MN_DrawReadThis1;
      ReadDef1.x = 330;
      ReadDef1.y = 165;
      ReadMenu1[0].routine = MN_FinishReadThis;
      break;
    case registered:
      // Episode 2 and 3 are handled,
      //  branching to an ad screen.

      // killough 2/21/98: Fix registered Doom help screen
      // killough 10/98: moved to second screen, moved up to the top
      ReadDef2.y = 15;

    case shareware:
      // We need to remove the fourth episode.
      EpiDef.numitems--;
      break;
    case retail:
      // We are fine.
    default:
      break;
    }

  MN_InitHelpScreen();   // init the help screen       // phares 4/08/98
  MN_InitExtendedHelp(); // init extended help screens // phares 3/30/98
  
  //e6y
  MN_ChangeSpeed();
  MN_ChangeMaxViewPitch();
  MN_ChangeMouseLook();
  MN_ChangeMouseInvert();
#ifdef GL_DOOM
  MN_ChangeFOV();
  MN_ChangeSpriteClip();
  MN_ChangeAllowBoomColormaps();
#endif
  MN_ChangeInterlacedScanning();

  MN_ChangeDemoSmoothTurns();
  MN_ChangeDemoExtendedFormat();

  MN_ChangeMapMultisamling();

  render_stretch_hud = render_stretch_hud_default;

  MN_ChangeMIDIPlayer();
}

//
// End of General Routines
//
/////////////////////////////////////////////////////////////////////////////

/* vi: set et ts=2 sw=2: */

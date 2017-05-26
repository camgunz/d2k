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

#include "d_event.h"
#include "g_keys.h"
#include "x_intern.h"
#include "x_main.h"

static int XG_KeysGetRight(lua_State *L) {
  lua_pushinteger(L, key_right);

  return 1;
}

static int XG_KeysGetLeft(lua_State *L) {
  lua_pushinteger(L, key_left);

  return 1;
}

static int XG_KeysGetUp(lua_State *L) {
  lua_pushinteger(L, key_up);

  return 1;
}

static int XG_KeysGetDown(lua_State *L) {
  lua_pushinteger(L, key_down);

  return 1;
}

static int XG_KeysGetMouseLook(lua_State *L) {
  lua_pushinteger(L, key_mlook);

  return 1;
}

static int XG_KeysGetMenuToggle(lua_State *L) {
  lua_pushinteger(L, key_menu_toggle);

  return 1;
}

static int XG_KeysGetMenuRight(lua_State *L) {
  lua_pushinteger(L, key_menu_right);

  return 1;
}

static int XG_KeysGetMenuLeft(lua_State *L) {
  lua_pushinteger(L, key_menu_left);

  return 1;
}

static int XG_KeysGetMenuUp(lua_State *L) {
  lua_pushinteger(L, key_menu_up);

  return 1;
}

static int XG_KeysGetMenuDown(lua_State *L) {
  lua_pushinteger(L, key_menu_down);

  return 1;
}

static int XG_KeysGetMenuBackspace(lua_State *L) {
  lua_pushinteger(L, key_menu_backspace);

  return 1;
}

static int XG_KeysGetMenuEscape(lua_State *L) {
  lua_pushinteger(L, key_menu_escape);

  return 1;
}

static int XG_KeysGetMenuEnter(lua_State *L) {
  lua_pushinteger(L, key_menu_enter);

  return 1;
}

static int XG_KeysGetStrafeLeft(lua_State *L) {
  lua_pushinteger(L, key_strafeleft);

  return 1;
}

static int XG_KeysGetStrafeRight(lua_State *L) {
  lua_pushinteger(L, key_straferight);

  return 1;
}

static int XG_KeysGetFlyUp(lua_State *L) {
  lua_pushinteger(L, key_flyup);

  return 1;
}

static int XG_KeysGetFlyDown(lua_State *L) {
  lua_pushinteger(L, key_flydown);

  return 1;
}

static int XG_KeysGetFire(lua_State *L) {
  lua_pushinteger(L, key_fire);

  return 1;
}

static int XG_KeysGetUse(lua_State *L) {
  lua_pushinteger(L, key_use);

  return 1;
}

static int XG_KeysGetStrafe(lua_State *L) {
  lua_pushinteger(L, key_strafe);

  return 1;
}

static int XG_KeysGetSpeed(lua_State *L) {
  lua_pushinteger(L, key_speed);

  return 1;
}

static int XG_KeysGetEscape(lua_State *L) {
  lua_pushinteger(L, key_escape);

  return 1;
}

static int XG_KeysGetSaveGame(lua_State *L) {
  lua_pushinteger(L, key_savegame);

  return 1;
}

static int XG_KeysGetLoadGame(lua_State *L) {
  lua_pushinteger(L, key_loadgame);

  return 1;
}

static int XG_KeysGetAutorun(lua_State *L) {
  lua_pushinteger(L, key_autorun);

  return 1;
}

static int XG_KeysGetReverse(lua_State *L) {
  lua_pushinteger(L, key_reverse);

  return 1;
}

static int XG_KeysGetZoomIn(lua_State *L) {
  lua_pushinteger(L, key_zoomin);

  return 1;
}

static int XG_KeysGetZoomOut(lua_State *L) {
  lua_pushinteger(L, key_zoomout);

  return 1;
}

static int XG_KeysGetChat(lua_State *L) {
  lua_pushinteger(L, key_chat);

  return 1;
}

static int XG_KeysGetBackspace(lua_State *L) {
  lua_pushinteger(L, key_backspace);

  return 1;
}

static int XG_KeysGetEnter(lua_State *L) {
  lua_pushinteger(L, key_enter);

  return 1;
}

static int XG_KeysGetHelp(lua_State *L) {
  lua_pushinteger(L, key_help);

  return 1;
}

static int XG_KeysGetSoundVolume(lua_State *L) {
  lua_pushinteger(L, key_soundvolume);

  return 1;
}

static int XG_KeysGetHUD(lua_State *L) {
  lua_pushinteger(L, key_hud);

  return 1;
}

static int XG_KeysGetQuickSave(lua_State *L) {
  lua_pushinteger(L, key_quicksave);

  return 1;
}

static int XG_KeysGetEndGame(lua_State *L) {
  lua_pushinteger(L, key_endgame);

  return 1;
}

static int XG_KeysGetMessages(lua_State *L) {
  lua_pushinteger(L, key_messages);

  return 1;
}

static int XG_KeysGetQuickLoad(lua_State *L) {
  lua_pushinteger(L, key_quickload);

  return 1;
}

static int XG_KeysGetQuit(lua_State *L) {
  lua_pushinteger(L, key_quit);

  return 1;
}

static int XG_KeysGetGamma(lua_State *L) {
  lua_pushinteger(L, key_gamma);

  return 1;
}

static int XG_KeysGetSpy(lua_State *L) {
  lua_pushinteger(L, key_spy);

  return 1;
}

static int XG_KeysGetPause(lua_State *L) {
  lua_pushinteger(L, key_pause);

  return 1;
}

static int XG_KeysGetSetup(lua_State *L) {
  lua_pushinteger(L, key_setup);

  return 1;
}

static int XG_KeysGetForward(lua_State *L) {
  lua_pushinteger(L, key_forward);

  return 1;
}

static int XG_KeysGetTurnLeft(lua_State *L) {
  lua_pushinteger(L, key_leftturn);

  return 1;
}

static int XG_KeysGetTurnRight(lua_State *L) {
  lua_pushinteger(L, key_rightturn);

  return 1;
}

static int XG_KeysGetBackward(lua_State *L) {
  lua_pushinteger(L, key_backward);

  return 1;
}

static int XG_KeysGetWeaponToggle(lua_State *L) {
  lua_pushinteger(L, key_weapontoggle);

  return 1;
}

static int XG_KeysGetWeaponOne(lua_State *L) {
  lua_pushinteger(L, key_weapon1);

  return 1;
}

static int XG_KeysGetWeaponTwo(lua_State *L) {
  lua_pushinteger(L, key_weapon2);

  return 1;
}

static int XG_KeysGetWeaponThree(lua_State *L) {
  lua_pushinteger(L, key_weapon3);

  return 1;
}

static int XG_KeysGetWeaponFour(lua_State *L) {
  lua_pushinteger(L, key_weapon4);

  return 1;
}

static int XG_KeysGetWeaponFive(lua_State *L) {
  lua_pushinteger(L, key_weapon5);

  return 1;
}

static int XG_KeysGetWeaponSix(lua_State *L) {
  lua_pushinteger(L, key_weapon6);

  return 1;
}

static int XG_KeysGetWeaponSeven(lua_State *L) {
  lua_pushinteger(L, key_weapon7);

  return 1;
}

static int XG_KeysGetWeaponEight(lua_State *L) {
  lua_pushinteger(L, key_weapon8);

  return 1;
}

static int XG_KeysGetWeaponNine(lua_State *L) {
  lua_pushinteger(L, key_weapon9);

  return 1;
}

static int XG_KeysGetWeaponNext(lua_State *L) {
  lua_pushinteger(L, key_nextweapon);

  return 1;
}

static int XG_KeysGetWeaponPrevious(lua_State *L) {
  lua_pushinteger(L, key_prevweapon);

  return 1;
}

static int XG_KeysGetDestinationKeyOne(lua_State *L) {
  lua_pushinteger(L, destination_key_green);

  return 1;
}

static int XG_KeysGetDestinationKeyTwo(lua_State *L) {
  lua_pushinteger(L, destination_key_indigo);

  return 1;
}

static int XG_KeysGetDestinationKeyThree(lua_State *L) {
  lua_pushinteger(L, destination_key_brown);

  return 1;
}

static int XG_KeysGetDestinationKeyFour(lua_State *L) {
  lua_pushinteger(L, destination_key_red);

  return 1;
}

static int XG_KeysGetMapRight(lua_State *L) {
  lua_pushinteger(L, key_map_right);

  return 1;
}

static int XG_KeysGetMapLeft(lua_State *L) {
  lua_pushinteger(L, key_map_left);

  return 1;
}

static int XG_KeysGetMapUp(lua_State *L) {
  lua_pushinteger(L, key_map_up);

  return 1;
}

static int XG_KeysGetMapDown(lua_State *L) {
  lua_pushinteger(L, key_map_down);

  return 1;
}

static int XG_KeysGetMapZoomIn(lua_State *L) {
  lua_pushinteger(L, key_map_zoomin);

  return 1;
}

static int XG_KeysGetMapZoomOut(lua_State *L) {
  lua_pushinteger(L, key_map_zoomout);

  return 1;
}

static int XG_KeysGetMap(lua_State *L) {
  lua_pushinteger(L, key_map);

  return 1;
}

static int XG_KeysGetMapGoBig(lua_State *L) {
  lua_pushinteger(L, key_map_gobig);

  return 1;
}

static int XG_KeysGetMapFollow(lua_State *L) {
  lua_pushinteger(L, key_map_follow);

  return 1;
}

static int XG_KeysGetMapMark(lua_State *L) {
  lua_pushinteger(L, key_map_mark);

  return 1;
}

static int XG_KeysGetMapClear(lua_State *L) {
  lua_pushinteger(L, key_map_clear);

  return 1;
}

static int XG_KeysGetMapGrid(lua_State *L) {
  lua_pushinteger(L, key_map_grid);

  return 1;
}

static int XG_KeysGetMapRotate(lua_State *L) {
  lua_pushinteger(L, key_map_rotate);

  return 1;
}

static int XG_KeysGetMapOverlay(lua_State *L) {
  lua_pushinteger(L, key_map_overlay);

  return 1;
}

static int XG_KeysGetMapTexture(lua_State *L) {
  lua_pushinteger(L, key_map_textured);

  return 1;
}

static int XG_KeysGetScreenshot(lua_State *L) {
  lua_pushinteger(L, key_screenshot);

  return 1;
}

static int XG_KeysGetSpeedUp(lua_State *L) {
  lua_pushinteger(L, key_speed_up);

  return 1;
}

static int XG_KeysGetSpeedDown(lua_State *L) {
  lua_pushinteger(L, key_speed_down);

  return 1;
}

static int XG_KeysGetSpeedDefault(lua_State *L) {
  lua_pushinteger(L, key_speed_default);

  return 1;
}

static int XG_KeysGetLevelRestart(lua_State *L) {
  lua_pushinteger(L, key_level_restart);

  return 1;
}

static int XG_KeysGetNextLevel(lua_State *L) {
  lua_pushinteger(L, key_nextlevel);

  return 1;
}

static int XG_KeysGetDemoJoinToGame(lua_State *L) {
  lua_pushinteger(L, key_demo_jointogame);

  return 1;
}

static int XG_KeysGetDemoEndLevel(lua_State *L) {
  lua_pushinteger(L, key_demo_endlevel);

  return 1;
}

static int XG_KeysGetDemoSkip(lua_State *L) {
  lua_pushinteger(L, key_demo_skip);

  return 1;
}

static int XG_KeysGetWalkCamera(lua_State *L) {
  lua_pushinteger(L, key_walkcamera);

  return 1;
}

static int XG_KeysGetShowAlive(lua_State *L) {
  lua_pushinteger(L, key_showalive);

  return 1;
}

static int XG_KeysGetMouseButtonFire(lua_State *L) {
  lua_pushinteger(L, mousebfire);

  return 1;
}

static int XG_KeysGetMouseButtonStrafe(lua_State *L) {
  lua_pushinteger(L, mousebstrafe);

  return 1;
}

static int XG_KeysGetMouseButtonForward(lua_State *L) {
  lua_pushinteger(L, mousebforward);

  return 1;
}

static int XG_KeysGetMouseButtonBackward(lua_State *L) {
  lua_pushinteger(L, mousebbackward);

  return 1;
}

static int XG_KeysGetMouseButtonUse(lua_State *L) {
  lua_pushinteger(L, mousebuse);

  return 1;
}

static int XG_KeysGetJoyButtonFire(lua_State *L) {
  lua_pushinteger(L, joybfire);

  return 1;
}

static int XG_KeysGetJoyButtonStrafe(lua_State *L) {
  lua_pushinteger(L, joybstrafe);

  return 1;
}

static int XG_KeysGetJoyButtonStrafeLeft(lua_State *L) {
  lua_pushinteger(L, joybstrafeleft);

  return 1;
}

static int XG_KeysGetJoyButtonStrafeRight(lua_State *L) {
  lua_pushinteger(L, joybstraferight);

  return 1;
}

static int XG_KeysGetJoyButtonUse(lua_State *L) {
  lua_pushinteger(L, joybuse);

  return 1;
}

static int XG_KeysGetJoyButtonSpeed(lua_State *L) {
  lua_pushinteger(L, joybspeed);

  return 1;
}

void XG_KeysRegisterInterface(void) {
  X_RegisterObjects("KeyBinds", 100,
    "get_right",                   X_FUNCTION, XG_KeysGetRight,
    "get_left",                    X_FUNCTION, XG_KeysGetLeft,
    "get_up",                      X_FUNCTION, XG_KeysGetUp,
    "get_down",                    X_FUNCTION, XG_KeysGetDown,
    "get_mouse_look",              X_FUNCTION, XG_KeysGetMouseLook,
    "get_menu_toggle",             X_FUNCTION, XG_KeysGetMenuToggle,
    "get_menu_right",              X_FUNCTION, XG_KeysGetMenuRight,
    "get_menu_left",               X_FUNCTION, XG_KeysGetMenuLeft,
    "get_menu_up",                 X_FUNCTION, XG_KeysGetMenuUp,
    "get_menu_down",               X_FUNCTION, XG_KeysGetMenuDown,
    "get_menu_backspace",          X_FUNCTION, XG_KeysGetMenuBackspace,
    "get_menu_escape",             X_FUNCTION, XG_KeysGetMenuEscape,
    "get_menu_enter",              X_FUNCTION, XG_KeysGetMenuEnter,
    "get_strafe_left",             X_FUNCTION, XG_KeysGetStrafeLeft,
    "get_strafe_right",            X_FUNCTION, XG_KeysGetStrafeRight,
    "get_fly_up",                  X_FUNCTION, XG_KeysGetFlyUp,
    "get_fly_down",                X_FUNCTION, XG_KeysGetFlyDown,
    "get_fire",                    X_FUNCTION, XG_KeysGetFire,
    "get_use",                     X_FUNCTION, XG_KeysGetUse,
    "get_strafe",                  X_FUNCTION, XG_KeysGetStrafe,
    "get_speed",                   X_FUNCTION, XG_KeysGetSpeed,
    "get_escape",                  X_FUNCTION, XG_KeysGetEscape,
    "get_save_game",               X_FUNCTION, XG_KeysGetSaveGame,
    "get_load_game",               X_FUNCTION, XG_KeysGetLoadGame,
    "get_autorun",                 X_FUNCTION, XG_KeysGetAutorun,
    "get_reverse",                 X_FUNCTION, XG_KeysGetReverse,
    "get_zoom_in",                 X_FUNCTION, XG_KeysGetZoomIn,
    "get_zoom_out",                X_FUNCTION, XG_KeysGetZoomOut,
    "get_chat",                    X_FUNCTION, XG_KeysGetChat,
    "get_backspace",               X_FUNCTION, XG_KeysGetBackspace,
    "get_enter",                   X_FUNCTION, XG_KeysGetEnter,
    "get_help",                    X_FUNCTION, XG_KeysGetHelp,
    "get_sound_volume",            X_FUNCTION, XG_KeysGetSoundVolume,
    "get_hud",                     X_FUNCTION, XG_KeysGetHUD,
    "get_quick_save",              X_FUNCTION, XG_KeysGetQuickSave,
    "get_end_game",                X_FUNCTION, XG_KeysGetEndGame,
    "get_messages",                X_FUNCTION, XG_KeysGetMessages,
    "get_quick_load",              X_FUNCTION, XG_KeysGetQuickLoad,
    "get_quit",                    X_FUNCTION, XG_KeysGetQuit,
    "get_gamma",                   X_FUNCTION, XG_KeysGetGamma,
    "get_spy",                     X_FUNCTION, XG_KeysGetSpy,
    "get_pause",                   X_FUNCTION, XG_KeysGetPause,
    "get_setup",                   X_FUNCTION, XG_KeysGetSetup,
    "get_forward",                 X_FUNCTION, XG_KeysGetForward,
    "get_turn_left",               X_FUNCTION, XG_KeysGetTurnLeft,
    "get_turn_right",              X_FUNCTION, XG_KeysGetTurnRight,
    "get_backward",                X_FUNCTION, XG_KeysGetBackward,
    "get_weapon_toggle",           X_FUNCTION, XG_KeysGetWeaponToggle,
    "get_weapon_one",              X_FUNCTION, XG_KeysGetWeaponOne,
    "get_weapon_two",              X_FUNCTION, XG_KeysGetWeaponTwo,
    "get_weapon_three",            X_FUNCTION, XG_KeysGetWeaponThree,
    "get_weapon_four",             X_FUNCTION, XG_KeysGetWeaponFour,
    "get_weapon_five",             X_FUNCTION, XG_KeysGetWeaponFive,
    "get_weapon_six",              X_FUNCTION, XG_KeysGetWeaponSix,
    "get_weapon_seven",            X_FUNCTION, XG_KeysGetWeaponSeven,
    "get_weapon_eight",            X_FUNCTION, XG_KeysGetWeaponEight,
    "get_weapon_nine",             X_FUNCTION, XG_KeysGetWeaponNine,
    "get_weapon_next",             X_FUNCTION, XG_KeysGetWeaponNext,
    "get_weapon_previous",         X_FUNCTION, XG_KeysGetWeaponPrevious,
    "get_destination_key_one",     X_FUNCTION, XG_KeysGetDestinationKeyOne,
    "get_destination_key_two",     X_FUNCTION, XG_KeysGetDestinationKeyTwo,
    "get_destination_key_three",   X_FUNCTION, XG_KeysGetDestinationKeyThree,
    "get_destination_key_four",    X_FUNCTION, XG_KeysGetDestinationKeyFour,
    "get_map_right",               X_FUNCTION, XG_KeysGetMapRight,
    "get_map_left",                X_FUNCTION, XG_KeysGetMapLeft,
    "get_map_up",                  X_FUNCTION, XG_KeysGetMapUp,
    "get_map_down",                X_FUNCTION, XG_KeysGetMapDown,
    "get_map_zoom_in",             X_FUNCTION, XG_KeysGetMapZoomIn,
    "get_map_zoom_out",            X_FUNCTION, XG_KeysGetMapZoomOut,
    "get_map",                     X_FUNCTION, XG_KeysGetMap,
    "get_map_go_big",              X_FUNCTION, XG_KeysGetMapGoBig,
    "get_map_follow",              X_FUNCTION, XG_KeysGetMapFollow,
    "get_map_mark",                X_FUNCTION, XG_KeysGetMapMark,
    "get_map_clear",               X_FUNCTION, XG_KeysGetMapClear,
    "get_map_grid",                X_FUNCTION, XG_KeysGetMapGrid,
    "get_map_rotate",              X_FUNCTION, XG_KeysGetMapRotate,
    "get_map_overlay",             X_FUNCTION, XG_KeysGetMapOverlay,
    "get_map_texture",             X_FUNCTION, XG_KeysGetMapTexture,
    "get_screenshot",              X_FUNCTION, XG_KeysGetScreenshot,
    "get_speed_up",                X_FUNCTION, XG_KeysGetSpeedUp,
    "get_speed_down",              X_FUNCTION, XG_KeysGetSpeedDown,
    "get_speed_default",           X_FUNCTION, XG_KeysGetSpeedDefault,
    "get_level_restart",           X_FUNCTION, XG_KeysGetLevelRestart,
    "get_next_level",              X_FUNCTION, XG_KeysGetNextLevel,
    "get_demo_join_to_game",       X_FUNCTION, XG_KeysGetDemoJoinToGame,
    "get_demo_end_level",          X_FUNCTION, XG_KeysGetDemoEndLevel,
    "get_demo_skip",               X_FUNCTION, XG_KeysGetDemoSkip,
    "get_walk_camera",             X_FUNCTION, XG_KeysGetWalkCamera,
    "get_show_alive",              X_FUNCTION, XG_KeysGetShowAlive,
    "get_mouse_button_fire",       X_FUNCTION, XG_KeysGetMouseButtonFire,
    "get_mouse_button_strafe",     X_FUNCTION, XG_KeysGetMouseButtonStrafe,
    "get_mouse_button_forward",    X_FUNCTION, XG_KeysGetMouseButtonForward,
    "get_mouse_button_backward",   X_FUNCTION, XG_KeysGetMouseButtonBackward,
    "get_mouse_button_use",        X_FUNCTION, XG_KeysGetMouseButtonUse,
    "get_joy_button_fire",         X_FUNCTION, XG_KeysGetJoyButtonFire,
    "get_joy_button_strafe",       X_FUNCTION, XG_KeysGetJoyButtonStrafe,
    "get_joy_button_strafe_left",  X_FUNCTION, XG_KeysGetJoyButtonStrafeLeft,
    "get_joy_button_strafe_right", X_FUNCTION, XG_KeysGetJoyButtonStrafeRight,
    "get_joy_button_use",          X_FUNCTION, XG_KeysGetJoyButtonUse,
    "get_joy_button_speed",        X_FUNCTION, XG_KeysGetJoyButtonSpeed
  );
}

/* vi: set et ts=2 sw=2: */


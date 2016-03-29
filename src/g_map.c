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

#include <jansson.h>

static json_t *map_list = NULL;
static size_t  map_list_index = 0;

bool G_MapListActive(void) {
  if (map_list) {
    return true;
  }

  return false;
}

bool G_MapListLoadPreviousMap(void) {
  size_t index = map_list_index;
  size_t map_count = G_MapListLength();

  if (map_count == 0) {
    return false;
  }

  if (index == 0) {
    index = map_count - 1;
  }
  else {
    index--;
  }

  return G_MapLoad(index);
}

bool G_MapListLoadNextMap(void) {
  size_t index = map_list_index;
  size_t map_count = G_MapListLength();

  if (map_count == 0) {
    return false;
  }

  if (index == (map_count - 1)) {
    index = 0;
  }
  else {
    index++;
  }

  return G_MapLoad(index);
}

bool G_MapListReloadMap(void) {
  if (G_MapListLength() == 0) {
    return false;
  }

  return G_MapLoad(map_list_index);
}

bool G_MapListLoadMap(size_t map_index) {
  size_t map_count = G_MapListLength();

  if (map_count == 0) {
    return false;
  }

  if (map_index >= map_count) {
    return false;
  }
}

size_t G_MapListLength(void) {
  if (!map_list) {
    return 0;
  }

  return json_array_size(map_list);
}

bool G_MapListSet(const char *map_list_contents) {
  json_error_t json_error;
  json_t *new_map_list = json_loads(
    map_list_contents, JSON_REJECT_DUPLICATES, &json_error
  );

  if (!new_map_list) {
    D_Msg(MSG_WARN, "Invalid map list: blah blah blah");
    return false;
  }

  /*
   * [CG] TODO: Some basic map list validation, like get the info of every map
   */
  if (map_list) {
    json_decref(map_list);
    map_list = NULL;
  }

  map_list = new_map_list;
}

char* G_MapListGet(void) {
  if (!map_list) {
    return NULL;
  }

  return json_dumps(map_list, JSON_INDENT(4) | JSON_PRESERVE_ORDER);
}

bool G_MapListToFile(const char *file_path) {
  int res;

  res = json_dump_file(
    map_list, file_path, JSON_INDENT(4) | JSON_PRESERVE_ORDER
  );

  if (res != 0) {
    D_Msg(MSG_WARN, "Error writing JSON to file: blah blah blah");
    return false;
  }

  return true;
}

/* vi: set et ts=2 sw=2: */


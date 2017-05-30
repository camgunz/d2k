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


#ifndef MN_DEF_H__
#define MN_DEF_H__

#define DEF_IS_STRING(dv) ((dv).type == def_str)

#define UL (-123456789) /* magic number for no min or max for parameter */

// phares 4/21/98:
// Moved from m_misc.c so m_menu.c could see it.

// CPhipps - struct to hold a value in a config file
// Cannot be a union, as it must be initialised
typedef struct default_s {
  const char* name;
  /* cph -
   * The location struct holds the pointer to the variable holding the
   *  setting. For int's we do nothing special.
   * For strings, the string is actually stored on our heap with Z_Strdup()
   *  BUT we don't want the rest of the program to be able to modify them,
   *  so we declare it const. It's not really const though, and m_misc.c and
   *  m_menu.c cast it back when they need to change it. Possibly this is
   *  more trouble than it's worth.
   */
  // Note: casts are now made via unions to avoid discarding qualifier warnings
  struct {
    int          *pi;
    const char  **ppsz;
    //e6y: arrays
    int          *array_size;
    char       ***array_data;
    int           array_index;
  } location;
  struct {
    int         i;
    const char *psz;
    //e6y: arrays
    int          array_size;
    const char **array_data;
  } defaultvalue; // CPhipps - default value
  // Limits (for an int)
  int minvalue; // jff 3/3/98 minimum allowed value
  int maxvalue; // jff 3/3/98 maximum allowed value
  enum {
    def_none,              // Dummy entry
    def_str,               // A string
    def_int,               // Integer
    def_hex,               // Integer (write in hex)
    def_arr,               // e6y: arrays
    def_bool   = def_int,  // Boolean
    def_key    = def_hex,  // Key code (byte)
    def_mouseb = def_int,  // Mouse button
    def_colour = def_hex   // Colour (256 colour palette entry)
  } type; // CPhipps - type of entry
  int   setupscreen; // phares 4/19/98: setup screen where this appears
  int  *current; /* cph - MBF-like pointer to current value */
  // cph - removed the help strings from the config file
  // const char* help;       // jff 3/3/98 description of parameter
  // CPhipps - remove unused "lousy hack" code
  struct setup_menu_s *setup_menu;   /* Xref to setup menu item, if any */
} default_t;

void       MN_LoadDefaults (void);
void       MN_SaveDefaults (void);
default_t* MN_LookupDefault(const char *name); /* killough 11/98 */

#endif

/* vi: set et ts=2 sw=2: */

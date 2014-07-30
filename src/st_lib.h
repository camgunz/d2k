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


#ifndef ST_LIB_H__
#define ST_LIB_H__

// We are referring to patches.
#include "r_defs.h"
#include "v_video.h"  // color ranges

//
// Background and foreground screen numbers
//
#define BG 4
#define FG 0

//
// Typedefs of widgets
//

// Number widget

typedef struct
{
  // upper right-hand corner
  //  of the number (right-justified)
  int   x;
  int   y;

  // max # of digits in number
  int width;

  // last number value
  int   oldnum;

  // pointer to current value
  int*  num;

  // pointer to dboolean stating
  //  whether to update number
  dboolean*  on;

  // list of patches for 0-9
  const patchnum_t* p;

  // user data
  int data;
} st_number_t;

// Percent widget ("child" of number widget,
//  or, more precisely, contains a number widget.)
typedef struct
{
  // number information
  st_number_t   n;

  // percent sign graphic
  const patchnum_t*    p;
} st_percent_t;

// Multiple Icon widget
typedef struct
{
  // center-justified location of icons
  int     x;
  int     y;

  // last icon number
  int     oldinum;

  // pointer to current icon
  int*    inum;

  // pointer to dboolean stating
  //  whether to update icon
  dboolean*    on;

  // list of icons
  const patchnum_t*   p;

  // user data
  int     data;

} st_multicon_t;

// Binary Icon widget

typedef struct
{
  // center-justified location of icon
  int     x;
  int     y;

  // last icon value
  dboolean oldval;

  // pointer to current icon status
  dboolean*    val;

  // pointer to dboolean
  //  stating whether to update icon
  dboolean*    on;

  const patchnum_t*    p;  // icon
  int     data;   // user data
} st_binicon_t;

//
// Widget creation, access, and update routines
//

// Initializes widget library.
// More precisely, initialize STMINUS,
//  everything else is done somewhere else.
//
void STlib_init(void);

// Number widget routines
void STlib_initNum
( st_number_t* n,
  int x,
  int y,
  const patchnum_t* pl,
  int* num,
  dboolean* on,
  int width );

void STlib_updateNum
( st_number_t* n,
  int cm,
  dboolean refresh );


// Percent widget routines
void STlib_initPercent
( st_percent_t* p,
  int x,
  int y,
  const patchnum_t* pl,
  int* num,
  dboolean* on,
  const patchnum_t* percent );


void STlib_updatePercent
( st_percent_t* per,
  int cm,
  int refresh );


// Multiple Icon widget routines
void STlib_initMultIcon
( st_multicon_t* mi,
  int x,
  int y,
  const patchnum_t*   il,
  int* inum,
  dboolean* on );


void STlib_updateMultIcon
( st_multicon_t* mi,
  dboolean refresh );

// Binary Icon widget routines

void STlib_initBinIcon
( st_binicon_t* b,
  int x,
  int y,
  const patchnum_t* i,
  dboolean* val,
  dboolean* on );

void STlib_updateBinIcon
( st_binicon_t* bi,
  dboolean refresh );

#endif

/* vi: set et ts=2 sw=2: */


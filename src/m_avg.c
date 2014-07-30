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

#include "lprintf.h"
#include "m_avg.h" 

avg_t* M_AverageNew(void) {
  avg_t *avg = calloc(1, sizeof(avg_t));

  if (avg == NULL)
    I_Error("M_NewAverage: calloc returned NULL\n");

  return avg;
}

void M_AverageInit(avg_t *avg) {
  avg->count = 0;
  avg->value = 0;
}

void M_AverageUpdate(avg_t *avg, int new_data_point) {
  unsigned int old_count = avg->count;

  avg->count++;

  avg->value = ((avg->value * old_count) + new_data_point) / avg->count;
}

/* vi: set et ts=2 sw=2: */


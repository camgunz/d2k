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


#ifndef I_SMP_H__
#define I_SMP_H__

typedef struct smp_item_s
{
  volatile int size;
  volatile int index;
  volatile int count;

  union
  {
    void *item;
    draw_column_vars_t* segs;
    draw_span_vars_t* spans;
  } data;
} smp_item_t;

extern int use_smp;
extern int use_smp_default;

void SMP_Init(void);
void SMP_Free(void);

void SMP_WakeRenderer(void);
void SMP_FrontEndSleep(void);

void SMP_ColFunc(draw_column_vars_t *data);
void SMP_SpanFunc(draw_span_vars_t *data);

#endif // __I_SMP__

/* vi: set et ts=2 sw=2: */


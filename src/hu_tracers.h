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


#ifndef HU_TRACERS_H__
#define HU_TRACERS_H__

struct mobj_s;
typedef struct mobj_s mobj_t;

struct line_s;
typedef struct line_s line_t;

#define MAXTRACEITEMS 8

typedef enum {
  TRACE_HEALTH,
  TRACE_PICKUP,
  TRACE_CROSS,
  TRACE_DAMAGE,
  NUMTRACES
} tracertype_t;

typedef struct {
  int  index;
  char value[16];
  int  data1;
} traceitem_t;

typedef void (*TRACERFUNC)(tracertype_t index);

typedef struct traceslist_s {
  traceitem_t items[MAXTRACEITEMS];
  int         count;
  char        hudstr[80];
  char        cmd[32];
  char        prefix[32];
  TRACERFUNC  ApplyFunc;
  TRACERFUNC  ResetFunc;
} traceslist_t;

typedef struct traceslistinit_s {
  char       cmd[32];
  char       prefix[32];
  TRACERFUNC ApplyFunc;
  TRACERFUNC ResetFunc;
} traceslistinit_t;

extern traceslist_t traces[];
extern bool traces_present;

void InitTracers(void);

void CheckGivenDamageTracer(mobj_t *mobj, int damage);
void CheckThingsPickupTracer(mobj_t *mobj);
void CheckThingsHealthTracer(mobj_t *mobj);
void CheckLinesCrossTracer(line_t *line);
void ClearLinesCrossTracer(void);

void TracerClearStarts(void);
void TracerAddDeathmatchStart(int num, int index);
void TracerAddPlayerStart(int num, int index);
int  TracerGetDeathmatchStart(int index);
int  TracerGetPlayerStart(int index);

#endif // HU_TRACERS_H__

/* vi: set et ts=2 sw=2: */


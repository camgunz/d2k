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


#ifndef G_OVERFLOW_H__
#define G_OVERFLOW_H__

#include "doomtype.h"
#include "doomdata.h"
#include "p_maputl.h"

typedef struct overrun_param_s
{
  int warn;
  int emulate;
  int footer;
  int footer_emulate;
  int promted;
  int shit_happens;
} overrun_param_t;

typedef enum overrun_list_s
{
  OVERFLOW_SPECHIT,
  OVERFLOW_REJECT,
  OVERFLOW_INTERCEPT,
  OVERFLOW_PLYERINGAME,
  OVERFLOW_DONUT,
  OVERFLOW_MISSEDBACKSIDE,

  OVERFLOW_MAX //last
} overrun_list_t;

extern overrun_param_t overflows[];
extern const char *overflow_cfgname[OVERFLOW_MAX];

#define EMULATE(overflow) (overflows[overflow].footer ? overflows[overflow].footer_emulate : overflows[overflow].emulate)
#define PROCESS(overflow) (overflows[overflow].warn || EMULATE(overflow))

// e6y
//
// intercepts overrun emulation
// See more information on:
// doomworld.com/vb/doom-speed-demos/35214-spechits-reject-and-intercepts-overflow-lists
//
// Thanks to Simon Howard (fraggle) for refactor the intercepts
// overrun code so that it should work properly on big endian machines
// as well as little endian machines.

#define MAXINTERCEPTS_ORIGINAL 128

typedef struct
{
    int len;
    void *addr;
    bool int16_array;
} intercepts_overrun_t;

extern intercepts_overrun_t intercepts_overrun[];
void InterceptsOverrun(int num_intercepts, intercept_t *intercept);

//
// playeringame overrun emulation
//

int PlayeringameOverrun(const mapthing_t* mthing);

//
// spechit overrun emulation
//

// Spechit overrun magic value.
#define DEFAULT_SPECHIT_MAGIC 0x01C09C98

typedef struct spechit_overrun_param_s
{
  line_t *line;

  line_t ***spechit;
  int *numspechit;

  fixed_t *tmbbox;
  fixed_t *tmfloorz;
  fixed_t *tmceilingz;

  bool *crushchange;
  bool *nofit;
} spechit_overrun_param_t;

extern unsigned int spechit_baseaddr;

void SpechitOverrun(spechit_overrun_param_t *params);

//
// reject overrun emulation
//

void RejectOverrun(int rejectlump, const byte **rejectmatrix, int totallines);

//
// donut overrun emulation (linedef action 9)
//

int DonutOverrun(fixed_t *pfloorheight, short *pfloorpic);

int MissedBackSideOverrun(line_t *line);
sector_t* GetSectorAtNullAddress(void);

#endif // __G_OVERFLOW__

/* vi: set et ts=2 sw=2: */


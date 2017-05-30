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

#include "m_argv.h"
#include "m_misc.h"
#include "g_game.h"
#include "p_map.h"
#include "p_mobj.h"
#include "pl_main.h"
#include "r_defs.h"
#include "v_video.h"

#include "hu_lib.h"
#include "hu_tracers.h"

typedef struct {
  int init_index;
  int index;
} PACKEDATTR tracer_mapthing_t;

static int given_damage_pertic[MAXTRACEITEMS];
static int given_damage_pertic_saved[MAXTRACEITEMS];
static int given_damage_total[MAXTRACEITEMS];
static int given_damage_processed[MAXTRACEITEMS];

static tracer_mapthing_t *deathmatchstarts_indexes = NULL;
static tracer_mapthing_t playerstarts_indexes[VANILLA_MAXPLAYERS];

int num_deathmatchstarts_indexes = 0;

int crossed_lines_count = 0;

bool traces_present;

void TracerApply(tracertype_t index);

void GivenDamageReset(tracertype_t index);

void GivenDamageApply(tracertype_t index);

traceslist_t traces[NUMTRACES];

void InitTracers(void) {
  traceslistinit_t traces_init[NUMTRACES] = {
    { "-trace_thingshealth", "health ", TracerApply, NULL },
    { "-trace_thingspickup", "pickup ", TracerApply, NULL },
    { "-trace_linescross", "lcross ", TracerApply, NULL },
    { "-trace_givendamage", "damage ", GivenDamageApply, GivenDamageReset },
  };

  traces_present = false;

  for (int i = 0; i < NUMTRACES; i++) {
    int p;
    int count;
    int value;

    strcpy(traces[i].cmd, traces_init[i].cmd);
    strcpy(traces[i].prefix, traces_init[i].prefix);
    traces[i].ApplyFunc = traces_init[i].ApplyFunc;
    traces[i].ResetFunc = traces_init[i].ResetFunc;

    count = 0;
    traces[i].count = 0;

    if ((p = M_CheckParm(traces[i].cmd)) && (p < myargc - 1)) {
      while (count < 3 && p + count < myargc - 1 &&
        M_StrToInt(myargv[p + 1 + count], &value)) {
        sprintf(traces[i].items[count].value, "\x1b\x36%d\x1b\x33 0", value);
        traces[i].items[count].index = value;

        if (traces[i].ApplyFunc) {
          traces[i].ApplyFunc(i);
        }

        traces_present = true;
        count++;
      }
      traces[i].count = count;
    }
  }
}

void TracerApply(tracertype_t index) {
  strcpy(traces[index].hudstr, traces[index].prefix);

  for (int i = 0; i < traces[index].count; i++) {
    sprintf(traces[index].hudstr + strlen(traces[index].hudstr), "\x1b\x33%s ",
      traces[index].items[i].value
    );
  }
}

void CheckThingsPickupTracer(mobj_t *mobj) {
  if (traces[TRACE_PICKUP].count) {
    for (int i = 0; i < traces[TRACE_PICKUP].count; i++) {
      if (mobj->index == traces[TRACE_PICKUP].items[i].index) {
        sprintf(traces[TRACE_PICKUP].items[i].value,
          "\x1b\x36%d \x1b\x33%05.2f",
          traces[TRACE_PICKUP].items[i].index,
          (float)(leveltime) / 35
        );
      }
    }
  }
}

void CheckThingsHealthTracer(mobj_t *mobj) {
  if (traces[TRACE_HEALTH].count) {
    for (int i = 0; i < traces[TRACE_HEALTH].count; i++) {
      if (mobj->index == traces[TRACE_HEALTH].items[i].index) {
        sprintf(traces[TRACE_HEALTH].items[i].value,
          "\x1b\x36%d \x1b\x33%d",
          mobj->index,
          mobj->health
        );
      }
    }
  }
}

void CheckLinesCrossTracer(line_t *line) {
  if (traces[TRACE_CROSS].count) {
    crossed_lines_count++;
    for (int i = 0; i < traces[TRACE_CROSS].count; i++) {
      if (line->iLineID == traces[TRACE_CROSS].items[i].index) {
        if (!traces[TRACE_CROSS].items[i].data1) {
          sprintf(traces[TRACE_CROSS].items[i].value,
            "\x1b\x36%d \x1b\x33%05.2f",
            traces[TRACE_CROSS].items[i].index, (float)(leveltime) / 35);
          traces[TRACE_CROSS].items[i].data1 = 1;
        }
      }
    }
  }
}

void ClearLinesCrossTracer(void) {
  if (traces[TRACE_CROSS].count) {
    if (!crossed_lines_count) {
      for (int i = 0; i < traces[TRACE_CROSS].count; i++) {
        traces[TRACE_CROSS].items[i].data1 = 0;
      }
    }
    crossed_lines_count = 0;
  }
}

void CheckGivenDamageTracer(mobj_t *mobj, int damage) {
  if (traces[TRACE_DAMAGE].count) {
    for (int i = 0; i < traces[TRACE_DAMAGE].count; i++) {
      if (mobj->index == traces[TRACE_DAMAGE].items[i].index) {
        given_damage_processed[i] = false;
        given_damage_pertic[i] += damage;
        given_damage_total[i] += damage;
      }
    }
  }
}

void GivenDamageApply(tracertype_t index) {
  if (traces[index].count) {
    for (int i = 0; i < traces[index].count; i++) {
      if (!given_damage_processed[i]) {
        given_damage_processed[i] = true;
        given_damage_pertic_saved[i] = given_damage_pertic[i];
      }

      sprintf(traces[index].items[i].value,
        "\x1b\x36%d \x1b\x33%d/\x1b\x33%d",
        traces[index].items[i].index,
        given_damage_pertic_saved[i],
        given_damage_total[i]);
      TracerApply(index);
    }
  }
}

void GivenDamageReset(tracertype_t index) {
  for (int i = 0; i < traces[index].count; i++) {
    given_damage_pertic[i] = 0;
  }
}

void TracerAddDeathmatchStart(int num, int index) {
  if (num >= num_deathmatchstarts_indexes) {
    num_deathmatchstarts_indexes = num + 1;

    deathmatchstarts_indexes = realloc(
      deathmatchstarts_indexes,
      num_deathmatchstarts_indexes * sizeof(deathmatchstarts_indexes[0])
    );
  }

  deathmatchstarts_indexes[num].index = index;
}

void TracerAddPlayerStart(int num, int index) {
  if (traces_present) {
    // init
    if (gametic == 0) {
      playerstarts_indexes[num].init_index = index;
      playerstarts_indexes[num].index = index;
    }
    else {
      playerstarts_indexes[num].index = playerstarts_indexes[num].init_index;
    }
  }
}

int TracerGetDeathmatchStart(int index) {
  if (index >= num_deathmatchstarts_indexes) {
    I_Error("TracerGetDeathmatchStart: index out of bounds");
  }

  return deathmatchstarts_indexes[index].index;
}

int TracerGetPlayerStart(int index) {
  if (index >= VANILLA_MAXPLAYERS) {
    I_Error("TracerGetDeathmatchStart: index out of bounds");
  }

  return playerstarts_indexes[index].index;
}

void TracerClearStarts(void) {
  for (int i = 0; i < VANILLA_MAXPLAYERS; i++) {
    playerstarts_indexes[i].index = 0;
  }

  num_deathmatchstarts_indexes = 0;

  if (deathmatchstarts_indexes) {
    free(deathmatchstarts_indexes);
    deathmatchstarts_indexes = NULL;
  }
}

/* vi: set et ts=2 sw=2: */

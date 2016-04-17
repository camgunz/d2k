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

#include "w_wad.h"
#include "tables.h"

// killough 5/3/98: reformatted

int SlopeDiv(unsigned int num, unsigned int den) {
  unsigned ans;

  if (den < 512) {
    return SLOPERANGE;
  }

  ans = (num << 3) / (den >> 8);

  return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

// [crispy] catch SlopeDiv overflows, only used in rendering
int SlopeDivEx(unsigned int num, unsigned int den) {
  uint64_t ans;

  if (den < 512) {
    return SLOPERANGE;
  }

  ans = ((uint64_t)num << 3) / (den >> 8);

  return ans <= SLOPERANGE ? (int)ans : SLOPERANGE;
}

fixed_t finetangent[4096];

//const fixed_t *const finecosine = &finesine[FINEANGLES/4];

fixed_t finesine[10240];

angle_t tantoangle[2049];

#include "m_swap.h"

// R_LoadTrigTables
// Load trig tables from a wad file lump
// CPhipps 24/12/98 - fix endianness (!)
//
void R_LoadTrigTables(void) {
  int lump;

  {
    lump = (W_CheckNumForName)("SINETABL", ns_prboom);

    if (lump == -1) {
      I_Error("Failed to locate trig tables");
    }

    if (W_LumpLength(lump) != sizeof(finesine)) {
      I_Error("R_LoadTrigTables: Invalid SINETABL");
    }

    W_ReadLump(lump, (unsigned char *)finesine);
  }

  {
    lump = (W_CheckNumForName)("TANGTABL", ns_prboom);

    if (lump == -1) {
      I_Error("Failed to locate trig tables");
    }

    if (W_LumpLength(lump) != sizeof(finetangent)) {
      I_Error("R_LoadTrigTables: Invalid TANGTABL");
    }

    W_ReadLump(lump, (unsigned char *)finetangent);
  }

  {
    lump = (W_CheckNumForName)("TANTOANG", ns_prboom);

    if (lump == -1) {
      I_Error("Failed to locate trig tables");
    }

    if (W_LumpLength(lump) != sizeof(tantoangle)) {
      I_Error("R_LoadTrigTables: Invalid TANTOANG");
    }

    W_ReadLump(lump,(unsigned char *)tantoangle);
  }
  // Endianness correction - might still be non-portable, but is fast where possible
  {
    size_t n;
    D_Msg(MSG_INFO, "Endianness...");

    // This test doesn't assume the endianness of the tables, but deduces them from
    // en entry. I hope this is portable.
    if ((10 < finesine[1]) && (finesine[1] < 100)) {
      D_Msg(MSG_INFO, "ok.");
      return; // Endianness is correct
    }

    // Must correct endianness of every long loaded (!)
#define CORRECT_TABLE_ENDIAN(tbl)                        \
    for (n = 0; n < sizeof(tbl) / sizeof(tbl[0]); n++) { \
      tbl[n] = doom_swap_l(tbl[n]);                      \
    }

    CORRECT_TABLE_ENDIAN(finesine)
    CORRECT_TABLE_ENDIAN(finetangent)
    CORRECT_TABLE_ENDIAN(tantoangle)

    D_Msg(MSG_INFO, "corrected.");
  }
}

/* vi: set et ts=2 sw=2: */


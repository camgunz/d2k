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

#include "doomdef.h"
#include "r_defs.h"
#include "r_sky.h"
#include "r_main.h"
#include "r_state.h"
#include "e6y.h"

//
// sky mapping
//
int skyflatnum;
int skytexture;
int skytexturemid;

int r_stretchsky = 1;
int skystretch;
fixed_t freelookviewheight;

//
// R_InitSkyMap
// Called whenever the view size changes.
//
void R_InitSkyMap(void)
{
  if (!GetMouseLook())
  {
    skystretch = false;
    skytexturemid = 100*FRACUNIT;
    if (viewwidth != 0)
    {
      skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT));
    }
  }
  else
  {
    int skyheight;

    if (!textureheight)
      return;

    // There are various combinations for sky rendering depending on how tall the sky is:
    //        h <  128: Unstretched and tiled, centered on horizon
    // 128 <= h <  200: Can possibly be stretched. When unstretched, the baseline is
    //                  28 rows below the horizon so that the top of the texture
    //                  aligns with the top of the screen when looking straight ahead.
    //                  When stretched, it is scaled to 228 pixels with the baseline
    //                  in the same location as an unstretched 128-tall sky, so the top
    //					of the texture aligns with the top of the screen when looking
    //                  fully up.
    //        h == 200: Unstretched, baseline is on horizon, and top is at the top of
    //                  the screen when looking fully up.
    //        h >  200: Unstretched, but the baseline is shifted down so that the top
    //                  of the texture is at the top of the screen when looking fully up.

    skyheight = textureheight[skytexture]>>FRACBITS;
    skystretch = false;
    skytexturemid = 0;
    if (skyheight >= 128 && skyheight < 200)
    {
      skystretch = (r_stretchsky && skyheight >= 128);
      skytexturemid = -28*FRACUNIT;
    }
    else if (skyheight > 200)
    {
      skytexturemid = (200 - skyheight) << FRACBITS;
    }

    if (viewwidth != 0 && viewheight != 0)
    {
      //skyiscale = 200 * FRACUNIT / freelookviewheight;
      skyiscale = (fixed_t)(((uint64_t)FRACUNIT * SCREENWIDTH * 200) / (viewwidth * SCREENHEIGHT));
      // line below is from zdoom, but it works incorrectly with prboom
      // with widescreen resolutions (eg 1280x720) by some reasons
      //skyiscale = (fixed_t)((int64_t)skyiscale * FieldOfView / 2048);
    }

    if (skystretch)
    {
      skyiscale = (fixed_t)((int64_t)skyiscale * skyheight / SKYSTRETCH_HEIGHT);
      skytexturemid = (int)((int64_t)skytexturemid * skyheight / SKYSTRETCH_HEIGHT);
    }
    else
    {
      skytexturemid = 100*FRACUNIT;
    }
  }
}

/* vi: set et ts=2 sw=2: */


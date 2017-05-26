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

//
// M_GetCurrentTime
//

double M_GetCurrentTime(void) {
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_sec + (((double)tv.tv_usec) / 1000000.0);
}

int M_StrToInt(const char *s, int *l) {
  return (
    (sscanf(s, " 0x%x", l) == 1) ||
    (sscanf(s, " 0X%x", l) == 1) ||
    (sscanf(s, " 0%o", l) == 1) ||
    (sscanf(s, " %d", l) == 1)
  );
}

int M_StrToFloat(const char *s, float *f) {
  return sscanf(s, " %f", f) == 1;
}

int M_DoubleToInt(double x) {
#ifdef __GNUC__
  double tmp = x;
  return (int)tmp;
#else 
  return (int)x;
#endif
}

char* M_Strlwr(char *str) {
  char *p;

  for (p = str; *p; p++) {
    *p = tolower(*p);
  }

  return str;
}

char* M_Strupr(char *str) {
  char *p;

  for (p = str; *p; p++) {
    *p = toupper(*p);
  }

  return str;
}

char* M_StrRTrim(char *str) {
  char *end;

  if (str) {
    end = str + strlen(str) - 1;

    while (end > str && isspace(*end)) {
      end--;
    }

    *(end + 1) = 0;
  }

  return str;
}

//
// NormalizeSlashes
//
// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// jff 4/19/98 Make killoughs slash fixer a subroutine
//
char* M_StrNormalizeSlashes(char *str) {
  size_t l;

  // killough 1/18/98: Neater / \ handling.
  // Remove trailing / or \ to prevent // /\ \/ \\, and change \ to /

  if (!str) {
    return str;
  }

  l = strlen(str);

  if (!l) {
    return str;
  }

  if (str[--l] == '/' || str[l] == '\\') {   // killough 1/18/98
    str[l] = 0;
  }

  while (l--) {
    if (str[l] == '\\') {
      str[l] = '/';
    }
  }

  return str;
}

/* vi: set et ts=2 sw=2: */

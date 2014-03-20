/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Low level UDP network interface. This is shared between the server
 *  and client, with SERVER defined for the former to select some extra
 *  functions. Handles socket creation, and packet send and receive.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.h"
#include "doomtype.h"
#include "m_utf.h"

#include <iconv.h>

#define CHUNK_SIZE 32
/*
 * #define INTERNAL_ENCODING "UTF-32LE"
 */
#define INTERNAL_ENCODING "UTF-8"

static char *utf_error = NULL;

static void set_utf_error(const char *error) {
  if (utf_error)
    free(utf_error);

  utf_error = strdup(error);
}

const char* M_GetUTFError(void) {
  return utf_error;
}

dboolean M_IsControlChar(wchar_t sc) {
  if (sc < 0x20)
    return 1;
  if (sc == 0x7f)
    return 1;
  if ((sc > 0x7e) && (sc < 0xa0))
    return 1;

  return 0;
}

dboolean M_MustFeedLine(rune r) {
  switch (r) {
    case 0x000A:
    case 0x000D:
    case 0x0085:
    case 0x2028:
    case 0x2029:
      return true;
    default:
      return false;
  }
}

dboolean M_CanBreakLine(rune r) {
  switch (r) {
    case 0x0009:
    case 0x0020:
    case 0x1680:
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200A:
        return true;
    default:
        return false;
  }
}

size_t M_DecodeASCII(rune **out, char *in, size_t in_size) {
  iconv_t cd;
  rune *working_out, *saved_out;
  size_t old_size = 0;
  size_t chunk_count = 1;
  size_t res, out_size, out_bytes_left;

  cd = iconv_open(INTERNAL_ENCODING, "ASCII");
  if (cd == ((iconv_t) - 1)) {
    set_utf_error("Couldn't initialize conversion from ASCII to Unicode");
    return 0;
  }

  out_size = out_bytes_left = sizeof(rune) * CHUNK_SIZE * chunk_count;
  working_out = saved_out = malloc(out_size);

  while (1) {
    res = iconv(cd, &in, &in_size, (char **)&working_out, &out_bytes_left);

    if (!res) {
      iconv_close(cd);
      break;
    }
    else if (errno == EILSEQ) {
      set_utf_error("Input byte does not belong to input codeset.");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
    else if (errno == EINVAL) {
      set_utf_error("Incomplete char or shift sequence in input");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
    else if ((errno == E2BIG) || (out_bytes_left < CHUNK_SIZE)) {
      old_size = working_out - saved_out;
      out_size = sizeof(rune) * CHUNK_SIZE * ++chunk_count;
      working_out = saved_out = realloc(saved_out, out_size);
      working_out += old_size;
      out_bytes_left = CHUNK_SIZE;
    }
    else {
      set_utf_error("Unknown error occurred");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
  }

  (*out) = saved_out;
  return (working_out - saved_out) * sizeof(rune);
}

size_t M_EncodeWCHAR(wchar_t **out, rune *in, size_t in_size) {
  iconv_t cd;
  wchar_t *working_out, *saved_out;
  size_t old_size = 0;
  size_t chunk_count = 1;
  size_t res, out_size, out_bytes_left;

  cd = iconv_open("wchar_t", INTERNAL_ENCODING);
  if (cd == ((iconv_t) - 1)) {
    set_utf_error("Couldn't initialize conversion from Unicode to wchar_t");
    return 0;
  }

  out_size = out_bytes_left = sizeof(wchar_t) * CHUNK_SIZE * chunk_count;
  working_out = saved_out = malloc(out_size);

  while (1) {
    res = iconv(
      cd, (char **)&in, &in_size, (char **)&working_out, &out_bytes_left
    );

    if (!res) {
      iconv_close(cd);
      break;
    }

    if (errno == EILSEQ) {
      set_utf_error("Input byte does not belong to input codeset.");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
    else if (errno == EINVAL) {
      set_utf_error("Incomplete char or shift sequence in input");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
    else if ((errno == E2BIG) || (out_bytes_left < CHUNK_SIZE)) {
      old_size = working_out - saved_out;
      out_size = sizeof(wchar_t) * CHUNK_SIZE * ++chunk_count;
      working_out = saved_out = realloc(saved_out, out_size);
      working_out += old_size;
      out_bytes_left = CHUNK_SIZE;
    }
    else {
      set_utf_error("Unknown error occurred");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
  }

  (*out) = saved_out;
  return (working_out - saved_out) * sizeof(wchar_t);
}

size_t M_DecodeWCHAR(rune **out, wchar_t *in, size_t in_size) {
  iconv_t cd;
  rune *working_out, *saved_out;
  size_t old_size = 0;
  size_t chunk_count = 1;
  size_t res, out_size, out_bytes_left;

  cd = iconv_open(INTERNAL_ENCODING, "wchar_t");
  if (cd == ((iconv_t) - 1)) {
      set_utf_error("Couldn't initialize conversion from wchar_t to Unicode");
      return 0;
  }

  out_size = out_bytes_left = sizeof(rune) * CHUNK_SIZE * chunk_count;
  working_out = saved_out = malloc(out_size);

  while (1) {
    res = iconv(
      cd, (char **)&in, &in_size, (char **)&working_out, &out_bytes_left
    );

    if (!res) {
      iconv_close(cd);
      break;
    }
    else if (errno == EILSEQ) {
      set_utf_error("Input byte does not belong to input codeset.");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
    else if (errno == EINVAL) {
      set_utf_error("Incomplete char or shift sequence in input");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
    else if ((errno == E2BIG) || (out_bytes_left < CHUNK_SIZE)) {
      old_size = working_out - saved_out;
      out_size = sizeof(rune) * CHUNK_SIZE * ++chunk_count;
      working_out = saved_out = realloc(saved_out, out_size);
      working_out += old_size;
      out_bytes_left = CHUNK_SIZE;
    }
    else {
      set_utf_error("Unknown error occurred");
      free(saved_out);
      (*out) = NULL;
      iconv_close(cd);
      return 0;
    }
  }

  (*out) = saved_out;
  return (working_out - saved_out) * sizeof(rune);
}

size_t M_DecodeWCHARNoAlloc(rune *out, uint16_t *in, size_t out_size,
                            size_t in_size) {
  iconv_t cd;
  size_t res, out_bytes_left;
  rune *saved_out = out;

  cd = iconv_open(INTERNAL_ENCODING, "wchar_t");
  if (cd == ((iconv_t) - 1)) {
    set_utf_error("Couldn't initialize conversion from wchar_t to Unicode");
    return 0;
  }

  out_bytes_left = out_size;

  res = iconv(cd, (char **)&in, &in_size, (char **)&out, &out_bytes_left);
  iconv_close(cd);

  if (res) {
    if (errno == EILSEQ)
      set_utf_error("Input byte does not belong input codeset.");
    else if (errno == EINVAL)
      set_utf_error("Incomplete character or shift sequence in input.");
    else if (errno == E2BIG)
      set_utf_error("Output buffer exhausted.");
    else
      set_utf_error("Unknown error occurred.");
    return 0;
  }

  return (out - saved_out) * sizeof(rune);
}

size_t M_EncodeLocal(char **out, rune *in, size_t in_size) {
  wchar_t *temp = NULL;
  size_t wchar_size = M_EncodeWCHAR(&temp, in, in_size);
  size_t char_size = wcstombs(NULL, temp, 0);
  size_t r;

  if (!wchar_size)
    return 0;

  if (char_size == ((size_t)-1)) {
    free(temp);
    return 0;
  }

  (*out) = malloc(char_size * sizeof(char));
  r = wcstombs((*out), temp, char_size * sizeof(char));
  free(temp);

  return r;
}

size_t M_DecodeLocal(rune **out, char *in, size_t in_size) {
  size_t len = mbstowcs(NULL, in, 0);
  wchar_t *temp = malloc(len * sizeof(wchar_t));
  size_t r = M_DecodeWCHAR(out, temp, len * sizeof(wchar_t));

  free(temp);

  return r;
}


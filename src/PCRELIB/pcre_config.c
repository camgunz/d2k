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


/*************************************************
*      Perl-Compatible Regular Expressions       *
*************************************************/

/* PCRE is a library of functions to support regular expressions whose syntax
and semantics are as close as possible to those of the Perl 5 language.

                       Written by Philip Hazel
           Copyright (c) 1997-2006 University of Cambridge

-----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of the University of Cambridge nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/


/* This module contains the external function pcre_config(). */


#include "pcre_internal.h"


/*************************************************
* Return info about what features are configured *
*************************************************/

/* This function has an extensible interface so that additional items can be
added compatibly.

Arguments:
  what             what information is required
  where            where to put the information

Returns:           0 if data returned, negative on error
*/

PCRE_DATA_SCOPE int
pcre_config(int what, void *where)
{
switch (what)
  {
  case PCRE_CONFIG_UTF8:
#ifdef SUPPORT_UTF8
  *((int *)where) = 1;
#else
  *((int *)where) = 0;
#endif
  break;

  case PCRE_CONFIG_UNICODE_PROPERTIES:
#ifdef SUPPORT_UCP
  *((int *)where) = 1;
#else
  *((int *)where) = 0;
#endif
  break;

  case PCRE_CONFIG_NEWLINE:
  *((int *)where) = NEWLINE;
  break;

  case PCRE_CONFIG_LINK_SIZE:
  *((int *)where) = LINK_SIZE;
  break;

  case PCRE_CONFIG_POSIX_MALLOC_THRESHOLD:
  *((int *)where) = POSIX_MALLOC_THRESHOLD;
  break;

  case PCRE_CONFIG_MATCH_LIMIT:
  *((unsigned int *)where) = MATCH_LIMIT;
  break;

  case PCRE_CONFIG_MATCH_LIMIT_RECURSION:
  *((unsigned int *)where) = MATCH_LIMIT_RECURSION;
  break;

  case PCRE_CONFIG_STACKRECURSE:
#ifdef NO_RECURSE
  *((int *)where) = 0;
#else
  *((int *)where) = 1;
#endif
  break;

  default: return PCRE_ERROR_BADOPTION;
  }

return 0;
}

/* End of pcre_config.c */

/* vi: set et ts=2 sw=2: */


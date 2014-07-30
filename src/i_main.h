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
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef __I_MAIN__
#define __I_MAIN__

//
// e6y: exeptions handling
//

typedef enum
{
  EXEPTION_NONE,
  EXEPTION_glFramebufferTexture2DEXT,
  EXEPTION_MAX
} ExeptionsList_t;

typedef struct
{
  const char * error_message;
} ExeptionParam_t;

extern ExeptionParam_t ExeptionsParams[];

void I_ExeptionBegin(ExeptionsList_t exception_index);
void I_ExeptionEnd(void);
void I_ExeptionProcess(void);

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__INTEL_COMPILER))
void I_Warning(const char *message, ...);
#define PRBOOM_TRY(exception_index) __try
#define PRBOOM_EXCEPT(exception_index) __except(EXCEPTION_EXECUTE_HANDLER) { I_Warning("%s", ExeptionsParams[exception_index]); }
#else
#define PRBOOM_TRY(exception_index) I_ExeptionBegin(exception_index);
#define PRBOOM_EXCEPT(exception_index) I_ExeptionEnd();
#endif

void I_Init(void);
void I_SafeExit(int rc);

extern int (*I_GetTime)(void);

#endif


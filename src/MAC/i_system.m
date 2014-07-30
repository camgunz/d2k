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


/* Mac path finding and WAD selection UI */

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomdef.h"
#include "doomtype.h"
#include "dstrings.h"
#include "d_main.h"
#include "m_fixed.h"
#include "i_system.h"

#import <Foundation/NSArray.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSString.h>
#import <Foundation/NSFileManager.h>

static NSString *libraryDir(void)
{
  return [@"~/Library/Application Support/PrBoom-Plus" stringByExpandingTildeInPath];
}

static char *NSStringToCString(NSString *str)
{
  char *cStr = malloc([str lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1);
  strcpy(cStr, [str UTF8String]);
  return cStr;
}

static char *NSStringToCStringStatic(NSString *str)
{
  static char *cStr = 0;
  static size_t cStrLen = 0;

  int len = [str lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1;

  if(!cStr)
  {
    cStr = malloc(len);
    cStrLen = len;
  }
  else if(cStrLen < len)
  {
    free(cStr);
    cStr = malloc(len);
    cStrLen = len;
  }

  strcpy(cStr, [str UTF8String]);
  return cStr;
}

static char *macExeDir = 0;
const char *I_DoomExeDir(void)
{
  if(macExeDir)
    return macExeDir;

  NSString *exeDir = libraryDir();
  [[NSFileManager defaultManager] createDirectoryAtPath:exeDir attributes:nil];

  macExeDir = NSStringToCString(exeDir);
  return macExeDir;
}

const char* I_GetTempDir(void)
{
  if(macExeDir)
    return macExeDir;

  NSString *exeDir = libraryDir();
  [[NSFileManager defaultManager] createDirectoryAtPath:exeDir attributes:nil];

  macExeDir = NSStringToCString(exeDir);
  return macExeDir;
}

char *I_FindFileInternal(const char *wf_name, const char *ext, bool isStatic)
{
  NSArray *paths = [NSArray arrayWithObject:libraryDir()];
  paths = [paths arrayByAddingObject: [[NSBundle mainBundle] resourcePath]];
  paths = [paths arrayByAddingObject: @""];

  char *retval = 0;
  int i;
  for(i = 0; !retval && (i < [paths count]); ++i)
  {
    NSString *path = [NSString stringWithFormat:@"%@/%@",
                      [paths objectAtIndex:i],
                      [NSString stringWithUTF8String:wf_name]];


    if([[NSFileManager defaultManager] isReadableFileAtPath:path])
    {
      if(isStatic)
        retval = NSStringToCStringStatic(path);
      else
        retval = NSStringToCString(path);
    }
  }

  return retval;
}

char *I_FindFile(const char *wf_name, const char *ext)
{
  return I_FindFileInternal(wf_name, ext, false);
}

const char *I_FindFile2(const char *wf_name, const char *ext)
{
  return I_FindFileInternal(wf_name, ext, true);
}

/* vi: set et ts=2 sw=2: */


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


// This file is hereby placed in the Public Domain -- Neil Stevens

#import <Cocoa/Cocoa.h>

@interface ConsoleController : NSWindowController
{
	IBOutlet NSTextView *textView;

	NSMutableString *log;
	id launchDelegate;
}

- (id)initWithWindow:(id)window;
- (void)awakeFromNib;
- (void)dealloc;

- (void)launch:(NSString *)launchPath args:(NSArray *)args delegate:(id)delegate;

- (void)taskComplete:(NSNotification *)notification;
- (void)dataReady:(NSNotification *)notification;

@end

/* vi: set et ts=2 sw=2: */


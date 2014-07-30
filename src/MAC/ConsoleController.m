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

#import "ANSIString.h"
#import "ConsoleController.h"
#import "LauncherApp.h"

@implementation ConsoleController

- (id)initWithWindow:(id)window
{
	return [super initWithWindow:window];
}

- (void)awakeFromNib
{
	launchDelegate = nil;
	log = [[[NSMutableString alloc] init] retain];
}

- (void)dealloc
{
	[log release];
	[super dealloc];
}

- (void)launch:(NSString *)path args:(NSArray *)args delegate:(id)delegate
{
	launchDelegate = delegate;

	// clear console
	[log setString:@""];
	[textView setString:@""];

	NSTask *task = [[NSTask alloc] init];
	[task retain];
	[task setLaunchPath:path];
	[task setArguments:args];
	NSPipe *standardOutput = [[NSPipe alloc] init];
	[standardOutput retain];
	[task setStandardOutput:standardOutput];

	[[NSNotificationCenter defaultCenter] addObserver:self
	 selector:@selector(dataReady:)
	 name:NSFileHandleReadCompletionNotification
	 object:[standardOutput fileHandleForReading]];

	[[NSNotificationCenter defaultCenter]
	 addObserver:self selector:@selector(taskComplete:)
	 name:NSTaskDidTerminateNotification object:nil];

	[task launch];
	[[standardOutput fileHandleForReading] readInBackgroundAndNotify];
}

- (void)dataReady:(NSNotification *)notification
{
	NSData *data = [[notification userInfo]
	               objectForKey:NSFileHandleNotificationDataItem];
	NSFileHandle *handle = [notification object];
	if([data length])
	{
		NSString *string = [[NSString alloc] initWithData:data
		                    encoding:NSUTF8StringEncoding];
		[log appendString:string];
		[string release];

		[[textView textStorage] beginEditing];
		[[textView textStorage] setAttributedString:[ANSIString parseColorCodes:log]];
		[[textView textStorage] endEditing];

		// Scroll to bottom
		[textView scrollRangeToVisible:NSMakeRange([[textView string] length], 0)];

		[handle readInBackgroundAndNotify];
	}
	else
	{
		[[NSNotificationCenter defaultCenter]
		 removeObserver:self
		 name:NSFileHandleReadCompletionNotification
		 object:[notification object]];
	}
}

- (void)taskComplete:(NSNotification *)notification
{
	NSTask *task = [notification object];
	if ([task terminationStatus] != 0)
		[self showWindow:nil];
	[[NSNotificationCenter defaultCenter]
	 removeObserver:self
	 name:NSTaskDidTerminateNotification
	 object:[notification object]];
	if(launchDelegate)
		[launchDelegate taskEnded:self];
}

@end

/* vi: set et ts=2 sw=2: */


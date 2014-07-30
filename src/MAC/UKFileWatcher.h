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


/* =============================================================================
	FILE:		UKFileWatcher.h
	PROJECT:	Filie
    
    COPYRIGHT:  (c) 2005 M. Uli Kusterer, all rights reserved.
    
	AUTHORS:	M. Uli Kusterer - UK
    
    LICENSES:   GPL, Modified BSD

	REVISIONS:
		2005-02-25	UK	Created.
   ========================================================================== */

/*
    This is a protocol that file change notification classes should adopt.
    That way, no matter whether you use Carbon's FNNotify/FNSubscribe, BSD's
    kqueue or whatever, the object being notified can react to change
    notifications the same way, and you can easily swap one out for the other
    to cater to different OS versions, target volumes etc.
*/

// -----------------------------------------------------------------------------
//  Protocol:
// -----------------------------------------------------------------------------

@protocol UKFileWatcher

-(void) addPath: (NSString*)path;
-(void) removePath: (NSString*)path;

-(id)   delegate;
-(void) setDelegate: (id)newDelegate;

@end

// -----------------------------------------------------------------------------
//  Methods delegates need to provide:
// -----------------------------------------------------------------------------

@interface NSObject (UKFileWatcherDelegate)

-(void) watcher: (id<UKFileWatcher>)kq receivedNotification: (NSString*)nm forPath: (NSString*)fpath;

@end


// Notifications this sends:
//  (object is the file path registered with, and these are sent via the workspace notification center)
#define UKFileWatcherRenameNotification				@"UKKQueueFileRenamedNotification"
#define UKFileWatcherWriteNotification              @"UKKQueueFileWrittenToNotification"
#define UKFileWatcherDeleteNotification				@"UKKQueueFileDeletedNotification"
#define UKFileWatcherAttributeChangeNotification    @"UKKQueueFileAttributesChangedNotification"
#define UKFileWatcherSizeIncreaseNotification		@"UKKQueueFileSizeIncreasedNotification"
#define UKFileWatcherLinkCountChangeNotification	@"UKKQueueFileLinkCountChangedNotification"
#define UKFileWatcherAccessRevocationNotification	@"UKKQueueFileAccessRevocationNotification"

/* vi: set et ts=2 sw=2: */


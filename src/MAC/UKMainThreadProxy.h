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
	FILE:		UKMainThreadProxy.h
	PROJECT:	UKMainThreadProxy
    
    PURPOSE:    Send a message to object theObject to [theObject mainThreadProxy]
                instead and the message will be received on the main thread by
                theObject.

    COPYRIGHT:  (c) 2004 M. Uli Kusterer, all rights reserved.
    
	AUTHORS:	M. Uli Kusterer - UK
    
    LICENSES:   GPL, Modified BSD

	REVISIONS:
		2004-10-14	UK	Created.
   ========================================================================== */

// -----------------------------------------------------------------------------
//  Headers:
// -----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>


// -----------------------------------------------------------------------------
//  Categories:
// -----------------------------------------------------------------------------

@interface NSObject (UKMainThreadProxy)

-(id)	mainThreadProxy;		// You can't init or release this object.
-(id)	copyMainThreadProxy;	// Gives you a retained version.

@end


// -----------------------------------------------------------------------------
//  Classes:
// -----------------------------------------------------------------------------

/*
	This object is created as a proxy in a second thread for an existing object.
	All messages you send to this object will automatically be sent to the other
	object on the main thread, except NSObject methods like retain/release etc.
*/

@interface UKMainThreadProxy : NSObject
{
	IBOutlet id		target;
}

-(id)	initWithTarget: (id)targ;

@end

/* vi: set et ts=2 sw=2: */


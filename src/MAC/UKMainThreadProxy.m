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

#import "UKMainThreadProxy.h"


@implementation UKMainThreadProxy

-(id)	initWithTarget: (id)targ
{
	self = [super init];
	if( self )
		target = targ;
	
	return self;
}

-(id)	performSelector: (SEL)itemAction
{
	BOOL	does = [super respondsToSelector: itemAction];
	if( does )
		return [super performSelector: itemAction];
	
	if( ![target respondsToSelector: itemAction] )
		[self doesNotRecognizeSelector: itemAction];
	
	[target retain];
	[target performSelectorOnMainThread: itemAction withObject: nil waitUntilDone: YES];
	[target release];
	
	return nil;
}


-(id)	performSelector: (SEL)itemAction withObject: (id)obj
{
	BOOL	does = [super respondsToSelector: itemAction];
	if( does )
		return [super performSelector: itemAction withObject: obj];
	
	if( ![target respondsToSelector: itemAction] )
		[self doesNotRecognizeSelector: itemAction];
	
	[target retain];
	[obj retain];
	[target performSelectorOnMainThread: itemAction withObject: obj waitUntilDone: YES];
	[obj release];
	[target release];
	
	return nil;
}

-(BOOL)	respondsToSelector: (SEL)itemAction
{
	BOOL	does = [super respondsToSelector: itemAction];
	
	return( does || [target respondsToSelector: itemAction] );
}


// -----------------------------------------------------------------------------
//	Forwarding unknown methods to the target:
// -----------------------------------------------------------------------------

-(NSMethodSignature*)	methodSignatureForSelector: (SEL)itemAction
{
	NSMethodSignature*	sig = [super methodSignatureForSelector: itemAction];

	if( sig )
		return sig;
	
	return [target methodSignatureForSelector: itemAction];
}

-(void)	forwardInvocation: (NSInvocation*)invocation
{
    SEL itemAction = [invocation selector];

    if( [target respondsToSelector: itemAction] )
	{
		[invocation retainArguments];
		[target retain];
		[invocation performSelectorOnMainThread: @selector(invokeWithTarget:) withObject: target waitUntilDone: YES];
		[target release];
	}
	else
        [self doesNotRecognizeSelector: itemAction];
}


-(id)	mainThreadProxy     // Just in case someone accidentally sends this message to a main thread proxy.
{
	return self;
}

-(id)	copyMainThreadProxy	// Just in case someone accidentally sends this message to a main thread proxy.
{
	return [self retain];
}

@end


@implementation NSObject (UKMainThreadProxy)

-(id)	mainThreadProxy
{
	return [[[UKMainThreadProxy alloc] initWithTarget: self] autorelease];
}

-(id)	copyMainThreadProxy
{
	return [[UKMainThreadProxy alloc] initWithTarget: self];
}

@end

/* vi: set et ts=2 sw=2: */


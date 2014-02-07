/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
/*!	\file		EmailPreferencesView
 *	\brief		The central view for managing Email account settings.
 */
#ifndef _ABOUT_VIEW_H_
#define _ABOUT_VIEW_H_

#include <Bitmap.h>
#include <GraphicsDefs.h>
#include <InterfaceDefs.h>
#include <ScrollView.h>
#include <View.h>

#include "URLView.h"


class AboutView
	:
	public BView
{
	public:
		AboutView( BRect frame );
		virtual ~AboutView();
		
//		virtual void MessageReceived( BMessage* in );
		virtual void AttachedToWindow( void ) {};
		
		virtual void Draw( BRect temp );
		
	private:
		BBitmap* logo;
};

#endif // _EMAIL_PREFERENCES_VIEW_H_

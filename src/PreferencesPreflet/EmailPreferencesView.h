/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
/*!	\file		EmailPreferencesView
 *	\brief		The central view for managing Email account settings.
 */
#ifndef _EMAIL_PREFERENCES_VIEW_H_
#define _EMAIL_PREFERENCES_VIEW_H_

#include <Button.h>
#include <GraphicsDefs.h>
#include <InterfaceDefs.h>
#include <MenuField.h>
#include <Message.h>
#include <Rect.h>
#include <ScrollView.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <TextControl.h>
#include <View.h>

/* Message constants */
const uint32	kVerifyEmail 	= 'VEma';
const uint32	kVerifyServer 	= 'VSer';
const uint32	kVerifyServerPort	=	'VSeP';


class EmailPreferencesView
	:
	public BView
{
	public:
		EmailPreferencesView( BRect frame );
		virtual ~EmailPreferencesView();
		
		virtual void MessageReceived( BMessage* in );
		virtual void AttachedToWindow( void );
		
	private:
		BTextControl* 	senderEmail;
		BTextControl* 	mailServerAddress;
		BTextControl*	mailServerPort;
		
		BStringView*	explanationString;
		BStringView*	explanationString2;

};

#endif // _EMAIL_PREFERENCES_VIEW_H_

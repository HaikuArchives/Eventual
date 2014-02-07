/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _COLOR_UPDATE_WINDOW_H_
#define _COLOR_UPDATE_WINDOW_H_

// OS includes
#include <Button.h>
#include <ColorControl.h>
#include <Handler.h>
#include <InterfaceDefs.h>
#include <Looper.h>
#include <Message.h>
#include <Point.h>
#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>
#include <View.h>
#include <Window.h>

// Project includes
#include "Category.h"

// Message constants
const uint32	kColorSelected		= 'CSEL';
const uint32	kColorChanged		= 'CCHG';
const uint32 	kColorReverted 		= 'CREV';


/*!
 *	\brief		A control that allows editing color and optionally text label.
 *	\details	Opens in a new modal window that can't be closed except by clicking "Ok".
 */
class 	ColorUpdateWindow
	:
	public BWindow
{
	protected:
		rgb_color currentColor, originalColor;
		BString originalString;
		BColorControl* colorControl;
		BView* labelView;
		BButton* okButton;
		BButton* revertButton;
		BMessage* messageToSend;
		BHandler* target;
		BLooper* looper;
		bool dirty;		//!< Defines the settings were changed.
		bool enableEditingLabel;	//!< Defines the label can be edit.

	public:
		ColorUpdateWindow( const Category& catIn,
						   bool enableEditingLabelIn,
						   const char* title,
						   BHandler *target = NULL,
						   BLooper* looper = NULL,
						   BMessage* message = NULL );
		ColorUpdateWindow( BPoint corner,
						   BString &label,
						   rgb_color	&defaultColor,
						   bool enableEditingLabel = false,
						   const char* title = "Color selector",
						   BHandler *target = NULL,
						   screen_id currentScreen = B_MAIN_SCREEN_ID,
						   BMessage *message = NULL );
		virtual ~ColorUpdateWindow();
		virtual void MessageReceived(BMessage* message);
};

#endif // _COLOR_UPDATE_WINDOW_H_

/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
/*!	\file		TimePreferencesView
 *	\brief		The central view for managing Time settings.
 */
#ifndef _TIME_PREFERENCES_VIEW_H_
#define _TIME_PREFERENCES_VIEW_H_

// OS includes
#include <CheckBox.h>
#include <GraphicsDefs.h>
#include <InterfaceDefs.h>
#include <Message.h>
#include <Rect.h>
#include <SupportDefs.h>
#include <View.h>

// Local includes
#include "GeneralHourMinControl.h"
#include "TimeRepresentation.h"
#include "TimePreferences.h"

/* Message constants */
const uint32	k24hClockToggled					= '24CI';
const	uint32	kAppointmentDurationChanged	= 'ApDC';
const uint32	kReminderTimeChanged				= 'RmTC';
const uint32	kSnoozeTimeChanged				= 'SnTC';


class TimePreferencesView
	:
	public BView
{
	public:
		TimePreferencesView( BRect frame );
		virtual ~TimePreferencesView();
		
		virtual void MessageReceived( BMessage* in );
		virtual void AttachedToWindow( void );
		
	protected:
		// UI elements
		BCheckBox* use24hClock;
		GeneralHourMinControl* defaultAppointmentDuration;
		GeneralHourMinControl* defaultReminderTime;
		GeneralHourMinControl* defaultSnoozeTime;

};

#endif // _TIME_PREFERENCES_VIEW_H_

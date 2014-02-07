/*
 * Copyright 2011 AlexeyB Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// C++ includes
#include <stdio.h>
#include <stdlib.h>
 
// System includes

#include <GridLayout.h>
#include <GraphicsDefs.h>
#include <GroupLayout.h>
#include <Layout.h>
#include <LayoutItem.h>
#include <InterfaceDefs.h>
#include <Rect.h>
#include <SeparatorItem.h>
#include <View.h>
 
// Local includes

#include "TimePreferencesView.h"
#include "TimePreferences.h"			// Preferences of Time
#include "Utilities.h"

/*==============================================================================
 		Implementation of class TimePreferencesView
==============================================================================*/

/*!	\brief		Constructor of TimePreferencesView
 *	\details	Basically, sets up the view, and that's all.
 */
TimePreferencesView::TimePreferencesView( BRect frame )
	:
	BView( frame,
		   "Time preferences",
		   B_FOLLOW_LEFT | B_FOLLOW_TOP,
		   B_WILL_DRAW | B_FRAME_EVENTS ),
	use24hClock( NULL ),
	defaultAppointmentDuration( NULL ),
	defaultReminderTime( NULL )
{
	int time1, time2;
	TimePreferences* TimePrefs = pref_GetTimePreferences();
	if ( !TimePrefs )
	{
		/* Panic! */
		exit( 1 );
	}
	
	BLayoutItem *layoutItem = NULL;
	BGroupLayout* groupLayout = new BGroupLayout( B_VERTICAL );
	if ( ! groupLayout ) {
		/* Panic! */
		exit( 1 );
	}
	this->SetLayout( groupLayout );
	groupLayout->SetInsets( 10, 5, 10, 5 );
	this->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	
	// 24h clock checkbox
	BMessage* toSend = new BMessage( k24hClockToggled );
	if ( ! toSend ) {
		/* Panic! */
		exit( 1 );
	}
	this->use24hClock = new BCheckBox( BRect( 0, 0, 1, 1 ),
												  "use24h checkbox",
												  "The clock is 24 hours",
												  toSend );
	if ( !use24hClock ) {
		/* Panic! */
		exit( 1 );
	}
	use24hClock->ResizeToPreferred();
	use24hClock->SetValue( TimePrefs->Get24hClock() );
	if ( ( layoutItem = groupLayout->AddView( use24hClock ) ) != NULL ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	}

	// Appointment duration
	TimePrefs->GetDefaultAppointmentDuration( &time1, &time2 );
	toSend = new BMessage( kAppointmentDurationChanged );
	if ( ! toSend ) {
		/* Panic! */
		exit( 1 );
	}
	this->defaultAppointmentDuration = new GeneralHourMinControl( BRect( 0, 0, 1, 1 ),
																					  "AppointmentDuration",
																					  "Default appointment duration time:",
																					  "",
																					  toSend );
	if ( ! this->defaultAppointmentDuration ) {
		/* Panic! */
		exit( 1 );
	}
	this->defaultAppointmentDuration->SetCurrentTime( ( unsigned int )time1,
																	  ( unsigned int )time2 );
	this->defaultAppointmentDuration->ResizeToPreferred();
	if ( ( layoutItem = groupLayout->AddView( this->defaultAppointmentDuration ) ) != NULL )
	{
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	}
	
	// Reminder time
	TimePrefs->GetDefaultReminderTime( &time1, &time2 );
	toSend = new BMessage( kReminderTimeChanged );
	if ( ! toSend ) {
		/* Panic! */
		exit( 1 );
	}
	this->defaultReminderTime = new GeneralHourMinControl( BRect( 0, 0, 1, 1 ),
																					  "ReminderTime",
																					  "Reminder and event are separated by:",
																					  "",
																					  toSend );
	if ( ! this->defaultReminderTime ) {
		/* Panic! */
		exit( 1 );
	}
	this->defaultReminderTime->SetCurrentTime( ( unsigned int )time1,
															 ( unsigned int )time2 );
	this->defaultReminderTime->ResizeToPreferred();
	if ( ( layoutItem = groupLayout->AddView( this->defaultReminderTime ) ) != NULL )
	{
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	}
	
	// Snooze time
	TimePrefs->GetDefaultSnoozeTime( &time1, &time2 );
	toSend = new BMessage( kSnoozeTimeChanged );
	if ( ! toSend ) {
		/* Panic! */
		exit( 1 );
	}
	this->defaultSnoozeTime = new GeneralHourMinControl( BRect( 0, 0, 1, 1 ),
																		  "SnoozeTime",
																		  "By default, snooze to:",
																		  "",
																		  toSend );
	if ( ! this->defaultSnoozeTime ) {
		/* Panic! */
		exit( 1 );
	}
	this->defaultSnoozeTime->SetCurrentTime( ( unsigned int )time1,
															 ( unsigned int )time2 );
	this->defaultSnoozeTime->ResizeToPreferred();
	if ( ( layoutItem = groupLayout->AddView( this->defaultSnoozeTime ) ) != NULL )
	{
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	}
	
	
}	// <-- end of constructor for TimePreferencesView



/*!	\brief		Destructor.
 */
TimePreferencesView::~TimePreferencesView()
{
	TimePreferences* prefs = pref_GetTimePreferences();
	if ( !prefs ) {
		utl_Deb = new DebuggerPrintout( "There's no modified Time preferences!" );
	}
	
	if ( use24hClock ) {
		use24hClock->RemoveSelf();
		delete use24hClock;
		use24hClock = NULL;
	}
	
	if ( defaultReminderTime ) {
		defaultReminderTime->RemoveSelf();
		delete defaultReminderTime;
		defaultReminderTime = NULL;
	}
	
	if ( defaultAppointmentDuration ) {
		defaultAppointmentDuration->RemoveSelf();
		delete defaultAppointmentDuration;
		defaultAppointmentDuration = NULL;
	}
	
	if ( defaultSnoozeTime ) {
		defaultSnoozeTime->RemoveSelf();
		delete defaultSnoozeTime;
		defaultSnoozeTime = NULL;
	}
}	// <-- end of destructor	   


/*!	\brief		This function adds BViews's message handler to window's looper.
 */
void		TimePreferencesView::AttachedToWindow()
{
	BView::AttachedToWindow();
	
	BLooper* looper = this->Looper();
	if ( looper && looper->LockLooper() )
	{
		looper->AddHandler( ( BHandler* )this );
		looper->UnlockLooper();	
	}
	use24hClock->SetTarget( this );
	defaultAppointmentDuration->SetTarget( this );
	defaultReminderTime->SetTarget( this );
	defaultSnoozeTime->SetTarget( this );
}	// <-- end of function TimePreferencesView::AttachedToWindow()



/*!	\brief		Updating preferences according to the data entered by the user.
 */
void		TimePreferencesView::MessageReceived( BMessage* in )
{
	TimePreferences* prefs = pref_GetTimePreferences();
	int hours = 0, mins = 0;
	if ( !prefs )
	{
		/* Panic! */
		return BView::MessageReceived( in );
	}	

	switch( in->what ) {
		case k24hClockToggled:
			prefs->Set24hClock( ( use24hClock->Value() != 0 ) );
			break;
		
		case kAppointmentDurationChanged:
			if ( in->FindInt32( kHoursValueKey.String(), ( int32* )&hours ) != B_OK ) {
				hours = 0;
			}
			if ( in->FindInt32( kMinutesValueKey.String(), ( int32* )&mins ) != B_OK ) {
				mins = 0;
			}
			prefs->SetDefaultAppointmentDuration( hours, mins );
			break;
		
		case kReminderTimeChanged:
			if ( in->FindInt32( kHoursValueKey.String(), ( int32* )&hours ) != B_OK ) {
				hours = 0;
			}
			if ( in->FindInt32( kMinutesValueKey.String(), ( int32* )&mins ) != B_OK ) {
				mins = 0;
			}
			prefs->SetDefaultReminderTime( hours, mins );
			break;
		
		case kSnoozeTimeChanged:
			if ( in->FindInt32( kHoursValueKey.String(), ( int32* )&hours ) != B_OK ) {
				hours = 0;
			}
			if ( in->FindInt32( kMinutesValueKey.String(), ( int32* )&mins ) != B_OK ) {
				mins = 0;
			}
			prefs->SetDefaultSnoozeTime( hours, mins );
			break;
		
		default:
			BView::MessageReceived( in );	
	}
	
}	// <-- end of function TimePreferencesView::MessageReceived

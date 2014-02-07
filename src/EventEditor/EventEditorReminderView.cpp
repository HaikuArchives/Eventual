/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// OS includes
#include <GroupLayout.h>
#include <LayoutItem.h>
#include <Looper.h>
 
// Project includes
#include "EventEditorReminderView.h"
#include "TimePreferences.h"

/*-----------------------------------------
 *				Message constants
 *----------------------------------------*/
 
const uint32	kReminderEnabled		= 'ReEn';
const uint32	kReminderTimeUpdated = 'ReTU';



/*-------------------------------------------------------------------
 *				Implementation of class EventEditor_ReminderView
 *------------------------------------------------------------------*/

/*!	\brief		Constructor.
 *		\param[in]	frame		The rectangle of the View. Passed unmodified to
 *									constructor of BView.
 *		\param[in]	data		Data of the Event.
 *		\details		The user must check \c InitCheck() value. If it's not B_OK,
 *						the class did not initialize properly.
 */
EventEditor_ReminderView::EventEditor_ReminderView( BRect rect, EventData* data )
	:
	BView( rect,
			 "Reminder Activity",
			 B_FOLLOW_ALL,
			 B_WILL_DRAW | B_FRAME_EVENTS | B_PULSE_NEEDED ),
	fData( data ),
	fLastError( B_OK ),
	fReminderEnabler( NULL ),
	fExplanation( NULL ),
	fExplanation2( NULL ),
	fHourMinControl( NULL ),
	fActivityView( NULL )
{	
	// Sanity check
	if ( !data ) {
		/* Panic! */
		fLastError = B_BAD_VALUE;
		return;
	}
	
	// Create the layout
	BGroupLayout* layout = new BGroupLayout( B_VERTICAL );
	if ( !layout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	this->SetLayout( layout );
	layout->SetInsets( 5, 5, 5, 5 );
	
	// Get the preferences
	TimePreferences*	pref = pref_GetTimePreferences();	
	
	// Construct the checkbox
	BMessage* toSend = new BMessage( kReminderEnabled );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fReminderEnabler = new BCheckBox( BRect( 0, 0, 1, 1 ),
												 "Enable or disable the reminder",
												 "Remind me about this Event",
												 toSend );
	if( !fReminderEnabler ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fReminderEnabler->ResizeToPreferred();
	BLayoutItem* layoutItem = layout->AddView( fReminderEnabler );
	
	// Construct the explanation string
	fExplanation = new BStringView( BRect( 0, 0, 1, 1 ),
											  "Explanation",
											  "Reminder will not work if it's set up to start at" );
	fExplanation2 = new BStringView( BRect( 0, 0, 1, 1 ),
											  "Explanation2",
											  "the time of the Event itself. (Don't choose 00:00)!" );
	if ( !fExplanation || !fExplanation2 ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fExplanation->ResizeToPreferred();
	fExplanation2->ResizeToPreferred();
	fExplanation->SetFont( be_bold_font );
	fExplanation2->SetFont( be_bold_font );
	layoutItem = layout->AddView( fExplanation );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_BOTTOM ) );
	}
	layoutItem = layout->AddView( fExplanation2 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
	}
	
	// Construct the time control
	toSend = new BMessage( kReminderTimeUpdated );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	
	fHourMinControl = new GeneralHourMinControl( BRect ( 0, 0, 1, 1 ),
																"Reminder time",
																BString( "Reminder is set" ),
																BString( "before the Event" ),
																toSend );
	if ( !fHourMinControl ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fHourMinControl->ResizeToPreferred();
	fHourMinControl->SetHoursLimit( 71 );
	fHourMinControl->SetMinutesLimit( 55 );
	
	layoutItem = layout->AddView( fHourMinControl );
	
	// Activity control
	fActivityView = new ActivityView( BRect( 0, 0, 1, 1 ),
												 "Reminder activity",
												 fData->GetReminderActivity() );
	if ( !fActivityView ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fActivityView->ResizeToPreferred();
	layoutItem = layout->AddView( fActivityView );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT ) );
	}
	
	// Set up options
	bool beforeEvent = true;
	time_t fOffset = fData->GetReminderOffset( &beforeEvent );
	int seconds = ( int )( fOffset % 60 );
	fOffset /= 60;
	fMinutes = ( int )( fOffset % 60 );
	fHours = ( int )( fOffset / 60 );
	
	if ( fOffset == 0 && pref ) {		// First usage
		pref->GetDefaultReminderTime( ( int* )&fHours, ( int* )&fMinutes );
	}
	
	fHourMinControl->SetCurrentTime( fHours, fMinutes );
	fHourMinControl->SetCheckBoxValue( beforeEvent );
	
	// Initial enable / disable of the items	
	fReminderEnabler->SetValue( ( fOffset != 0 ) );
	fActivityView->SetEnabled( ( fOffset != 0 ) );
	fHourMinControl->SetEnabled( ( fOffset != 0 ) );

}	// <-- end of constructor



/*!	\brief		Main function of the class
 */
void		EventEditor_ReminderView::MessageReceived( BMessage* in )
{
	bool tempBool = true;
	
	switch ( in->what )
	{
		case 	kReminderEnabled:
			if ( !fReminderEnabler ) break;
			
			tempBool = ( fReminderEnabler->Value() != 0 );
			fHourMinControl->SetEnabled( tempBool );
			fActivityView->SetEnabled( tempBool );
			if ( tempBool ) {
				fData->SetReminderOffset( 60 * ( 60 * fHours + fMinutes ),
												  fHourMinControl->GetCheckBoxValue( NULL ) );
				fData->SetReminderActivityFired( false );	// Enable activity start
			} else {
				fData->SetReminderOffset( 0, true );
				fData->SetReminderActivityFired( true );	// Disable activity start
			}
			break;
		
		case kReminderTimeUpdated:
			if ( ! fHourMinControl ) { break; }
			fHourMinControl->GetCurrentTime( ( int* )&fHours,
														( int* )&fMinutes,
														NULL,
														&tempBool );
			fData->SetReminderOffset( 60 * ( 60 * fHours + fMinutes ), tempBool );
			fData->SetReminderActivityFired( false );	// Enable activity start
			break;
		
		case kSaveRequested:
			fActivityView->SaveData();
			break;
		
		default:
			BView::MessageReceived( in );
		
	};

}	// <-- end of function EventEditor_ReminderView::MessageReceived



/*!	\brief		Get the view color, register the handler, update targets of children.
 */
void		EventEditor_ReminderView::AttachedToWindow()
{
	if ( this->Parent() ) {
		this->SetViewColor( Parent()->ViewColor() );
	}
	
	if ( Looper() && Looper()->Lock() ) {
		Looper()->AddHandler( this );
		Looper()->Unlock();
	}
	
	fReminderEnabler->SetTarget( this );
	fHourMinControl->SetTarget( this );
	
}	// <-- end of function EventEditor_ReminderView::AttachedToWindow



/*!	\brief		Destructor for the class EventEditor_ReminderView.
 */
EventEditor_ReminderView::~EventEditor_ReminderView() {
	if ( fReminderEnabler ) {
		fReminderEnabler->RemoveSelf();
		delete fReminderEnabler;
		fReminderEnabler = NULL;
	}
	if ( fHourMinControl ) {
		fHourMinControl->RemoveSelf();
		delete fHourMinControl;
		fHourMinControl = NULL;
	}
	if ( fActivityView ) {
		fActivityView->RemoveSelf();
		delete fActivityView;
		fActivityView = NULL;
	}
	if ( fExplanation ) {
		fExplanation->RemoveSelf();
		delete fExplanation;
		fExplanation = NULL;
	}
	if ( fExplanation2 ) {
		fExplanation2->RemoveSelf();
		delete fExplanation2;
		fExplanation2 = NULL;
	}
}	// <-- end of destructor for class EventEditor_ReminderView

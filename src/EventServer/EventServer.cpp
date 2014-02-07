/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

// Project includes
#include "AboutWindow.h"
#include "ActivityData.h"
#include "ActivityWindow.h"
#include "Category.h"
#include "CategoryItem.h"
#include "Event.h"
#include "EventServer.h"
#include "Preferences.h"
#include "Utilities.h"

// OS includes
#include <Handler.h>
#include <InterfaceDefs.h>
#include <Looper.h>
#include <Message.h>
#include <Volume.h>
#include <VolumeRoster.h>

// POSIX includes
#include <time.h>



/*!	\brief		Return value of the program.
 */
uint32	global_toReturn = B_OK;


/*!	\brief		User pressed "Snooze".
 */
const		uint32	kUserSnoozedEvent	= 'UsSn';


/*!	\brief		Main function of the Event Server.
 *		\details		It just constructs the object and makes it run.
 */
int main( int argc, char* argv[] ) {
	EventServer* app = new EventServer();
	if ( app ) {
		app->Run();	
	} else {
		global_toReturn = B_NO_MEMORY;
	}
	
	delete app;
	return global_toReturn;
	
}	// <-- end of main function



/*---------------------------------------------------------------------------
 *			Implementation of class EventServer
 *--------------------------------------------------------------------------*/

/*!	\brief		Constructor for the application
 */
EventServer::EventServer()
	:
	BApplication( kEventServerApplicationSignature ),
	fEventQuery(),
	fReminderQuery(),
	fCurrentMessenger( NULL )
{
	this->SetPulseRate( 30000000 );		// Pulse is once per 30 secs
	this->fCurrentMessenger = new BMessenger( ( BHandler* )be_app, ( BLooper* )be_app );
	if ( !fCurrentMessenger ) {
		global_toReturn = B_NO_MEMORY;
		return;
	}
}	// <-- end of constructor




/*!	\brief		Update the categories recognized by the server.
 *		\details		I have no choise but to fully reload the whole Preferences message.
 */
void	EventServer::UpdateCategories()
{
	pref_ReloadAllPreferences();
}	// <-- end of function EventServer::UpdateCategories



/*!	\brief		Run the queries
 */
void	EventServer::Pulse()
{
	static unsigned char pendulum = 0;
	
	// To ease loads on the system, I read preferences only once in 2 minutes.
	if ( pendulum == 3 ) {
		pendulum = 0;
		UpdateCategories();
	} else {
		++pendulum;
	}
	
	fCurrentTime = time( NULL );
	
	PerformEventQuery();
	
	PerformReminderQuery();

}	// <-- end of function EventServer::Pulse	



/*!	\brief		Perform the event query fetch
 */
void	EventServer::PerformEventQuery()
{
	PrepareEventQuery();
	fEventQuery.Fetch();
	DealWithEntries( &fEventQuery, false );
}	// <-- end of function EventServer::PerformEventQuery



/*!	\brief		Prepare the event query for re-run
 */
void 	EventServer::PrepareEventQuery()
{
	BVolumeRoster volumeRoster;
	BVolume bootVolume;
	volumeRoster.GetBootVolume( &bootVolume );

	/*---------------------------------------
	 *			 Prepare the Event query
	 *--------------------------------------*/	
	 
	// Clear the query
	fEventQuery.Clear();
	
	// Setting the query to look in the boot volume
	fEventQuery.SetVolume( &bootVolume );
	
	// Build the predicate
		// Only files of the "Eventual" type
	fEventQuery.PushAttr( "BEOS:TYPE" );
	fEventQuery.PushString( kEventFileMIMEType );
	fEventQuery.PushOp( B_EQ );
	
		// Where time of the next event has arrived
	fEventQuery.PushAttr( "EVNT:next_occurrence" );
	fEventQuery.PushUInt32( fCurrentTime );
	fEventQuery.PushOp( B_LE );
	fEventQuery.PushOp( B_AND );
	
		// And the event wasn't fired yet
	fEventQuery.PushAttr( "EVNT:activity_fired" );
	fEventQuery.PushUInt32( 0 );
	fEventQuery.PushOp( B_EQ );
	fEventQuery.PushOp( B_AND );
	
}	// <-- end of function PerformEventQuery



/*!	\brief		Receive the results from the Entries query and run activities.
 *		\param[in]	toFetchFrom 	The BQuery object that represents the query
 *											which results are to be analized.
 *		\param[in]	bReminder		\c true if we should run the reminder activity,
 *											\c false (default) if we should run the Event activity.
 */
void 	EventServer::DealWithEntries( BQuery* toFetchFrom, bool bReminder )
{
	ActivityWindow* actWindow;
	entry_ref ref;
	EventData* eventData = NULL;
	ActivityData* activityData = NULL;
	status_t	status = B_OK;
	Category* category = NULL;
	BString eventName;
	BMessage* toSend = NULL;
	
	if ( !toFetchFrom ) { return; }
	
	int temp = 0;
	
	while ( B_OK == ( status = toFetchFrom->GetNextRef( &ref ) ) )
	{
		eventData = new EventData( ref );	// Read the entry
		if ( !eventData ) {
			/* Panic! */
			global_toReturn = B_NO_MEMORY;
			be_app->PostMessage( B_QUIT_REQUESTED );
		}
		
		// Set the activity as fired
		if ( bReminder ) {
			eventData->SetReminderActivityFired( true );
		} else {
			eventData->SetEventActivityFired( true );
		}
		
		// Save the new data
		eventData->SaveToFile( &ref );
		
		// Obtain the activity data
		if ( bReminder ) {
			activityData = eventData->GetReminderActivity();	
		} else {
			activityData = eventData->GetEventActivity();
		}
		
		// Get the Event name
		eventName = eventData->GetEventName();
		
		// Obtain the category name and color
		if ( NULL == ( category = FindCategory( eventData->GetCategory() ) ) ) {
			category = FindDefaultCategory();	
			
			// If unsuccessfully, failback to "Default"
			if ( category == NULL ) {
				category = new Category( "Default", ui_color( B_WINDOW_TAB_COLOR ) );
				if ( !category ) {
					/* Panic! */
					global_toReturn = B_NO_MEMORY;
					be_app->PostMessage( B_QUIT_REQUESTED );
				}
			}
		}
		
		// Build the template message
		toSend = new BMessage( kUserSnoozedEvent );
		if ( !toSend ) {
			/* Panic! */
			global_toReturn = B_NO_MEMORY;
			be_app->PostMessage( B_QUIT_REQUESTED );
		}
		toSend->AddRef( "Event to snooze", &ref );
		toSend->AddBool( "Reminder", bReminder );
		
		// Open the activity window
		actWindow = new ActivityWindow( activityData,
												  fCurrentMessenger,
												  eventName,
												  category,
												  toSend,
												  bReminder );
		if ( !actWindow ) {
			/* Panic! */
			global_toReturn = B_NO_MEMORY;
			be_app->PostMessage( B_QUIT_REQUESTED );
		} else {
			actWindow->Show();
		}
		
		// Run the activity
		ActivityData::PerformActivity( activityData );
		
		// Delete temporary allocated data
		delete eventData;
		
	}	// <-- end of "while ( there are entries in query results )"
	
}	// <-- end of function EventServer::DealWithEntries



/*!	\brief		Perform the reminder query fetch
 */
void	EventServer::PerformReminderQuery()
{
	PrepareReminderQuery();
	fReminderQuery.Fetch();
	DealWithEntries( &fReminderQuery, true );
}	// <-- end of function EventServer::PerformReminderQuery



/*!	\brief		Prepare the Reminders query for running
 */
void 	EventServer::PrepareReminderQuery()
{
	BVolumeRoster volumeRoster;
	BVolume bootVolume;
	volumeRoster.GetBootVolume( &bootVolume );
	
	/*---------------------------------------
	 *			 Prepare the Reminder query
	 *--------------------------------------*/	

	// Clear the previous query
	fReminderQuery.Clear();
	
	// Setting the query to look in the boot volume
	fReminderQuery.SetVolume( &bootVolume );
	
	// Build the predicate
		// Only Eventual files
	fReminderQuery.PushAttr( "BEOS:TYPE" );
	fReminderQuery.PushString( kEventFileMIMEType );
	fReminderQuery.PushOp( B_EQ );
	
		// When next reminder should be already run
	fReminderQuery.PushAttr( "EVNT:next_reminder" );
	fReminderQuery.PushUInt32( fCurrentTime );
	fReminderQuery.PushOp( B_LE );
	fReminderQuery.PushOp( B_AND );
	
		// And was not fired yet
	fReminderQuery.PushAttr( "EVNT:reminder_fired" );
	fReminderQuery.PushUInt32( 0 );
	fReminderQuery.PushOp( B_EQ );
	fReminderQuery.PushOp( B_AND );
	
		// And the reminder is not disabled
	fReminderQuery.PushAttr( "EVNT:reminder_offset" );
	fReminderQuery.PushUInt32( 0 );
	fReminderQuery.PushOp( B_NE );
	fReminderQuery.PushOp( B_AND );	
	
}	// <-- end of constructor for the application



/*!	\brief		Destructor for the application.
 */
EventServer::~EventServer()
{
	if ( fCurrentMessenger ) {
		delete fCurrentMessenger;
	}
	
}	// <-- end of destructor for the application



/*!	\brief		Snooze activity
 *		\details		It's an independent, static function.
 *		\param[in]	ref			Reference to the file with activity to be snoozed.
 *		\param[in]	bReminder	If \c true, snoozing reminder activity. If \c false,
 *										snoozing Event activity.
 *		\param[in]	hours			Snoozing to how many hours? (Usually 0).
 *		\param[in]	minutes		Snoozing to how many minutes?	(Usually 10).
 */
void		EventServer::SnoozeActivity( entry_ref ref, bool bReminder,
												  int32 hours, int32 minutes )
{
	entry_ref	actualRef;
	time_t		currentTime = time( NULL );
	BEntry entry( &ref, true );
	if ( entry.InitCheck() != B_OK || !entry.Exists() ) {
		return;
	}
	entry.GetRef( &actualRef );
	
	// New firing time of the activity
	currentTime += ( minutes * 60 + hours * 60 * 60 );
	
	EventData fData( actualRef );
	
	if ( bReminder ) {
		// Modify reminder time only if it's schedulled to start anyway
		if ( fData.GetReminderOffset( NULL ) != 0 ) {
			fData.SetReminderSnoozeTime( currentTime );
		}
	} else {
		fData.SetActivitySnoozeTime( currentTime );
	}
	
	fData.SaveToFile( &actualRef );
	
}	// <-- end of function EventServer::SnoozeActivity



/*!	\brief		Responds to the messages sent to this application.
 *		\details		Actually, the sole purpose is snoozing events.
 *		\param[in]	in		The message that was received.
 */
void		EventServer::MessageReceived( BMessage* in ) {
	entry_ref ref;
	bool bReminder = false;
	int32	hours = 0, minutes = 10;
	
	switch ( in->what ) {
		case kUserSnoozedEvent:
			if ( ( B_OK == in->FindRef( "Event to snooze", &ref ) ) 	&&
				  ( B_OK == in->FindBool( "Reminder" , &bReminder ) ) &&
				  ( B_OK == in->FindInt32( "Hours", &hours ) )			&&
				  ( B_OK == in->FindInt32( "Minutes", &minutes ) ) )
			{
				EventServer::SnoozeActivity( ref, bReminder, hours, minutes );				
			}
			break;

		default:
			BApplication::MessageReceived( in );
	};
	
}	// <-- end of function EventServer::MessageReceived



/*!	\brief		Loads the list of categories and the file type.
 */
void		EventServer::ReadyToRun()
{
	GregorianCalendar* gregorianCalMod = new GregorianCalendar();
	global_ListOfCalendarModules.AddItem( gregorianCalMod );
	
	status_t status = pref_PopulateAllPreferences();
	if ( status != B_OK )
	{
		utl_Deb = new DebuggerPrintout( "Did not succeed to read the preferences!" );
	}
	
	utl_RegisterFileType();	
	
		
	// Immediately perform the first check
	this->Pulse();

}	// <-- end of function EventServer::ReadyToRun



/*!	\brief		User wants the "about" information.
 */
void		EventServer::AboutRequested() {
	AboutWindow* aw = new AboutWindow();
}	// <-- end of function EventServer::AboutRequested

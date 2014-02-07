/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// Project includes
#include "ActivityData.h"
#include "CalendarModule.h"
#include "Event.h"
#include "Preferences.h"
#include "Utilities.h"

// OS includes
#include <Message.h>
#include <File.h>
#include <NodeInfo.h>
#include <SupportDefs.h>
#include <fs_attr.h>

// POSIX includes
#include <limits.h>	// For various constants
#include <math.h>		// For floor() and abs() functions
#include <time.h>		// For time() function
#include	<stdio.h>	// For memset() function
#include <string.h>	// For strcmp() function



/*!	\brief		Strings that represent the types of the Events.
 *		\details		Since the enum starts with 0, the strings' places exactly
 *						match the corresponding Event Types.
 */
#define	PARSER(a, b)	b,
// const char*	EventType_name[] = {
//	EVENT_TYPE_CROSS_MACRO
//};
#undef	PARSER



/*!	\brief		Destructor
 */
EventData::~EventData() {
	// The only thing we need to destroy is fEventFile.
	if ( fEventFile )
		delete fEventFile;	
}	// <-- end of destructor



/*!	\brief		Constructor from time_t
 */
EventData::EventData( time_t startingTime )
	:
	fEventFile( NULL )
{
	_InitDefaults();
	startTime = startingTime;	// Save the time for "Revert" feature
	if ( startingTime != 0 ) {
		fStart = fCalModule->FromTimeTToLocalCalendar( startingTime );
	}
}	// <-- end of constructor from time_t



/*!	\brief		Constructor from file
 *		\param[in]	fileIn 	Reference to the file
 */
EventData::EventData( const entry_ref& fileIn )
	:
	fEventFile( NULL )
{
	_InitDefaults();
	InitFromFile( fileIn );
}



/*!	\brief		Reset the Event Data to previously saved version.
 *		\details		If there was no previously saved version - just clear everything.
 */
void		EventData::Revert( void )
{
	_InitDefaults();
	if ( fEventFile != NULL ) {
		InitFromFile( *fEventFile );
	} else {
		if ( startTime != 0 ) {
			fStart = fCalModule->FromTimeTToLocalCalendar( startTime );
		}
	}
}	// <-- end of function EventData::Revert()


/*!	\brief		Set data structures to their default values.
 *		\attention	It is assumed that calendar modules and preferences are
 *						already set up before call to this function. 
 *		\warning
 *						If calendar module is \b NOT initialized before call to this
 *						function, then it probably will crash. 
 */
void 		EventData::_InitDefaults()
{
	int hours, mins;
	
	fEventName.SetTo( "" );
//	fEventFile = NULL;
	fCategory.SetTo( "Default" );		// This category surely exists
	bPrivate = false;						// This data member is not used
	bVerified = true;						// This data member is not used too
	fEventType = kEventType_Appointment;	// Default event type is "Appointment"
	bLastsWholeDays = false;			// It does not last whole days
	
	// Get the preferences and the default calendar module
	TimePreferences*	pref = pref_GetTimePreferences();
	fCalModule = utl_FindCalendarModule( "Gregorian" );
	
	time_t	currentMoment = time( NULL );
	
	/*!	\note		Explanations about the value changes in the next line
	 *					One command ago I received time in seconds from Jan 1st, 1970
	 *					and until current moment. This is the time I will use to initialize
	 *					the Event start moment with. However, this number is updated:
	 *		\par
	 *					I add 420 seconds to achieve time moment which is located
	 *					roundly 5 minutes in the future. If I were not doing this,
	 *					the Start time would be in the past as soon as user creates
	 *					the Event - which is clearly not the intended behavior, especially
	 *					concidering someone who creates an Alarm clock at midnight and
	 *					forgets to verify the date. 420 is ( 5 + 2 ) * 60, which is "5
	 *					minutes delay + 2 minutes for mathematically-correct round-up,
	 *					in seconds".
	 *		\par
	 *					I add 30 more seconds in order to round the seconds to minutes.
	 *					After this addition, I'll be able to simply nullify the seconds -
	 *					the minutes value will still be correct no matter what was
	 *					the seconds value.
	 *		\par
	 *					Total added value is 450 seconds.
	 */
	fStart = fCalModule->FromTimeTToLocalCalendar( currentMoment + 450 );
	
	// Seconds are always 0
	fStart.tm_sec = 0;
	
	// Minutes are always rounded down to closest dividend of 5. I count on them being
	// non-negative!
	fStart.tm_min = ( int )( floor( fStart.tm_min / 5 ) ) * 5;
	
	// Next occurrence is the start date.
	fNextOccurrence = fCalModule->FromLocalCalendarToTimeT( fStart );
	
	fReminderSnoozedTime = 0;
	fActivitySnoozedTime = 0;
	
	if ( !pref ) {
		fDuration = 30 * 60;		// Default duration is 30 mins
	} else {
		pref->GetDefaultAppointmentDuration( &hours, &mins );
		fDuration = ( hours * 3600 ) + ( mins * 60 );		
	}
	
	fOffsetBetweenReminderAndEvent = 0;	// By default, reminder is turned off
	bReminderIsFiredBeforeEvent = true;
	
	// Clear the rules (which should be clear anyway)
	fAndRules.MakeEmpty();
	fNotRules.MakeEmpty();
	
	// Clear reminder activity and event activity
	fEventActivity.Instantiate( NULL );
	bEventActivityWasFired = false;
	fReminderActivity.Instantiate( NULL );
	bReminderActivityWasFired = false;

	// Initializing the Note to nothing
	fNote.SetTo( "" );
	
	// Initialize the Location to nothing
	fLocation.SetTo( "" );

}	// <-- end of function EventData::_InitDefaults;



/*!	\brief		Init from file.
 *		\param[in]	fileIn		Reference to the file to initialize the Event from.
 *		\attention	The \c fileIn entry_ref structure used to initialize the EventData
 *						object is stored in the object itself ( \c fEventFile member).
 *						However, \c fEventFile is a new entry_ref initialized using the \c fileIn
 *						entry_ref. In other words, this function creates a new pointer, and
 *						\em not adopts an existing one. It's up to caller to delete the
 *						pointer after initialization.
 */
void		EventData::InitFromFile( const entry_ref& fileIn )
{
	bool			bLocked = false;
	attr_info	ai;
	char			nameBuffer[ B_ATTR_NAME_LENGTH ];	// 255 bytes
	uint8			*buffer = NULL;
	uint32 		bufferSize = 0;
	
	// Temporary variables for reading the data
	uint32		tempUint32;
	BMessage		tempMessage;
	size_t		tempSize = 0;
	
	_InitDefaults();
	
	// Save the entry_ref parameter into the data structure
	// The entry_ref is copied, not adopted.
	if ( this->fEventFile ) {
		delete this->fEventFile;
	}
	fEventFile = new entry_ref( fileIn );
	
	// Pointer to the placeholder which will be filled with the read data
	void*			placeholder = NULL;
	
	status_t		status = B_OK;
		
	BFile file;
	file.SetTo( &fileIn, B_READ_ONLY );
	if ( file.InitCheck() != B_OK )
	{
		return;
	}
	
	/* Here, "file" is set, and the file is opened for reading.
	 */
	
	// Lock the node to prevent tampering with its attributes while I'm reading
	if ( B_OK == file.Lock() ) {
		bLocked = true;
	}
	
	// If unsuccessful, still try to continue
	
	// Reading attributes is performed in the infinite loop.
	// Breaking out of the loop when all attributes were read.
	while ( true ) {
		if ( ( ( status = file.GetNextAttrName( nameBuffer ) ) == B_ENTRY_NOT_FOUND ) ||
		 	  ( status == B_FILE_ERROR ) )
		{
			break;
		}
		
		// Allocating memory for reading the attribute
		if ( file.GetAttrInfo( nameBuffer, &ai ) == B_OK )
		{
			// Maybe it's a constant-length type, and we have a placeholder for it.
			if ( ( ai.size <= sizeof( uint32 ) ) && ( ai.type != B_STRING_TYPE ) )
			{
				placeholder = ( void* )&tempUint32;
				tempSize = sizeof( uint32 );
			}
			else
			{
				// No placeholder suits the data.
				// Allocate new buffer for the attribute's data, but
				// only in case we actually need it
				if ( buffer )
					delete []buffer;	// Clearing the old buffer
				buffer = new uint8[ ai.size + 1 ];
				if ( !buffer ) {
					// Did not succeed to allocate buffer - unlock and exit.
					if ( bLocked ) { file.Unlock(); }
				   file.Unset();	// Close the file.
					return;
				}
					
				tempSize = ( size_t )ai.size;			
				
				placeholder = ( void* )buffer;
			}
		}
		else
		{
			// If we didn't succeed to get attribute-info for the attribute, we just continue
			continue;
		}
		
		memset( buffer, 0, ai.size + 1 );
		
		// Read the attribute
		tempSize = file.ReadAttr( nameBuffer, ai.type, 0, placeholder, ai.size );
		
		/* Ok, the attribute was read successfully. What is it?
		 */
		if ( strcmp( nameBuffer, "EVNT:name" ) == 0 )
		{
			fEventName.SetTo( ( char* )buffer );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:category" ) == 0 )
		{
			fCategory.SetTo( ( char* )buffer );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:where" ) == 0 )
		{
			fLocation.SetTo( ( char* )buffer );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:private" ) == 0 )
		{
			bPrivate = ( tempUint32 != 0 );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:verified" ) == 0 )
		{
			bVerified = ( tempUint32 != 0 );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:whole_day" ) == 0 )
		{
			bLastsWholeDays = ( tempUint32 != 0 );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:type" ) == 0 )
		{
			fEventType = ( EventType )tempUint32;
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:duration" ) == 0 )
		{
			fDuration = ( time_t )tempUint32;
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:next_occurrence" ) == 0 )
		{
			fNextOccurrence = ( time_t )tempUint32;
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:and_rules" ) == 0 )
		{
			// Now buffer holds the flattened message of "And" rules.
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:not_rules" ) == 0 )
		{
			// Now buffer holds the flattened message of "Not" rules.
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:start_TR" ) == 0 )
		{
			tempMessage.MakeEmpty();
			tempMessage.Unflatten( ( char* )placeholder );
			fStart.Unarchive( &tempMessage );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:event_activity" ) == 0 )
		{
			tempMessage.MakeEmpty();
			tempMessage.Unflatten( ( char* )placeholder );
			fEventActivity.Instantiate( &tempMessage );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:activity_fired" ) == 0 )
		{
			bEventActivityWasFired = ( tempUint32 != 0 );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:reminder_offset" ) == 0 )
		{
			fOffsetBetweenReminderAndEvent = ( time_t )abs( ( time_t )tempUint32 );
			if ( tempUint32 < 0 ) {
				bReminderIsFiredBeforeEvent = 0;
			}
			else
			{
				bReminderIsFiredBeforeEvent = 1;
			}
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:reminder_activity" ) == 0 )
		{
			tempMessage.MakeEmpty();
			tempMessage.Unflatten( ( char* )placeholder );
			fReminderActivity.Instantiate( &tempMessage );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:reminder_fired" ) == 0 )
		{
			bReminderActivityWasFired = ( tempUint32 != 0 );
			continue;
		}
		else if ( strcmp( nameBuffer, "EVNT:cal_module" ) == 0 )
		{
			BString tempString( ( char* )buffer );
			fCalModule = utl_FindCalendarModule( tempString );
			
			// Note: the start time representation is not verified for
			// consistency with the calendar module saved here.
		}
	};	// <-- end of "while ( not all attributes were read )"
	
	/*!	\note		Note about reading attributes
	 *					Not all attributes of the file are parsed - mostly because not
	 *					all of them are interesting. And I don't speak about icon or
	 *					filetype (which are attributes, but not interesting). The firing
	 *					time of Activity or Reminder, or if they were already fired,
	 *					are attributes which don't have anything to do with editing an
	 *					Event, only with saving it.
	 */
	
	// Now, last thing we should do is read the note.
	BString note;
	do {
		memset( nameBuffer, 0, B_ATTR_NAME_LENGTH );
		tempSize = file.Read( ( void* )nameBuffer, B_ATTR_NAME_LENGTH-1 );
		if ( tempSize < 0 ) {
			utl_Deb = new DebuggerPrintout( "Didn't succeed to read file's data!" );
		} else {
			nameBuffer[ tempSize ] = 0;
			note.Append( ( char* )nameBuffer );
		}
	} while( tempSize > 0 );	
	
	SetNoteText( note );
	
	if ( bLocked ) { file.Unlock(); }
	file.Unset();	// Close the file.	
}	// <-- end of function EventData::InitFromFile



/*!	\brief		Saving the Event data into file.
 *		\details		This function is a gateway for another, private function,
 *						that actually performs saving.
 */
status_t		EventData::SaveToFile( entry_ref* fileIn )
{
	entry_ref* newRef;
	
	if ( fileIn == NULL ) {
		fileIn = fEventFile;
	} 
	else
	{ 
		newRef = new entry_ref( *fileIn );
		if ( !newRef ) {
			return B_NO_MEMORY;	
		}
		if ( fEventFile )
			delete this->fEventFile;
		fEventFile = newRef;
		fileIn = newRef;
	}
	if ( fileIn == NULL ) { return B_ENTRY_NOT_FOUND; }
	BFile file( fileIn, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE );
	status_t toReturn = _SaveToFile( &file );
	file.Unset();
	return toReturn;
}	// <-- end of function EventData::SaveToFile



/*!	\brief		Saving the Event data into file.
 *		\details		This function is a gateway for another, private function,
 *						that actually performs saving.
 *		\param[in]	fileIn 		Pointer to the (previously opened) file.
 */
status_t		EventData::SaveToFile( BFile *fileIn )
{
	if ( !fileIn ) { return B_NAME_NOT_FOUND; }
	if ( ( fileIn->InitCheck() != B_OK ) ||
	     ( !fileIn->IsWritable() ) )
	{
		return B_BAD_VALUE;
	}
	status_t toReturn = _SaveToFile( fileIn );
	fileIn->Unset();		// Close the file
	return toReturn;
}	// <-- end of function EventData::SaveToFile
	

/*!	\brief		The private function that actually performs saving.
 */
status_t		EventData::_SaveToFile( BFile* file )
{
	BMessage tempMessage;
	uint8		*buffer;
	uint32	tempUint32 = 0;
	bool		tempBool = true;		// For creating "verified" attribute
	status_t	status 	= B_OK;
	bool		bLocked 	= false;
	ssize_t	size = 0;
	time_t	currentMoment = time( NULL );
	BNodeInfo		nodeInfo;
	
	if ( !file || ( status = file->InitCheck() ) != B_OK )
	{
		// Did not succeed to create the file
		return status;
	}
	
	// Lock the file.
	// From now on, every exit from this function should perform Unlock()
	if ( file->Lock() ) {
		bLocked = true;
	}


	// Time Representation of the starting moment
	tempMessage.MakeEmpty();
	TimeRepresentation toSave( fStart );
	if ( bLastsWholeDays ) {
		toSave.tm_hour = toSave.tm_min = 0;
	}
	fCalModule = utl_FindCalendarModule( fStart.GetCalendarModule() );
	toSave.Archive( &tempMessage );
	size = tempMessage.FlattenedSize();
	buffer = new uint8[ size ];
	if ( buffer ) {
		status = tempMessage.Flatten( ( char* )buffer, size );
		if ( status == B_OK ) {
			file->WriteAttr( "EVNT:start_TR", B_RAW_TYPE, 0, buffer, size );
		}
		delete buffer;
		buffer = NULL;
	}
	
	// Calculate next occurrence
	fNextOccurrence = fCalModule->FromLocalCalendarToTimeT( toSave );
	
	// Next occurrence
	if ( fActivitySnoozedTime == 0 ) {
		file->WriteAttr( "EVNT:next_occurrence", 	B_UINT32_TYPE,	0, ( uint32* )&fNextOccurrence,	sizeof( uint32 ) );
	} else {
		file->WriteAttr( "EVNT:next_occurrence", 	B_UINT32_TYPE,	0, ( uint32* )&fActivitySnoozedTime,	sizeof( uint32 ) );
	}
	
	// Calculate next reminder
	tempUint32 = ( uint32 )fNextOccurrence + ( bReminderIsFiredBeforeEvent ? ( -1 ) : 1 ) * ( uint32 )fOffsetBetweenReminderAndEvent;
	
	file->WriteAttr( "EVNT:name", 				B_STRING_TYPE,	0, fEventName.String(),				fEventName.Length() );
	file->WriteAttr( "EVNT:category",			B_STRING_TYPE,	0, fCategory.String(),				fCategory.Length() );
	file->WriteAttr( "EVNT:where", 				B_STRING_TYPE,	0, fLocation.String(),				fLocation.Length() );
	file->WriteAttr( "EVNT:private", 			B_INT32_TYPE,	0, ( int32* )&bPrivate,				sizeof( int32 ) );
	file->WriteAttr( "EVNT:verified",			B_INT32_TYPE,	0, ( int32* )&tempBool,				sizeof( int32 ) );
	file->WriteAttr( "EVNT:type",	 				B_INT32_TYPE,	0, ( int32* )&fEventType,			sizeof( int32 ) );
	file->WriteAttr( "EVNT:reminder_offset",	B_UINT32_TYPE, 0, ( uint32* )&fOffsetBetweenReminderAndEvent, sizeof( uint32 ) );
	
	// Next time of the reminder invocation
	// This value doesn't depend on the reminder being enabled or disabled; it's written anyway
	if ( fReminderSnoozedTime != 0 ) {
		// The reminder was snoozed - calculate new time based on snooze time
		file->WriteAttr( "EVNT:next_reminder",		B_UINT32_TYPE, 0, ( uint32* )&fReminderSnoozedTime, sizeof( uint32 ) );
	} else {
		// Else, calculate time based on start time and offset
		file->WriteAttr( "EVNT:next_reminder",		B_UINT32_TYPE, 0, ( uint32* )&tempUint32, sizeof( uint32 ) );
	}
	
	// Duration
	file->WriteAttr( "EVNT:duration", B_UINT32_TYPE,	0, ( uint32* )&fDuration, sizeof( uint32 ) );	
	
	// "And" rules
	tempMessage.MakeEmpty();
//	CreateMessageFromRules( &fAndRules, &tempMessage );
	size = tempMessage.FlattenedSize();
	if ( size ) {
		buffer = new uint8[ size ];
		if ( buffer ) {
			status = tempMessage.Flatten( ( char* )buffer, size );
			if ( status == B_OK ) {
				file->WriteAttr( "EVNT:and_rules", B_RAW_TYPE,	0, buffer, size );
			}
			delete [] buffer;
			buffer = NULL;
		}
	}
	

	
	// "Not" rules
	tempMessage.MakeEmpty();
//	CreateMessageFromRules( &fNotRules, &tempMessage );
	size = tempMessage.FlattenedSize();
	if ( size ) {
		buffer = new uint8[ size ];
		if ( buffer ) {
			status = tempMessage.Flatten( ( char* )buffer, size );
			if ( status == B_OK ) {
				file->WriteAttr( "EVNT:not_rules", B_RAW_TYPE,	0, buffer, size );
			}
			delete buffer;
			buffer = NULL;
		}
	}
	
	// Calendar module
	if ( fCalModule ) {
		file->WriteAttr( "EVNT:cal_module", B_STRING_TYPE, 0, fCalModule->Identify().String(), fCalModule->Identify().Length() );
	}
	
	// Event activity
	tempMessage.MakeEmpty();
	fEventActivity.Archive( &tempMessage );
	size = tempMessage.FlattenedSize();
	buffer = new uint8[ size ];
	if ( buffer ) {
		status = tempMessage.Flatten( ( char* )buffer, size );
		if ( status == B_OK ) {
			file->WriteAttr( "EVNT:event_activity", B_RAW_TYPE, 0, buffer, size );
		}
		delete buffer;
		buffer = NULL;
	}
	
	// Reminder activity
	tempMessage.MakeEmpty();
	fReminderActivity.Archive( &tempMessage );
	size = tempMessage.FlattenedSize();
	buffer = new uint8[ size ];
	if ( buffer ) {
		status = tempMessage.Flatten( ( char* )buffer, size );
		if ( status == B_OK ) {
			file->WriteAttr( "EVNT:reminder_activity", B_RAW_TYPE, 0, buffer, size );
		}
		delete buffer;
		buffer = NULL;
	}
	
	// Was activity fired? 
	// If it's schedulled to occur in the past - don't set the flag.
	if ( !bEventActivityWasFired && ( currentMoment > fNextOccurrence ) ) {
		tempUint32 = 0;
	}
	else
	{
		tempUint32 = 1;
	}
	file->WriteAttr( "EVNT:activity_fired", B_INT32_TYPE, 0, &tempUint32, sizeof( uint32 ) );
	
	// Was reminder fired? 
	// If it's schedulled to occur in the past - don't set the flag.
	if ( !bReminderActivityWasFired && ( currentMoment > fNextOccurrence + ( ( bReminderIsFiredBeforeEvent ? -1 : 1 ) * fOffsetBetweenReminderAndEvent ) ) ) {
		tempUint32 = 0;
	}
	else
	{
		tempUint32 = 1;
	}
	// tempBool = ( !bReminderActivityWasFired ) && ( currentMoment > ( fCalModule->FromLocalCalendarToTimeT( toSave ) ) );
	file->WriteAttr( "EVNT:reminder_fired", B_INT32_TYPE, 0, &tempUint32, sizeof( uint32 ) );

	// Does the Event lasts full days?
	file->WriteAttr( "EVNT:whole_day",		B_INT32_TYPE, 0, &bLastsWholeDays, sizeof( int32 ) );


	// Adding some general attributes
	if ( B_OK == nodeInfo.SetTo( file ) )
	{
		nodeInfo.SetType( kEventFileMIMEType );
		nodeInfo.SetPreferredApp( kEventEditorApplicationSignature, B_OPEN );
	}
	
	// Saving the note
	file->Write( fNote.String(), fNote.Length() );

	// Unlock the node before exit
	if ( bLocked ) {
		file->Unlock();
	}
	
	return B_OK;
	
}	// <-- end of function EventData::_SaveToFile


/*!	\brief		Sets the Event duration
 *		\details		It checks the input and works according to the Event type.
 *		\param[in]	durIn		The new duration.
 *		\param[in]	newType	The new EventType. By default, it's \c NULL - i.e. no change.
 */
status_t		EventData::SetDuration( time_t durIn, EventType* newType )
{
	if ( newType ) {
		this->fEventType = *newType;
	}

	switch ( fEventType ) {
		case kEventType_Note:
			fDuration = 0;
			break;
		
		case kEventType_Anniversary:		// Intentional fall-through
		case kEventType_Holiday:
			bLastsWholeDays = true;
			fDuration = durIn;
		
		case	kEventType_Appointment:		// Intentional fall-through
			bLastsWholeDays = false;
		default:
			// Just set the duration, and let the user decide what happend...
			fDuration = durIn;
	};
	
	return B_OK;
}	// <-- end of function EventData::SetDuration



/*!	\brief		Set the Event type.
 */
status_t		EventData::SetEventType( EventType newType )
{
	this->fEventType = newType;

	switch ( fEventType ) {
		case kEventType_Note:
			fDuration = 0;
			break;
		
		case kEventType_Anniversary:		// Intentional fall-through
		case kEventType_Holiday:
			bLastsWholeDays = true;
		
		case	kEventType_Appointment:		// Intentional fall-through
			bLastsWholeDays = false;
	};

	return B_OK;	
	
}	// <-- end of function EventData::SetEventType



/*!	\brief		Set starting time
 */
status_t		EventData::SetStartTime( int hour, int min )
{
	if ( hour >= 0 && hour <= 23 )
		fStart.tm_hour = hour;
		
	if ( min >= 0 && min <= 55 )
		fStart.tm_min = min;

	return B_OK;	
}	// <-- end of function EventData::SetStartTime


/*!	\brief		SetStartingDate
 *		\warning		The input data is copied into the TimeRepresentation "as is",
 *						without any check. It's up to user to verify it suits chosen
 *						Calendar Module.
 */
status_t		EventData::SetStartDate( int day, int month, int year, BString calModule )
{
	if ( calModule != "" ) {
		CalendarModule* toSet = utl_FindCalendarModule( calModule );
		if ( toSet != NULL )
		fCalModule = toSet;
	}
		
	fStart.tm_mday = day;
	fStart.tm_mon = month;
	fStart.tm_year = year;
	return B_OK;
}	// <-- end of function EventData::SetStartDate


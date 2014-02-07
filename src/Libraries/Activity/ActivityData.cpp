/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// Project includes
#include "ActivityData.h"
#include "ActivityWindow.h"

// OS includes
#include <Alert.h>
#include <Application.h>
#include <Entry.h>
#include <Errors.h>
#include <Handler.h>
#include <Looper.h>
#include <Messenger.h>
#include <Roster.h>

// POSIX includes
#include <string.h>
#include <cstdlib>


/*---------------------------------------------------------------------------
 *								Definition of class ActivityData
 *--------------------------------------------------------------------------*/

/*!	\brief		Default constructor
 */
ActivityData::ActivityData( BMessage* in )
	:
	bNotification( false ),
	bSound( false ),
	bEmailToSend( false ),
	bProgramRun( false ),
	bVerifiedByUser( true )
{
	for ( int i = 0; i < ACTIVITY_NUMBER_OF_EMAIL_ADDRESSES; ++i )
	{
		bIsAddressEmpty[ i ] = true;
	}
	
	if ( in ) {
		Instantiate( in );
	}
}	// <-- end of default constructor



/*!	\brief		Read the activity data from the submitted message.
 *		\details		If the data can't be found, then some kind of defaults is used.
 */
void		ActivityData::Instantiate( BMessage* in )
{
	BString	tempString;
	bool		emailSubjectEmpty 	= false,
				emailContentsEmpty	= false;
			
	/* Notification section */
	if ( ( !in ) || ( ( in->FindBool( "Notification Enabled", &bNotification ) ) != B_OK ) )
	{
		bNotification = false;
	}
	if ( ( !in ) || ( ( in->FindString( "Notification Text", &fNotificationText ) ) != B_OK ) )
	{
		fNotificationText.SetTo( "" );
		bNotification = false;		// No need to launch notification if no text exists
	}
	
	/* Sound play section */
	if ( ( !in ) || ( ( in->FindBool( "Sound Play Enabled", &bSound ) ) != B_OK ) )
	{
		bSound = false;
	}
	if ( ( in ) && ( ( in->FindString( "Sound File Path", &tempString ) ) == B_OK ) )
	{
		// Read the path data successfully.
		// The file may be not there, but it will be checked when the Activity launches.
		fSoundFile.SetTo( tempString.String() );
	}
	else
	{
		fSoundFile.Unset();
		bSound = false;	// No need to play something we didn't find.
	}
	
	/* Program run section */
	if ( ( !in ) || ( ( in->FindBool( "Program Run Enabled", &bProgramRun ) ) != B_OK ) )
	{
		bProgramRun = false;
	}
	if ( ( !in ) || ( ( in->FindBool( "Program Verified", &bVerifiedByUser ) ) != B_OK ) )
	{
		bVerifiedByUser = true;
	}
	if ( ( in ) && ( ( in->FindString( "Program Path", &tempString ) ) == B_OK ) )
	{
		// Read the data successfully.
		// The program file itself may not be there, but it will be checked upon launch.
		fProgramPath.SetTo( tempString.String() );
	}
	else
	{
		fProgramPath.Unset();
		bProgramRun = false;		// No need to launch a program if it couldn't be found.
	}
	if ( ( !in ) || ( ( in->FindString( "Program Command Line Params", &fCommandLineOptions ) ) != B_OK ) )
	{
		// If didn't succeed to read the options, it's ok - just assume there were none.
		fCommandLineOptions.SetTo( "" );
	}
	
	/* Email sending section - deprecated*/

}	// <-- end of function ActivityData::Instantiate



/*!	\brief		Save the activity data to the submitted message.
 *		\param[out]	out	The BMessage to be filled.
 *		\returns		B_OK if everything was Ok.
 */
status_t		ActivityData::Archive( BMessage* out )
{
	status_t	toReturn = B_OK;

	/* Sanity check */	
	if ( !out ) {
		toReturn = B_NO_INIT;
		return toReturn;
	}
	
	/* Clear the message */
	if ( ( toReturn = out->MakeEmpty() ) != B_OK ) {
		return toReturn;
	}
	
	/* Adding data about notification */
	if ( ( toReturn = out->AddBool( "Notification Enabled", bNotification ) ) != B_OK )
	{
		return toReturn;
	}
	
	if ( ( strlen( fNotificationText.String() ) > 0 ) &&
	     ( ( toReturn = out->AddString( "Notification Text", fNotificationText ) ) != B_OK ) )
	{
		return toReturn;
	}
	
	/* Adding data about sound play */
	if ( ( toReturn = out->AddBool( "Sound Play Enabled", bSound ) ) != B_OK )
	{
		return toReturn;
	}
	if ( ( fSoundFile.InitCheck() == B_OK ) &&
		  ( ( toReturn = out->AddString( "Sound File Path", fSoundFile.Path() ) ) != B_OK ) )
	{
		return toReturn;
	}
	
	/* Adding data about program to run */
	if ( ( ( toReturn = out->AddBool( "Program Run Enabled", bProgramRun ) ) != B_OK ) ||
		  ( ( toReturn = out->AddBool( "Program Verified", bVerifiedByUser ) ) != B_OK ) ||
		  ( ( toReturn = out->AddString( "Program Command Line Params", fCommandLineOptions ) ) != B_OK ) )
	{
		return toReturn;
	}
	if ( fProgramPath.InitCheck() == B_OK ) {
		if ( ( toReturn = out->AddString( "Program Path", fProgramPath.Path() ) ) != B_OK )
		{
			return toReturn;
		}
	}
	
	/* Adding data about Email - deprecated */
	
	return toReturn;
}	// <-- end of function ActivityData::Archive



/*!	\brief						Adds an Email address.
 *		\param[in]	addrIn		Address to be added
 */
void		ActivityData::SetEmailAddress( const char* toSet, int placeholder )
{
	fEmailAddress[ placeholder ].SetTo( toSet );
	if ( strlen( toSet ) == 0 ) {
		bIsAddressEmpty[ placeholder ] = true;
	} else {
		bIsAddressEmpty[ placeholder ] = false;
	}
}	// <-- end of ActivityData::SetEmailAddress



/*!	\brief		Verify command line parameters.
 *		\details		Trim the parameters' line; remove everything after && or ; to prevent maluse.
 *		\param[in]	in		Original parameters as string
 *		\returns		Modified parameters.
 */
BString		ActivityData::VerifyCommandLineParameters( const BString& in )
{
	BString 	toReturn( in );
	int32		tempInt;
	
	toReturn.Trim();
	
	if ( ( ( tempInt = toReturn.FindFirst( "&&" ) ) != B_ERROR ) ||
		  ( ( tempInt = toReturn.FindFirst( ";" ) ) != B_ERROR ) )
	{
		toReturn.Truncate( tempInt );
	}
	
	return toReturn;
}	// <-- end of function ActivityData::VerifyCommandLineParameters


/*!	\brief		Same function as the other version, but working on another input.
 *		\details		Implemented as a call to the other function.
 *		\param[in]	in		Array of chars with parameters.
 *		\returns		\c BString with the corrected string of parameters.
 */
BString		ActivityData::VerifyCommandLineParameters( const char* in )
{
	if ( in )
		return ActivityData::VerifyCommandLineParameters( BString( in ) );
	else
		return BString( "" );
}	// <-- end of second version of function ActivityData::VerifyCommandLineParameters



/*!	\brief		Perform the desired activity
 *		\details		This function is the central function of the activity mechanism.
 *						However, it does not display the notification message - mainly because
 *						it has no idea about the Event it belongs to, therefore it doesn't know
 *						the Event's name, category, and can't handle the "Snooze" message.
 *		\param[in]	in		Pointer to the \c Activity to be performed.
 */
void			ActivityData::PerformActivity( ActivityData* in )
{
	if ( !in ) { return; }
	
	BEntry		entry;
	entry_ref	fileRef, appRef;
	BPath			path;
	BString		tempString, anotherTempString;
	int			thread_id;
	
	// The notification will be displayed separately

	// Run a program
	if ( in->GetProgram( &path, &tempString ) )
	{
		entry.SetTo( path.Path(), true );		// Find the application to run
		if ( ( entry.InitCheck() == B_OK )			&&		// The initialization passed
		     ( entry.Exists() )							&&		// The entry exists
		     ( entry.GetPath( &path ) == B_OK ) )			// Got path to file (which may be not what the user entered)		
		{
			entry.Unset();		// Don't need the entry anymore
			anotherTempString.SetTo( path.Path() );
			anotherTempString << ' ';
			tempString.Prepend( anotherTempString );
		
				// Launch the program!	
			thread_id = fork();
			if ( thread_id == 0 ) {
				exit( system( tempString.String() ) );
			}
		}
	}
	
	// Play a sound file
	if ( in->GetSound( &path ) ) {
		entry.SetTo( path.Path(), true );		// Find the file to play
		if ( ( entry.InitCheck() == B_OK ) 			&&		// The initialization passed
			  ( entry.Exists() )							&&		// The file exists
			  ( entry.GetRef( &fileRef ) == B_OK ) &&		// Got the reference to the file
			  ( be_roster->Launch( &fileRef ) == B_OK ) )	// Launched the file!
		{
			entry.Unset();	// Close the file descriptor
			// Launch was already performed
		}
	}

}	// <-- end of function ActivityData::PerformActivity

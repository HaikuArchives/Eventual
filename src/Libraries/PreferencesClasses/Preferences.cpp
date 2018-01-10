/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

// Project includes
#include	"Utilities.h"
#include "Preferences.h"

#include "CalendarModulePreferences.h"
// #include "CategoriesPreferences.h"
#include "EmailPreferences.h"


// OS includes
#include <Directory.h>
#include <Path.h>
#include <File.h>
#include <FindDirectory.h>
#include <NodeInfo.h>
#include <String.h>
#include <StorageDefs.h>


// General includes
#include <stdlib.h>
#include <stdio.h>



/*****************************************************************************
 *				Definitions of global variables
 ****************************************************************************/


/*!	\brief	This is the placeholder for BMessage with all preferences.
 *		\note		
 *					This placeholder may be NULL, in which case the program will
 *					work with default preferences.
 */
BMessage*		global_PreferencesMessage; 



/*****************************************************************************
 *				Declarations of static functions
 ****************************************************************************/

	/* Opens the file with preferences' message for reading */
static status_t		OpenFileWithPreferences( BFile* out, uint32 openMode );

	/* Reads the file with preferences' message into preallocated BMessage */
static status_t		ReadFileWithPreferences( BFile* in );

	/* Writes the BMessage with preferences into previously opened file. */
static status_t		WriteFileWithPreferences( BFile* in );

/*****************************************************************************
 *				Definitions of global functions
 ****************************************************************************/

/*!	\brief		Loads all preferences from the preferences file.
 */
status_t		pref_PopulateAllPreferences( void )
{
	status_t toReturn = B_OK;
	BFile		preferencesFile;
	BString sb;
	
	/* Prepare the global preferences message placeholder. */
	global_PreferencesMessage = new BMessage( kOverallPreferences );
	if ( !global_PreferencesMessage )
	{
		sb.SetTo( "Did not succeed to allocate memory for the preferences." );
		utl_Deb = new DebuggerPrintout( sb.String() );
	}
	
	toReturn = OpenFileWithPreferences( &preferencesFile, B_READ_ONLY | B_CREATE_FILE );
	if ( toReturn != B_OK ) {
		sb.SetTo( "Did not succeed to open the file with preferences! " );
		sb << "Error is " << toReturn;
		utl_Deb = new DebuggerPrintout( sb.String() );
	}
	
	toReturn = ReadFileWithPreferences( &preferencesFile );
	if ( toReturn != B_OK && pref_SaveAllPreferences() != B_OK) {
		sb.SetTo( "Did not succeed to read the preferences from the file! Error = " );
		sb << toReturn;
		utl_Deb = new DebuggerPrintout( sb.String() );
		
		if ( global_PreferencesMessage )
			delete global_PreferencesMessage;

		global_PreferencesMessage = NULL;
	}
	
	/* At this point, the file is either read, and we can parse it, or it's not,
	 * and we should failback to default preferences.
	 */
	pref_PopulateCalendarModulePreferences( global_PreferencesMessage );
	
	toReturn = pref_PopulateEmailPreferences( global_PreferencesMessage );
	
	pref_PopulateTimePreferences( global_PreferencesMessage );
	
	pref_PopulateCategories( global_PreferencesMessage );
	
	preferencesFile.Unset();
	
	return B_OK;	
}	// <-- end of function	pref_PopulateAllPreferences



/*!	\brief		Reloads the preferences from the disk.
 */
status_t		pref_ReloadAllPreferences( void )
{
	status_t status = B_OK;
	BFile preferencesFile;
	
		/* Open the file - its location is predefined */
	status = OpenFileWithPreferences( &preferencesFile, B_READ_ONLY );
	if ( status == B_OK )
	{	
		if ( !global_PreferencesMessage ) {
				/* Allocate the global preferences placeholder */
			global_PreferencesMessage = new BMessage( kOverallPreferences );
		} else {
				/* If it was already allocated, clear it */
			status = global_PreferencesMessage->MakeEmpty();
		}
		if ( status == B_OK && global_PreferencesMessage )
		{
			/* Read the preferences */
			status = ReadFileWithPreferences( &preferencesFile );
			if ( status == B_OK )
			{
				pref_PopulateCalendarModulePreferences( global_PreferencesMessage );
				pref_PopulateEmailPreferences( global_PreferencesMessage );
				pref_PopulateTimePreferences( global_PreferencesMessage );
				pref_PopulateCategories( global_PreferencesMessage );
			}
		}	// <-- the placeholder for global messages exists
		preferencesFile.Unset();
	} // <-- end of "if ( succeeded to open the file with preferences )"

	return status;	
}	// <-- end of function pref_ReloadAllPreferences



/*!	\brief		Saves all preferences into a file.
 */
status_t		pref_SaveAllPreferences( void )
{
	BFile preferencesFile;
	BString sb;
	status_t toReturn;
	
	if ( !global_PreferencesMessage ) {
		global_PreferencesMessage = new BMessage( kOverallPreferences );
		if ( !global_PreferencesMessage )
		{
			sb.SetTo( "Can't create placeholder for the preferences!" );
			utl_Deb = new DebuggerPrintout( sb.String() );
			return B_ERROR;
		}
	}

	toReturn = OpenFileWithPreferences( &preferencesFile, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE );
	if ( toReturn != B_OK )
	{
		sb.SetTo( "Didn't succeed to open the file with preferences for writing! Error = " );
		sb << toReturn;
		utl_Deb = new DebuggerPrintout( sb.String() );
		return B_ERROR;
	}

	/* At this point, the file is open, and the message exists - either a newly created,
	 * or the same that was read.
	 * Time to fill it with data!
	 */
	
	toReturn = pref_SaveEmailPreferences( global_PreferencesMessage );
	
	toReturn = pref_SaveTimePreferences( global_PreferencesMessage );
	
	toReturn = pref_SaveCalendarModulePreferences( global_PreferencesMessage );
	if ( toReturn != B_OK )
	{
		sb.SetTo( "Did not save the Calendar Module preferences. Error = " );
		sb << ( unsigned int )toReturn;
		utl_Deb = new DebuggerPrintout( sb.String() );
	}
	
	toReturn = pref_SaveCategories( global_PreferencesMessage );
	
	/* Save the file to disk.
	 */
	toReturn = WriteFileWithPreferences( &preferencesFile );
	if ( toReturn != B_OK ) {
		sb.SetTo( "Didn't succeed to write the file with preferences! Error = " );
		sb << toReturn;
		utl_Deb = new DebuggerPrintout( sb.String() );
		return B_ERROR;
	}
	
	BNodeInfo nodeInfo( ( BNode* )&preferencesFile );
	
	nodeInfo.SetPreferredApp( kPreferencesPrefletApplicationSignature );
	
	/* Close the file */
	preferencesFile.Unset();

	return B_OK;
	
}	// <-- end of function pref_SaveAllPreferences
 

 
/*****************************************************************************
 *				Definitions of static functions
 ****************************************************************************/

/*!	\brief		Opens the file with preferences packed into BMessage for reading.
 *		\param[out]		out		Link to the BFile used to read the preferences from.
 *		\param[in]		openMode	Defines how the file should be opened. Read / write...
 *		\returns		B_OK in case of success.
 *		\note			
 *						Whatever was in the "out" variable, will be overwritten.
 */
status_t			OpenFileWithPreferences( BFile* out, uint32 openMode )
{
	status_t 	status = B_OK;
	BPath path;
	BString sb;
	BDirectory eventualSettingsDir;
	
	//	Access the overall settings directory
	status = find_directory( B_USER_SETTINGS_DIRECTORY,
							 		 &path,
							 		 true );	// Create directory if necessary
	if ( status != B_OK )
	{
		return status;
	}
	
	// Descent to application's settings directory
	path.Append( "Eventual" );
	eventualSettingsDir.SetTo( path.Path() );
	status = eventualSettingsDir.InitCheck();
	switch ( status )
	{
		case B_ENTRY_NOT_FOUND:
			// The directory does not exist. Create it!
			status = eventualSettingsDir.CreateDirectory( path.Path(),
																	    &eventualSettingsDir );
			if ( status != B_OK )
			{
				/* Panic! */
				sb.SetTo( "Error in creating Eventual Settings directory! Error = " );
				sb << ( uint32 )status;
				utl_Deb = new DebuggerPrintout( sb.String() );
				return status;
			}
			
			/* I assume at this point the directory is set. */
			break;
		
		case B_NAME_TOO_LONG:	/* Intentional fall-through */
		case B_BAD_VALUE:
		case B_FILE_ERROR:
		case B_NO_MORE_FDS:
			utl_Deb = new DebuggerPrintout( "Name is too long, input is invalid or node is busy." );
			return status;
			break;
			
		case B_LINK_LIMIT:
			utl_Deb = new DebuggerPrintout( "Loop is detected in the filesystem!" );
			return status;
			break;
			
		case B_BUSY:
			utl_Deb = new DebuggerPrintout( "The directory does not exist!" );
			return status;
			break;
		
		case B_OK:				/* Everything went smooth */
			break;
			
		default:
			utl_Deb = new DebuggerPrintout( "Unknown error has occurred." );
			break;
	};
	
	/* Anyway, at this point the directory is set, or we have exitted. */
	
	// Descend into preferences file.
	path.Append( "Preferences" );
	out->SetTo( path.Path(),
				   openMode );
							   
	if ( ( status = out->InitCheck() ) != B_OK )
	{
		/* Panic! */
		return status;
	}
	
	return B_OK;
	
}	// <-- end of function OpenFileWithPreferences



/*!	\brief		Deserialize the file with preferences into BMessage.
 *		\param[in]	in		Pointer to BFile to read the preferences from.
 *		\returns		B_OK if everything is Ok.
 */
status_t		ReadFileWithPreferences( BFile* in )
{
	status_t toReturn = B_OK;
	
	// If there's no preallocated preferences, no need to read.
	if ( global_PreferencesMessage )
	{
		toReturn = global_PreferencesMessage->Unflatten( in );
	}
	return toReturn;
	
}	// <-- end of function ReadFileWithPreferences



/*!	\brief		Serializes the BMessage with preferences into file.
 *		\param[in]	in		Pointer to BFile to write the preferences to.
 *		\returns		B_OK if everything is Ok.
 */
static
status_t		WriteFileWithPreferences( BFile* in )
{
	status_t toReturn = B_OK;
	
	// If there's no preallocated preferences, no need to read.
	if ( global_PreferencesMessage )
	{
		toReturn = global_PreferencesMessage->Flatten( in );
	}
	return toReturn;
	
}	// <-- end of function WriteFileWithPreferences

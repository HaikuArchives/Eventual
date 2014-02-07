/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "EmailPreferences.h"
#include	"Utilities.h"

#include <stdlib.h>
#include <stdio.h>



/*****************************************************************************
 *				Definitions of global variables
 ****************************************************************************/

	/*!	\brief	Modified Email preferences 	*/
			EmailPreferences* pref_EmailPreferences_modified;
			
	/*!	\brief	Original Email preferences		
	 *		\details	Declared as static to prevent unnecessary access. */
static	EmailPreferences* pref_EmailPreferences_original;

	/*!	\brief	This boolean defines if I can safely delete non-NULL pointers.
	 */
static	bool bPreferencesWereRead = false;

/*****************************************************************************
 *				Definitions of global functions
 ****************************************************************************/

/*!	\brief		Obtain Email preferences from the preferences message.
 *		\param[in]	in		BMessage which contains another message with the preferences.
 *		\details		In case of any error, defaults are used.
 *		\note
 *						in may be NULL!
 */
status_t		pref_PopulateEmailPreferences( BMessage* in ) {
	BMessage toCheck( kEmailPreferences );
	
	/*!	\warning		The following section is not thread-safe! */
	if ( bPreferencesWereRead )
	{
		if ( pref_EmailPreferences_original ) {
			delete pref_EmailPreferences_original;
			pref_EmailPreferences_original = NULL;
		}
		if ( pref_EmailPreferences_modified ) {
			delete pref_EmailPreferences_modified;
			pref_EmailPreferences_modified = NULL;
		}
	}
	
	if ( in && ( B_OK == in->FindMessage( "Email Preferences", &toCheck ) ) )
	{
		pref_EmailPreferences_original = new EmailPreferences( &toCheck );
	} else {
		pref_EmailPreferences_original = new EmailPreferences();	// Defaults
	}
	if ( !pref_EmailPreferences_original )	{
		/* Probably error, but hey, let's run anyway.
		 *
		return B_ERROR;
		 */
	}
	
	pref_EmailPreferences_modified = new EmailPreferences( pref_EmailPreferences_original );
	if ( !pref_EmailPreferences_modified ) {
		/* Well, now this IS an error. */
		return B_ERROR;
	}
	
	bPreferencesWereRead = true;
	
	return B_OK;
	
}	// <-- end of function	pref_PopulateEmailPreferences



/*!	\brief		Saves the preferences.
 *		\details		The save is performed only if the preferences differ
 *						or if the original preferences did not exist.
 *		\param[out]		out		The BMessage to which the preferences should be added.
 */
status_t		pref_SaveEmailPreferences( BMessage* out )
{
	if (  !out ||										// Nowhere to save
			!pref_EmailPreferences_modified )	// Nothing to save
	{
		return B_ERROR;		// Duh
	}
	
	status_t		status = B_OK;
	
	BMessage* toAdd = new BMessage( kEmailPreferences );
	if ( !toAdd )				// Nothing to save
	{
		return B_ERROR;
	}
	
	/* Save data into the message */
	if ( B_OK != ( status = pref_EmailPreferences_modified->Archive( toAdd ) ) )
	{
		return status;
	}
	
	
	/* If there were no original preferences or
	 *	if the preferences were modified or
	 * if the message didn't have information on the preferences
	 *		add the created message to the output
	 */
	if ( ( !pref_EmailPreferences_original ) || 
	     ( !pref_EmailPreferences_original->Compare( pref_EmailPreferences_modified ) ) )
	{
		if ( out->HasMessage( "Email Preferences" ) )
		{
			status = out->ReplaceMessage( "Email Preferences", toAdd );
		}
		else
		{
			status = out->AddMessage( "Email Preferences", toAdd );
		}
	}
	return status;		// Error or B_OK - it doesn't matter here.
	
}	// <-- end of function pref_SaveEmailPreferences
 
 

/*****************************************************************************
 *				Implementation of class EmailPreferences
 ****************************************************************************/

/*!	\brief				Constructor from BMessage which can be null
 *		\param[in]	in		The BMessage to construct this object from.
 */
EmailPreferences::EmailPreferences( BMessage* in )
{
	if ( ( !in ) || B_OK != in->FindString( "ReplyToAddress", &replyToAddress ) )
	{
		replyToAddress.SetTo( "" );
	}
	
	if ( ( !in ) || B_OK != in->FindString( "MailServerAddress", &mailServerAddress ) )
	{
		mailServerAddress.SetTo( "" );
	}
	
	if ( ( !in ) || ( B_OK != in->FindInt32( "MailServerPort", ( int32* )&mailServerPort ) ) )
	{
		mailServerPort = 23;
	}
}	// <-- end of constructor from BMessage



/*!	\brief		Copy constructor.
 */
EmailPreferences::EmailPreferences( const EmailPreferences& other )
{
	*this = other;
}	// <-- end of copy constructor


/*!	\brief		Assignment operator.
 */
EmailPreferences	EmailPreferences::operator= ( const EmailPreferences& other )
{
	this->replyToAddress.SetTo( other.replyToAddress );
	this->mailServerAddress.SetTo( other.mailServerAddress );
	this->mailServerPort = other.mailServerPort;

	return *this;	
}	// <-- end of assignment operator.


/*!	\brief		Another copy constructor.
 */
EmailPreferences::EmailPreferences( const EmailPreferences* other )
{
	if ( other )
	{
		this->replyToAddress.SetTo( other->replyToAddress );
		this->mailServerAddress.SetTo( other->mailServerAddress );
		this->mailServerPort = other->mailServerPort;
	}
	else		// Setting default preferences
	{
		replyToAddress.SetTo( "" );
		mailServerAddress.SetTo( "" );
		mailServerPort = 23;
	}
}	// <-- end of copy constructor



/*!	\brief		Pack the information from this object into the submitted message.
 *		\param[out]	out	The BMessage to add the information to.
 *		\note			Note:
 *						This function uses obsolete code!
 *		\returns		B_OK if everything was Ok.
 */
status_t		EmailPreferences::Archive( BMessage* out ) const
{
	status_t	status;
	if ( !out ) return B_ERROR;
	
	if ( out->HasString( "ReplyToAddress" ) ) {
		status = out->ReplaceString( "ReplyToAddress", this->replyToAddress );
	} else {
		status = out->AddString( "ReplyToAddress", this->replyToAddress );
	}
	if ( status != B_OK ) { return status; }
	
	if ( out->HasString( "MailServerAddress" ) )	{
		status = out->ReplaceString( "MailServerAddress", this->mailServerAddress );
	} else {
		status = out->AddString( "MailServerAddress", this->mailServerAddress );
	}
	if ( status != B_OK ) { return status; }
	
	if ( out->HasInt32( "MailServerPort" ) ) {
		status = out->ReplaceInt32( "MailServerPort", this->mailServerPort );
	} else {
		status = out->AddInt32( "MailServerPort", this->mailServerPort );
	}
	return status;	
	
}	// <-- end of function EmailPreferences::Archive



/*!	\brief		Comparison operator.
 *		\details		Returns "true" if objects are exactly equal, else returns "false".
 *		\param[in]	other	The object to compare with.
 */
bool		EmailPreferences::operator== ( const EmailPreferences& other ) const
{
	if ( ( this->replyToAddress == other.replyToAddress ) &&
		  ( this->mailServerAddress == other.mailServerAddress ) &&
		  ( this->mailServerPort == other.mailServerPort ) )
	{
		return true;
	}
	return false;	
}	// <-- end of comparison operator


/*!	\brief		Comparison operator on pointers.
 *		\details		Returns "true" if objects are exactly equal, else returns "false".
 *		\param[in]	other	The pointer to object to compare with.
 */
bool		EmailPreferences::Compare ( const EmailPreferences* other ) const
{
	if ( ( this->replyToAddress == other->replyToAddress ) &&
		  ( this->mailServerAddress == other->mailServerAddress ) &&
		  ( this->mailServerPort == other->mailServerPort ) )
	{
		return true;
	}
	return false;	
}	// <-- end of comparison operator




/*!	\brief		Update the reply-to address
 *		\details		Performs some basic check to verify the input looks like Email address
 *		\param[in]	in		The address.
 *		\returns		B_OK if the string looks like Email address, B_ERROR otherwise.
 */
status_t		EmailPreferences::UpdateReplyToAddress( const BString& in )
{
	if ( utl_VerifyEmailAddress( in.String() ) )
	{
		this->replyToAddress = in;
		return B_OK;
	} else {
		return B_ERROR;
	}
}


/*!	\brief		Update the mail server address.
 *		\details		Performs some basic check to verify the input looks like domain address.
 *		\param[in]	in		The address
 *		\returns		B_OK if the string looks like Email address, B_ERROR otherwise.
 */
status_t		EmailPreferences::UpdateMailServerAddress( const BString& in )
{
	if ( utl_VerifySeverAddress( in.String() ) )
	{
		this->mailServerAddress = in;
		return B_OK;
	}
	else
	{
		return B_ERROR;
	}
}



/*!	\brief		Update the mail server port.
 *		\details		Parse the submitted string for the numbers. Take first number found.
 *		\returns		B_OK on success, B_ERROR on failure.
 */
status_t		EmailPreferences::UpdateMailServerPort( const BString& in )
{
	uint32 port;
	
	/* Trying first to parse the string as unsigned decimal integer,
	 *	then as hexadecimal integer,
	 * last as octal integer.
	 *	If anything succeeds, surely return B_OK.
	 */
	if ( ( sscanf( in.String(), "0x%X", ( unsigned int* )&port ) ) ||
		  ( sscanf( in.String(), "0x%x", ( unsigned int* )&port ) ) ||
		  ( sscanf( in.String(), "0%o",  ( unsigned int* )&port ) ) ||
		  ( sscanf( in.String(), "%u",   ( unsigned int* )&port ) ) )
	{
		return this->UpdateMailServerPort( port );	
	}
	
	/* Could not parse the submitted string. Return failure. */
	return B_ERROR;
	
}	// <-- end of function EmailPreferences::UpdateMailServerPort



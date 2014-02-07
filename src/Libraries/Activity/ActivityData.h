/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ACTIVITY_DATA_H_
#define _ACTIVITY_DATA_H_

// POSIX includes
#include <stdio.h>

// OS includes
#include <Path.h>
#include <String.h>
#include <SupportDefs.h>

// Project includes



/*---------------------------------------------------------------------------
 *								Message constants
 *--------------------------------------------------------------------------*/

const uint32	kActivityData = 'ACTV';


/*---------------------------------------------------------------------------
 *								Declaration of class ActivityData
 *--------------------------------------------------------------------------*/

/*!	\brief		Holds data about the activity to be performed when time arrives.
 *		\attention	Number of possible Email recipients is hard-coded to be 3.
 */
#define		ACTIVITY_NUMBER_OF_EMAIL_ADDRESSES		( 3 )
 
 
 
/*!	\brief		Holds maximum length of command-line options for a scheduled prorgam.
 *		\details		This number is purely arbitrary; I just don't want users to write
 *						"War and Peace" in the command-line parameters.
 */
#define		ACTIVITY_MAX_ALLOWED_COMMAND_LINE_OPTIONS_LENGTH		( 511 )
 
class ActivityData 
{
public:
	//!	\name			Constructor and destructor
	///@{
	ActivityData( BMessage* in = NULL );
	virtual	~ActivityData() {}	// No dynamically allocated data - no need in destructor
	///@}

	//!	\name			Help and support static functions
	///@{
	static BString	VerifyCommandLineParameters( const BString& in );
	static BString	VerifyCommandLineParameters( const char* in );
	static void		PerformActivity( ActivityData* in );
	///@}

	//!	\name			Archive and unarchive functions
	///@{
	virtual status_t	Archive( BMessage* out );
	virtual void		Instantiate( BMessage* in = NULL );
	///@}
	
	//!	\name			Getters and setters for the Notification
	///@{
	inline virtual void	SetNotification( bool toSet ) { bNotification = toSet; }
	inline virtual void	SetNotification( bool toSet, const BString& textIn ) {
		bNotification = toSet;
		fNotificationText = textIn;
	}
	inline virtual void	SetNotificationText( const BString& textIn ) {
		fNotificationText = textIn;
	}
	inline virtual bool	GetNotification( BString* textOut = NULL ) const {
		if ( textOut ) { textOut->SetTo( fNotificationText ); }
		return bNotification;
	}
	///@}
	
	//!	\name			Getters and setters for sound play
	///@{
			 virtual void	SetSound( bool toSet ) { bSound = toSet; }
	inline virtual void	SetSound( bool toSet, const BPath& pathIn ) {
		bSound = toSet;
		fSoundFile = pathIn;
	}
	inline virtual void	SetSoundFile( const BPath& pathIn ) { fSoundFile = pathIn; }
	inline virtual bool	GetSound( BPath* pathOut = NULL ) const {
		if ( pathOut ) { pathOut->SetTo( fSoundFile.Path() ); }
		return bSound;
	}
	///@}	

	//!	\name 		Getters and setters for the program
	///@{
	inline virtual void	SetProgram( bool toSet ) { bProgramRun = toSet; }
	inline virtual void	SetProgram( bool toSet, const BPath& pathIn ) {
		bProgramRun = toSet; fProgramPath = pathIn;
	}
	inline virtual void 	SetProgram( bool toSet, const BPath& pathIn, const BString& paramsIn ) {
		bProgramRun = toSet; fProgramPath = pathIn; fCommandLineOptions = paramsIn;
	}
	inline virtual void	SetProgramPath( const BPath& pathIn ) {
		fProgramPath = pathIn;
	}
	inline virtual void	SetProgramOptions( const BString& paramsIn ) {
		fCommandLineOptions = paramsIn;
	}
	inline virtual bool	GetProgram( BPath* pathOut = NULL, BString* paramsOut = NULL ) const {
		if ( pathOut ) 	{ pathOut->SetTo( fProgramPath.Path() ); }
		if ( paramsOut )	{ paramsOut->SetTo( fCommandLineOptions ); }
		return bProgramRun;
	}
	inline virtual void	SetProgramVerified( bool toSet ) { bVerifiedByUser = toSet; }
	inline virtual bool 	GetProgramVerified( void ) const { return bVerifiedByUser; }
	///@}
	
	//!	\name	 Getters and setters for Email
	///@{
	inline virtual void	SetEmail( bool toSet ) { bEmailToSend = toSet; }
	inline virtual void	SetEmailSubject( const BString& subjIn ) { fEmailSubject = subjIn; }
	inline virtual void	SetEmailContents( const BString& contIn ) { fEmailContents = contIn; }
	inline virtual void	SetEmailAddress( const char* addrIn, int placeholder = 0 );
	inline virtual bool	GetEmailSubjectAndContents( BString* subjOut, BString* contOut ) {
		if ( subjOut ) { subjOut->SetTo( fEmailSubject ); }
		if ( contOut ) { contOut->SetTo( fEmailContents ); }
		return bEmailToSend;
	}
	inline virtual BString GetEmailAddress( int placeholder = 0 ) {
		return BString( fEmailAddress[ placeholder ] );
	}
	///@}
	
protected:

	/* Data holders */
	bool		bNotification;			//!< Should a notification be displayed?
	BString	fNotificationText;	//!< What should be the text of notification?
	
	bool		bSound;					//!< Should a sound file be played?
	BPath		fSoundFile;				//!< What is the file to play?
	
	bool		bEmailToSend;			//!< Should the Email be sent?
	BString	fEmailContents;		//!< Contents of the Email (message body)
	BString	fEmailSubject;			//!< Subject of the Email
	BString	fEmailAddress[ ACTIVITY_NUMBER_OF_EMAIL_ADDRESSES ];	//!< Addresses of receivers
	bool		bIsAddressEmpty[ ACTIVITY_NUMBER_OF_EMAIL_ADDRESSES ];//!< "true" if corresponding address placeholder is empty
	
	bool		bProgramRun;			//!< Should a program be run?
	bool		bVerifiedByUser;		//!< Did the user check the program?
	BPath		fProgramPath;			//!< Path to the program to be run
	BString	fCommandLineOptions;	//!< Additional options for the program
	
	/* Service functions */
	
};	// <-- end of class ActivityData

#endif // _ACTIVITY_DATA_H_

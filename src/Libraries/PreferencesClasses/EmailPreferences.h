/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EMAIL_PREFERENCES_H_
#define _EMAIL_PREFERENCES_H_


#include <Message.h>
#include <String.h>
#include <SupportDefs.h>

/*----------------------------------------------------------------------------
 *							Message constant
 *---------------------------------------------------------------------------*/

const uint32	kEmailPreferences		= 'EMAP';


/*----------------------------------------------------------------------------
 *							Declaration of class EmailPreferences
 *---------------------------------------------------------------------------*/

class EmailPreferences
{
	protected:
		BString 	replyToAddress;
		BString 	mailServerAddress;
		uint32	mailServerPort;
	
	public:
		EmailPreferences( BMessage* in = NULL );
		EmailPreferences( const EmailPreferences& other );
		EmailPreferences( const EmailPreferences* other );
		
		virtual status_t		Archive( BMessage* out ) const;
		
		virtual BString		GetReplyToAddress() const { return replyToAddress; }
		virtual BString		GetMailServerAddress() const { return mailServerAddress; }
		virtual uint32			GetMailServerPort() const { return mailServerPort; }
		virtual BString		GetMailServerPortAsString() const { BString port; port << mailServerPort; return port; }
		
		virtual status_t		UpdateMailServerAddress( const BString& in );
		virtual status_t		UpdateReplyToAddress( const BString& in );
		virtual status_t		UpdateMailServerPort( const BString& in );
		inline virtual status_t		UpdateMailServerPort( uint32 in ) { mailServerPort = in; return B_OK; }
		
		virtual EmailPreferences operator= ( const EmailPreferences& other );
		virtual bool	Compare( const EmailPreferences* other ) const;
		virtual bool	operator== ( const EmailPreferences& other ) const;
		virtual inline bool	operator!= ( const EmailPreferences& other ) const { return !(*this == other); }
};



/*----------------------------------------------------------------------------
 *							Declaration of global variables
 *---------------------------------------------------------------------------*/

extern	EmailPreferences*	pref_EmailPreferences_modified;



/*----------------------------------------------------------------------------
 *							Declarations of global functions
 *---------------------------------------------------------------------------*/

status_t		pref_PopulateEmailPreferences( BMessage* in = NULL );

status_t		pref_SaveEmailPreferences( BMessage* out );

inline	EmailPreferences*		pref_GetEmailPreferences() { return pref_EmailPreferences_modified; }

#endif // _EMAIL_PREFERENCES_H_

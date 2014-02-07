/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _TIME_PREFERENCES_H_
#define _TIME_PREFERENCES_H_


#include <Message.h>
#include <String.h>
#include <SupportDefs.h>

// Local includes
#include "TimeRepresentation.h"

/*----------------------------------------------------------------------------
 *							Message constant
 *---------------------------------------------------------------------------*/

const uint32	kTimePreferences		= 'TPRF';


/*----------------------------------------------------------------------------
 *							Declaration of class TimePreferences
 *---------------------------------------------------------------------------*/

class TimePreferences
{
	protected:
		bool		use24hClock;
		TimeRepresentation		defaultAppointmentDuration;
		TimeRepresentation		defaultReminderTime;
		TimeRepresentation		defaultSnoozeTime;
	
	public:
		TimePreferences( BMessage* in = NULL );
		TimePreferences( const TimePreferences& other );
		TimePreferences( const TimePreferences* other );
		// No need in destructor - nothing was allocated dynamically.
		
		virtual 	status_t			Archive( BMessage* out ) const;
		
		// Get and set routines for 24 hours clock
		inline virtual bool		Get24hClock() const { return use24hClock; }
		inline virtual void		Set24hClock( bool in ) { use24hClock = in; }

		// Retrieve and update routines for default appointment duration
		inline virtual TimeRepresentation		GetDefaultAppointmentDuration( void ) {
			return defaultAppointmentDuration;
		}
		inline virtual void		GetDefaultAppointmentDuration( int* hours, int* mins ) {
			if ( hours ) { *hours = defaultAppointmentDuration.tm_hour; }
			if ( mins  ) { *mins  = defaultAppointmentDuration.tm_min;  }
		}
		inline virtual void		SetDefaultAppointmentDuration( const TimeRepresentation& in ) {
			defaultAppointmentDuration = in;
		}		
		virtual void				SetDefaultAppointmentDuration( int hours, int minutes );
		
		// Retrieve and update routines for default reminder firing time
		inline virtual TimeRepresentation		GetDefaultReminderTime( void ) {
			return defaultReminderTime;
		}
		inline virtual void		GetDefaultReminderTime( int* hours, int* mins ) {
			if ( hours ) { *hours = defaultReminderTime.tm_hour; }
			if ( mins  ) { *mins  = defaultReminderTime.tm_min;  }
		}
		inline virtual void		SetDefaultReminderTime( const TimeRepresentation& in ) {
			defaultReminderTime = in;
		}		
		virtual void				SetDefaultReminderTime( int hours, int minutes );
		
		
		// Retrieve and update routines for default snooze time
		inline virtual TimeRepresentation		GetDefaultSnoozeTime( void ) const {
			return	defaultSnoozeTime;
		}
		inline virtual void		GetDefaultSnoozeTime( int* hours, int* mins ) {
			if ( hours ) { *hours = defaultSnoozeTime.tm_hour; }
			if ( mins  ) { *mins  = defaultSnoozeTime.tm_min;  }
		}
		inline virtual void		SetDefaultSnoozeTime( const TimeRepresentation& in ) {
			defaultSnoozeTime = in;
		}		
		virtual void				SetDefaultSnoozeTime( int hours, int minutes );
		
		virtual TimePreferences operator= ( const TimePreferences& other );
		virtual bool	Compare( const TimePreferences* other ) const;
		virtual bool	operator== ( const TimePreferences& other ) const;
		virtual inline bool	operator!= ( const TimePreferences& other ) const { return !(*this == other); }
};



/*----------------------------------------------------------------------------
 *							Declaration of global variables
 *---------------------------------------------------------------------------*/

extern	TimePreferences*	pref_TimePreferences_modified;



/*----------------------------------------------------------------------------
 *							Declarations of global functions
 *---------------------------------------------------------------------------*/

status_t		pref_PopulateTimePreferences( BMessage* in = NULL );

status_t		pref_SaveTimePreferences( BMessage* out );

inline		TimePreferences*		pref_GetTimePreferences() { return pref_TimePreferences_modified; }

#endif // _TIME_PREFERENCES_H_

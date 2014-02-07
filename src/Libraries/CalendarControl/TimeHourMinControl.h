/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _TIME_HOUR_MIN_CONTROL_H_
#define _TIME_HOUR_MIN_CONTROL_H_

// System includes

// Project includes
#include "GeneralHourMinControl.h"
#include "Preferences.h"

// Messages constant
const uint32		kTimeControlUpdated	= 'TCUP';



/*!
 * 	\brief		This class is used to display a "real" time - am/pm.
 *		\details		It extends (or, rather, narrows) GeneralHourMinControl to
 *						a constant label (PM) which is enabled according to the user's choise.
 *						Hours menu has always either 24 or 12 items. No limits supported.
 */
class TimeHourMinControl
	:
	public GeneralHourMinControl
{
protected:
	bool	bUse24hClock;	//!< This member defines if 24-hours clock is used. It's set once and never changes.
	
	// Redefining the function of the base class
	virtual BMenu*		CreateHoursMenu( void );
	virtual void		UpdateSelectedValuesInMenus( void );

public:	
	TimeHourMinControl( BRect frame,
							  const char* name,
							  const BString& label,
							  BMessage *message = NULL );
	virtual ~TimeHourMinControl() {};
	
	// Disabling some functions of the base class
	virtual void 		SetCheckBoxLabel( const BString& label = BString( "" ) ) { return; }
	virtual void		SetHoursLimit( unsigned int hoursLimit ) { return; }
	virtual void		SetMinutesLimit( unsigned int minutesLimit ) { return; }
	
	// Redefining some functions of the base class
	virtual void		SetEnabled( bool in );
	virtual void		MessageReceived( BMessage* in );

};	// <-- end of class TimeHourMinControl

#endif // _TIME_HOUR_MIN_CONTROL_H_

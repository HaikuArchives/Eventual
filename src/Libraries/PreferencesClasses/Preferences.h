/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <Message.h>
#include <SupportDefs.h>

#include "EmailPreferences.h"
#include "CalendarModulePreferences.h"
#include "TimePreferences.h"
#include "CategoriesPreferences.h"

/*----------------------------------------------------------------------------
 *							Message constant
 *---------------------------------------------------------------------------*/

const uint32	kOverallPreferences		= 'GLPR';


/*----------------------------------------------------------------------------
 *							Declaration of class 
 *---------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------
 *							Declaration of global variables
 *---------------------------------------------------------------------------*/

extern	BMessage* global_PreferencesMessage;


/*----------------------------------------------------------------------------
 *							Declarations of global functions
 *---------------------------------------------------------------------------*/

status_t		pref_PopulateAllPreferences( void );

status_t		pref_SaveAllPreferences( void );

status_t		pref_ReloadAllPreferences( void );

inline BMessage*	pref_GetOverallPreferencesMessage() { return global_PreferencesMessage; }


#endif // _PREFERENCES_H_

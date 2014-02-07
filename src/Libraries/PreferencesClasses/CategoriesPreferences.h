/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CATEGORIES_PREFERENCES_H_
#define _CATEGORIES_PREFERENCES_H_

#include "Category.h"

#include <Message.h>
#include <String.h>
#include <SupportDefs.h>

/*----------------------------------------------------------------------------
 *							Message constant
 *---------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------
 *							Declaration of class 
 *---------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------
 *							Declaration of global variables
 *---------------------------------------------------------------------------*/




/*----------------------------------------------------------------------------
 *							Declarations of global functions
 *---------------------------------------------------------------------------*/

status_t		pref_PopulateCategories( BMessage* in = NULL );

status_t		pref_SaveCategories( BMessage* out );

inline	BList*		pref_GetCategoriesList() { return &global_ListOfCategories; }

#endif // _CATEGORIES_PREFERENCES_H_

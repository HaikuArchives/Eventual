/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "CalendarModulePreferences.h"
#include "Utilities.h"

#include <Box.h>
#include <CheckBox.h>
#include <GridLayout.h>
#include <GroupLayout.h>
#include <InterfaceDefs.h>
#include <Layout.h>
#include <LayoutItem.h>
#include <ListView.h>
#include <ListItem.h>
#include <Message.h>
#include <MenuItem.h>
#include <String.h>

#include <stdio.h>
#include <stdlib.h>

/****************************************************************************
 *			Global variables and definitions
 ***************************************************************************/

	/*!	\brief	Original (loaded from file) preferences for every calendar module */
static 
CalendarModulePreferences*	pref_CalendarModulePrefs_original[ NUMBER_OF_CALENDAR_MODULES ];

	/*!	\brief	Modified by the user preferences for every calendar module */
CalendarModulePreferences*	pref_CalendarModulePrefs_modified[ NUMBER_OF_CALENDAR_MODULES ];

	/*!	\brief	Were the preferences already read? */
static 	bool bPreferencesWereRead = false;

	/*!	\brief	Names of order items. */
#define	PARSER( value, name )	name,
char*	DmyOrderNames[] = {
	DMY_ORDER
};
#undef	PARSER


/****************************************************************************
 *			Static functions' declarations
 ***************************************************************************/

	/* Create a message with specified module's preferences */
static
BMessage* 		PackPreferencesIntoMessage( const BString& id = BString("") );

	/* Get the original preferences - as opposed to the modified ones,
	 *	which are public, these are hidden.
	 */
static
CalendarModulePreferences*		GetOriginalPreferencesForCalendarModule( const BString& id );


/****************************************************************************
 *			Global functions implementation
 ***************************************************************************/


/*!	\brief			This function loads the Preferences.
 *	\details		Initially, the original and the modified preferences are equal.
 */
void			pref_PopulateCalendarModulePreferences( BMessage* in )
{
	int			index;
	BString		sb;
	BMessage	preferences;
	CalendarModule*		calModule;
	
	/*!	\warning		This section is not thread-safe!
	 */
	
	/* I can delete old Calendar Module preferences only if I allocated them before.
	 * This is where bPreferencesWereRead comes in. If it's true, I can safely delete
	 * what is not NULL.
	 */
	if ( bPreferencesWereRead ) 
	{
		// Cleaning old preferences
		for ( index = 0; index < NUMBER_OF_CALENDAR_MODULES; ++index )
		{
			if ( pref_CalendarModulePrefs_original[ index ] != NULL ) {
				delete pref_CalendarModulePrefs_original[ index ];
				pref_CalendarModulePrefs_original[ index ] = NULL;
			}
			if ( pref_CalendarModulePrefs_modified[ index ] != NULL ) {
				delete pref_CalendarModulePrefs_modified[ index ];
				pref_CalendarModulePrefs_modified[ index ] = NULL;
			}
		}
	}
	// <-- end of cleanup section.
		
	for ( index = 0; index < NUMBER_OF_CALENDAR_MODULES; ++index )
	{
		// Get the calendar module
		// We can't use utility string here, because we must pass over all possible
		//		calendar modules - we still don't know their names.
		calModule = ( CalendarModule* )global_ListOfCalendarModules.ItemAt( index );
		if ( !calModule ) {
			pref_CalendarModulePrefs_original[ index ] = NULL;
			pref_CalendarModulePrefs_modified[ index ] = NULL;
			continue;
		}

		// Build the name of the message with preferences to search for
		sb.SetTo( "CalendarModulePreferences" );
		sb << calModule->Identify();
		
		if ( !in ||
		     ( in->FindMessage( sb.String(), &preferences) != B_OK ) )
		{
			// Didn't find the data - using default preferences
			pref_CalendarModulePrefs_original[ index ] = new CalendarModulePreferences( calModule->Identify() );
			pref_CalendarModulePrefs_modified[ index ] = new CalendarModulePreferences( calModule->Identify() );
			if ( ( !pref_CalendarModulePrefs_original[ index ] ) ||
			     ( !pref_CalendarModulePrefs_modified[ index ] ) ) 
			{
				utl_Deb = new DebuggerPrintout( "Did not succeed to set up default Calendar Module preferences!" );
			}
		}
		else
		{
			// Found the data - parse it to create the preferences
			
			pref_CalendarModulePrefs_original[ index ] = new CalendarModulePreferences( calModule->Identify(), &preferences );
			pref_CalendarModulePrefs_modified[ index ] = new CalendarModulePreferences( calModule->Identify(), &preferences );
			if ( pref_CalendarModulePrefs_modified[index]->InitCheck() != B_OK )
			{
				sb.SetTo( "Error - didn't initialize the prefs! " );
				sb << "Error = " << pref_CalendarModulePrefs_modified[index]->InitCheck();
				utl_Deb = new DebuggerPrintout( sb.String() );
			}
		}
	}	// <-- end of "for ( every possible Calendar Module, read its preferences )	
	
	// Mark the non-NULL pointer placeholders as safe for deletion.
	bPreferencesWereRead = true;
	
}	// <-- end of function PopulateCalendarModulePreferences



/*!	\brief		This function returns pointer to preferences of given module.
 *		\details		If such Calendar Module exists, this function returns pointer
 *						to its preferences. In case of any error, it will return NULL.
 *		\note
 *						This function returns only pointer to modified preferences.
 *						No-one can touch original preferences.
 */
CalendarModulePreferences*		pref_GetPreferencesForCalendarModule( const BString& id )
{	
	CalendarModulePreferences* toReturn = NULL, *inTest = NULL;
	
	for ( int index = 0; index < NUMBER_OF_CALENDAR_MODULES; ++index )
	{
		inTest = pref_CalendarModulePrefs_modified[ index ];
		
		if  ( ( inTest ) && 
				( inTest->GetId() == id ) )
		{
			toReturn = inTest;
			break;	
		}		
	}

	return toReturn;
}	// <-- end of function pref_GetPreferencesForCalendarModule


/*!	\brief		Attaches modified Calendar Modules' preferences to BMessage.
 *		\details		This function compares the modified Calendar Modules' preferences
 *						to the original ones. If there's difference, the function
 *						overwrites the saved preferences in BMessage with modified ones.
 *						If the message didn't contain data on specific Calendar Module
 *						preferences, the modified version is saved anyway.
 *		\param[out]		message	The message to which the preferences should be attached.
 *		\returns		B_OK if everything was Ok.
 */
status_t		pref_SaveCalendarModulePreferences( BMessage* message )
{
	status_t	status = B_OK;
	BMessage* toAdd = NULL;
	
	CalendarModulePreferences *prefOrig = NULL, *prefModif = NULL;
	CalendarModule *module;
	BString toFetch, sb;
		
		/* We should save all preferences for all calendar modules in the system.
		 */

	for ( int index = 0; index < NUMBER_OF_CALENDAR_MODULES; ++index )
	{
		// Obtain the calendar module. The only thing required from it is the ID.
		module = ( CalendarModule* )global_ListOfCalendarModules.ItemAt( index );
		if ( !module ) { continue; }
		
		toFetch = module->Identify();
		
		/* All the rest is exactly like in previous "if" section. */

		prefModif = pref_GetPreferencesForCalendarModule( toFetch );
		if ( !prefModif ) { continue; }
		
		prefOrig = GetOriginalPreferencesForCalendarModule( toFetch );
		
		toAdd = PackPreferencesIntoMessage( toFetch );

		sb.SetTo( "CalendarModulePreferences" );
		sb << toFetch;
		
		if ( toAdd &&
		     ( !prefOrig || *prefOrig != *prefModif ) )
		{
			if ( true == message->HasMessage( sb.String() ) )
			{
				status = message->ReplaceMessage( sb.String(), toAdd );
			}
			else
			{
				status = message->AddMessage( sb.String(), toAdd );
			}
			
			switch( status )
			{
				case B_ERROR:
					utl_Deb = new DebuggerPrintout( "Data is too massive (B_ERROR)." );
					break;
					
				case B_BAD_TYPE:
					utl_Deb = new DebuggerPrintout( "Wrong type of data (B_BAD_TYPE)." );
					break;
					
				case B_NO_MEMORY:
					utl_Deb = new DebuggerPrintout( "Not enough memory (B_NO_MEMORY)." );
					break;
					
				case B_BAD_VALUE:
					utl_Deb = new DebuggerPrintout( "Proposed name is too long (B_BAD_VALUE)." );
					break;
					
				case B_NAME_NOT_FOUND:					
					utl_Deb = new DebuggerPrintout( "Trying to replace something nonexistent (B_NAME_NOT_FOUND)." );
					break;

				case B_BAD_INDEX:
					utl_Deb = new DebuggerPrintout( "Index out of range (B_BAD_INDEX)." );
					break;
				
				default:		// Intentional fall-through
				case B_OK:
					continue;	
			}		
		}	// <-- end of "if ( we should actually save the preferences )"		
	}	// <-- end of "for (all calendar modules )"
	
	return B_OK;
	
}	// <-- end of function pref_SaveCalendarModulePreferences



/*!	\brief		Destructor for all Calendar Modules' preferences.
 *		\details		It does not check if the preferences were saved.
 */
void 		pref_DeleteCalendarModulePreferences( void )
{
	for ( int index = 0; index < NUMBER_OF_CALENDAR_MODULES; ++index )
	{
		if ( pref_CalendarModulePrefs_original[ index ] ) {
			delete pref_CalendarModulePrefs_original[ index ];
			pref_CalendarModulePrefs_original[ index ] = NULL;
		}
		
		if ( pref_CalendarModulePrefs_modified[ index ] ) {
			delete pref_CalendarModulePrefs_modified[ index ];
			pref_CalendarModulePrefs_modified[ index ] = NULL;
		}
	}
	
}	// <-- end of function pref_DeleteCalendarModulePreferences


/****************************************************************************
 *			Static functions implementation
 ***************************************************************************/

/*!	\brief		Creates a message with preferences.
 *		\details		For a given module, create a new BMessage and pack into it
 *						the modified preferences for this calendar module.
 *		\returns		Pointer to BMessage with preferences, if everything is Ok.
 *						NULL in case of any error.
 *		\note			Ownership of the created BMessage
 *						This function only creates the message. It's caller's task
 *						to get rid of the created BMessage when it's not needed.
 */
static
BMessage* 		PackPreferencesIntoMessage( const BString& id )
{
	BMessage* toReturn = NULL;
	CalendarModulePreferences* pref = NULL;
	
	if ( id == "" ) { return NULL; }
	
	pref = pref_GetPreferencesForCalendarModule( id );
	if ( ! pref ) { return NULL; }
	
	toReturn = new BMessage( kPref_CalendarModulePreferences );
	if ( !toReturn ) { return NULL; }
	
	if ( pref->Archive( toReturn, true ) != B_OK ) {
		delete toReturn;
		return NULL;
	}
	
	return toReturn;
	
}	// <-- end of function PackPreferencesIntoMessage



/*!	\brief		Return pointer to the original preferences.
 *		\details		As opposed to modified preferences, the original preferences
 *						are hidden. Well, not actually hidden, but not easily accessible.
 *		\param[in]	id		The identifier of the calendar module.
 *		\returns		Pointer to the original CalendarModulePreferences
 *						or NULL in case of any error.
 */
static
CalendarModulePreferences*		GetOriginalPreferencesForCalendarModule( const BString& id )
{
	// Sanity check
	if ( id == "" ) { return NULL; }
	
	CalendarModulePreferences* toReturn = NULL, *inTest = NULL;
		
	for ( int index = 0; index < NUMBER_OF_CALENDAR_MODULES; ++index )
	{
		inTest = pref_CalendarModulePrefs_original[ index ];
		
		if  ( ( inTest ) && 
				( inTest->GetId() == id ) )
		{
			toReturn = inTest;
			break;	
		}		
	}

	return toReturn;	
}	// <-- end of function GetOriginalPreferencesForCalendarModule


/****************************************************************************
 * 		Implementation of class CalendarModulePreferences
 ***************************************************************************/

/*!	\brief				Constructor of the preferences for a calendar module.
 *	\param[in]	id 		Identifier of the calendar module.
 *	\note				Return value
 *						Constructor can't return anything, therefore user
 *						must check the function InitCheck and verify it
 *						returns B_OK.
 *	\sa			CalendarModulePreferences::InitCheck()
 */
CalendarModulePreferences::CalendarModulePreferences( const BString& id )
{
	/*!	\note
	 *	Since we don't have anything useful from previously saved preferences,
	 *	all we can is just set up some defaults and pray user will correct them.
	 */
	this->id = id;
	
	weekendsColorForMenu.set_to( 255, 0, 0, 255 );	// True red
	weekdaysColorForMenu 	= ui_color( B_MENU_ITEM_TEXT_COLOR );
	serviceItemsColorForMenu.set_to( 0, 0, 128, 255 );	// Dark blue
	weekendsColorForViewer.set_to( 255, 0, 0, 255 );	// True red
	weekdaysColorForViewer 	= ui_color( B_MENU_ITEM_TEXT_COLOR );
	serviceItemsColorForViewer.set_to( 0, 0, 128, 255 );	// Dark blue
	
	dateOrder = kDayMonthYear;
	
	correspondingModule = utl_FindCalendarModule( id );
	
	if ( NULL == correspondingModule ) {
		status = B_ERROR;
		return;
	}
	
	/* If the Calendar Module does not know what its weekends are, try to
	 * construct an empty set.
	 */
	weekends = correspondingModule->GetDefaultWeekend();
	if ( !weekends && !( weekends = new BList() ) ) {
		status = B_ERROR;
		return;
	}
	
	ucFirstDayOfWeek = correspondingModule->GetDefaultStartingDayOfWeek();
	
	status = B_OK;
}	// <-- end of default constructor.


/*! \brief		Destructor
 *	\details	Note the deletion of the BList* weekends. Usually the deletion
 *				of BList should be performed with caution, since the allocated
 *				areas of memory, pointers to which were stored in the BList, 
 *				would be lost, and the memory will be leaked. However, this is
 *				not the case here: "weekends" does not store pointers.
 */
CalendarModulePreferences::~CalendarModulePreferences()
{
	correspondingModule = NULL;	
	delete weekends;	
}	// <-- end of destructor for CalendarModulePreferences


/*!	\brief				Constructor from a message.
 *	\param[in]	id			Identifier of the Calendar Module
 *	\param[in]	archive		The message which should be parsed.
 *	\note				Return value
 *						The user is required to check the output of InitCheck()
 *						function after the constructor returns.
 */
CalendarModulePreferences::CalendarModulePreferences( const BString& id, BMessage* archive )
{
	uint32	tempUint32;
	int8		tempInt8;
	this->status = B_OK;
	BString identifier;	
	
	// Get the ID of the CalendarModule
	if ( B_OK != archive->FindString( "Identifier", &identifier ) ||
	     identifier != id )
	{
		// We can't continue without the ID.
		utl_Deb = new DebuggerPrintout( "Can't continue without the ID." );
		status = B_ERROR;
		return;
	}		
	
	correspondingModule = utl_FindCalendarModule( id );
	if ( !correspondingModule )
	{	// Can't continue without the module either
		utl_Deb = new DebuggerPrintout( "Can't continue without the module." );
		status = B_ERROR;
		return;
	}
	
	this->id = id;	// Identifier
	
	/* For all of the rest of the preferences, we may use default values
	 *	if the values from the message aren't read good.
	 */
	
	// Get the First Day of Week
	if ( B_OK != archive->FindInt8( "FirstDayOfWeek", ( int8* )&ucFirstDayOfWeek ) )
	{
		ucFirstDayOfWeek = correspondingModule->GetDefaultStartingDayOfWeek();
	}
	
	// Get the colors
	if ( B_OK != archive->FindInt32( "WeekdaysColorForMenu", ( int32* )&tempUint32 ) )
	{
		weekdaysColorForMenu = ui_color( B_MENU_ITEM_TEXT_COLOR );
	} else {
		weekdaysColorForMenu = RepresentUint32AsColor( tempUint32 );
	}
	
	if ( B_OK != archive->FindInt32( "WeekendsColorForMenu", ( int32* )&tempUint32 ) )
	{
		weekendsColorForMenu.set_to( 255, 0, 0, 255 );
	} else {
		weekendsColorForMenu = RepresentUint32AsColor( tempUint32 );
	}
	
	if ( B_OK != archive->FindInt32( "ServiceItemsColorForMenu", ( int32* )&tempUint32 ) )
	{
		serviceItemsColorForMenu.set_to( 0, 0, 128, 255 );
	} else {
		serviceItemsColorForMenu = RepresentUint32AsColor( tempUint32 );
	}
	
	if ( B_OK != archive->FindInt32( "WeekdaysColorForViewer", ( int32* )&tempUint32 ) )
	{
		weekdaysColorForViewer = ui_color( B_MENU_ITEM_TEXT_COLOR );
	} else {
		weekdaysColorForViewer = RepresentUint32AsColor( tempUint32 );
	}
	
	if ( B_OK != archive->FindInt32( "WeekendsColorForViewer", ( int32* )&tempUint32 ) )
	{
		weekendsColorForViewer.set_to( 255, 0, 0, 255 );
	} else {
		weekendsColorForViewer = RepresentUint32AsColor( tempUint32 );
	}
	
	if ( B_OK != archive->FindInt32( "ServiceItemsColorForViewer", ( int32* )&tempUint32 ) )
	{
		serviceItemsColorForViewer.set_to( 0, 0, 128, 255 );
	} else {
		serviceItemsColorForViewer = RepresentUint32AsColor( tempUint32 );
	}
	
	// Get the day-month-year order
	if ( B_OK != archive->FindInt8( "DayMonthYearOrder", ( int8* )&tempInt8 ) )
	{
		dateOrder = kDayMonthYear;
	} else {
		dateOrder = ( DmyOrder )tempInt8;
	}	

	
	// Obtain the weekends
	unsigned char index = 0, limit;
	if ( B_OK != archive->FindInt8( "NumberOfWeekends", ( int8* )&limit ) )
	{
		limit = correspondingModule->GetDaysInWeek();
	}
	this->weekends = new BList( limit );
	if ( ! this->weekends )
	{
		/* Panic! */
		status = B_NO_MEMORY;
		return;
	}
	while ( index < limit )
	{
		if ( B_OK != archive->FindInt32( "Weekend", index, ( int32* )&tempUint32 ) ) {
			break;
		} else {
			( this->weekends )->AddItem( ( void* ) tempUint32 );
		}		
		++index;		
	}	// <-- end of "while ( looping on all possible week days )"
	
	if ( index == 0 )	// No weekends stored in the message - revert to defaults
	{
		// Try to get the data from the CalendarModule
		BList* tempList;
		tempList = correspondingModule->GetDefaultWeekend();
		if ( tempList ) {	// Got the data successfully
			delete this->weekends;
			this->weekends = tempList;
		} else {
			status = B_ERROR;
			return;
		}
	}

	return;
}	// <-- end of function CalendarModulePreferences::Instantiate



/*!	\brief			Copy constructor.
 */
CalendarModulePreferences::CalendarModulePreferences( const CalendarModulePreferences& other )
{
	this->id = other.id;
	this->correspondingModule = other.correspondingModule;
	this->ucFirstDayOfWeek = other.ucFirstDayOfWeek;
	this->weekdaysColorForViewer = other.weekdaysColorForViewer;
	this->weekdaysColorForMenu = other.weekdaysColorForMenu;
	this->weekendsColorForViewer = other.weekendsColorForViewer;
	this->weekendsColorForMenu = other.weekendsColorForMenu;
	this->serviceItemsColorForViewer = other.serviceItemsColorForViewer;
	this->serviceItemsColorForMenu = other.serviceItemsColorForMenu;
	this->dateOrder = other.dateOrder;
	
	int i = 0, limit = other.weekends->CountItems();
	weekends = new BList( limit );
	for ( ; i < limit; ++i ) {
		this->weekends->AddItem( other.weekends->ItemAt( i ) );
	}
	
}	// <-- end of copy constructor



/*!	\brief		Assignment operator.
 */
CalendarModulePreferences CalendarModulePreferences::operator= ( const CalendarModulePreferences& other )
{
	this->id = other.id;
	this->correspondingModule = other.correspondingModule;
	this->ucFirstDayOfWeek = other.ucFirstDayOfWeek;
	this->weekdaysColorForViewer = other.weekdaysColorForViewer;
	this->weekdaysColorForMenu = other.weekdaysColorForMenu;
	this->weekendsColorForViewer = other.weekendsColorForViewer;
	this->weekendsColorForMenu = other.weekendsColorForMenu;
	this->serviceItemsColorForViewer = other.serviceItemsColorForViewer;
	this->serviceItemsColorForMenu = other.serviceItemsColorForMenu;
	this->dateOrder = other.dateOrder;
	
	if ( weekends )
		delete weekends;
	
	int i = 0, limit = other.weekends->CountItems();
	weekends = new BList( limit );
	for ( ; i < limit; ++i ) {
		this->weekends->AddItem( other.weekends->ItemAt( i ) );
	}

	return *this;
}	// <-- end of assignment operator.



/*!	\brief			Adds a day (specified by parameter "day") to list of weekends.
 *	\param[in]	day		The day to be added as a weekend day.
 *	\details		If the day is already in the list, this function does nothing.
 *					Else, it adds the day to the end of list.
 */
void 	CalendarModulePreferences::AddToWeekends( uint32 day )
{
	int index, limit = weekends->CountItems();
	for ( index = 0; index < limit; ++index )
	{
		if ( day == ( uint32 )weekends->ItemAt( index ) )
		{
			return;		// This day is already in the list of weekends
		}	
	}
	
	this->weekends->AddItem( (void*)day );
}	// <-- end of function CalendarModulePreferences::AddToWeekends



/*!	\brief			Removes a day from the list of weekends.
 *	\param[in]	day		The day to be removed.
 *	\details		If the day is not in the list, does nothing.
 */
void	CalendarModulePreferences::RemoveFromWeekends( uint32 day )
{
	int index, limit = this->weekends->CountItems();
	
	for ( index = 0; index < limit; ++index )
	{
		if ( day == ( uint32 )weekends->ItemAt( index ) )
		{
			weekends->RemoveItem( index );
			return;
		}
	}
	return;		// The day was not found - nothing to do.

}	// <-- end of function CalendarModulePreferences::RemoveDayFromWeekends



/*!	\brief			Returns a new list with Calendar Module's weekends.
 *	\details		The ownership on the return value is caller's. It's the
 *					caller's task to delete the list when it's not needed.
 */
BList*	CalendarModulePreferences::GetWeekends() const
{
	/* Here the BList' copy constructor is good enough. It thinks the items
	 *	inside of "this->weekends" are pointers, and does not copy the data.
	 *	But this exactly suits our needs - we do need only these "pointers".
	 */
	int i, limit = this->weekends->CountItems();
	BList* toReturn = new BList( limit );
	for ( i = 0; i < limit; ++i ) {
		toReturn->AddItem( (void*) (this->weekends->ItemAt( i ) ) );
	} 
	return toReturn;
}	// <-- end of function CalendarModulePreferences::GetWeekends



/*!	\brief		Returns color of weekends
 *		\returns		Color associated with the input
 *		\param[in]	viewer		If "true", looking at settings for Event Viewer, else for controls.
 */
rgb_color CalendarModulePreferences::GetWeekendsColor( bool viewer ) const
{
	if ( viewer ) { 
		return weekendsColorForViewer;
	} else { 
		return weekendsColorForMenu;
	}
}



/*!	\brief		Returns color of weekdays
 *		\returns		Color associated with the input
 *		\param[in]	viewer		If "true", looking at settings for Event Viewer, else for controls.
 */
rgb_color CalendarModulePreferences::GetWeekdaysColor( bool viewer ) const
{
	if ( viewer ) { 
		return weekdaysColorForViewer;
	} else { 
		return weekdaysColorForMenu;
	}
}



/*!	\brief		Returns color of service items
 *		\returns		Color associated with the input
 *		\param[in]	viewer		If "true", looking at settings for Event Viewer, else for controls.
 */
rgb_color CalendarModulePreferences::GetServiceItemsColor( bool viewer ) const
{
	if ( viewer ) {
		return serviceItemsColorForViewer;
	} else {
		return serviceItemsColorForMenu;
	}
}



/*!	\brief		Update color for displaying weekdays.
 *		\param[in]	color			The new value
 *		\param[in]	viewer		If "true", updating settings for Event Viewer, else for controls.
 */
void		CalendarModulePreferences::SetWeekdaysColor( rgb_color color, bool viewer )
{
	if ( viewer ) { 
		weekdaysColorForViewer = color;
	} else {
		weekdaysColorForMenu = color;
	}
}	// <-- end of function CalendarModulePreferences::SetColor


/*!	\brief		Update color for displaying weekends.
 *		\param[in]	color			The new value
 *		\param[in]	viewer		If "true", updating settings for Event Viewer, else for controls.
 */
void		CalendarModulePreferences::SetWeekendsColor( rgb_color color, bool viewer )
{
	if ( viewer ) {
		weekendsColorForViewer = color;
	} else {
		weekendsColorForMenu = color;
	}
}	// <-- end of function CalendarModulePreferences::SetColor



/*!	\brief		Update color for displaying service items
 *		\param[in]	color			The new value
 *		\param[in]	viewer		If "true", updating settings for Event Viewer, else for controls.
 */
void		CalendarModulePreferences::SetServiceItemsColor( rgb_color color, bool viewer )
{
	if ( viewer ) { 
		serviceItemsColorForViewer = color;
	} else { 
		serviceItemsColorForMenu = color;
	}
}	// <-- end of function CalendarModulePreferences::SetColor




/*!	\brief			Archiving function
 *	\details		The caller provides a BMessage, this function packs
 *					the object into BMessage.
 *	\param[in]	in		The message to be packed into. It's also the output.
 *	\param[in]	deep	Pack also included classes. Irrelevant in this case.
 */
status_t	CalendarModulePreferences::Archive( BMessage* in, bool deep )
{
	status_t toReturn;
	unsigned char index, limit;
	
	if ( ! in ) { return B_ERROR; }
	
	toReturn = in->AddString( "Identifier", this->id );
	if ( toReturn != B_OK ) { return toReturn; }
	
	toReturn = in->AddInt8( "FirstDayOfWeek", this->ucFirstDayOfWeek );
	if ( toReturn != B_OK ) { return toReturn; }
	
	toReturn = in->AddInt8( "NumberOfWeekends",
							( limit = ( unsigned int8 )( this->weekends->CountItems() ) ) );
	if ( toReturn != B_OK ) { return toReturn; }
	
	for ( index = 0; index < limit; ++index )
	{
		toReturn = in->AddInt32( "Weekend", ( int32 )( weekends->ItemAt( index ) ) );
		if ( toReturn != B_OK ) { return toReturn; }
	}	// <-- end of "for ( passing on all weekends )"
	
	toReturn = in->AddInt32( "WeekendsColorForMenu", RepresentColorAsUint32( weekendsColorForMenu ) );
	if ( toReturn != B_OK ) { return toReturn; }
	toReturn = in->AddInt32( "WeekdaysColorForMenu", RepresentColorAsUint32( weekdaysColorForMenu ) );
	if ( toReturn != B_OK ) { return toReturn; }
	toReturn = in->AddInt32( "ServiceItemsColorForMenu", RepresentColorAsUint32( serviceItemsColorForMenu ) );
	if ( toReturn != B_OK ) { return toReturn; }
	toReturn = in->AddInt32( "WeekendsColorForViewer", RepresentColorAsUint32( weekendsColorForViewer ) );
	if ( toReturn != B_OK ) { return toReturn; }
	toReturn = in->AddInt32( "WeekdaysColorForViewer", RepresentColorAsUint32( weekdaysColorForViewer ) );
	if ( toReturn != B_OK ) { return toReturn; }
	toReturn = in->AddInt32( "ServiceItemsColorForViewer", RepresentColorAsUint32( serviceItemsColorForViewer ) );
	if ( toReturn != B_OK ) { return toReturn; }
	
	toReturn = in->AddInt8( "DayMonthYearOrder", ( int8 )dateOrder );
	if ( toReturn != B_OK ) { return toReturn; }
	
	return B_OK;
}	// <-- end of function CalendarModulePreferences::Archive



/*!	\brief		Comparison operator.
 */
bool	CalendarModulePreferences::operator== ( const CalendarModulePreferences& other ) const
{
	
	if ( ( this->id 					!= other.id ) 											||
		 ( this->ucFirstDayOfWeek 		!= other.ucFirstDayOfWeek ) 					||
		 ( this->weekendsColorForMenu	!= other.weekendsColorForMenu ) 				||
		 ( this->weekendsColorForViewer	!= other.weekendsColorForViewer )		||
		 ( this->weekdaysColorForMenu	!= other.weekdaysColorForMenu )				||
		 ( this->weekdaysColorForViewer != other.weekdaysColorForViewer ) 		||
		 ( this->serviceItemsColorForMenu	!= other.serviceItemsColorForMenu ) ||
		 ( this->serviceItemsColorForViewer != other.serviceItemsColorForViewer ) ||
		 ( this->dateOrder 			!= other.dateOrder ) )
	{
		return false;
	}
	
	int index, limitThis = this->weekends->CountItems(), limitOther = other.weekends->CountItems();
	if ( limitThis != limitOther )
		return false;
		
	for ( index = 0; index < limitThis; ++index )
	{
		if ( ! ( other.weekends->HasItem( this->weekends->ItemAt( index ) ) ) )
			return false;
	}
	
	return true;
	
}	// <-- end of comparison operator.

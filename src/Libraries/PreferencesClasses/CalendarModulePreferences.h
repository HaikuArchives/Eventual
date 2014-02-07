/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CALENDAR_MODULE_PREFERENCES_H_
#define _CALENDAR_MODULE_PREFERENCES_H_

#include <GraphicsDefs.h>
#include <List.h>
#include <Message.h>
#include <String.h>
#include <SupportDefs.h>

#include "CalendarModule.h"
#include "GregorianCalendarModule.h"
#include "Utilities.h"


class CalendarModulePreferences;


/*------------------------------------------------------------------
 *			Message constants
 *-----------------------------------------------------------------*/
 
const uint32	kPref_CalendarModulePreferences		= 'CMPF';


/*------------------------------------------------------------------
 *			Global variables
 *-----------------------------------------------------------------*/

extern	CalendarModulePreferences*	pref_CalendarModulePrefs_modified[ NUMBER_OF_CALENDAR_MODULES ];

/*------------------------------------------------------------------
 *			Global functions
 *-----------------------------------------------------------------*/

	/*	This function loads the preferences from the message. */
void			pref_PopulateCalendarModulePreferences( BMessage* in = NULL );

	/* Fetch pointer to preferences of some calendar module. */
CalendarModulePreferences*		pref_GetPreferencesForCalendarModule( const BString& id );

	/* Save all modified calendar module preferences into a message. */
status_t		pref_SaveCalendarModulePreferences( BMessage* message );

	/* Delete all preferences and free memory. */
void			pref_DeleteCalendarModulePreferences( void );


// Fun with x-macros
#define	DMY_ORDER_NUMBER_OF_ELEMENTS	3
#define	DMY_ORDER	\
	PARSER(	kDayMonthYear,	"Day Month Year" )	\
	PARSER(	kMonthDayYear,	"Month Day Year" )	\
	PARSER(	kYearMonthDay, "Year Month Day" )
	

#ifdef	PARSER
#undef	PARSER
#endif	// PARSER

#define	PARSER( value, name )	value,
enum	DmyOrder	{
	DMY_ORDER
};
#undef PARSER

extern char* DmyOrderNames[];


/*!	\brief	Defines the order of the elements in the control string.
 *
typedef enum DMY_ORDER {
	kDayMonthYear = 0,		//!< Day, then Month, then Year
	kMonthDayYear,				//!< Month, then Day, then Year
	kYearMonthDay				//!< Year, then Month, then Day
} DmyOrder;
 */


/*------------------------------------------------------------------
 * 		Preferences of an individual CalendarModule
 *-----------------------------------------------------------------*/

/*! 	\brief		These preferences define every calendar module.
 * 	\details	There's an instance of this class for every
 *				calendar module in system. It defines weekends, colors
 *				and starting day in week.
 *	\note
 *				Note that length of the week is not configurable: 
 *				weekends can be different in different countries, 
 *				but Gregorian Calendar defines the week as 7 days
 *				worldwide.
 * \note		Consequent note on Arabic calendars
 *				Consequently, there probably will be a problem with
 *				some Arabic calendars, where number of days in month
 *				differs in different countries. However, even they
 *				don't define week length differently.
 */
class CalendarModulePreferences
{
protected:
	BList* 	weekends;	//!< List of the weekends for this module
	unsigned char	ucFirstDayOfWeek;	//!< Duh
	
	//! \brief	Color in which weekends are displayed in the CalendarControl. 
	rgb_color	weekendsColorForMenu;
	
	//! \brief	Color in which weekdays are displayed in the CalendarControl.
	rgb_color	weekdaysColorForMenu;	
	
	//!	\brief	Color in which service items are displayed in the CalendarControl.
	rgb_color	serviceItemsColorForMenu;
	
	//!	\brief	Color in which weekends are displayed in the EventViewer.
	rgb_color	weekendsColorForViewer;
	
	//!	\brief	Color in which weekdays are displayed in the EventViewer.
	rgb_color	weekdaysColorForViewer;
	
	//!	\brief	Color in which service items are displayed in the EventViewer.
	rgb_color	serviceItemsColorForViewer;
	
	BString id;		//!< ID of the relevant calendar module

	//!	\brief	Defines how the data is represented by this module
	DmyOrder		dateOrder;
	
	CalendarModule*		correspondingModule;
	
	status_t	status;

public:
	CalendarModulePreferences( const BString& id );
	CalendarModulePreferences( const BString& id, BMessage* archive );
	CalendarModulePreferences( const CalendarModulePreferences& other );
	virtual ~CalendarModulePreferences();
	
	virtual status_t Archive( BMessage* archive, bool deep = true );
	
	inline virtual CalendarModule* GetCorrespondingModule( void ) {
		return correspondingModule;
	}
	
	inline virtual status_t InitCheck( void ) {
		return status;
	}
	
	inline virtual BString GetId( void ) const { return id; }
	
	virtual void 	AddToWeekends (uint32 day);
	virtual void	RemoveFromWeekends	( uint32 day );
	virtual BList* 		GetWeekends (void) const;
	inline virtual int	GetNumberOfWeekends( void ) const { return weekends->CountItems(); }

	inline virtual void 	SetFirstDayOfWeek(uint32 day) {
		ucFirstDayOfWeek = day;
	}
	inline virtual uint32	GetFirstDayOfWeek( void ) const {
		return ucFirstDayOfWeek;
	}
	
	virtual void 	SetWeekendsColor( rgb_color	color, bool viewer = true );
	virtual rgb_color GetWeekendsColor( bool viewer = true ) const;
	virtual void 	SetWeekdaysColor( rgb_color	color, bool viewer = true );
	virtual rgb_color GetWeekdaysColor( bool viewer = true ) const;
	virtual void 	SetServiceItemsColor( rgb_color	color, bool viewer = true );
	virtual rgb_color GetServiceItemsColor( bool viewer = true ) const;
	
	virtual DmyOrder	GetDayMonthYearOrder( void ) const { return dateOrder; }
	virtual void	SetDayMonthYearOrder( DmyOrder in ) { dateOrder = in; }
	
	virtual CalendarModulePreferences operator= (const CalendarModulePreferences& other);
	
	virtual bool operator== (const CalendarModulePreferences& other ) const;
	inline virtual bool operator!= ( const CalendarModulePreferences& in ) const {
		return (! ( *this == in ) );
	}
	
};	// <-- end of class CalendarModulePreferences




#endif // _CALENDAR_MODULE_PREFERENCES_H_

/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CALENDAR_MODULE_PREFERENCES_VIEW_H_
#define _CALENDAR_MODULE_PREFERENCES_VIEW_H_



#include <Archivable.h>
#include <Box.h>
#include <CheckBox.h>
#include <List.h>
#include <Message.h>
#include <GraphicsDefs.h>

#include <String.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <SupportDefs.h>

#include "CalendarModule.h"
#include "CategoryItem.h"	// For ColorSelectorWindow
#include "Utilities.h"

/*------------------------------------------------------------------
 * 		Constants of the messages
 *-----------------------------------------------------------------*/
const uint32	kCalendarModuleChosen = 'CALM';
const uint32	kCalendarModuleWeekendDaySelected = 'WKDS';
const uint32	kCalendarModuleFirstDayOfWeekSelected = 'FDOW';
const uint32	kCalendarModuleDateOrderSelected = 'DOrS';

/*----------------------------------------------------------------------------
 *		Calendar module preferences view
 *--------------------------------------------------------------------------*/
class CalendarModulePreferencesView
	:
	public BView
{
	protected:
		
		BMenuField* calendarModuleSelector;
		BPopUpMenu* calendarModules;
		
		virtual BPopUpMenu*	PopulateModulesMenu();
		
		virtual BBox*	CreateWeekendSelectionBox( BRect frame, const BString &id );
		virtual BMenuField*	CreateWeekStartDayChooser( BRect frame, const BString &id );
		virtual BMenuField* CreateDayMonthYearOrderChooser( BRect frame, const BString& id );
		virtual BBox*	BuildColorSelectors( BRect frame, const BString& id );
		
		virtual void	ClearOldInterface( void );
		virtual void	BuildInterfaceForModule( const BString& id );
		
		virtual void	UpdateTargetting( void );
		
	public:
		CalendarModulePreferencesView( BRect frame );
		virtual ~CalendarModulePreferencesView();
			
		virtual void MessageReceived( BMessage *in );
		virtual void AttachedToWindow();
};	// <-- end of class CalendarModulePreferencesView


#endif //  _PREFERENCES_PREFLET_H_

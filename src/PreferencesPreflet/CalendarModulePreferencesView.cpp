/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "CalendarModulePreferencesView.h"
#include "CalendarModulePreferences.h"
#include "CategoryItem.h"
#include "TimeRepresentation.h"
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
 * 		Implementation of class CalendarModulePreferencesView
 ***************************************************************************/

/*!	
 *	\brief			Default constructor
 *	\param[in]	frame	The rectangle enclosing the view
 *	\param[in]	name	Name of the view. Will be passed to BView's constructor.
 *
 */
CalendarModulePreferencesView::CalendarModulePreferencesView( BRect frame )
	:
	BView( BRect( frame.left, frame.top, frame.right, frame.bottom-10 ), 
		 "Calendar Module Preferences",
		 B_FOLLOW_LEFT | B_FOLLOW_TOP,
		 B_NAVIGABLE | B_WILL_DRAW | B_FRAME_EVENTS )
{
	BRect tempFrame = this->Bounds();	// Got the boundaries
	
	this->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	tempFrame.InsetBySelf( 5, 5 );
	tempFrame.bottom -= 10;
	
	/* Add the chooser for the calendar modules */
	BGroupLayout* groupLayout = new BGroupLayout( B_VERTICAL );
	if ( !groupLayout ) {
		/* Panic! */
		exit( 1 );
	}
	groupLayout->SetSpacing( 2 );
	this->SetLayout( groupLayout );
	
	// Create the menu with all supported calendar modules
	calendarModules = PopulateModulesMenu();
	if ( ! calendarModules ) {
		/* Panic! */
		exit ( 1 );
	}
	
	calendarModuleSelector = new BMenuField( BRect( 0, 0, this->Bounds().Width() - 10, 1 ),
											 "Calendar Modules selector",
											 "Calendar module:",
											 calendarModules,
											 B_FOLLOW_H_CENTER | B_FOLLOW_TOP );
	if ( !calendarModuleSelector ) {
		/* Panic! */
		exit ( 1 );
	}
	
	calendarModuleSelector->ResizeToPreferred();
	
	// Add the menu with all calendar modules to the layout
	BLayoutItem* layoutItem = groupLayout->AddView( 0, calendarModuleSelector, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
	}
	
	// Relayout
	groupLayout->Relayout();
}	// <-- end of constructor for CalendarModulePreferencesView



/*!	\brief			Destructor.
 */
CalendarModulePreferencesView::~CalendarModulePreferencesView()
{
	if ( calendarModuleSelector ) {
		calendarModuleSelector->RemoveSelf();
		delete calendarModuleSelector;
		calendarModuleSelector = NULL;
	}
	
	ClearOldInterface();
	
}	// <-- end of destructor for CalendarModulePreferencesView


/*!	\brief			Clears interface of previously selected Calendar Module prefs
 *	\details		Used in destructor and in move from one Calendar Module to another.
 */
void 			CalendarModulePreferencesView::ClearOldInterface()
{
	BGroupLayout* layout = ( BGroupLayout* )( BView::GetLayout() );
	BBox*		 tempBBox = NULL;
	BMenuField*	 tempMenuField = NULL;

	// Clean up old interface items if they existed
	if ( ( tempBBox = ( BBox* )this->FindView( "Weekend selector" ) ) != NULL )
	{
		layout->RemoveView( tempBBox );
		tempBBox->RemoveSelf();
		delete tempBBox;
	}
	
	if ( ( tempBBox = ( BBox* )this->FindView( "Color selector" ) ) != NULL )
	{
		layout->RemoveView( tempBBox );
		tempBBox->RemoveSelf();
		delete tempBBox;
	}
	
	if ( ( tempMenuField = ( BMenuField* )this->FindView( "First day of week chooser" ) ) != NULL )
	{
		layout->RemoveView( tempMenuField );
		tempMenuField->RemoveSelf();
		delete tempMenuField;
	}
	
	if ( ( tempMenuField = ( BMenuField* )this->FindView( "Day-Month-Year order chooser" ) ) != NULL )
	{
		layout->RemoveView( tempMenuField );
		tempMenuField->RemoveSelf();
		delete tempMenuField;
	}
	
}	// <-- end of function CalendarModulePreferencesView::ClearOldInterface()


/*!	
 *	\brief			Create a menu to select the module user is setting up.
 *	\note			This is an internal function.
 */
BPopUpMenu*		CalendarModulePreferencesView::PopulateModulesMenu( void )
{
	BPopUpMenu* toReturn = NULL;
	BMenuItem* 	toAdd = NULL;
	BMessage*  	toSend = NULL;
	BString		moduleId;	// Note: this is not a pointer!
	CalendarModule* currentCalModule = NULL;
	
	toReturn = new BPopUpMenu("Calendar Modules");
	if ( ! toReturn )
	{ 
		/* Panic! */
		exit (1);
	}
	
	// Looping on all calendar modules available in the system.
	for ( uint i=0; i < NUMBER_OF_CALENDAR_MODULES; ++i )
	{
		// Obtain the next calendar module
		currentCalModule = (CalendarModule *)global_ListOfCalendarModules.ItemAt( i );
		if ( !currentCalModule )
		{
			continue;	// Skipping unavailable ones
		}
		
		// Create the message
		toSend = new BMessage( kCalendarModuleChosen );
		if ( !toSend )
		{
			/* Panic! */
			exit(1);
		}
		
		// Add the ID of the module to the message
		moduleId = currentCalModule->Identify();
		toSend->AddString( "Module ID", moduleId );
		
		// Create the menu item
		toAdd = new BMenuItem( moduleId.String(), toSend );
		if ( ! toAdd )
		{
			/* Panic! */
			exit(1);
		}
		toAdd->SetTarget( this );
		
		// Add the newly created item to the menu
		toReturn->AddItem( toAdd );
		
		// Gregorian calendar module is the default one
		if ( moduleId == "Gregorian" )
		{
			toAdd->SetMarked( true );
		}
		
	}	/* <-- end of "for ( every calendar module )" */
	
	// At least something should be marked.
	if ( ( ! toReturn->FindMarked() ) && ( toReturn->CountItems() > 0 ) )
	{
		toReturn->ItemAt( 0 )->SetMarked( true );	
	}
	
	return toReturn;
}	// <-- end of function "CalendarModulePreferencesView::PopulateModulesMenu"



/*!	
 *	\brief			Create box for selection of the weekend days
 *	\note			Additionally, select the color for weekends and weekdays
 *	\param[in]	frame	Enclosing rectangle.
 *	\param[in]	id		Reference to name of the selected Calendar module.
 *	\returns		Pointer to all-set-up BBox. Or NULL in case of error.
 */
BBox*	CalendarModulePreferencesView::CreateWeekendSelectionBox( BRect frame,
															  const BString &id )
{
	/*!	\par	Notes on implementation:
	 *			It's not all that straightforward - to create this selection box.
	 *			The problem is that number of days in week is dependent on the
	 *			Calendar Module, therefore the frame rectangle must be divided
	 *			properly. We should take into account the possibility that there's
	 *			not enough place for all days in the submitted frame.
	 *
	 *	\par	
	 *			The solution will be as follows:
	 *			Let number of days in week be N. I create two columns and 
	 *			several rows (the number depends on N). Days in week will be
	 *			proceeded in the order <em>as Calendar Module supplies them</em>.
	 *			The days occupy both columns, and are located in rows
	 *			[0, (ceiling of (N/2)) ). Days returned from CalendarModule are
	 *			placed as follows: days from 0 to (ceiling of (N/2)-1) in the left
	 *			column, days from (ceiling of (N/2)-1) to (N-1) in right column.
	 *
	 *	\par	
	 *			There will be an empty cell in the right column, if number
	 *			of days in week is odd, (which is usually the case).
	 */
	frame.InsetBySelf( 5, 0 );
	BMessage* 	toSend = NULL;
	BCheckBox* 	dayCheckBox = NULL;
	BString		tempString;
	BLayoutItem*	layoutItem = NULL;
	CalendarModulePreferences* prefs = NULL;
	CalendarModule*	calModule = NULL;
	int height = 0;		//!< this is used to resize the BBox to proper size
	
	calModule = utl_FindCalendarModule( id );
	if ( calModule == NULL ) {
		/* Error */
		utl_Deb = new DebuggerPrintout( "Did not succeed to find the calendar module." );
		return NULL;
	}
	// Get the data on days of week
	uint32 daysInWeek = ( uint32 )( calModule->GetDaysInWeek() );
	map<uint32, DoubleNames> weekdayNames = calModule->GetWeekdayNames();

	
	/* Obtain the current Calendar Module preferences */
	prefs = pref_GetPreferencesForCalendarModule( id );
	if ( !prefs ) {
		utl_Deb = new DebuggerPrintout( "Did not succeed to find the preferences for the calendar module." );
		return NULL;
	}
	
	// At this point, "pref" points to current preferences of this calendar module.
	
	BList* weekends = prefs->GetWeekends();		// Get info on currently selected weekends
	
	// Prepare the item to be returned
	BBox*	enclosingBox = new BBox( frame, "Weekend selector" );
	if ( !enclosingBox )
	{
		/* Panic! */
		exit(1);
	}
	enclosingBox->SetLabel( "Select the non-working days (weekends)" );

	// Prepare the layout to be used
	BGridLayout* layout = new BGridLayout();
	if ( !layout)
	{
		/* Panic! */
		exit(1);
	}	
	enclosingBox->SetLayout( layout );
	layout->SetInsets( 10, 15, 10, 5 );
	layout->SetVerticalSpacing( 1 );
	
	/* indexX is 0 for left column or 1 for right column.
	 * indexY is 0 for topmost row, 1 for second from top row, etc.
	 * Max value for indexY = (ceiling of (N/2)).
	 */
	int indexX = 0, indexY = 0;
	
	for (uint32 day = prefs->GetFirstDayOfWeek(), i = 0; i < ( uint32 )daysInWeek; ++i )
	{
		/* Creating the message to be sent */
		toSend = new BMessage( kCalendarModuleWeekendDaySelected );
		if ( !toSend )
		{
			/* Panic! */
			exit(1);
		}
		toSend->AddInt32( "Weekday const", day );
		toSend->AddString( "Calendar module", id );

		/* Set the name of the checkbox.
		 * This is used to identify if the checkbox was checked or unchecked.
		 */
		tempString.SetTo( "Weekday" );
		tempString << day;
		
		/* Creating the checkbox */
		dayCheckBox = new BCheckBox( BRect(0, 0, 1, 1),
									 tempString.String(),
									 weekdayNames[ day ].longName.String(),
									 toSend );
		if (!dayCheckBox)
		{
			// Panic!
			exit(1);
		}
		dayCheckBox->ResizeToPreferred();
		
		// Check if the checkbox should be checked
		if ( weekends->HasItem( ( void* )day ) ) {
			dayCheckBox->SetValue( 1 );
		} else {
			dayCheckBox->SetValue( 0 );
		}
		
		/* Adding the item to the BBox */
		layoutItem = layout->AddView( dayCheckBox, indexX, indexY );
		if ( layoutItem )
		{
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
//			layoutItem->SetExplicitMaxSize( BSize( (int )dayCheckBox->Bounds().Width(), (int )dayCheckBox->Bounds().Height() ) );
			layout->SetMaxRowHeight( indexY, (int )dayCheckBox->Bounds().Height() + 10 );
			layout->SetRowWeight( indexY, 0 );
		}
		
		/* Advancing to the next cell in grid */
		// If arrived to the last item in the first column, advancing to second
		// The +1 is needed because i starts from 0, but days are starting from 1
		if ( ( i + 1 ) == ( unsigned int )( ( daysInWeek + 1 ) / 2 ) )
		{
			indexX = 1;	
			indexY = 0;
		}
		else 	// Staying in the same column, but advancing down
		{
			++indexY;	
		}
		
		/* Advancing to the next day */
		( day == daysInWeek ) ? day = kSunday : ++day;
		
	}	// <-- end of "for (all days in week)"
	
	// Resizing the BBox to the correct size.
	// Note: dayCheckBox is surely not NULL; if it were, we would exit earlier.
	height =(int )( ( dayCheckBox->Bounds().Height() + 5           ) * ( int )( ( daysInWeek + 1 ) / 2 ) - 5 );
	// Formula:	    ( ^height of one checkbox^       + ^separator^ ) * ( ^number of days in column^      ) - ^one unneeded extra separator^
	
	enclosingBox->ResizeTo( enclosingBox->Bounds().Width() - 10, ( int )height );
//	layout->SetExplicitMaxSize( BSize( enclosingBox->Bounds().Width() - 5, ( int )height + 25 ) );
	
	return enclosingBox;
}


/*!	\brief		Create drop-down menu for day-month-year order.
 *	\param[in]	frame	Frame for this control
 *	\param[in]	id		ID of the CalendarControl
 */
BMenuField*		CalendarModulePreferencesView::CreateDayMonthYearOrderChooser( BRect frame, const BString& id )
{
	BMenuItem* 	toAdd = NULL;
	BMessage*	toSend = NULL;
	
	CalendarModulePreferences* prefs = pref_GetPreferencesForCalendarModule( id );
	BPopUpMenu* dmyOrderMenu = new BPopUpMenu( "DmyOrderChooser" );
	if ( ( !prefs ) || ( !dmyOrderMenu ) ) {
		return NULL;
	}
	
	for ( int8 i = 0; i < DMY_ORDER_NUMBER_OF_ELEMENTS; ++i ) {
		toSend = new BMessage( kCalendarModuleDateOrderSelected );
		if ( !toSend ) { return NULL; }
		toSend->AddString( "Calendar module", id );
		toSend->AddInt8( "DayMonthYearOrder", i );
		
		toAdd = new BMenuItem( DmyOrderNames[ i ], toSend );
		if ( !toAdd ) { delete toSend; return NULL; }
		dmyOrderMenu->AddItem( toAdd );
		toAdd->SetTarget( this );
		if ( i == ( int8 )prefs->GetDayMonthYearOrder() ) {
			toAdd->SetMarked( true );
		}
	}
	
	BMenuField* toReturn = new BMenuField( frame,
											"Day-Month-Year order chooser",
											"Select the order for day, month and year:",
											dmyOrderMenu );
	if ( !toReturn ) {
		delete dmyOrderMenu;
		return NULL;
	}
	
	toReturn->ResizeToPreferred();
	
	return toReturn;
}	// <-- end of function CalendarModulePreferencesView::CreateDayMonthYearOrderChooser



/*!	\brief		Create drop-down menu for week start day.
 */
BMenuField*		CalendarModulePreferencesView::CreateWeekStartDayChooser( BRect frame, const BString& id )
{
	BMenuItem* 	toAdd = NULL;
	BMessage*	toSend = NULL;
	
	CalendarModule* calModule = utl_FindCalendarModule( id );
	if ( calModule == NULL ) {
		/* Error */
		return NULL;
	}

	CalendarModulePreferences* calModule_prefs = pref_GetPreferencesForCalendarModule( id );
	if ( !calModule_prefs ) {
		return NULL;
	}
	
	map<uint32, DoubleNames> weekdayNames = calModule->GetWeekdayNames();

	BPopUpMenu* startDayChooserMenu = new BPopUpMenu( "First day of week" );
	if ( !startDayChooserMenu )
	{
		return NULL;
	}
	
	uint32	currentlySelectedStartingDay = calModule_prefs->GetFirstDayOfWeek(),
			daysInWeek = ( uint32 )( calModule->GetDaysInWeek() );
	
	// Pass on all days in week
	for ( uint32 i = kSunday; i <= daysInWeek; ++i ) {
		
		toSend = new BMessage( kCalendarModuleFirstDayOfWeekSelected );
		if ( ! toSend ) { /* Panic! */
			exit( 1 );
		}
		
		toSend->AddInt32( "Day", i );
		toSend->AddString( "Calendar module", id );
		
		toAdd = new BMenuItem( weekdayNames[ i ].longName.String(),
							   toSend );
		if ( !toAdd ) {
			/* Panic! */
			exit( 1 );
		}
		toAdd->SetTarget( this );
		startDayChooserMenu->AddItem( toAdd );
		if ( i == currentlySelectedStartingDay ) {
			toAdd->SetMarked( true );
		}
	}	// <-- end of pass on all days in week
	
	BMenuField* startDayChooser = new BMenuField( frame,
												  "First day of week chooser",
												  "Select first day of week:",
												  startDayChooserMenu );
	
	if ( ! startDayChooser ) {
		/* Panic! */
		delete startDayChooserMenu;
		return NULL;
	}
	
	startDayChooser->ResizeToPreferred();

	return startDayChooser;
	
}	// <-- end of function CalendarModulePreferencesView::CreateWeekStartDayChooser



/*!	\brief		Builds the interface for a selected calendar module.
 *	\param[in]		id		Name of the calendar module to build the interface for.
 *	\details	If the interface for a module already exists, this function
 *				deletes it without affecting the already set preferences.
 */
void	CalendarModulePreferencesView::BuildInterfaceForModule( const BString& id )
{
	BGroupLayout* layout = ( BGroupLayout* )( BView::GetLayout() );
	BLayoutItem* 	layoutItem = NULL;
	BBox*		 tempBBox = NULL;
	
	if ( !this->Window() || this->Window()->Lock() )
	{
		// First, clean up old interface items if they existed
		this->ClearOldInterface();
		
		// Prepare the frame for the BBox for weekends selection
		BRect 		 r = this->Bounds();	
		r.InsetBySelf( 5, 5 );
		r.right -= 10;
		r.bottom -= 10;
		if ( calendarModuleSelector )
			r.top += ( calendarModuleSelector->Bounds().Height() + 10 );
		
		// Build weekend selection
		tempBBox = CreateWeekendSelectionBox( r, id );
		if ( tempBBox )
		{
			// After the call, the BBox is resized to minimal required size.
			layoutItem = layout->AddView( 1, tempBBox, 0 );
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
		}
		else
		{
			utl_Deb = new DebuggerPrintout( "Error - Weekend selection box is NULL." );
		}

		// Start day chooser	
		BMenuField* startDayChooser = CreateWeekStartDayChooser( r, id );
		if ( startDayChooser )
		{
			layoutItem = layout->AddView( 2, startDayChooser, 0 );
			if ( layoutItem )
				layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
		
		// Day-month-year order
		BMenuField* dmyChooser = CreateDayMonthYearOrderChooser( r, id );
		if ( dmyChooser )
		{
			layoutItem = layout->AddView( 3, dmyChooser, 0 );
			if ( layoutItem )
				layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
		
		// Colors selection
		tempBBox = BuildColorSelectors( r, id );
		if ( tempBBox ) {
			layoutItem = layout->AddView( 4, tempBBox, 0 );
		}
		
		
		/* At the end, all children are targetted at current window */
		UpdateTargetting();
		
		if ( this->Window() ) { this->Window()->Unlock(); }
	}	// <-- end of lock-only section
	
	
}	// <-- end of function CalendarModulePreferencesView::BuildInterfaceForModule


/*!	\brief		Updates targets of all controls currently in the view.
 *	\details	BView::AttachToWindow(), among other things, modifies the targets
 *				of controls to point to main looper of the application. This is
 *				not desirable way. This function corrects it.
 */
void		CalendarModulePreferencesView::UpdateTargetting( void )
{
	BCheckBox* tempCheckBox = NULL;
	int i, limit;

	// Updating calendar chooser part
	BMenuItem* menuItem = NULL;
	if ( calendarModules ) {
		limit = calendarModules->CountItems();
		for ( i = 0; i < limit; ++i )
		{
			menuItem = dynamic_cast< BMenuItem* >( calendarModules->ItemAt( i ) );
			if ( menuItem )
				menuItem->SetTarget( this );
		}
	}

	// Update Weekends selector box
	BBox* tempBBox = ( BBox* )this->FindView( "Weekend selector" );
	if ( tempBBox ) {
		limit = tempBBox->CountChildren();
		for ( i = 0; i < limit; ++i )
		{
			tempCheckBox = dynamic_cast< BCheckBox* >( tempBBox->ChildAt( i ) );
			if ( tempCheckBox )
				tempCheckBox->SetTarget( this );
		}
	}
	
	// Update First day of week chooser
	BPopUpMenu* tempMenu = ( BPopUpMenu* )this->FindView( "First day of week" );
	if ( tempMenu )
	{
		limit = tempMenu->CountItems();
		for ( i = 0; i < limit; ++i )
		{
			menuItem = dynamic_cast< BMenuItem* >( tempMenu->ItemAt( i ) );
			if ( menuItem )
				menuItem->SetTarget( this );
		}
	}
	
	// Update day-month-year order chooser
	tempMenu = ( BPopUpMenu* )this->FindView( "DmyOrderChooser" );
	if ( tempMenu )
	{
		limit = tempMenu->CountItems();
		for ( i = 0; i < limit; ++i )
		{
			menuItem = dynamic_cast< BMenuItem* >( tempMenu->ItemAt( i ) );
			if ( menuItem )
				menuItem->SetTarget( this );
		}
	}
	
	// Update the target of Color selector
	CategoryListView* catListView = ( CategoryListView* )this->FindView("Colors list view");
	if ( catListView )
	{
		catListView->SetTarget( this );
	}
	
}	// <-- end of function CalendarModulePreferencesView::UpdateTargetting



/*!	\brief		Add the interface for configuring colors.
 *	\param[in]	frame	The frame of the view.
 *	\param[in]	id		The ID of the interface to configure the colors for.
 */
BBox*		CalendarModulePreferencesView::BuildColorSelectors( BRect frame,
															    const BString& id )
{
	BBox* toReturn = NULL;
	BLayoutItem*	layoutItem = NULL;
	BString sb;
	BRect tempRect = frame;
//	tempRect.InsetBySelf( 10, 10 );
	BMessage* toSend = NULL;
	
	// Access the preferences
	CalendarModulePreferences* prefs = pref_GetPreferencesForCalendarModule( id );
	if ( !prefs ) {
		return NULL;
	}
	
	// Prepare the overall BBox
	toReturn = new BBox( frame, "Color selector" );
	if ( ! toReturn )
		return NULL;
		
	toReturn->SetLabel( "Set up colors" );
	
	// Set up the layout for this BBox
	BGroupLayout* groupLayout = new BGroupLayout( B_VERTICAL );
	if ( !groupLayout ) {
		delete toReturn;
		return NULL;
	}
	groupLayout->SetInsets( 10, 15, 10, 5 );
	groupLayout->SetSpacing( 2 );
	toReturn->SetLayout( groupLayout );
	
	// Label over the CategoryListView for the viewer
	BStringView* viewerString = new BStringView( BRect( 0, 0, 1, 1 ),
											   "Colors Changer Explanation",
											   "Double-click the color to edit it." );
	if ( !viewerString )
	{
		delete toReturn;
		return NULL;
	}
	viewerString->ResizeToPreferred();
	layoutItem = groupLayout->AddView( 0, viewerString, 0 );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );

	toSend = new BMessage( kCategoryInvoked );
	if ( !toSend )
	{
		/* Panic! */
		exit( 1 );
	}
	toSend->AddString( "Calendar module", id );
	CategoryListView* menuColors = new CategoryListView( BRect( 0, 
																0,
																tempRect.Width() + B_V_SCROLL_BAR_WIDTH,
																tempRect.Height() + B_H_SCROLL_BAR_HEIGHT ),
														 "Colors list view" );
	if ( !menuColors ) {
//		menuString->RemoveSelf(); delete menuString;
//		viewerString->RemoveSelf(); delete viewerString;
		delete toReturn; return NULL;
	}
	menuColors->SetInvocationMessage( toSend );
	
	// Fill the CategoryListView for the menu colors
	sb.SetTo( "Color for displaying weekdays in the controls" );
	CategoryListItem* toAdd = new CategoryListItem( prefs->GetWeekdaysColor( false ),
													sb );
	if ( toAdd ) {
		menuColors->AddItem( toAdd );
	}
	
	sb.SetTo( "Color for displaying weekends in the controls" );
	toAdd = new CategoryListItem( prefs->GetWeekendsColor( false ),
		 					  	  sb );
	if ( toAdd ) {
		menuColors->AddItem( toAdd );
	}
	sb.SetTo( "Color for displaying weekdays in Viewer" );
	toAdd = new CategoryListItem( prefs->GetWeekdaysColor( true ),
								  sb );
	if ( toAdd ) {
		menuColors->AddItem( toAdd );
	}
	sb.SetTo( "Color for displaying weekends in Viewer" );
	toAdd = new CategoryListItem( prefs->GetWeekendsColor( true ),
		 					  	  sb );
	if ( toAdd ) {
		menuColors->AddItem( toAdd );
	}
	sb.SetTo( "Color for displaying service items in the controls" );
	toAdd = new CategoryListItem( prefs->GetServiceItemsColor( false ),
		 					  	  sb );
	if ( toAdd ) {
		menuColors->AddItem( toAdd );
	}
	sb.SetTo( "Color for displaying service items in the Viewer" );
	toAdd = new CategoryListItem( prefs->GetServiceItemsColor( true ),
		 					  	  sb );
	if ( toAdd ) {
		menuColors->AddItem( toAdd );
	}
	
	menuColors->ResizeToPreferred();	
//	BSize setSize( menuColors->Bounds().Width(), menuColors->Bounds().Height() );
	BSize setSize( frame.Width(), menuColors->Bounds().Height()+5 );
	
	layoutItem = groupLayout->AddView( 1, menuColors, 0 );
	layoutItem->SetExplicitMaxSize( setSize );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );
	groupLayout->InvalidateLayout();
		
	return toReturn;
}	// <-- end of function CalendarModulePreferecesView::BuildColorSelectors



/*!	\brief		Adds the handler to window's looper
 */
void 		CalendarModulePreferencesView::AttachedToWindow()
{
	BLooper* looper = this->Looper();
	if ( looper && looper->LockLooper() )
	{
		looper->AddHandler( ( BHandler* )this );
		looper->UnlockLooper();	
	}
	
	UpdateTargetting();		// Updates targets of all contents
	
	BView::AttachedToWindow();
	
	// Fill the rest of the interface
	this->Looper()->PostMessage( calendarModules->FindMarked()->Message(), this );
	
}	// <-- end of CalendarModulePreferencesView::AttachedToWindow()


/*!	\brief		Main function of the View.
 *	\details	Receives and parses the messages and updates the preferences if needed.
 *	\param[in]	in		The message to be parsed.
 */
void 		CalendarModulePreferencesView::MessageReceived( BMessage* in )
{
	CalendarModulePreferences* prefs = NULL;
	uint32	tempUint32 = 0;
	int8	tempInt8 = 0;
	int tempInt = 0;
	BString sb;
	BAlert* alert = NULL;
	BCheckBox* cb = NULL;
	BMessage* toSend;
	CalendarModule* calModule = NULL;
	CategoryListView* catListView = NULL;
	CategoryListItem* catListItem = NULL;
	Category receivedFromUpdate( BString("") );
	ColorUpdateWindow*	cuWindow = NULL;
	bool weekend, weekday, viewer, service;
	
	switch ( in->what )
	{
		case kCalendarModuleChosen:
			
			if ( B_OK != in->FindString( "Module ID" , &sb ) )
				return BView::MessageReceived( in );
			BuildInterfaceForModule( sb );
			break;

		case kCalendarModuleFirstDayOfWeekSelected:
			
			if ( ( B_OK != in->FindString( "Calendar module", &sb ) ) 		||
				 ( B_OK != in->FindInt32( "Day", ( int32* )&tempUint32 ) )	||
				 ( ( prefs = pref_GetPreferencesForCalendarModule( sb ) ) == NULL ) )
			{
				// Can't update - don't know what the module is!
				BView::MessageReceived( in );
			}
		
			prefs->SetFirstDayOfWeek( tempUint32 );
			
			// Refresh the view
			BuildInterfaceForModule( sb );
			
			break;
		
		case kCalendarModuleWeekendDaySelected:
		
			if ( ( B_OK != in->FindString( "Calendar module", &sb ) ) 		||
				 ( B_OK != in->FindInt32( "Weekday const", ( int32* )&tempUint32 ) )	||
				 ( ( prefs = pref_GetPreferencesForCalendarModule( sb ) ) == NULL ) )
			{
				// Can't update - don't know what the module is!
				BView::MessageReceived( in );
			}
			calModule = utl_FindCalendarModule( sb );			
			
			sb.SetTo( "Weekday" );
			sb << tempUint32;
			
			if ( B_OK != ( cb = ( BCheckBox* )this->FindView( sb.String() ) ) )
				BView::MessageReceived( in );
			
			if ( cb->Value() == 0 )		// Deselected - removing from weekends
			{
				prefs->RemoveFromWeekends( tempUint32 );
			} else {					// Selected - adding to weekends
				prefs->AddToWeekends( tempUint32 );
			}
			
			// :)
			if ( calModule ) {
				if ( prefs->GetNumberOfWeekends() == calModule->GetDaysInWeek() ) {
					alert = new BAlert( "I envy you!",
										"Wow! Nice week you have! I really envy you!",
										":)",
										NULL,
										NULL,
										B_WIDTH_AS_USUAL,
										B_EVEN_SPACING,
										B_IDEA_ALERT );
					if ( alert )
						alert->Go();
				}
			}
			
			break;
			
		case ( kCategoryInvoked ):
		
			catListView = dynamic_cast< CategoryListView* >( this->FindView( "Colors list view" ) );
			if ( !catListView ) {
				break;
			}
		
			// Modifying currently existing category
			tempInt = catListView->CurrentSelection();
			if ( tempInt < 0 )
			{
				break;
			}
			catListItem = ( CategoryListItem* )catListView->ItemAt( tempInt );
			toSend = new BMessage( kColorSelected );
			if ( toSend ) {
				in->FindString( "Calendar module", &sb );
				toSend->AddString( "Calendar module", sb );
			}
			
			cuWindow = new ColorUpdateWindow( Category( catListItem->GetLabel(), catListItem->GetColor() ),
											 false,		// Name shouldn't be edited
											 "Edit category",
											 ( BHandler* )this,
											 this->Looper(),
											 toSend );
			break;
			
		case ( kCalendarModuleDateOrderSelected ):
			if ( ( B_OK != in->FindString( "Calendar module", &sb ) ) ||
				 ( ( prefs = pref_GetPreferencesForCalendarModule( sb ) ) == NULL ) ||
				 ( B_OK != in->FindInt8( "DayMonthYearOrder", ( int8* )&tempInt8 ) ) )
			{
				return BView::MessageReceived( in );
			}
			
			prefs->SetDayMonthYearOrder( ( DmyOrder )tempInt8 );
			
			break;
		case ( kColorSelected ):	// Intentional fall-through
		case ( kColorReverted ):
			catListView = dynamic_cast< CategoryListView* >( this->FindView( "Colors list view" ) );
			if ( !catListView ) {
				break;
			}
		
			in->FindString( "Calendar module", &sb );
			in->FindString( "New string", &receivedFromUpdate.categoryName );
			in->FindInt32( "New color", ( int32* )&tempUint32 );
			
			prefs = pref_GetPreferencesForCalendarModule( sb );
				 
			if ( prefs == NULL )
			{
				return BView::MessageReceived( in );
			}
			
			receivedFromUpdate.categoryColor = RepresentUint32AsColor( tempUint32 );
			
			// If the received category name is empty, don't change anything.
			if ( receivedFromUpdate.categoryName == "" )
			{
				in->FindString( "Original string", &receivedFromUpdate.categoryName );
			}
			
			catListItem = new CategoryListItem( receivedFromUpdate );
			if ( ! catListItem )
			{
				/* Panic! */
				exit( 1 );	
			}
			catListView->AddItem( catListItem );

			// It's time to update the preferences
			if ( B_ERROR == receivedFromUpdate.categoryName.IFindFirst( "weekend" ) ) {
				weekend = false;
			} else {
				weekend = true;
			}
			if ( B_ERROR == receivedFromUpdate.categoryName.IFindFirst( "weekday" ) ) {
				weekday = false;
			} else {
				weekday = true;
			}
			if ( B_ERROR == receivedFromUpdate.categoryName.IFindFirst( "viewer" ) ) {
				viewer = false;
			} else {
				viewer = true;
			}
			if ( B_ERROR == receivedFromUpdate.categoryName.IFindFirst( "service" ) ) {
				service = false;
			} else {
				service = true;
			}
			
			if ( prefs ) {
				if ( weekend ) {
					prefs->SetWeekendsColor( receivedFromUpdate.categoryColor, viewer );
				} else if ( weekday ) {
					prefs->SetWeekdaysColor( receivedFromUpdate.categoryColor, viewer );
				} else if ( service ) {
					prefs->SetServiceItemsColor( receivedFromUpdate.categoryColor, viewer );
				} else {
					utl_Deb = new DebuggerPrintout( "Didn't find the type of color to update!" );
				}
			}
			else
			{
				utl_Deb = new DebuggerPrintout( "Didn't find the preferences to update!" );
			}
			
			break;
		
		default:
			BView::MessageReceived( in );
		
	};	// <-- end of "switch( the "what" field )"
	
}	// <-- end of CalendarModulePrerefencesView::MessageReceived

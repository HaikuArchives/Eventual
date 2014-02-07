/*!	\file		PreferencesPrefletMainWindow.cpp
 *	\brief		Implementation of main window of the Eventual's preferences.
 *	\details	Based on Skeleton application by Kevin H. Patterson.
 */

//Title: Skeleton Main Window Class
//Platform: BeOS 5
//Version: 2001.12.18
//Description:
//	A class that creates the application's "main" window.
//	This class inherits from BWindow, and runs a message loop for the window.
//Copyright (C) 2001, 2002 Kevin H. Patterson

#include <Button.h>
#include <Directory.h>
#include <Errors.h>
#include <File.h>
#include <FindDirectory.h>
#include <GridLayout.h>
#include <GroupLayout.h>
#include <Message.h>
#include <LayoutItem.h>
#include <Path.h>
#include <StorageDefs.h>
#include <String.h>
#include <TabView.h>

#include <stdlib.h>

#include "CalendarModule.h"
#include "GregorianCalendarModule.h"
#include "CategoryItem.h"
#include "PreferencesPrefletMainWindow.h"
#include "CalendarModulePreferences.h"
#include "EmailPreferences.h"
#include "TimePreferences.h"
#include "Utilities.h"
#include "Preferences.h"
#include "AboutView.h"

/*!	
 *	\brief		Constructor for the window
 *	\note
 *				This constructor positiions itself in the center of the screen.
 */
PreferencesPrefletMainWindow::PreferencesPrefletMainWindow()
	:
	BWindow(
		BRect( 0, 0, 639, 479 ),	// The window will be centered on screen later
		"Eventual Preferences",
		B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE,
		B_ALL_WORKSPACES )
{	
	/*!	\note		Function contents
	 *				This function consists of two parts. First loads the old
	 *				preferences; second sets up the window look.
	 */
	/* Part 1.	Load old preferences. */
	status_t status = B_OK;
	
	GregorianCalendar* gregorianCalMod = new GregorianCalendar();
	global_ListOfCalendarModules.AddItem( gregorianCalMod );
	
	status = pref_PopulateAllPreferences();
	if ( status != B_OK )
	{
		utl_Deb = new DebuggerPrintout( "Did not succeed to read the preferences!" );
	}
	
	utl_RegisterFileType();
	
	
	/* Part 2.  Set up the window. */
	BMessage* toSend = NULL;
	BLayoutItem* layoutItem = NULL;
	BView* background = new BView( BWindow::Bounds(), "Background", B_FOLLOW_ALL, 0 );
	BGroupLayout* backgroundLayout = new BGroupLayout( B_VERTICAL );
	if ( !background || !backgroundLayout )
	{
		/* Panic! */
		exit(1);
	}
	
	// Set background color and prepare placeholder for the main view
	background->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	background->SetLayout( backgroundLayout );
	backgroundLayout->SetInsets( 5, 5, 5, 5 );

	// Another layout for the buttons
	BGridLayout* gridLayout = new BGridLayout();
	if ( !gridLayout )
	{
		/* Panic! */
		exit( 1 );
	}
	gridLayout->SetInsets( 0, 0, 0, 0 );
	
	backgroundLayout->AddItem( 1, gridLayout, 0 );
	gridLayout->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_BOTTOM ) );

	// Adding buttons
	toSend = new BMessage( kSaveAndClose );
	okButton = new BButton( BRect( 0, 0, 1, 1),
				  		     "Ok button",
					   		 "Save and close",
					   		 toSend );
	if ( !okButton || !toSend )
	{
		/* Panic! */
		exit( 1 );
	}
	
	toSend = new BMessage( kJustSave );
	saveButton = new BButton( BRect( 0, 0, 1, 1),
					 	 		 "Save button",
							     "Save without closing",
							     toSend );
	if ( !toSend || !saveButton )
	{
		/* Panic! */
		exit( 1 );
	}
	okButton->ResizeToPreferred();
	saveButton->ResizeToPreferred();
	
	// Lay out all items
	layoutItem = gridLayout->AddView( saveButton, 0, 0 );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_MIDDLE ) );
	
	layoutItem = gridLayout->AddView( okButton, 1, 0 );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );

	gridLayout->SetMaxRowHeight( 0, ( okButton->Bounds() ).Height() + 6 );
	gridLayout->SetExplicitMinSize( BSize( Bounds().Width() - 10, ( okButton->Bounds() ).Height() + 6 ) );
	gridLayout->InvalidateLayout();
	
	/* Constructing the Tab View */
	BRect r = background->Bounds();
	r.InsetBySelf( 5, 5 );
	r.bottom -= ( ( okButton->Bounds() ).Height() + 6 ) ;
	r.bottom -= 10;
	mainView = new BTabView( r, "Main view", B_WIDTH_FROM_LABEL );
	if ( !mainView ) {
		/* Panic! */
		exit( 1 );
	}
	mainView->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	
	/* Constructing the first tab */
	r.InsetBySelf( 5, 5 );
	r.bottom -= ( mainView->TabHeight() + 10 );
	r.right -= 13;
	BTab* tab = new BTab();
	
	catPrefView = new CategoryPreferencesView( r );
	if ( !catPrefView )
	{
		/* Panic! */
		exit( 1 );
	}
	if ( this->LockLooper() )
	{
		AddHandler( catPrefView );
		this->UnlockLooper();	
	}
	mainView->AddTab( catPrefView, tab );
	tab->SetLabel( "Categories" );
	
	/* Constructing the second tab */
	emailPrefView = new EmailPreferencesView( r );
	if ( !emailPrefView ) {
		/* Panic! */
		exit( 1 );
	}
//	tab = new BTab();
//	mainView->AddTab( emailPrefView, tab );
//	tab->SetLabel( "E-mail settings" );

	
	// Constructing the third tab
	calModPrefView = new CalendarModulePreferencesView( r );
	if ( !calModPrefView ) {
		/* Panic! */
		exit( 1 );
	}
	tab = new BTab();
	mainView->AddTab( calModPrefView, tab );
	tab->SetLabel( "Calendars" );
	
	// Constructing the fourth tab
	timePrefView = new TimePreferencesView( r );
	if ( !timePrefView ) {
		/* Panic! */
		exit( 1 );
	}
	tab = new BTab();
	mainView->AddTab( timePrefView, tab );
	tab->SetLabel( "Times" );
	
	// Constructing the fivth tab
	aboutView = new AboutView( r );
	if ( !aboutView ) {
		/* Panic! */
		exit( 1 );
	}
	tab = new BTab();
	mainView->AddTab( aboutView, tab );
	tab->SetLabel( "About" );
	
	
	mainView->Select( 0 );	// Selecting the first tab	
	backgroundLayout->AddView( 0, mainView, 1 );
	backgroundLayout->InvalidateLayout();
	
	BWindow::AddChild( background );
	this->SetDefaultButton( okButton );
	
	this->ResizeTo( 640, 480 );
	this->CenterOnScreen();
}


/*!	
 *	\brief			Window's destructor.
 */
PreferencesPrefletMainWindow::~PreferencesPrefletMainWindow()
{
	
}


/*!	
 *	\brief			Main function of the program
 *	\param[in]	message		The received message.
 */
void PreferencesPrefletMainWindow::MessageReceived(BMessage* message)
{
	status_t status = B_OK;
	
	switch( message->what )
	{
		case kJustSave:		// Intentional fall-through
		case kSaveAndClose:
		
			// Saving the preferences
			status = pref_SaveAllPreferences();
			if ( B_OK != status )
			{
				utl_Deb = new DebuggerPrintout( "Did not succeed to write the preferences!" );	
			}
			
			// Quitting only if specially requested
			if ( message->what == kSaveAndClose )
			{
				this->PostMessage( B_QUIT_REQUESTED );
			}
			
			break;
		default:
			BWindow::MessageReceived( message );
			break;
	}
}	// <-- end of function PreferencesPrefletMainWindow::MessageReceived



/*!	
 *	\brief			Closes the application
 */
bool PreferencesPrefletMainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}	// <-- end of function PreferencesPrefletMainWindow::QuitRequested

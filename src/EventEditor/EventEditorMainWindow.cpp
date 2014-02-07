/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

//Copyright (C) 2001, 2002 Kevin H. Patterson

// Project includes
#include "EventEditorApp.h"
#include "EventEditorMainWindow.h"
#include "Utilities.h"

// OS includes
#include <Button.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <GridLayout.h>
#include <GroupLayout.h>
#include <InterfaceDefs.h>
#include <LayoutItem.h>
#include <Path.h>
#include <TabView.h>

// POSIX includes
#include <stdio.h>



/*--------------------------------------------------------------------
 *					Implementation of class EventFileRefFilter
 *-------------------------------------------------------------------*/

bool EventFileRefFilter::Filter( const entry_ref* ref,
											BNode* node,
											struct stat_beos* st,
											const char* filetype )
{
	BString 		fileType( filetype );
	BDirectory 	testDir( ref );
	BEntry		tempEntry;
	BNode			tempNode;
	char			buffer[ B_MIME_TYPE_LENGTH ];

	// All directories are allowed - else the user won't be able to travel
	if ( testDir.InitCheck() == B_OK ) {
		return true;
	}
	
	// All Event files are allowed
	if ( fileType.IFindFirst( kEventFileMIMEType ) == 0 ) {
		return true;
	}
	
	// Symlinks are traversed and allowed only if they point to audio file
	while ( fileType.IFindFirst( "application/x-vnd.Be-symlink" ) == 0 )
	{
		if ( ( B_OK == tempEntry.SetTo( ref, true ) ) &&	// Find the entry referenced by symlink
		     ( B_OK == tempNode.SetTo( &tempEntry ) ) &&	// Access the attributes (needed to read file type)
		     ( 0 != tempNode.ReadAttr( "BEOS:TYPE", B_STRING_TYPE, 0, buffer, 255 ) ) &&
		     ( NULL != fileType.SetTo( buffer ) ) &&		// This check is really unnecessary
		     ( fileType.IFindFirst( kEventFileMIMEType ) == 0 ) )
	   {
			return true;
	   }
	}
	
	return false;	
}	// <-- end of function EventFileRefFilter::Filter



/*--------------------------------------------------------------------
 *					Implementation of class EventEditorMainWindow
 *-------------------------------------------------------------------*/

/*!	\brief		Constructor for the class.
 *		\details		This constructs the main window of the class and
 *						initializes the starting moment to the input.
 */
EventEditorMainWindow::EventEditorMainWindow( time_t startingMoment )
	:
	BWindow(	BRect( 0, 0, 400, 600 ),
				"Eventual Editor",
				B_TITLED_WINDOW,
				0 ),
	fData( startingMoment ),
	MainView( NULL ),
	genView( NULL ),
	remView( NULL ),
	actView( NULL ),
	noteView( NULL ),
	saveAndClose( NULL )
{
	InitUI();
	fRefFilter = new EventFileRefFilter();
	InitializeFilePanels();
	CenterOnScreen();
}	// <-- end of constructor



/*!	\brief		Creates and initializes the file panels.
 *	
 */
void 	EventEditorMainWindow::InitializeFilePanels( BString path )
{
	BPath eventualDirectoryPath;
	BPath pathToFile;
	bool	initToDefaults = false;
	BDirectory parentDirectory, eventualDirectory;
	status_t	status;

	// Initializing from a submitted path	
	do {
		if ( path != BString("") ) {
			pathToFile.SetTo( path.String() );
			if ( ( pathToFile.InitCheck() != B_OK ) ||
			 	  ( pathToFile.GetParent( &eventualDirectoryPath ) != B_OK ) ||
			 	  ( eventualDirectory.SetTo( eventualDirectoryPath.Path() ) != B_OK ) )
			{
				initToDefaults = true;
				break;	// Go to after "while"
			}
		}
		else
		{
			initToDefaults = true;
		}
	} while ( false );
	
	// Else, initialize to defaults
	if ( initToDefaults )
	{
		find_directory( B_USER_DIRECTORY,
						 &eventualDirectoryPath,
						 true );	// Useless flag - system can't boot without "home"
		parentDirectory.SetTo( eventualDirectoryPath.Path() );
		status = parentDirectory.CreateDirectory( "events", &eventualDirectory );
		if ( status == B_FILE_EXISTS ) {
			eventualDirectory.SetTo( &parentDirectory, "events" );
			status = eventualDirectory.InitCheck();
		}
		if ( status != B_OK )
		{
			global_toReturn = ( uint32 )status;
			be_app->PostMessage( B_QUIT_REQUESTED );
		}
	} // <-- end of setting default directories
	
	
	// At this point, eventualDirectory is set to the directory where files
	// should be stored.
	
	BEntry entry;
	entry_ref ref;
	eventualDirectory.GetEntry( &entry );
	entry.GetRef( &ref );

	// Construct file panels
	fOpenFile = new BFilePanel( B_OPEN_PANEL,
										 new BMessenger( Looper(), this ),
										 &ref,
										 B_FILE_NODE,
										 false,
										 new BMessage( kFileOpenConfirmed ),
										 fRefFilter );

	fSaveFile = new BFilePanel( B_SAVE_PANEL,
										 new BMessenger( Looper(), this ),
										 &ref,
										 B_FILE_NODE,
										 false,
										 new BMessage( kFileSaveConfirmed ),
										 fRefFilter );
	entry.Unset();
	if ( !fOpenFile || !fSaveFile ) {
		global_toReturn = ( uint32 )B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}

}	// <-- end of function EventEditorMainWindow::InitializeFilePanels



/*!	\brief		Constructor for the class from path to file.
 *		\details		This constructs the main window of the class and
 *						loads Event data from a file.
 */
EventEditorMainWindow::EventEditorMainWindow( BString path )
	:
	BWindow(	BRect( 0, 0, 400, 600 ),
				"Event Editor",
				B_TITLED_WINDOW,
				0 ),
	MainView( NULL ),
	genView( NULL ),
	remView( NULL ),
	actView( NULL ),
	noteView( NULL ),
	saveAndClose( NULL )
{
	BEntry entry( path, true );
	entry_ref ref;
	if ( entry.InitCheck() == B_OK && 
		  entry.Exists() &&
		  entry.GetRef( &ref ) == B_OK )
	{
		fData.InitFromFile( ref );
//		if ( Looper()->Lock() ) {
//			fData.Revert();
//			MainView->RemoveSelf();
//			delete MainView;
//			InitUI();
//			Looper()->Unlock();
//		}
		entry.Unset();
	} else {
		printf( "Invalid file.\n" );	
	}
	 
	InitUI();
	fRefFilter = new EventFileRefFilter();
	InitializeFilePanels( path );
	CenterOnScreen();
}




/*!	\brief		Init user interface regardless of the parameter of constructor.
 */
void		EventEditorMainWindow::InitUI() 
{
	ClearUI();
	
	MainView = new BView( BWindow::Bounds(),
								  "Event Editor Main View",
								  B_FOLLOW_ALL,
								  B_WILL_DRAW | B_FRAME_EVENTS );
	if ( MainView != NULL )
	{
		MainView->SetViewColor( ui_color(B_PANEL_BACKGROUND_COLOR ) );
		BWindow::AddChild( MainView );
	}
	else
	{
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}
	
	BGridLayout* layout = new BGridLayout( B_VERTICAL );
	if ( !layout ) {
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}
	MainView->SetLayout( layout );
	layout->SetInsets( 0, 0, 0, 5 );
	layout->SetSpacing( 0, 2 );
	
	menuBar = CreateMenuBar();
	layout->AddView( menuBar, 0, 0 );
	
	BTabView* tabView = new BTabView( Bounds().InsetBySelf( 5, 30 ),
												 "Tab view" );
	if ( !tabView ) {
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}
	BLayoutItem* layoutItem = layout->AddView( tabView, 0, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT ) );
	}
	
	BRect individualTab = tabView->Bounds();
	individualTab.bottom -= ( tabView->TabHeight() + 20 + menuBar->Bounds().Height() );
	
	// Enable firing the activity in any case
	fData.SetEventActivityFired( false );
	
	// General view
	genView = new EventEditor_GeneralView( individualTab, &fData );
	if ( !genView || genView->InitCheck() != B_OK ) {		
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}	
	BTab* tab = new BTab();	
	tabView->AddTab( genView, tab );
	tab->SetLabel( "General" );
	
	// Reminder view
	remView = new EventEditor_ReminderView( individualTab, &fData );
	if ( !remView || remView->InitCheck() != B_OK ) {
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}	
	tab = new BTab();	
	tabView->AddTab( remView, tab );
	tab->SetLabel( "Reminder" );
	
	// Event activity
	actView = new ActivityView( individualTab.InsetByCopy( 5, 5 ), "Event activity", fData.GetEventActivity() );
	if ( !actView || actView->InitCheck() != B_OK ) {
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}	
	tab = new BTab();	
	tabView->AddTab( actView, tab );
	tab->SetLabel( "Activity" );
	
	// Note view
	noteView = new EventEditor_NoteView( individualTab.InsetByCopy( 5, 5 ), &fData );
	if ( !noteView || noteView->InitCheck() != B_OK ) {
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}
	tab = new BTab();	
	tabView->AddTab( noteView, tab );
	tab->SetLabel( "Note" );
	
	menuBar->SetTargetForItems( this );
	
	// Save button
	saveAndClose = new BButton( BRect( 0, 0, 1, 1 ),
												  "Save",
												  "Save",
												  new BMessage( kFileSave ) );
	if ( !saveAndClose ) {
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}										  
	BLayoutItem* layoutItem2 = layout->AddView( saveAndClose, 0, 2 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_BOTTOM ) );
	}
	saveAndClose->SetTarget( this );
	
	layout->SetMaxRowHeight( 1, 520 );
	layout->SetMinRowHeight( 2, 25 );
	
	// Refresh view
	InvalidateLayout();
	MainView->Invalidate();
}	// <-- end of UI initialization for MainWindow



/*!	\brief		Clears the User Interface.
 */
void	EventEditorMainWindow::ClearUI()
{
/*	BView* toDelete = NULL;
	
	if ( this->Lock() )
	{
		
		if ( genView != NULL  ) {
			genView->RemoveSelf();
			delete genView;
			genView = NULL;
		}
		
		if ( remView != NULL  ) {
			remView->RemoveSelf();
			delete remView;
			remView = NULL;
		}
		
		if ( actView != NULL  ) {
			actView->RemoveSelf();
			delete actView;
			actView = NULL;
		}
		
		if ( noteView != NULL  ) {
			noteView->RemoveSelf();
			delete noteView;
			noteView = NULL;
		}
	
		if ( saveAndClose != NULL  ) {
			saveAndClose->RemoveSelf();
			delete saveAndClose;
			saveAndClose = NULL;
		}

		toDelete = this->FindView( "Tab view" );
		if ( toDelete != NULL ) {
			toDelete->RemoveSelf();
			delete toDelete;
			toDelete = NULL;
		}
		
		if ( MainView != NULL ) {
			MainView->RemoveSelf();
			delete MainView;
			MainView = NULL;
		}

		this->Unlock();
	}
	
	*/	
	
}	// <-- end of function EventEditorMainWindow::ClearUI




/*!	\brief		Destuctor for the main window
 */
EventEditorMainWindow::~EventEditorMainWindow()
{
	ClearUI();
}	// <-- end of destructor for main window



/*!	\brief		Main function of the class.
 *		\param[in]	in		BMessage to respond to.
 */
void EventEditorMainWindow::MessageReceived( BMessage *in )
{
	BView* view;
	BEntry entry;
	BFile  file;
	BString tempString;
	BDirectory directory;
	BMessage saveMessage( kSaveRequested );
	entry_ref	ref;
	
	switch( in->what )
	{
		case B_SELECT_ALL:
		case B_COPY:
		case B_CUT:
		case B_PASTE:
		case B_UNDO:
			view = CurrentFocus();
			if ( view )
				view->MessageReceived( in );
			break;
		
		case kFileOpen:
			fOpenFile->Show();
			break;

			
		case kFileSaveAs:
			fSaveFile->Show();
			break;
		
		case kFileRevert:				// Intentional fall-through
		case kFileOpenConfirmed:
			if ( in->what == kFileRevert ) {		
				if ( fData.GetRef() != NULL )
				{
						// ...Prepare it for usage
					ref = *( fData.GetRef() );
				} else {
						// Ask the user what to do
					fOpenFile->Show();
					break;
				}
			} else {
				if ( B_OK != in->FindRef( "refs", &ref ) ||
					  B_OK != entry.SetTo( &ref, true ) ||
					  B_OK != entry.GetRef( &ref ) )
				{
					entry.Unset();
					break;
				}
			}
			fData.InitFromFile( ref );
			if ( Looper()->Lock() ) {
//				fData.Revert();
				MainView->RemoveSelf();
				delete MainView;
				InitUI();
				Looper()->Unlock();
			}
			entry.Unset();
			break;

		case kFileSave:				// Intentional fall-through			
		case kFileSaveConfirmed:
			
				// Save user's changes
			if ( genView ) genView->MessageReceived( &saveMessage );
			if ( remView ) remView->MessageReceived( &saveMessage );
			if ( actView ) actView->SaveData();
			if ( noteView ) noteView->SaveText();
			
			fData.SetEventActivityFired( false );
			fData.SetReminderActivityFired( false );
		
				// If we have the reference to file...
			if ( in->what == kFileSave ) {
				if ( fData.GetRef() != NULL )
				{
						// ...Prepare it for usage
					ref = *( fData.GetRef() );
				} else {
						// Ask the user what to do
					fSaveFile->Show();
					break;
				}				
				
			} else {
				if ( B_OK != in->FindRef( "directory", &ref ) ||
				     B_OK != in->FindString( "name", &tempString ) ||
				     B_OK != directory.SetTo( &ref ) ||
				     B_OK != directory.CreateFile( tempString.String(), NULL, false ) ||
				     B_OK != directory.FindEntry( tempString.String(), &entry, true ) ||
				     B_OK != entry.GetRef( &ref ) )
				{
					break;
				}
			}
			if ( fData.SaveToFile( &ref ) != B_OK ) {
				utl_Deb = new DebuggerPrintout( "Couldn't save file!" );	
			}
			break;
		
		case B_ABOUT_REQUESTED:
			be_app->AboutRequested();
		
		default:
		  BWindow::MessageReceived( in );
		  break;
	}
}	// <-- end of function EventEditorMainWindow::MessageReceived



/*!	\brief		This function is called when the user wants to bail out.
 */
bool EventEditorMainWindow::QuitRequested()
{
	be_app->PostMessage( B_QUIT_REQUESTED );	// Notify the application
	return BWindow::QuitRequested();				// Close the window
}	// <-- end of function EventEditorMainWindow::QuitRequested



/*!	\brief		Build the menubar
 */
BMenuBar*		EventEditorMainWindow::CreateMenuBar()
{
	BMenuBar* toReturn = new BMenuBar( "Main menu bar" );
	if ( !toReturn ) {
		/* Panic! */
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}
	
	fileMenu = new BMenu( "File" );
	if ( !fileMenu ) {
		/* Panic! */
		global_toReturn = B_NO_MEMORY;
		be_app->PostMessage( B_QUIT_REQUESTED );
	}
	
	BMenuItem* item;
	
	item = new BMenuItem( "New" B_UTF8_ELLIPSIS,
								 new BMessage( kFileNew ),
								 'N' );
	fileMenu->AddItem( item );
	item = new BMenuItem( "Open" B_UTF8_ELLIPSIS,
								 new BMessage( kFileOpen ),
								 'O' );
	fileMenu->AddItem( item );
	item = new BMenuItem( "Close",
								 new BMessage( B_QUIT_REQUESTED ),
								 'W' );
	fileMenu->AddItem( item );
	fileMenu->AddSeparatorItem();
	item = new BMenuItem( "Save",
								 new BMessage( kFileSave ),
								 'S' );
	fileMenu->AddItem( item );
	item = new BMenuItem( "Save as" B_UTF8_ELLIPSIS,
								 new BMessage( kFileSaveAs ),
								 'S',
								 B_SHIFT_KEY );
	fileMenu->AddItem( item );
	item = new BMenuItem( "Revert",
								 new BMessage( kFileRevert ),
								 'R' );
	fileMenu->AddItem( item );
	fileMenu->AddSeparatorItem();
	item = new BMenuItem( "Quit",
								 new BMessage( B_QUIT_REQUESTED ),
								 'Q' );
	fileMenu->AddItem( item );
	
	toReturn->AddItem( fileMenu );
	
	
	editMenu = new BMenu( "Edit" );
	item = new BMenuItem( "Undo",
								 new BMessage( B_UNDO ),
								 'Z' );
	editMenu->AddItem( item );
	item = new BMenuItem( "Select all",
								 new BMessage( B_SELECT_ALL ),
								 'X' );
	editMenu->AddItem( item );
	item = new BMenuItem( "Cut",
								 new BMessage( B_CUT ),
								 'X' );
	editMenu->AddItem( item );
	item = new BMenuItem( "Copy",
								 new BMessage( B_COPY ),
								 'C' );
	editMenu->AddItem( item );
	item = new BMenuItem( "Paste",
								 new BMessage( B_PASTE ),
								 'V' );
	editMenu->AddItem( item );
	
	toReturn->AddItem( editMenu );
	
	item = new BMenuItem( "About" B_UTF8_ELLIPSIS,
								 new BMessage( B_ABOUT_REQUESTED ) );
	toReturn->AddItem( item );
	
	return toReturn;
}	// <-- end of EventEditorMainWindow::CreateMenuBar

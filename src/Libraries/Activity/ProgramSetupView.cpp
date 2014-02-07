/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// Project includes
#include "ActivityData.h"
#include "ProgramSetupView.h"
#include "Utilities.h"

// OS includes
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <GridLayout.h>
#include <GroupLayout.h>
#include <InterfaceDefs.h>
#include <LayoutItem.h>
#include <Looper.h>
#include <Node.h>
#include <StorageDefs.h>



/*---------------------------------------------------------------------------
 *									Messages' constants
 *--------------------------------------------------------------------------*/

const uint32		kProgramActivityCheckBoxToggled				= 'PACB';
const uint32		kProgramActivityCommandLineOptionsChanged	= 'PACO';
const uint32		kProgramActivityStartFileSearch				= 'PAFS';
const uint32		kProgramActivityFileChosen						= 'PAFC';



/*---------------------------------------------------------------------------
 *									Useful colors
 *--------------------------------------------------------------------------*/

const rgb_color	kEnabledTextColor			= ui_color( B_CONTROL_TEXT_COLOR );
const rgb_color	kDisabledTextColor		= tint_color( ui_color( B_PANEL_BACKGROUND_COLOR ),
																		  B_DISABLED_LABEL_TINT );



/*---------------------------------------------------------------------------
 *						Implementation of class ProgramFileFilter
 *--------------------------------------------------------------------------*/
bool	ProgramFileFilter::Filter( const entry_ref *ref,
											BNode* node,
											struct stat_beos* st,
											const char* filetype )
{
	BString 		fileType( filetype );
	BDirectory 	testDir( ref );
	BEntry		tempEntry;
	BNode			tempNode;
	char			buffer[ B_MIME_TYPE_LENGTH ];

	// Allow all directories	
	if ( testDir.InitCheck() == B_OK ) {
		return true;
	}
	
	if ( ( fileType.IFindFirst( "application/x-be-executable" ) == 0 ) ||
	     ( fileType.IFindFirst( "application/x-vnd.Be-elfexecutable" ) == 0 ) )
	{
		return true;
	}
	
	while ( fileType.IFindFirst( "application/x-vnd.Be-symlink" ) == 0 )
	{
		if ( ( B_OK == tempEntry.SetTo( ref, true ) ) &&
		     ( B_OK == tempNode.SetTo( &tempEntry ) ) &&
		     ( 0 != tempNode.ReadAttr( "BEOS:TYPE", B_STRING_TYPE, 0, buffer, 255 ) ) &&
		     ( NULL != fileType.SetTo( buffer ) ) &&		// This check is really unnecessary
		     ( ( fileType.IFindFirst( "application/x-be-executable" ) == 0 ) ||
	     		 ( fileType.IFindFirst( "application/x-vnd.Be-elfexecutable" ) == 0 ) ) )
	   {
			return true;
	   }
	}
	return false;
}	// <-- end of function ProgramFileFilter::Filter


/*---------------------------------------------------------------------------
 *						Implementation of class ProgramSetupView
 *--------------------------------------------------------------------------*/

/*!	\brief		Constructor for the class ProgramSetupView.
 */
ProgramSetupView::ProgramSetupView( BRect frame, const char *name, ActivityData* data )
	:
	BBox( frame, name, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_FRAME_EVENTS ),
	fData( data ),
	fLastError( B_OK ),
	bIsEnabled( true ),
	fCheckBox( NULL ),
	fLabel( NULL ),
	fFileName( NULL ),
	fOpenFilePanel( NULL ),
	fFilePanel( NULL ),
	bFilePanelCreated( false ),
	fRefFilter( NULL ),
	fCommandLineOptionsInput( NULL )
{
	fCommandLineOptions.Truncate( 0 );
	
	// Set the layout for the global view
	BGroupLayout* groupLayout = new BGroupLayout( B_VERTICAL );
	if ( !groupLayout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	groupLayout->SetInsets( 5, 5, 5, 5 );
	this->SetLayout( groupLayout );
	
	// Create the enable / disable checkbox
	BMessage* toSend = new BMessage( kProgramActivityCheckBoxToggled );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}

	fCheckBox = new BCheckBox( BRect( 0, 0, 1, 1 ),
										"Enable running program",
										"Run a program",
										toSend );
	if ( !fCheckBox ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fCheckBox->ResizeToPreferred();
	
	BBox::SetLabel( fCheckBox );
	BLayoutItem* layoutItem;
	
	// Create the internal layout
	BGridLayout* gridLayout = new BGridLayout();
	if ( !gridLayout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	gridLayout->SetInsets( 10, ( fCheckBox->Bounds() ).Height(), 10, 10 );
	gridLayout->SetSpacing( 5, 2 );
	gridLayout->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH,
																 B_ALIGN_TOP ) );
	this->SetLayout( gridLayout );
	
	// Create the explanation string
	fLabel = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Program setup label",
									  "Select the program:" );
	if ( !fLabel ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fLabel->ResizeToPreferred();
	BSize size( fLabel->Bounds().Width(), fLabel->Bounds().Height() );
	fLabelLayoutItem = gridLayout->AddView( fLabel, 0, 1, 1, 1 );
	if ( fLabelLayoutItem ) {
		fLabelLayoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_MIDDLE ) );
		fLabelLayoutItem->SetExplicitMinSize( size );
		fLabelLayoutItem->SetExplicitPreferredSize( size );
		gridLayout->SetMinColumnWidth( 0, size.Width() );
	}
	
	// Create the message for button which opens the file panel
	toSend = new BMessage( kProgramActivityStartFileSearch );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	
	// Create the button itself
	fOpenFilePanel = new BButton( BRect( 0, 0, 1, 1 ),
											"Program setup open file panel button",
											"Choose",
											toSend );
	if ( !fOpenFilePanel ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fOpenFilePanel->ResizeToPreferred();
	size.Set( fOpenFilePanel->Bounds().Width(), fOpenFilePanel->Bounds().Height() );
	layoutItem = gridLayout->AddView( fOpenFilePanel, 1, 1, 1, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );
		layoutItem->SetExplicitMaxSize( size );
		layoutItem->SetExplicitPreferredSize( size );
		gridLayout->SetMaxColumnWidth( 1, size.Width() );
	}
	
	// Create the BStringView with name of currently chosen file.
	fFileName = new BStringView( BRect( 0, 0, 1, 1 ),
										  "Currently chosen program file",
										  "No file" );
	if ( !fFileName ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fFileName->ResizeToPreferred();
	layoutItem = gridLayout->AddView( fFileName, 0, 2, 2, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
	}	
	
	// Last, create the BTextControl with command line options
	toSend = new BMessage( kProgramActivityCommandLineOptionsChanged );
	if ( ! toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fCommandLineOptionsInput = new BTextControl( BRect( 0, 0, 1, ( this->Bounds().Width() - 10 ) ),
																"Command line options editor",
																"Command line parameters:",
																NULL,		// Contents will be set later
																toSend );
	if ( !fCommandLineOptionsInput ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fCommandLineOptionsInput->ResizeToPreferred();
	size.Set( fCommandLineOptionsInput->Bounds().Width(), fCommandLineOptionsInput->Bounds().Height() );
	( fCommandLineOptionsInput->TextView() )->SetMaxBytes( ACTIVITY_MAX_ALLOWED_COMMAND_LINE_OPTIONS_LENGTH );
	layoutItem = gridLayout->AddView( fCommandLineOptionsInput, 0, 3, 2, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_MIDDLE ) );
		layoutItem->SetExplicitMinSize( size );
		layoutItem->SetExplicitPreferredSize( size );
	}


	UpdateInitialValues();

	Relayout();
	fLastError = B_OK;
	
}	// <-- end of constructor for class ProgramSetupView



/*!	\brief		Updates initial values of the control upon startup.
 */
void			ProgramSetupView::UpdateInitialValues()
{
	bool enabled;
	BString fileName;
	BDirectory parentDir;
	
	if ( fData ) {
		enabled = fData->GetProgram( &fPathToFile, &fCommandLineOptions );
	}
	else
	{
		enabled = false;
		fileName.SetTo( "No program" );
		fCommandLineOptions.Truncate( 0 );
		find_directory( B_BEOS_BIN_DIRECTORY, &fPathToDirectory );
	}
	
	if ( fPathToFile.InitCheck() == B_OK ) {
		fileName.SetTo( fPathToFile.Leaf() );
		fPathToFile.GetParent( &fPathToDirectory );
		if ( fPathToDirectory.InitCheck() != B_OK ) {
			find_directory( B_BEOS_BIN_DIRECTORY, &fPathToDirectory );
		}
		
		// Is the chosen item a file?
		parentDir.SetTo( fPathToDirectory.Path() );
		if ( ( parentDir.InitCheck() != B_OK ) ||
			  !( parentDir.Contains( fileName.String(), B_FILE_NODE ) ) )
		{
			fileName.SetTo( "No file" );
			fData->SetProgramPath( "" );	// Removing invalid path.
			enabled = false;
		}
	}
	else
	{
		enabled = false;
		fileName.SetTo( "No program" );
		find_directory( B_BEOS_BIN_DIRECTORY, &fPathToDirectory );
	}
	
	// At this point, fPathToFile may be not initialized, but it doesn't matter
	// as long as the directory is initialized.
	
	fFileName->SetText( fileName.String() );
	fCommandLineOptionsInput->SetText( fCommandLineOptions.String() );
	
	if ( enabled ) {
		fCheckBox->SetValue( 1 );
		ToggleCheckBox( true );
	} else {
		fCheckBox->SetValue( 0 );
		ToggleCheckBox( false );
	}
	
	// Create the reference filter
	if ( !fRefFilter ) {
		fRefFilter = new ProgramFileFilter();
	}
	if ( !fRefFilter ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	
}	// <-- end of function ProgramSetupView::UpdateInitialValues



/*!	\brief		Destructor for the class ProgramSetupView
 */
ProgramSetupView::~ProgramSetupView() {
	
	if ( bFilePanelCreated && fFilePanel ) {
		delete fFilePanel;
		bFilePanelCreated = false;
		fFilePanel = NULL;
	}
	
	if ( fRefFilter ) {
//		delete fRefFilter;
	}
	
	if ( fThisMessenger ) {
//		delete fThisMessenger;
	}
	
	if ( fCommandLineOptionsInput ) {
		delete fCommandLineOptionsInput;
	}	
}	// <-- end of destructor for ProgramSetupView



/*!	\brief		Add current handler to looper
 */
void		ProgramSetupView::AttachedToWindow() {
	BFont plainFont( be_plain_font );
	
	BBox::AttachedToWindow();
	if ( this->Parent() ) {
		this->SetViewColor( this->Parent()->ViewColor() );
	}
	
	if ( this->Looper() && this->Looper()->Lock() ) {
		this->Looper()->AddHandler( this );
		this->Looper()->Unlock();
	}
	
	// Set targets
	if ( fCheckBox ) {
		fCheckBox->SetTarget( this );
	}
	if ( fOpenFilePanel ) {
		fOpenFilePanel->SetTarget( this );
	}
	if ( fCommandLineOptionsInput ) {
		fCommandLineOptionsInput->SetTarget( this );
		fCommandLineOptionsInput->SetDivider( plainFont.StringWidth( fCommandLineOptionsInput->Label() ) + 15 );
	}
	
	// This is the first place where we can construct our own BMessenger, since
	// we didn't have the Looper earlier. 
	if ( !fThisMessenger ) {
		fThisMessenger = new BMessenger( this, this->Looper() );
	}
	if ( !fThisMessenger ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}

}	// <-- end of function ProgramSetupView::AttachedToWindow



/*!	\brief		Main function of the class
 */
void 		ProgramSetupView::MessageReceived( BMessage* in ) {

	entry_ref	tempRef;
	BEntry		programFile;
	BString		sb;
	
	if ( !in ) { return; }
	
	switch ( in->what ) {
		case kProgramActivityCheckBoxToggled:
			if ( this->fCheckBox ) {
				if ( this->fCheckBox->Value() != 0 ) {
					ToggleCheckBox( true );
				}
				else
				{
					ToggleCheckBox( false );
				}
			}
			break;
		
		case kProgramActivityCommandLineOptionsChanged:
		
			SaveData();
		
			break;
			
		case kProgramActivityStartFileSearch:
			fLastError = CreateAndShowFilePanel();
			if ( fLastError != B_OK ) {
				utl_Deb = new DebuggerPrintout( "Couldn't create the file panel!" );
			}
			break;
			
		case kProgramActivityFileChosen:
		
			if ( ( B_OK != in->FindRef( "refs", &tempRef ) ) ||
			     ( B_OK != programFile.SetTo( &tempRef, true ) ) )
			{
				// Didn't succeed to get the new entry
				break;
			}
			// Ok, now soundFile is initialized to the file user wants to hear.
			
			// Set the paths to file and directory
			if ( B_OK == fPathToFile.SetTo( &programFile ) )
			{
				fPathToFile.GetParent( &fPathToDirectory );
				
				// Update the saved activity data
				fData->SetProgramPath( fPathToFile );
				
				// Update the name of the program
				if ( this->fFileName ) {
					fFileName->SetText( fPathToFile.Leaf() );
					fFileName->ResizeToPreferred();
					fFileName->Invalidate();
				}
			}
			
			break;
			
		case B_CANCEL:		// Intentional fall-through
		default:
			BBox::MessageReceived( in );
	};	
}	// <-- end of function ProgramSetupView::MessageReceived



/*!	\brief		Create and open the file panel for selecting the sound file.
 *		\return		B_OK	if everything went good. \n Some other constant, if not.
 */
status_t		ProgramSetupView::CreateAndShowFilePanel()
{
	// Note: the message is constructed on the stack, and will be freed when exitting.
	BMessage toSend( kProgramActivityFileChosen );

	if ( !bFilePanelCreated || !fFilePanel )
	{
		fFilePanel = new BFilePanel( B_OPEN_PANEL,
											  fThisMessenger,	// Hopefully this was set earier
											  NULL,			// Panel directory will be set later
											  0,				// B_FILE_NODE only
											  false,			// Disallow multiple selection
											  &toSend,		// Message to be sent
											  fRefFilter );// Filter of the entries
		if ( !fFilePanel ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return B_NO_MEMORY;
		}
		fFilePanel->SetPanelDirectory( fPathToDirectory.Path() );
		bFilePanelCreated = true;
	}
	
	fFilePanel->Show();
	
	return B_OK;
}	// <-- end of function "CreateAndShowFilePanel"


/*!	\brief			This function is called when the frame is resized.
 */
void		ProgramSetupView::FrameResized( float width, float height )
{
	BBox::FrameResized( width, height );
	
	BFont plainFont( be_plain_font );
	
	if ( fLabel ) {
		BRect labelRect = fLabel->Bounds();
		BSize size;
		size.SetHeight( labelRect.Height() );
		if ( fOpenFilePanel ) {
			size.SetWidth( width - ( 2 * 10 ) - 10 - fOpenFilePanel->Bounds().Width() );
		} else {
			size.SetWidth( width - ( 2 * 10 ) );
		}
		if ( fLabelLayoutItem ) {
			fLabelLayoutItem->SetExplicitMinSize( size );
			fLabelLayoutItem->SetExplicitPreferredSize( size );
		}
	}
	if ( fCommandLineOptionsInput ) {
		fCommandLineOptionsInput->SetDivider( plainFont.StringWidth( fCommandLineOptionsInput->Label() ) + 15 );
	}
	Relayout();
	Invalidate();
	if ( Window() ) {
		Window()->UpdateIfNeeded();
	}
	if ( fOpenFilePanel ) {
		fOpenFilePanel->Invalidate();
	}
	
}	// <-- end of function ProgramSetupView::FrameResized



/*!	\brief		This function should be called when a user - or code - toggles checkbox.
 *		\param[in]	enabled		\c true	if the control should be enabled
 *										\c false	otherwise.
 */
void		ProgramSetupView::ToggleCheckBox( bool enable )
{
	if ( enable ) {
		
		// UI changes
		if ( this->fOpenFilePanel ) {
			fOpenFilePanel->SetEnabled( true );
		}
		if ( this->fFileName ) {
			this->fFileName->SetHighColor( kEnabledTextColor );
			this->fFileName->Invalidate();
		}
		if ( fCommandLineOptionsInput ) {
			fCommandLineOptionsInput->SetEnabled( true );
		}
		// Saved activity changes
		if ( fData ) {
			fData->SetProgram( true );
		}
	}
	else
	{
		// UI changes
		if ( this->fOpenFilePanel ) {
			fOpenFilePanel->SetEnabled( false );
		}
		if ( this->fFileName ) {
			fFileName->SetHighColor( kDisabledTextColor );
			this->fFileName->Invalidate();
		}
		if ( fCommandLineOptionsInput ) {
			fCommandLineOptionsInput->SetEnabled( false );
		}
		
		// Saved ativity changes
		if ( fData ) {
			fData->SetProgram( false );
		}
	}
}	// <-- end of ProgramSetupView::ToggleCheckBox



/*!	\brief		Enable or disable the view.
 */
void		ProgramSetupView::SetEnabled( bool toSet )
{
	if ( toSet == IsEnabled() ) { return; }
	bIsEnabled = toSet;
	
	if ( fCheckBox ) {
		fCheckBox->SetEnabled( toSet );
		if ( fCheckBox->Value() != 0 ) {
			fOpenFilePanel->SetEnabled( toSet );
			fCommandLineOptionsInput->SetEnabled( toSet );
		}
	}	
}	// <-- end of function ProgramSetupView::SetEnabled



/*!	\brief		Saves the data to EventData.
 *		\details		Only command-line options need to be saved, since
 *						filename and checkbox are updated upon selection.
 */
void		ProgramSetupView::SaveData() {
	BString sb;
	if ( fData && fCommandLineOptionsInput ) {
		sb = ActivityData::VerifyCommandLineParameters( fCommandLineOptionsInput->Text() );
		fData->SetProgramOptions( sb );
		fCommandLineOptionsInput->SetText( sb.String() );
	}
}	// <-- end of function ProgramSetupView::SaveData

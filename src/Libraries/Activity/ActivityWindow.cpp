/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// Project includes
#include "ActivityWindow.h"
#include "Preferences.h"

// OS includes
#include <GridLayout.h>
#include <GroupLayout.h>
#include <InterfaceDefs.h>
#include <LayoutItem.h>
#include <Point.h>
#include <Rect.h>
#include <Size.h>


const uint32		kSnoozeTimeControlMessage 	= 'STCM';
const	uint32		kSnoozeButtonPressed			= 'SnBP';
const	uint32		kDismissButtonPressed		= 'DmBP';


/*!	\brief		Constructor for the ActivityWindow class.
 *		\param[in]	data			The data to be displayed.
 *		\param[in]	target		The process to be notified about user's choise.
 *		\param[in]	name			Name of the Event.
 *		\param[in]	category		Category of the Event.
 *		\param[in]	templateMessage		The message to be sent to the target.
 *										If \c NULL is passed, then a new message is constructed
 *										with \c kActivityWindowRepsonceMessage value in \c what.
 *		\param[in]	reminder		\c true if the window is constructed for a reminder, else
 *										\c false. Actually, it matters only for explanation to user.
 *										Default is \c false (it's not a reminder).
 *		\note			A note on memory management:
 *						\c data (the ActionData) belongs to the caller, but it's used only for
 *						initialization of this window. I. e., if the user makes changes to the
 *						data while an ActivityWindow is open, the changes won't be reflected.
 *						However, \c target and \c templateMessage belong to this object. User
 *						shouldn't free them or do anything else.
 */
ActivityWindow::ActivityWindow( ActivityData* data,
									 BMessenger* target,
									 BString		 name,
									 Category*	 category,
									 BMessage* templateMessage,
									 bool reminder )
	:
	BWindow( BRect( 0, 0, 400, 500 ),
				"Event occurred",
				B_FLOATING_WINDOW_LOOK,
				B_NORMAL_WINDOW_FEEL,
				B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS ),
	fTarget( target ),
	fData( data ),
	fTemplateMessage( templateMessage ),
	bIsReminder( reminder ),
	fLastError( B_OK ),
	fEventName( name ),
	fCategory( category ),
	fTitle( NULL ),
	fEventNameView( NULL ),
	fCategoryView( NULL ),
	fTextScroller( NULL ),
	fSnoozeTime( NULL ),
	fNoteText( NULL ),
	fSnooze( NULL ),
	fOk( NULL )
{
	BFont boldFont( be_bold_font );
	BFont plainFont( be_plain_font );
	BFont font;			// For various font-related activities
	font_height	fh;	// For setting the height of the Text View with notification text
	plainFont.GetHeight( &fh );
	int	numberOfColumnsInLayout = 2;
	
	// Sanity check
	if ( !data || !target ) {
		/* Panic! */
		fLastError = B_BAD_VALUE;
		return;
	}
	
	if ( ! fData->GetNotification( NULL ) &&
		  ! fData->GetSound( NULL ) &&
		  ! fData->GetProgram( NULL, NULL ) )
	{
		// Nothing to do! This is not an error!
		fLastError = B_NO_INIT;
		return;
	}
	BView*	background = new BView( Bounds(),
												"Background view",
												B_FOLLOW_ALL_SIDES,
												B_WILL_DRAW );
	if ( !background ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	this->AddChild( background );
	BGridLayout* gridLayout = new BGridLayout();
	if ( !gridLayout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	background->SetLayout( gridLayout );
	background->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	gridLayout->SetInsets( 5, 5, 5, 5 );
	
	/*-------------------------------------------------
	 * First line - explaining what's happening here
	 *------------------------------------------------*/
	BStringView* exp = new BStringView( BRect( 0, 0, 1, 1 ),
													"Explanation 1",
													( bIsReminder ? 
															"A Reminder has occured!" : 
															"An Event has occured!" ) );
	if ( ! exp ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;	
	}
	exp->SetFont( &boldFont );
	exp->ResizeToPreferred();
	BLayoutItem* layoutItem = gridLayout->AddView( exp, 0, 0, numberOfColumnsInLayout, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
	}
	
	/*-----------------------------------------------------------------
	 * Second line - event's name on category's color as background
	 *----------------------------------------------------------------*/
	 
	// Create background
	// Note: the pulse is requested for this Window to receive Pulse notifications.
	fBackground = new BView( BRect( 0, 0, 1, 1 ),
									 "Background",
									 B_FOLLOW_LEFT_RIGHT | B_FOLLOW_V_CENTER | B_PULSE_NEEDED,
									 B_WILL_DRAW );
	if ( !fBackground ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fBackground->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	layoutItem = gridLayout->AddView( fBackground, 0, 1, numberOfColumnsInLayout, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT ) );
	}
	
	BGroupLayout* bgLayout = new BGroupLayout( B_VERTICAL );
	if ( !bgLayout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fBackground->SetLayout( bgLayout	);
	bgLayout->SetInsets( 15, 10, 15, 10 );
	fBackground->SetViewColor( fCategory.categoryColor );
	BString sb;
	
	sb <<  "Category: " << fCategory.categoryName;
	
	printf( "%s\n", sb.String() );
	
	printf( "Event name: %s\n", fEventName.String() );
	
//	utl_Deb = new DebuggerPrintout( sb.String() );
	
	fBackground->SetToolTip( sb.String() );
	
	// Create Event's name view
	fTitle = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Event name",
									  fEventName.String(),
									  B_FOLLOW_H_CENTER | B_FOLLOW_V_CENTER );
	if ( !fTitle ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
		// Use big bold font for Event's name
	fTitle->SetFont( be_bold_font );
	fTitle->GetFont( &font );
	font.SetSize( font.Size() + 2 );
	fTitle->SetFont( &font, B_FONT_SIZE );
	
		// Add the title and set its tooltip
	fTitle->ResizeToPreferred();
//	fTitle->SetToolTip( sb.String() );
	bgLayout->AddView( fTitle );
	fTitle->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	fTitle->SetToolTip( fEventName.String() );
	
	int	nextLineInLayout = 2;
	
	/*================================================================
	 * If the notification was set by the user, display it.
	 *================================================================*/

	BString 	tempString;
	BPath		path;
	float rectHeight = fh.leading + fh.ascent + fh.descent;
	float rectWidth = this->Bounds().Width() - 10;		// Layout insets
	BSize size( rectWidth, rectHeight );
	
	if ( fData->GetNotification( &tempString ) )
	{
		/*-----------------------------------------------------------------
	 	 * Line of explanation
	 	 *----------------------------------------------------------------*/
		exp = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Text view explanation",
									  "You set the following notification:" );
		if ( !exp ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return;
		}
		exp->ResizeToPreferred();
		layoutItem = gridLayout->AddView( exp, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );

		/*-----------------------------------------------------------------
	 	 * Text view with notification text
	 	 *----------------------------------------------------------------*/
		BRect tempRect( BPoint( 0, 0 ), size );
		tempRect.right -= B_V_SCROLL_BAR_WIDTH;
		fNoteText = new BTextView( tempRect,
											"Notification text container",
											tempRect.InsetByCopy( 1, 1 ),
											B_FOLLOW_ALL_SIDES,
											B_WILL_DRAW );
		if ( !fNoteText ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return;
		}
		fNoteText->MakeEditable( false );
		fNoteText->SetText( tempString.String() );
		
		/*-----------------------------------------------------------------
	 	 * Scroll view to scroll the notification text
	 	 *----------------------------------------------------------------*/
		fTextScroller = new BScrollView( "Notification text scroller",
													fNoteText,
													B_FOLLOW_ALL_SIDES,
													0,
													false,
													true );
		if ( !fTextScroller ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return;
		}
		layoutItem = gridLayout->AddView( fTextScroller, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
		if ( layoutItem ) {
//			layoutItem->SetExplicitMaxSize( size );
			layoutItem->SetExplicitMinSize( size );
			layoutItem->SetExplicitPreferredSize( size );
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT ) );
		}
	}	// <-- end of adding information about the notification
	
	/*================================================================
	 * If user wanted to play a sound file, notify him about it.
	 *================================================================*/
	if ( fData->GetSound( &path ) )
	{
		/*-----------------------------------------------------------------
	 	 * Line of explanation
	 	 *----------------------------------------------------------------*/
		exp = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Sound file explanation",
									  "You wanted to play the file:",
									  B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP );
		if ( !exp ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return;
		}
		exp->ResizeToPreferred();
		layoutItem = gridLayout->AddView( exp, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
		}

		/*-----------------------------------------------------------------
	 	 * Display sound file name
	 	 *----------------------------------------------------------------*/
	 	 
	 	// What should we display - full path or just the leaf?
	 	if ( ( size.width - 10 ) > plainFont.StringWidth( path.Path() ) )
	 	{
	 		tempString.SetTo( path.Path() );
	 	}
	 	else
	 	{
	 		tempString.SetTo( path.Leaf() );
	 	}
		exp = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Sound file name",
									  tempString.String() );
		if ( !exp ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return;
		}
		exp->ResizeToPreferred();
		layoutItem = gridLayout->AddView( exp, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
		}
		
	}	// <-- end of displaying information about the sound file
	
	/*================================================================
	 * If user wanted to run a program, notify him about it.
	 *================================================================*/
	if ( fData->GetProgram( &path, &tempString ) )
	{
		/*-----------------------------------------------------------------
	 	 * Line of explanation
	 	 *----------------------------------------------------------------*/
		exp = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Program explanation",
									  "You wanted to run a program:" );
		if ( !exp ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return;
		}
		exp->ResizeToPreferred();
		layoutItem = gridLayout->AddView( exp, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
		}

		/*-----------------------------------------------------------------
	 	 * Display path to program file
	 	 *----------------------------------------------------------------*/
	 	 
	 	// What should we display - full path or just the leaf?
	 	exp = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Program file name",
									  ( ( size.width - 10 ) > plainFont.StringWidth( path.Path() ) ) ?
									  		path.Path() :
									  		path.Leaf() );
		if ( !exp ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			return;
		}
		exp->ResizeToPreferred();
		layoutItem = gridLayout->AddView( exp, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
		}
		
		/*-----------------------------------------------------------------
	 	 * Explanation about the program options
	 	 *----------------------------------------------------------------*/
	 	 
	 	if ( tempString.Length() > 0 ) {
		 	exp = new BStringView( BRect( 0, 0, 1, 1 ),
										  "Program file options explanation",
										  "With the following parameters:" );
			if ( !exp ) {
				/* Panic! */
				fLastError = B_NO_MEMORY;
				return;
			}
			exp->ResizeToPreferred();
			layoutItem = gridLayout->AddView( exp, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
			if ( layoutItem ) {
				layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
			}
			
			/*-----------------------------------------------------------------
		 	 * Display the program options
		 	 *----------------------------------------------------------------*/
		 	 
		 	// What should we display - full path or just the leaf?
		 	exp = new BStringView( BRect( 0, 0, 1, 1 ),
										  "Program file options",
										  tempString.String() );
			if ( !exp ) {
				/* Panic! */
				fLastError = B_NO_MEMORY;
				return;
			}
			exp->ResizeToPreferred();
			layoutItem = gridLayout->AddView( exp, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
			if ( layoutItem ) {
				layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
			}
	 	}	// <-- end of diplaying CLI options		
	}	// <-- end of displaying information about the program to run
	
	/*================================================================
	 * Now it's time to display the Snooze time selector
	 *================================================================*/
	TimePreferences* prefs = pref_GetTimePreferences();
	if ( prefs ) {
		prefs->GetDefaultSnoozeTime( ( int* )&fSnoozeHours, ( int* )&fSnoozeMins );
	} else {
		fSnoozeHours = 0;
		fSnoozeMins = 10;
	}
	
	BMessage* toSend = new BMessage( kSnoozeTimeControlMessage );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fSnoozeTime = new GeneralHourMinControl( BRect( 0, 0, 1, 1 ),
														  "Snooze time selector",
														  "Snooze this Activtiy for:",
														  BString( "" ),	// No check box
														  toSend );
	if ( !fSnoozeTime ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fSnoozeTime->SetHoursLimit( 23 );	// Max reminder time delay is 23 hours 55 minutes
	fSnoozeTime->SetMinutesLimit( 55 );
	fSnoozeTime->SetCurrentTime( fSnoozeHours, fSnoozeMins );
	
	layoutItem = gridLayout->AddView( fSnoozeTime, 0, nextLineInLayout++, numberOfColumnsInLayout, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_MIDDLE ) );
	}
	fSnoozeTime->SetTarget( this );
	

	/*================================================================
	 * Snooze button
	 *================================================================*/
	toSend = new BMessage( kSnoozeButtonPressed );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fSnooze = new BButton( BRect( 0, 0, 1, 1 ),
								  "Snooze button",
								  "Snooze",
								  toSend,
								  B_FOLLOW_LEFT | B_FOLLOW_TOP );
	if ( !fSnooze ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fSnooze->ResizeToPreferred();
	layoutItem = gridLayout->AddView( fSnooze, 0, nextLineInLayout );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	}
	fSnooze->SetTarget( this );

	/*================================================================
	 * Ok button
	 *================================================================*/
	toSend = new BMessage( kDismissButtonPressed );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fOk = new BButton( BRect( 0, 0, 1, 1 ),
							  "Dismiss button",
							  "Dismiss",
							  toSend,
							  B_FOLLOW_RIGHT | B_FOLLOW_TOP );
	if ( !fOk ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fOk->ResizeToPreferred();
	layoutItem = gridLayout->AddView( fOk, 1, nextLineInLayout );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
	}
	fOk->SetTarget( this );

	this->CenterOnScreen();	
}	// <-- end of constructor for ActivityWindow



/*!	\brief		Destructor for the class ActivityWindow
 */
ActivityWindow::~ActivityWindow()
{
	BView* toRemove;
	
	if ( NULL != ( toRemove = FindView( "Explanation 1" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	
	if ( NULL != ( toRemove = FindView( "Background" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Text view explanation" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != fTextScroller ) {
		fTextScroller->RemoveSelf();
		delete fTextScroller;
	}
	if ( NULL != fNoteText ) {
		fNoteText->RemoveSelf();
		delete fNoteText;
	}
	if ( NULL != ( toRemove = FindView( "Sound file explanation" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Sound file name" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Program explanation" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Program file name" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Program file options explanation" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Program file options" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Snooze time selector" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Snooze button" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	if ( NULL != ( toRemove = FindView( "Dismiss button" ) ) ) {
		toRemove->RemoveSelf();
		delete toRemove;
	}
	
}	// <-- end of destructor for ActivityWindow



/*!	\brief		Main function of the window
 */
void		ActivityWindow::MessageReceived( BMessage* in )
{
	BMessage* toSend = NULL;
	switch ( in->what )
	{
	case B_COPY:
		// Allow the user to copy notification text.
		if ( fNoteText ) {
			fNoteText->MessageReceived( in );
		}
		break;
	
	case kSnoozeButtonPressed:
		if ( fTemplateMessage ) {
			toSend = new BMessage( *fTemplateMessage );
		} else {
			toSend = new BMessage( kActitivyWindowRepsonceMessage );
		}
		
		if ( !fTemplateMessage ) {
			/* Panic! */
			fLastError = B_NO_MEMORY;
			break;
		}
		toSend->AddInt32( "Hours", ( int32 )fSnoozeHours );
		toSend->AddInt32( "Minutes", ( int32 )fSnoozeMins );
		fTarget->SendMessage( toSend );
		// Intentional fall-through
		
	case kDismissButtonPressed:
		// Close the window.
		this->Quit();
		break;
	
	case kSnoozeTimeControlMessage:	// Intentional fall-through
	case kGeneralHourMinControlUpdated:
		if ( B_OK == in->FindInt32( kHoursValueKey.String(), ( int32* )&fSnoozeHours ) &&
			  B_OK == in->FindInt32( kMinutesValueKey.String(), ( int32* )&fSnoozeMins ) )
		{
			// No action is needed
		} else {
			utl_Deb = new DebuggerPrintout( "Did not succeed to read the new time!" );
		}
		break;
		
	default:
		BWindow::MessageReceived( in  );
	};	// <-- end of "switch ( in->what )"
}	// <-- end of function MessageReceived

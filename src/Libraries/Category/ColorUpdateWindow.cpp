/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// POSIX includes
#include <limits.h>		/* For the USHRT_MAX limit  */
#include <math.h>		/* For the "ceilf" function */
#include <stdio.h>
#include <stdlib.h>

// OS includes
#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <fs_attr.h>
#include <Font.h>
#include <GroupLayout.h>
#include <GridLayout.h>
#include <InterfaceKit.h>
#include <LayoutItem.h>
#include <ListItem.h>
#include <ListView.h>
#include <Message.h>
#include <Query.h>
#include <Size.h>
#include <String.h>
#include <SupportDefs.h>
#include <TypeConstants.h>
#include <Volume.h>
#include <VolumeRoster.h>

// Project includes
#include "CategoryItem.h"
#include "Category.h"
#include "ColorUpdateWindow.h"
#include "Utilities.h"

#ifndef SPACING
	#define SPACING 2
#endif


/*======================================================================
 * 		Implementation of class ColorUpdateWindow
 *=====================================================================*/


/*!	\brief		Constructor
 *	\param[in]	catIn	Category to be edited.
 *	\param[in]	enableEditingLabelIn	If "true", the category's name can be edited.
 *										If "false", it's constant.
 *	\param[in]	title		Title of the window.
 *	\param[in]	message		Message to be sent.
 *	\param[in]	handler		Target that will receive the message.
 */
ColorUpdateWindow::ColorUpdateWindow( const Category& catIn,
									  bool enableEditingLabelIn,
									  const char* title,									  
									  BHandler* handler,
									  BLooper* looperIn,
									  BMessage* message )
	:
	BWindow( BRect( 100, 100, 300, 500 ),
			 title,
			 B_MODAL_WINDOW,
			 B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_NOT_MINIMIZABLE | B_ASYNCHRONOUS_CONTROLS | B_WILL_ACCEPT_FIRST_CLICK ),
	originalString( catIn.categoryName ),
	originalColor( catIn.categoryColor ),
	labelView( NULL ),
	enableEditingLabel( enableEditingLabelIn ),
	target( handler ),
	looper( looperIn ),
	okButton( NULL ),
	revertButton( NULL ),
	messageToSend( message ),
	dirty( false )
{
	BSize layoutSize;
	BLayoutItem* item = NULL;
	float width, height, dontCare;
	
	// Construct background view
	BView* background = new BView( this->Bounds(),
								   "Background",
								   B_FOLLOW_LEFT | B_FOLLOW_TOP,
								   B_FRAME_EVENTS | B_WILL_DRAW );
	if ( !background )
	{
		/* Panic! */
		exit(1);
	}
	background->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );	
	
	/* There are three parts in the interface of the control.
	 * Upmost is the label, which, according to the user's settings,
	 * can be editable or not.
	 * Next is the color control.
	 * Last is the row of two buttons, Revert and Ok, and current color displayed between.
	 */
	
	// Construct layout
	BGridLayout* layout = new BGridLayout( B_VERTICAL );
	if ( !layout ) { /* Panic! */ exit(1); }
	layout->SetInsets( 5, 5, 5, 5 );
	
	background->SetLayout( layout );
	this->AddChild( background );

	// Constructing the name label, editable or not.
	if ( enableEditingLabel )
	{
		labelView = new BTextControl( BRect(0, 0, 1, 1),
									  "Label",
								   	  NULL,
								   	  originalString.String(),
								   	  NULL );
	} else {
		labelView = new BStringView( BRect (0, 0, 1, 1),
									 "Label",
									 originalString.String() );
	}
	if ( !labelView )
	{
		/* Panic! */
		exit(1);
	}
	labelView->ResizeToPreferred();
	item = layout->AddView( labelView, 0, 0, 3, 1 );
	if ( !item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_VERTICAL_CENTER ) );

	// Construct color control
	BMessage* toSend = new BMessage( kColorChanged );
	if ( !toSend ) { /* Panic! */ exit(1); }
	colorControl = new BColorControl( BPoint( 0, 0 ),
								      B_CELLS_32x8,
								      4.0,
								      "Color Control",
								      toSend );
	if ( !colorControl )
	{
		/* Panic! */
		exit(1);
	}
	colorControl->GetPreferredSize( &width, &height );
	colorControl->ResizeTo( width, height );
	colorControl->SetTarget( this );
	item = layout->AddView( colorControl, 0, 1, 3, 1 );
	if ( !item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );

	// Construct last two buttons
	// Revert button
	toSend = new BMessage( kColorReverted );
	if ( !toSend ) { /* Panic! */ exit(1); }
	revertButton = new BButton( BRect( 0, 0, 1, 1),
								"Revert button",
								"Revert",
								toSend );
	if ( !revertButton ) { /* Panic! */ exit(1); }
	revertButton->ResizeToPreferred();
	
	// Ok button
	toSend = new BMessage( kColorSelected );
	if ( !toSend ) { /* Panic! */ exit(1); }
	okButton = new BButton( BRect( 0, 0, 1, 1),
						    "Ok button",
						    "Ok",
						    toSend,
						    B_FOLLOW_RIGHT | B_FOLLOW_TOP );
	if ( !okButton ) { /* Panic! */ exit(1); }
	okButton->ResizeToPreferred();
	
	// Attach the buttons to current layout
	item = layout->AddView( revertButton, 0, 2 );
	if ( ! item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_MIDDLE ) );
		// Note I'm skipping one cell - this is for showing current color!
	item = layout->AddView( okButton, 2, 2 );
	if ( ! item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );
	
	// Cancel label
	BStringView* cancelLabel = new BStringView( BRect( 0, 0, 1, 1 ),
												"Cancel label",
												"To cancel, click \"Revert\" and then \"Ok\"." );
	if ( !cancelLabel )
	{
		/* Panic! */
		exit( 1 );
	}
	cancelLabel->ResizeToPreferred();
	item = layout->AddView( cancelLabel, 0, 3, 3, 1 );
	item->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
	
	// Make "Ok" button the default
	okButton->MakeDefault( true );
	
	layout->Relayout( true );
	layoutSize = layout->PreferredSize();
	this->ResizeTo( layoutSize.width, layoutSize.height );
	background->ResizeTo( layoutSize.width, layoutSize.height );	
	
	colorControl->SetValue( originalColor );
	colorControl->Invoke();

	// Show the window
	if ( enableEditingLabel )
	{
		this->labelView->MakeFocus( true );
	}
	else
	{
		this->colorControl->MakeFocus( true );
	}
	this->CenterOnScreen();
	this->Show();
	
	
	this->PostMessage( kColorChanged );
}	// <-- end of constructor from Category.

 
/*!
 *	\brief		Constructor.
 *	\param[in]	corner	One of the corners of the window IN SCREEN COORDINATES!
 *	\param[in]	label	Label of the color. (Usually name of the category).
 *	\param[in]	enableEditingLabel	If "true", label can be edited. If "false", it's constant.
 *	\param[in]	title			Title of the window
 *	\param[in]	defaultColor	Original color. Defaults to black.
 *	\param[in]	targetLooper	The target which receives message with the results.
 *	\param[in]	currentScreen	Defines the screen the program runs in. Usually it's
 *								B_MAIN_SCREEN_ID.
 *	\param[in]	message			The template message to be used. If this parameter is NULL,
 *								a new message is constructed.
 */
ColorUpdateWindow::ColorUpdateWindow( BPoint corner,
								  	  BString& label,
								  	  rgb_color &defaultColor,
								  	  bool enableEditingLabel,
								  	  const char* title,
								      BHandler *targetHandler,
								      screen_id currentScreen,
								   	  BMessage* message )
	:
	BWindow( BRect( 100, 100, 300, 500 ),
			 title,
			 B_MODAL_WINDOW,
			 B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_NOT_MINIMIZABLE | B_ASYNCHRONOUS_CONTROLS | B_WILL_ACCEPT_FIRST_CLICK ),
	originalString( label ),
	originalColor( defaultColor),
	labelView( NULL ),
	okButton( NULL ),
	revertButton( NULL ),
	messageToSend( NULL ),
	target( targetHandler ),
	dirty( false )
{
	BSize layoutSize;
	BLayoutItem* item = NULL;
	float width, height, dontCare;
	BView* background = new BView( this->Bounds(),
								   "Background",
								   B_FOLLOW_LEFT | B_FOLLOW_TOP,
								   B_FRAME_EVENTS | B_WILL_DRAW );
	if ( !background )
	{
		/* Panic! */
		exit(1);
	}
	background->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );	
	
	this->enableEditingLabel = enableEditingLabel;
	
	/* There are three parts in the interface of the control.
	 * Upmost is the label, which, according to the user's settings,
	 * can be editable or not.
	 * Next is the color control.
	 * Last is the row of two buttons, Revert and Ok, and current color displayed between.
	 */

	// Debugging
	printf( "Color Selected = %u, Color Reverted = %u.\n",
			kColorSelected,
			kColorReverted );

	// Construct background view and layout
	BGridLayout* layout = new BGridLayout( B_VERTICAL );
	if ( !layout ) { /* Panic! */ exit(1); }
	layout->SetInsets( 5, 5, 5, 5 );
	
	background->SetLayout( layout );
	this->AddChild( background );

	// Constructing the name label, editable or not.
	if ( enableEditingLabel )
	{
		labelView = new BTextControl( BRect(0, 0, 1, 1),
									  "Label",
								   	  NULL,
								   	  label.String(),
								   	  NULL );
	} else {
		labelView = new BStringView( BRect (0, 0, 1, 1),
									 "Label",
									 label.String() );
	}
	if ( !labelView )
	{
		/* Panic! */
		exit(1);
	}
	labelView->ResizeToPreferred();
	item = layout->AddView( labelView, 0, 0, 3, 1 );
	if ( !item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_VERTICAL_CENTER ) );
//	item->SetExplicitMinSize( BSize( width, height ) );
	
	// Construct color control
	BMessage* toSend = new BMessage( kColorChanged );
	if ( !toSend ) { /* Panic! */ exit(1); }
	colorControl = new BColorControl( BPoint( 0, 0 ),
								      B_CELLS_32x8,
								      4.0,
								      "Color Control",
								      toSend );
	if ( !colorControl )
	{
		/* Panic! */
		exit(1);
	}
	colorControl->GetPreferredSize( &width, &height );
	colorControl->ResizeTo( width, height );
	colorControl->SetTarget( this );
	item = layout->AddView( colorControl, 0, 1, 3, 1 );
	if ( !item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );

	// Construct last two buttons
	// Revert button
	toSend = new BMessage( kColorReverted );
	if ( !toSend ) { /* Panic! */ exit(1); }
	revertButton = new BButton( BRect( 0, 0, 1, 1),
								"Revert button",
								"Revert",
								toSend );
	if ( !revertButton ) { /* Panic! */ exit(1); }
	revertButton->ResizeToPreferred();
	
	// Ok button
	toSend = new BMessage( kColorSelected );
	if ( !toSend ) { /* Panic! */ exit(1); }
	okButton = new BButton( BRect( 0, 0, 1, 1),
						    "Ok button",
						    "Ok",
						    toSend,
						    B_FOLLOW_RIGHT | B_FOLLOW_TOP );
	if ( !okButton ) { /* Panic! */ exit(1); }
	okButton->ResizeToPreferred();
	
	// Attach the buttons to current layout
	item = layout->AddView( revertButton, 0, 2 );
	if ( ! item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_MIDDLE ) );
		// Note I'm skipping one cell - this is for showing current color!
	item = layout->AddView( okButton, 2, 2 );
	if ( ! item ) { /* Panic! */ exit(1); }
	item->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );
	
	// Make "Ok" button the default
	okButton->MakeDefault( true );
	
	// Cancel label
	BStringView* cancelLabel = new BStringView( BRect( 0, 0, 1, 1 ),
												"Cancel label",
												"To cancel, click \"Revert\" and then \"Ok\"." );
	if ( !cancelLabel )
	{
		/* Panic! */
		exit( 1 );
	}
	cancelLabel->ResizeToPreferred();
	item = layout->AddView( cancelLabel, 0, 3, 3, 1 );
	item->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
	
	// Now, find the correct place for this window. 
	// We have one of the corners from constructor, we need to position the window
	// in such manner that it will be fully visible and keep one of the corners in
	// the specified point.
	layout->Relayout( true );
	layoutSize = layout->PreferredSize();
	this->ResizeTo( layoutSize.width, layoutSize.height );
	background->ResizeTo( layoutSize.width, layoutSize.height );
	
	float windowWidth = layoutSize.width, windowHeight = layoutSize.height;
	BScreen* mainScreen = new BScreen( currentScreen ); 	// Get access to current screen
	BRect currentDisplayDimensions = mainScreen->Frame();	// Obtain the width and height of screen
	
	// The following booleans uniquely define where the window be located regarding
	// given corner.
	bool leftFromCorner = false, upFromCorner = false;
	
	// Check where the window should span regarding to the corner
	if ( corner.x + windowWidth >= currentDisplayDimensions.IntegerWidth() )
	{
		if ( corner.x - windowWidth < 0 )
		{
			corner.x = 0;
			leftFromCorner = false;
		}
		else
		{
			leftFromCorner = true;
		}
	}
	
	if ( corner.y + windowHeight >= currentDisplayDimensions.IntegerHeight() )
	{
		if ( corner.y - windowHeight < 0 )
		{
			corner.y = 0;
			upFromCorner = false;
		}
		else
		{
			upFromCorner = true;
		}	
	}
	
	// Calculate new top-left corner of the window
	if ( leftFromCorner ) 	{ corner.x -= windowWidth; }
	if ( upFromCorner )		{ corner.y -= windowHeight; }
	
	// Move the window to calculated position
	this->MoveTo( corner );
	colorControl->SetValue( originalColor );
	colorControl->Invoke();
	if ( enableEditingLabel )
	{
		this->labelView->MakeFocus( true );
	}
	else
	{
		this->colorControl->MakeFocus( true );
	}
	
	// Show the window
	this->Show();
}	// <-- end of constructor for ColorUpdateWindow



/*!	
 *	\brief			Default destructor
 */
ColorUpdateWindow::~ColorUpdateWindow()
{
	this->Hide();
	
	int children = this->CountChildren();
	BView* tempView = NULL;
	
	for ( int i = 0; i < children; ++i )
	{
		tempView = this->ChildAt( i );
		this->RemoveChild( tempView );
		delete tempView;
	}
	
	colorControl = NULL;
	labelView = okButton = revertButton = NULL;
	originalString.Truncate( 0 );	
}	// <-- end of default destructor



/*!	
 *	\brief			Main function
 */
void ColorUpdateWindow::MessageReceived( BMessage* in )
{
	BAlert* al = NULL;				//!< User for notifying the user (as opposed to developer)
	BTextView* textView = NULL;		
	BView* background = NULL;		//!< Background of the ColorUpdateWindow.
	BMessenger* mesg = NULL;		//!< Used to send messages to other applications.
	status_t errorCode = B_OK;
	BString currentString( this->originalString );	//!< Name of the category
	rgb_color previousHighColor;
	BRect tempRect;
	
	switch ( in->what )
	{
		case kColorSelected:
			/* The user had make his choise. */
			/* Checking if the color has changed... */
			if ( colorControl )
			{
				currentColor = colorControl->ValueAsColor();
				
				if ( currentColor != originalColor )
				{
					this->dirty = true;
				}
			}
			
			/* Checking if the string has changed... */
			if ( enableEditingLabel )
			{
				if ( labelView )
				{
					// textView = ( BTextView* )labelView;	// For matter of simplicity only
					currentString.SetTo( ( ( BTextControl* )labelView )->Text() );
					
					if ( currentString != originalString )	// If there were changes
					{
						if ( ! utl_CheckStringValidity( currentString ) )
						{
							// User has changed the string to an invalid one -
							// he must correct it before proceeding
							al = new BAlert("Error detected!",
											"The string you've entered is invalid. Please correct.",
											"Ok",
											NULL,
											NULL,
											B_WIDTH_AS_USUAL,
											B_STOP_ALERT );
							if ( al ) { 
								al->Go();
								break; 		// Returning to main window
							} else {
								/* Panic! */
								exit(1);
							}						
						}	// <-- end of "if (string is not valid) "
						else
						{
							// String is valid - verify it's different
							if ( currentString != originalString )
							{
								this->dirty = true;
							}							
						}						
					}	// <-- end of "if (user changed the string)					
				}	// <-- end of "if ( BTextView exists )"
			}	// <-- end of "if (user has possibility to change the string)"
		
			// If anything has changed, send the update message. Else, send revert message.
			if ( ! messageToSend )
			{
				if ( this->dirty )
				{
					this->messageToSend = new BMessage( kColorSelected );
				} else {
					this->messageToSend = new BMessage( kColorReverted );
				}
				if ( ! this->messageToSend ) {
					/* Panic! */
					exit(1);
				}
			}
			
			// Stuff the message with needed data
			messageToSend->AddBool( "Dirty", this->dirty );
			messageToSend->AddString ( "Original string", this->originalString );
			if ( this->dirty ) {
				messageToSend->AddString("New string", currentString );
			}

			messageToSend->AddInt32( "Original color", RepresentColorAsUint32( this->originalColor ) );
			messageToSend->AddInt32( "New color", RepresentColorAsUint32( currentColor ) );
			
			// Send the message and close current window
			// mesg = new BMessenger( (BHandler* )target, NULL,  &errorCode );
			mesg = new BMessenger( target, looper );
			
			if ( errorCode == B_OK ) {
				errorCode = mesg->SendMessage( messageToSend, ( BHandler* )target );
			} else {
				utl_Deb = new DebuggerPrintout( "Message wasn't sent" );
			}
			
			this->Quit();
			
			break;

		case kColorReverted:
			/* Returning to original settings */
			if ( colorControl )
			{
				colorControl->SetValue( originalColor );
				colorControl->Invoke();
			}

			if ( enableEditingLabel && this->labelView )
			{
				( (BTextControl*)this->labelView )->SetText( this->originalString.String() );
				( (BTextControl*)this->labelView )->MakeFocus( false );	// Prevent accidental change of text
			}
			
			break;
			
		case kColorChanged:
		
			// We need to reflect the change in color.
			// We'll do it using the current view's high color. For this, we need to
			// back up current high color in order to restore it later.
			background = this->FindView( "Background" );
			if ( ! background )
			{
				utl_Deb = new DebuggerPrintout( "Didn't find background!" );
				return;
			}
			previousHighColor = background->HighColor();

			background->SetHighColor( 0, 0, 1 );	// almost black
			
			tempRect = BRect( ( revertButton->Frame() ).right + 10,
							  ( revertButton->Frame() ).top,
							  ( okButton->Frame() ).left - 10,
							  ( revertButton->Frame() ).bottom );

			// Actual drawing
			if ( this->Lock() ) {
				background->SetPenSize( 1 );
				
				// Create border
				background->StrokeRoundRect( tempRect.InsetBySelf( 1, 1 ), 4, 4 );
				
				// Fill the rectangle
				background->SetHighColor( colorControl->ValueAsColor() );
				background->FillRoundRect( tempRect.InsetBySelf( 1, 1 ), 4, 4 );
				background->Flush();
				this->Flush();
				this->Unlock();	
			}		

			background->SetHighColor( previousHighColor );
			break;
			
		case kCategorySelected:
		
			break;
			
		case kCategoryInvoked:
			break;
			
		default:
			BWindow::MessageReceived( in );
	};
	
}	// <-- end of function "ColorUpdateWindow::MessageReceived"

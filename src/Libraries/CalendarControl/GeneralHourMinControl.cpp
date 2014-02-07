/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <math.h>
 
#include "GeneralHourMinControl.h"
#include "Utilities.h"

// System includes
#include <Alignment.h>
#include <GridLayout.h>
#include	<GroupLayout.h>
#include <Font.h>
#include <Layout.h>
#include <LayoutItem.h>
#include <Looper.h>
#include <MenuItem.h>
#include <Point.h>
#include <Size.h>
#include <String.h>
#include <View.h>
#include <Window.h>

/*--------------------------------------------------------------------------
 * 			Strings used for packing messages to outside world.
 *-------------------------------------------------------------------------*/

const BString		kHoursValueKey( "Hours" );
const BString		kMinutesValueKey( "Minutes" );
const BString		kCheckBoxExistsKey( "CheckBox exists" );
const BString		kCheckBoxValueKey( "CheckBox value" );


/*==========================================================================
**			IMPLEMENTATION OF CLASS GeneralHourMinControl
**========================================================================*/


/*!	\brief		Constructor of class GeneralHourMinControl.
 *		\attention	The constructor does not allow to set initial value of the Control!
 *						The user has to set the value after the control is created! Same goes
 *						for the value of the checkbox.
 *		\param[in]	bounds		Frame of the control.
 *		\param[in]	name			Name of this object instance.
 *		\param[in]	label			What is displayed to the left of the menubar.
 *		\param[in]	checkBoxLabel			Label of the checkbox. If it's an empty string
 *										(default), checkbox is not created. It may be added later.
 *			\see GeneralHourMinControl::SetCheckBoxLabel( const BString label )
 *		\param[in]  invocationMessage		If specified, it is sent every time the control's
 *										represented time is updated.
 */
GeneralHourMinControl::GeneralHourMinControl( BRect bounds,
															const char* name,
															const BString& label,
															const BString& checkBoxLabel,
															BMessage* invocationMessage )
	:
	BControl( bounds, name, label.String(), invocationMessage,
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM,
		B_NAVIGABLE | B_WILL_DRAW | B_FRAME_EVENTS ),
	fRepresentedTime(),
	bDoesCheckBoxExist( false ),	// By default there is no checkbox
	fLabel( label ),
	fHoursLimit( 23 ),
	fMinutesLimit( 55 ),
	fLabelView( NULL ),
	fSelectorMenuBar( NULL ),
	fHoursMenu( NULL ),
	fMinutesMenu( NULL ),
	fCheckBox( NULL )
{
	if ( ( fCheckBoxLabel = checkBoxLabel ) != BString( "" ) ) {
		// There is a checkbox
		bDoesCheckBoxExist = true;
	}

	InitUI ();
}	/* End of constructor */



/*!	
 *	\brief			Main initialization and configuration function for HMControl.
 */
void GeneralHourMinControl::InitUI( void )
{
	/* Creating view for the label */
	fLabelView = new BStringView( BRect(0, 0, 1, 1),
								 			"label",
								 			fLabel.String() );
	if ( !fLabelView )
	{
		/* Panic! */
		exit(1);
	}
	fLabelView->ResizeToPreferred();
	
	/* Initializing the menubar */
	if ( !this->CreateMenuBar() )
	{
		/* Panic! */
		exit(1);
	}
	fSelectorMenuBar->ResizeToPreferred();
	
	/* Initialized the checkbox */
	this->CreateCheckBox();
	
	/* Laying all views out */
	BGroupLayout* layout = new BGroupLayout( B_HORIZONTAL );
	
	if ( !layout )
	{ 
		// Panic! 
		exit(1); 
	}
	layout->SetInsets( 0, 0, 0, 0 );
	layout->SetSpacing( 10 );
	SetLayout( layout );
	
	BLayoutItem *layoutItem;
	BSize size( fLabelView->Bounds().Width(), fLabelView->Bounds().Height() );
	
	fLabelItem = layout->AddView( 0, fLabelView );
	if ( fLabelItem ) {
		fLabelItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP) );
		fLabelItem->SetExplicitMinSize( size );
		fLabelItem->SetExplicitPreferredSize( size );
	}
	
	size.SetWidth( fSelectorMenuBar->Bounds().Width() );
	size.SetHeight( fSelectorMenuBar->Bounds().Height() );
	fMenuBarItem = layout->AddView( 1, fSelectorMenuBar );
	if ( fMenuBarItem ) {
		fMenuBarItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		fMenuBarItem->SetExplicitMaxSize( size );
		fMenuBarItem->SetExplicitPreferredSize( size );
	}
	
	if ( this->fCheckBox )
	{
		this->fCheckBox->ResizeToPreferred();
		size.SetWidth( fCheckBox->Bounds().Width() );
		size.SetHeight( fCheckBox->Bounds().Height() );
		layoutItem = layout->AddView( 2, fCheckBox );
		if ( layoutItem ) {
			layoutItem->SetExplicitPreferredSize( size );
			layoutItem->SetExplicitMaxSize( size );
			layoutItem->SetExplicitAlignment(BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
}	// <-- end of function GeneralHourMinControl::InitUI


/*!	\brief		Delete and recreate the menu bar and its submenus.
 */
void		GeneralHourMinControl::ClearUI( void )
{
	bool needToUnlock = false;
	if ( this->Window() )
	{
		if ( this->Window()->Lock() ) {
			needToUnlock = true;
		}
	}
	
	if ( fSelectorMenuBar ) {
		fSelectorMenuBar->RemoveSelf();
		delete fSelectorMenuBar;		// This also deletes all menus - hours and minutes
		
		fSelectorMenuBar = NULL;
		fHoursMenu = NULL;
		fMinutesMenu = NULL;
	}	
	
	if ( needToUnlock ) {
		// It can't be "true" if there's no window attached to, therefore call is safe
		this->Window()->Unlock();
	}
	
}	// <-- end of function GeneralHourMinControl::ClearUI



/*!	\brief		Destructor for the class GeneralHourMinControl.
 */
GeneralHourMinControl::~GeneralHourMinControl()
{
	if ( this->fSelectorMenuBar ) {
		this->RemoveChild( fSelectorMenuBar );
		delete fSelectorMenuBar;
	}
	
	if ( this->fCheckBox ) {
		this->RemoveChild( fCheckBox );
		delete fCheckBox;
	}
	
	if ( this->fLabelView ) {
		this->RemoveChild( fLabelView );
		delete fLabelView;
	}
}	/* <-- end of destructor for GeneralHourMinControl */



/*!	
 *	\brief		This function initializes the hours menu.
 *	\details		If displaying more then 12 hours, the items
 *					will be laid in matrix (columns of 12 items each).
 *					Else, the items will be laid in column of 12 items or less.
 */
BMenu* GeneralHourMinControl::CreateHoursMenu()
{
	BMenu*		toReturn = NULL;
	BMessage* 	toSend = NULL;
	BMenuItem*	toAdd = NULL;
	BString sb;
	
	// The menu to be created is a static and limited by "hoursLimit".
	if ( fHoursLimit < 12 ) {
		toReturn = new BMenu( "HoursMenu", B_ITEMS_IN_COLUMN );
	}
	else	// We're building a matrix menu
	{
		toReturn = new BMenu( "HoursMenu", B_ITEMS_IN_MATRIX );
	}
	if ( ! toReturn ) {
		/* Panic! */
		exit( 1 );
	}
	toReturn->SetLabelFromMarked( true );
	toReturn->SetRadioMode( true );
	
	if ( fHoursLimit < 12 ) {
		for ( unsigned int i = 0; i <= fHoursLimit; ++i )	
		{
			toSend = new BMessage( kHoursUpdated );
			if ( !toSend ) {
				/* Panic! */
				exit( 1 );
			}
			toSend->AddInt32( "SelectedHour", ( int32 )i );
			
			sb.Truncate( 0 );
			if ( i < 10 ) {
				sb << '0';
			}
			sb << i;
			toAdd = new BMenuItem( sb.String(),
										  toSend );
			if ( ! toAdd )	{
				/* Panic! */
				exit( 1 );						
			}
			if ( i == ( unsigned int )fRepresentedTime.tm_hour ) {
				toAdd->SetMarked( true );
			}
			toReturn->AddItem( toAdd );
		}
	}
	else
	{
		/* Build a matrix menu */
		BuildMatrixMenu( toReturn );
	}

	// Update targets of the menu
	BMenuItem* toCorrect = NULL;
	int limit = toReturn->CountItems();
	for ( int i = 0; i < limit; ++i )
	{
		toCorrect = ( BMenuItem* )toReturn->ItemAt( i );
		if ( toCorrect ) {
			toCorrect->SetTarget( this );
		}	
	}

	return toReturn;	
}	/* <-- end of function GeneralHourMinControl::CreateHoursMenu */



/*!	\brief					Builds a matrix menu according to time limit selected.
 *		\param[out]	toEdit	The menu to be filled.
 *		\note						Assumptions:
 *									It is assumed the menu is already allocated and empty.
 *									If the menu is not in matrix form, the function will fail.
 */
void		GeneralHourMinControl::BuildMatrixMenu( BMenu* toEdit )
{
	if ( !toEdit ) { return; }
	
	BMessage* toSend;
	BMenuItem* toAdd;
	int horizontalSpacing = 5;
	int verticalSpacing = 2;
	int tempVar = 0;
	int menuHeight = 0;
	int menuWidth = 0;
	BPoint	topLeftCorner( horizontalSpacing, verticalSpacing );
	BSize		itemSize;
	BFont font( be_plain_font );
	font_height fh;
	font.GetHeight( &fh );
	
	// What is the maximal length of the item?
	BString sb; sb << fHoursLimit;
	tempVar = sb.CountChars();
	if ( tempVar == 0 ) { tempVar = 1; }
	sb.Truncate( 0 );
	for ( int i = 0; i < tempVar; ++i ) {
		sb << "W";		// "W" is the widest char in latin alphabet
	}
	int itemWidth = ceil( font.StringWidth( sb.String() ) ) + 10; 	// 10 is needed for surrounding labels
	
	// What is height of every single item?
	int itemHeight = ( int )( fh.ascent + fh.descent + fh.leading ) + 2;
	
	/* Preparations before the main loop */	
	itemSize.SetHeight( itemHeight );
	itemSize.SetWidth( itemWidth );
	
	for ( uint32 i = 0; i <= fHoursLimit; ++i ) {
		
		// Message to be sent
		toSend = new BMessage( kHoursUpdated );
		if ( !toSend ) {
			/* Panic! */
			exit( 1 );
		}
		toSend->AddInt32( "SelectedHour", ( int32 )i );
		
		// Label of the item
		sb.Truncate( 0 );
		if ( i < 10 ) {
			sb << '0';
		}
		sb << i;
		
		// Item to be added		
		toAdd = new BMenuItem( sb.String(), toSend );
		if ( !toAdd ) {
			/* Panic! */
			exit( 1 );
		}
		if ( i == ( unsigned int )fRepresentedTime.tm_hour ) {
			toAdd->SetMarked( true );
		}		
		
		// Add the item to the menu
		toEdit->AddItem( toAdd, BRect( topLeftCorner, itemSize ) );
		
		// Moving to the next frame
		if ( ( i+1 ) % 12 == 0 ) {
			// Advancing one column right
			topLeftCorner.x += itemWidth + horizontalSpacing;
			topLeftCorner.y = verticalSpacing;
			
			// Calculating new width of the menu
			if ( menuWidth < topLeftCorner.x + itemWidth + horizontalSpacing ) {
				menuWidth = topLeftCorner.x + itemWidth + horizontalSpacing;
			}
		}
		else
		{
			// Descending one item down
			topLeftCorner.y += itemHeight + verticalSpacing;
			
			// Calculating new height of the menu
			if ( menuHeight < topLeftCorner.y + itemHeight + verticalSpacing ) {
				menuHeight = topLeftCorner.y + itemHeight + verticalSpacing;
			}
		}
		
	}
	
}	// <-- end of function GeneralHourMinControl::BuildMatrixMenu



/*!	
 *	\brief		Create the menu with minutes.
 *	\details		This menu is always layed out in column - there is no hours with
 *					more then 60 minutes.
 */
BMenu* GeneralHourMinControl::CreateMinutesMenu()
{
	BMenu* toReturn = new BMenu("MinutesMenu", B_ITEMS_IN_COLUMN);
	if (!toReturn) {
		/* Panic! */
		exit(1);
	}
	BString itemLabel;
	BMenuItem *toAdd = NULL;
	BMessage  *toSend = NULL;
	int minLimit = ( fMinutesLimit < 55 ) ? fMinutesLimit : 55;
	toReturn->SetRadioMode( true );
	toReturn->SetLabelFromMarked( true );
	for ( int mins = 0; mins <= minLimit; mins += 5 )
	{
		itemLabel.Truncate(0);
		if ( mins < 10 ) {
			itemLabel << '0';
		}
		itemLabel << mins;
		
		toSend = new BMessage( kMinutesUpdated );
		if ( !toSend )
		{
			/* Panic! */
			exit(1);
		} 
		
		toSend->AddInt8("SelectedMinutes", ( int8 )mins);
		
		toAdd = new BMenuItem( itemLabel.String(), toSend );
		if ( ! toAdd ) {
			/* Panic! */
			exit(1);
		}		
		
		toReturn->AddItem( toAdd );
		if ( mins == ( (int)(this->fRepresentedTime.tm_min / 5 )*5 ) )
		{
			toAdd->SetMarked( true );
		}
		
	}	/* <-- end of "for (minutes from 0 to the limit)" */
	
	// Update targets of the menu
	BMenuItem* toCorrect = NULL;
	int limit = toReturn->CountItems();
	for ( int i = 0; i < limit; ++i )
	{
		toCorrect = ( BMenuItem* )toReturn->ItemAt( i );
		if ( toCorrect ) {
			toCorrect->SetTarget( this );
		}	
	}
	
	return toReturn;	
}	/* <-- end of function GeneralHourMinControl::CreateMinutesMenu */




/*!	
 *	\brief		Creates and initializes the menus that manages hours and minutes.
 *	\attention	This function assumes that Control's label is already set.
 */
BMenuBar*	GeneralHourMinControl::CreateMenuBar( void )
{
	BRect frame = this->Bounds();
	BMenuItem* menuItem = NULL;
	BFont plainFont(be_plain_font);
	/* It's assumed the "label" variable is already set */
	frame.left += plainFont.StringWidth( fLabel.String() ) + 5;
	frame.top += 2;
	frame.right -= 2;
	frame.bottom -= 2;
	fSelectorMenuBar = new BMenuBar( frame, 
										  "GeneralHourMinControl menubar",
										  B_FOLLOW_LEFT | B_FOLLOW_TOP,
										  B_ITEMS_IN_ROW,
										  false );
	if (!fSelectorMenuBar) {
		/* Panic! */
		exit(1);
	}
	
	this->fHoursMenu = CreateHoursMenu();
	if ( !fHoursMenu ) {
		/* Panic! */
		exit(1);
	}
	fHoursMenu->SetRadioMode( true );
	fSelectorMenuBar->AddItem( fHoursMenu );
	
	/* Separator - it's disabled symbol ":" */
	menuItem = new BMenuItem( ":", NULL );
	if ( !menuItem ) {
		/* Panic! */
		exit(1);
	}
	menuItem->SetEnabled( false );
	fSelectorMenuBar->AddItem( menuItem );
	
	/* Minutes menu */
	this->fMinutesMenu = CreateMinutesMenu();
	if ( !fMinutesMenu ) {
		/* Panic! */
		exit(1);
	}
	fMinutesMenu->SetRadioMode( true );
	fSelectorMenuBar->AddItem( fMinutesMenu );
	
	return( fSelectorMenuBar );
	
}	/* <-- end of function GeneralHourMinControl::CreateMenuBar */



/*!	\brief		Sends a message to the default target.
 *		\details		The passed argument is the message to be sent. It's up to caller to 
 *						delete this message when this function returns.
 *		\param[in]	in		Pointer to the message to be sent.
 */
void	GeneralHourMinControl::SendInvocationMessage( BMessage* in )
{
	if ( ! in  ) { return; }
	
	bool checkBoxExists = ( bDoesCheckBoxExist && 
									( fCheckBox != NULL ) && 
									fCheckBox->IsEnabled() );
									
	in->AddPointer( "source", this );
	in->AddInt64( "when", system_time() );
	in->AddInt32( kHoursValueKey.String(), this->fRepresentedTime.tm_hour );
	in->AddInt32( kMinutesValueKey.String(), this->fRepresentedTime.tm_min );
	in->AddBool( kCheckBoxExistsKey.String(), checkBoxExists );
	if ( checkBoxExists ) {
		in->AddBool( kCheckBoxValueKey.String(), ( fCheckBox->Value() != 0 ) );
	}
	
	this->Invoke( in );	
	
}	// <-- end of function GeneralHourMinControl::SendInvocationMessage



/*!	\brief		Main function of the control.
 */
void GeneralHourMinControl::MessageReceived( BMessage* in ) {

	uint8 tempUint8 = 0;
	uint32 tempUint32 = 0;
	BMessage* toSend = NULL;
	
	
	if ( this->Message() ) {
		toSend = new BMessage( *this->Message() );	// Got copy
	} else {
		toSend = new BMessage( kGeneralHourMinControlUpdated );	// Created new message
	}
	
	// Note: here toSend may be NULL! Its value is checked in the "SendInvocationMessage".
	
	switch( in->what )
	{
		case kHoursUpdated:
			if ( B_OK != in->FindInt32("SelectedHour", ( int32* )&tempUint32 ) ) {
				/* error */
				return;	
			}
			this->fRepresentedTime.tm_hour = tempUint32;
			
			SendInvocationMessage( toSend );

			break;
		
		case kMinutesUpdated:
			if ( B_OK != in->FindInt8("SelectedMinutes", ( int8* )&tempUint8 ) ) {
				/* error */
				return;
			}
			this->fRepresentedTime.tm_min = tempUint8;
			
			SendInvocationMessage( toSend );
			
			break;
			
		case kCheckBoxToggled:
		
			SendInvocationMessage( toSend );
			
			break;
		
		default:
			BControl::MessageReceived( in );
			break;
	};
	
	return;
}	/* <-- end of function GeneralHourMinControl::MessageReceived */



/*!	\brief			Executed when the control is attached to the window.
 */
void GeneralHourMinControl::AttachedToWindow() {
	// Get the view color of the father
	if ( Parent() ) {
		SetViewColor( Parent()->ViewColor() );
	}
	
	// Attach to window both current view and all of its children
	BView::AttachedToWindow();
	
	// This view should respond to the messages - thus the Looper must know it
	BLooper* looper = (BLooper*)Looper();
	if (looper && looper->LockLooper()) {
		looper->AddHandler((BHandler*) this);
		looper->UnlockLooper();
	}
	
	// Update targets of all children
	BMenu* men; BMenuItem* item;
	if ( fSelectorMenuBar ) {
		for (int i=0; i<fSelectorMenuBar->CountItems(); i++) {
			if ( (men = fSelectorMenuBar->SubmenuAt(i) ) != NULL ) {
				men->SetTargetForItems(this);	
			} else {
				if ( (item = fSelectorMenuBar->ItemAt(i)) != NULL ) {
					item->SetTarget(this);	
				}	
			}
		}
	}
	
	if ( fCheckBox ) {
		fCheckBox->SetTarget( this );
	}
} // <-- end of function "HourMinControl::AttachedToWindow"



/*!	\brief		Creates and initializes the checkbox.
 */
BCheckBox* GeneralHourMinControl::CreateCheckBox( void )
{
	if ( !this->bDoesCheckBoxExist ) {
		return NULL;
	}
	
	BRect frame(0, 0, 1, 1);
	BMessage* message = new BMessage( kCheckBoxToggled );
	if ( ! message )
	{
		exit(1);	/* Panic! */
	}
	fCheckBox = new BCheckBox( frame,
										"Checkbox Control",
										fCheckBoxLabel.String(),
										message );
	if ( !fCheckBox )
	{
		/* Panic! */
		exit(1);
	}
	
	fCheckBox->ResizeToPreferred();
	fCheckBox->SetTarget( this );
	
	return ( fCheckBox );	
}



/*!	\brief			Return currently selected time.
 *		\param[out]		bHasCheckBox		Set to "true" if the check box exists.
 *		\param[out]		bCheckBoxValue		Set to "true" if the check box is checked.
 *													If there's no check box, this param is not touched.
 */
TimeRepresentation		GeneralHourMinControl::GetCurrentTime( bool* bHasCheckBox,
																					bool* bCheckBoxValue ) const
{
	if ( bHasCheckBox ) {
		*bHasCheckBox = ( bDoesCheckBoxExist && ( fCheckBox != NULL ) );
	}
	
	if ( bCheckBoxValue && ( fCheckBox != NULL ) ) {
		*bCheckBoxValue = ( bool )fCheckBox->Value();
	}
	
	return this->fRepresentedTime;
}	// <-- end of function GeneralHourMinControl::GetCurrentTime



/*!	\brief		Return currently selected time.
 *		\param[out]		hours					Currently selected hours
 *		\param[out]		minutes				Currently selected minutes
 *		\param[out]		bHasCheckBox		Set to "true" if the check box exists.
 *		\param[out]		bCheckBoxValue		Set to "true" if the check box is checked.
 *													If there's no check box, this param is not touched.
 */
void			GeneralHourMinControl::GetCurrentTime( int* hours,
																	int* minutes,
																	bool* bHasCheckBox,
																	bool* bCheckBoxValue ) const
{
	if ( hours ) {
		*hours = this->fRepresentedTime.tm_hour;
	}
	
	if ( minutes ) {
		*minutes = this->fRepresentedTime.tm_min;
	}
	
	if ( bHasCheckBox ) {
		*bHasCheckBox = ( bDoesCheckBoxExist && ( fCheckBox != NULL ) );
	}
	
	if ( bCheckBoxValue && ( fCheckBox != NULL ) ) {
		*bCheckBoxValue = ( bool )fCheckBox->Value();
	}
}	// <-- end of function GeneralHourMinControl::GetCurrentTime



/*!	\brief		Set current time using Time Representation structure
 *		\details		Shortcut to another function.
 */
void			GeneralHourMinControl::SetCurrentTime( const TimeRepresentation& trIn )
{
	this->SetCurrentTime( trIn.tm_hour, trIn.tm_min );	
}	// <-- end of function GeneralHourMinControl::SetCurrentTime



/*!	\brief		Set current time using two integers
 *		\details		Shortcut to another function.
 */
void			GeneralHourMinControl::SetCurrentTime( int hours, int minutes )
{
	if ( hours >= 0 ) {
		if ( ( unsigned int )hours > fHoursLimit ) {
			hours = fHoursLimit;
		}
	} else {
		hours = 0;
	}
	this->fRepresentedTime.tm_hour = hours;
	
	if ( minutes > 0 )
	{
		// If minutes are more then 55, set them to 55.
		if ( ( unsigned int )minutes > fMinutesLimit ) {
			this->fRepresentedTime.tm_min = fMinutesLimit;
		}
		else	// Minutes are between 0 and 55
		{
			// Minutes are always multiplication of 5
			minutes = ( int )( floor( ( minutes + 2 )/ 5 ) * 5 );
			this->fRepresentedTime.tm_min = minutes;
		}
	}
	else
	{
		this->fRepresentedTime.tm_min = 0;
	}
	
	UpdateSelectedValuesInMenus();
}	// <-- end of function GeneralHourMinControl::SetCurrentTime



/*!	\brief		Updates menus after a new time was set.
 *		\details		It is assumed that the fRepresentedTime was already updated.
 */
void 			GeneralHourMinControl::UpdateSelectedValuesInMenus()
{
	BMenuItem* 	toSelect = NULL;
	BString	 	sb;
	
	if ( fRepresentedTime.tm_hour < 10 ) {
		sb << '0';
	}
	sb << fRepresentedTime.tm_hour;
	if ( fHoursMenu &&
		  ( toSelect = fHoursMenu->FindItem( sb.String() ) ) != NULL )
	{
		toSelect->SetMarked( true );
	}
	
	sb.Truncate( 0 );
	if ( fRepresentedTime.tm_min < 10 )
	{
		sb << '0';
	}
	sb << fRepresentedTime.tm_min;
	if ( fMinutesMenu &&
	     ( toSelect = fMinutesMenu->FindItem( sb.String() ) ) != NULL )
	{
		toSelect->SetMarked( true );
	}
}	// <-- end of function GeneralHourMinControl::UpdateSelectedValuesInMenus



/*!	\brief		Returns check box value
 *		\param[out]		bHasCheckBox		"true" if there is a checkbox.
 */
bool		GeneralHourMinControl::GetCheckBoxValue( bool* bHasCheckBox ) const
{
	if ( bDoesCheckBoxExist && bHasCheckBox ) {
		*bHasCheckBox = ( bDoesCheckBoxExist && ( fCheckBox != NULL ) );
	}
	
	if ( fCheckBox != NULL ) {
		return ( bool )fCheckBox->Value();
	}
	else
	{
		return false;
	}
	
}	// <-- end of function GeneralHourMinControl::GetCheckBoxValue



/*!	\brief		Sets the check box value only in case check box exists.
 *		\param[in]	toSet		The value to be set.
 */
void		GeneralHourMinControl::SetCheckBoxValue( bool toSet )
{
	if ( bDoesCheckBoxExist && ( fCheckBox != NULL ) && ( ( bool )fCheckBox->Value() != toSet ) )
	{
		fCheckBox->SetValue( toSet );
	}
}	// <-- end of function GeneralHourMinControl::SetCheckBoxValue



/*!	\brief		Updates the label of the checkbox.
 *		\details		Also creates or deletes the checkbox if needed.
 */
void		GeneralHourMinControl::SetCheckBoxLabel( const BString& toSet )
{
	BGroupLayout* layout = ( BGroupLayout* )this->GetLayout();
	BLayoutItem* layoutItem;
	BSize size;
	bool unlockNeeded = false;
	
	// This check also covers the case where no checkbox existed, and no label is provided.
	if ( ( toSet == fCheckBoxLabel ) ||
		  ( ( toSet == BString( "" ) && !bDoesCheckBoxExist ) ) )
		return;

	if ( toSet.CountChars() == 0 )	// Removing the checkbox
	{
		bDoesCheckBoxExist = false;
		
		if ( this->Window() )
			if ( this->Window()->Lock() )
				unlockNeeded = true;

		fCheckBox->RemoveSelf();
		delete fCheckBox;
		fCheckBox = NULL;
		
		if ( unlockNeeded )
			this->Window()->Unlock();
			
		this->InvalidateLayout();
		this->Invalidate();
		return;
	}
	else 			// The checkbox does not exist or has another label
	{
		if ( bDoesCheckBoxExist ) {	// The checkbox had another label
			fCheckBoxLabel = toSet;
			fCheckBox->SetLabel( toSet.String() );
			fCheckBox->ResizeToPreferred();
			layoutItem = layout->ItemAt( 2 );
			if ( layoutItem ) {
				size.SetWidth( fCheckBox->Frame().Width() );
				size.SetHeight( fCheckBox->Frame().Height() );
				layoutItem->SetExplicitPreferredSize( size );
				layoutItem->SetExplicitMaxSize( size );
				layoutItem->SetExplicitAlignment(BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
			}
			this->Relayout();
			this->Invalidate();
			return;
		}
		
		// The checkbox did not exist
		bDoesCheckBoxExist = true;
		fCheckBoxLabel = toSet;
		if ( !CreateCheckBox() )
		{
			/* Panic! */
			exit( 1 );
		}
		
		if ( layout ) {
			if ( this->Window() )
				if ( this->Window()->Lock() )
					unlockNeeded = true;
			
			// I assume the menubar always exists
			fCheckBox->ResizeToPreferred();
			layoutItem = layout->AddView( fCheckBox );
			if ( layoutItem ) {
				size.SetWidth( fCheckBox->Bounds().Width() );
				size.SetHeight( fCheckBox->Bounds().Height() );
				layoutItem->SetExplicitPreferredSize( size );
				layoutItem->SetExplicitMaxSize( size );
				layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
			}
			this->Relayout();
			this->Invalidate();
			
			if ( unlockNeeded )
				this->Window()->Unlock();
		}
	}	// <-- end of "else ( we should add the checkbox ) "

}	// <-- end of function GeneralHourMinControl::SetCheckBoxLabel


/*!	\brief		Obtain the check box label
 */
BString		GeneralHourMinControl::GetCheckBoxLabel( void ) const
{
	return fCheckBoxLabel;
}	// <-- end of function GeneralHourMinControl::GetCheckBoxLabel


/*!	\brief				Set the label's control
 *		\param[in]	in		Label to set
 */
void			GeneralHourMinControl::SetLabel( const char* in )
{
	this->fLabel.SetTo( in );
	if ( this->fLabelView ) {
		fLabelView->SetText( in );
	}	
	BControl::SetLabel( in );
}	// <-- end of function GeneralHourMinControl::SetLabel


/*!	\brief				Another function for setting the label
 *		\param[in]	in 	Label to set.
 */
void			GeneralHourMinControl::SetLabel( const BString& in )
{
	BSize size;
	this->fLabel.SetTo( in );
	if ( this->fLabelView )
	{
		this->fLabelView->SetText( in.String() );
		fLabelView->ResizeToPreferred();
		if ( fLabelItem )	{
			size.Set( fLabelView->Bounds().Width(), fLabelView->Bounds().Height() );
			fLabelItem->SetExplicitMinSize( size );
			fLabelItem->SetExplicitPreferredSize( size );
		}
	}
	BControl::SetLabel( in.String() );
	this->Relayout();
	this->InvalidateLayout();
	if ( this->Window() ) {
		this->Window()->UpdateIfNeeded();
	}
}	// <-- end of function GeneralHourMinControl::SetLabel


/*!	\brief		Sets the hours limit.
 *		\details		Hours limit may be everything from 0 and up.
 */
void		GeneralHourMinControl::SetHoursLimit( unsigned int limit )
{
	bool bNeedsUnlock = false;
	
	this->fHoursLimit = limit;	// No need to check; "unsigned" means "0 and greater".
	
	// Can't display anything more then the limit
	if ( ( unsigned int )this->fRepresentedTime.tm_hour > this->fHoursLimit ) {
		this->fRepresentedTime.tm_hour = this->fHoursLimit;
	}
	
	/* We need also to recreate the minutes menu. */
		// Lock the window to perform UI change
	if ( this->Window() ) {
		if ( this->Window()->Lock() ) {
			bNeedsUnlock = true;
		}
	}
	
	ClearUI();	// Deleted both hours and minutes menu and the menubar
	
	if ( ! CreateMenuBar() ) {
		/* Panic! */
		exit( 1 );
	}
	
	BGroupLayout* layout = ( BGroupLayout* )this->GetLayout();
	
	BLayoutItem *layoutItem;
	BSize size;
	
	// I assume the label always exists, therefore add the menubar at index 1.
	fSelectorMenuBar->ResizeToPreferred();
	size.SetWidth( fSelectorMenuBar->Bounds().Width() );
	size.SetHeight( fSelectorMenuBar->Bounds().Height() );
	layoutItem = layout->AddView( 1, fSelectorMenuBar );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		layoutItem->SetExplicitMaxSize( size );
		layoutItem->SetExplicitPreferredSize( size );
	}
	InvalidateLayout();
	Invalidate();
	
	if ( bNeedsUnlock ) {
		if ( this->Window() ) {
			this->Window()->Unlock();
		}
	}

}	// <-- end of function GeneralHourMinControl::SetHoursLimit



/*!	\brief		Sets the minutes limit.
 *		\details		The minutes limit can't be less then 0 or more then 55.
 */
void		GeneralHourMinControl::SetMinutesLimit( unsigned int limit )
{
	bool bNeedsUnlock = false;
	
	if ( limit > 55 ) {
		limit = 55;
	}
	else	// Minutes are between 0 and 55
	{
		// Minutes are always multiplication of 5
		// Choosing the closest value.
		limit = ( int )( floor( ( limit + 2 ) / 5 ) * 5 );
	}
	
	// Update only if needed
	if ( limit != fMinutesLimit )
	{
		fMinutesLimit = limit;
		
		// Can't display anything more then the limit
		if ( ( unsigned int )this->fRepresentedTime.tm_min > this->fMinutesLimit ) 
		{
			this->fRepresentedTime.tm_min = this->fMinutesLimit;
		}
		
		// Can't display anything less then 0
		if ( this->fRepresentedTime.tm_min < 0 )
		{
			this->fRepresentedTime.tm_min = 0;
		}
		
		/* We need also to recreate the minutes menu. */
			// Lock the window to perform UI change
		if ( this->Window() ) {
			if ( this->Window()->Lock() ) {
				bNeedsUnlock = true;
			}
		}
		
		ClearUI();	// Deleted both hours and minutes menu and the menubar
		
		if ( ! CreateMenuBar() ) {	// Created both hours and minutes menu and the menubar
			/* Panic! */
			exit( 1 );
		}
		
		BGroupLayout* layout = ( BGroupLayout* )this->GetLayout();
		
		BLayoutItem *layoutItem;
		BSize size;
		
		// I assume the label always exists, therefore add the menubar at index 1.
		fSelectorMenuBar->ResizeToPreferred();
		size.SetWidth( fSelectorMenuBar->Bounds().Width() );
		size.SetHeight( fSelectorMenuBar->Bounds().Height() );
		layoutItem = layout->AddView( 1, fSelectorMenuBar );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
			layoutItem->SetExplicitMaxSize( size );
			layoutItem->SetExplicitPreferredSize( size );
		}
		InvalidateLayout();
		Invalidate();
		
		if ( bNeedsUnlock ) {
			if ( this->Window() ) {
				this->Window()->Unlock();
			}
		}
	}	// <-- end of "actually updated the minutes limit"
	
}	// <-- end of function GeneralHourMinControl::SetMinutesLimit



/*!	\brief		Sets the object's enabled state
 *		\param[in]	newState		"true" if enabling the object, "false" if disabling.
 */
void		GeneralHourMinControl::SetEnabled( bool newState )
{	
	// Checking if we have anything to do
	if ( this->IsEnabled() == newState )
		return;
		
	BControl::SetEnabled( newState );
		
	if ( fSelectorMenuBar )
		fSelectorMenuBar->SetEnabled( newState );
		
	if ( fHoursMenu )
		fHoursMenu->SetEnabled( newState );
		
	if ( fMinutesMenu )
		fMinutesMenu->SetEnabled( newState );
		
	if ( fCheckBox )
		fCheckBox->SetEnabled( newState );
		
}	// <-- end of GeneralHourMinControl::SetEnabled


/*!	\brief		Set value of the control
 *		\details		Hours, minutes and checkbox value are packed into single uint32 variable.
 *		\param[in]	in		Value to set - unsigned 32-bits integer!
 */
void		GeneralHourMinControl::SetValue( int32 in )
{
	uint32	toSet = ( uint32 )in;
	
	unsigned int hours 	= GeneralHourMinControl_GET_HOURS( toSet );
	unsigned int minutes	= GeneralHourMinControl_GET_MINUTES( toSet );
	this->SetCurrentTime( hours, minutes );
	
	if ( bDoesCheckBoxExist && this->fCheckBox )
	{
		this->SetCheckBoxValue( GeneralHourMinControl_GET_CHECK_BOX_VALUE( toSet ) );
	}
	
}	// <-- end of function GeneralHourMinControl::SetValue



/*!	\brief		Return value of the control
 *		\details		The checkbox data, hours and minutes are packed into single uint32.
 *		\attention	The returned data is actually unsigned int of 32 bits!
 */
int32			GeneralHourMinControl::Value() const
{
	uint32 toReturn = 0;
	
	if ( bDoesCheckBoxExist && fCheckBox ) {
		toReturn |= ( fCheckBox->Value() << 30 );	// Value
		toReturn |=	( 1 << 31 );						// Checkbox exists
	}
	
	toReturn |= ( ( fRepresentedTime.tm_hour << 16 ) & GeneralHourMinControl_HOURS_MASK );
	toReturn |= ( fRepresentedTime.tm_min & GeneralHourMinControl_MINUTES_MASK );
	
	return ( int32 )toReturn;
}	// <-- end of function "GeneralHourMinControl::Value"

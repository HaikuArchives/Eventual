/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "TimeHourMinControl.h"

// System includes
#include	<GroupLayout.h>
#include <Font.h>
#include <Layout.h>
#include <Menu.h>
#include <MenuItem.h>
#include <String.h>
#include <View.h>

// POSIX includes
#include <stdio.h>

/*==========================================================================
**			IMPLEMENTATION OF CLASS TimeHourMinControl
**========================================================================*/

/*!	\brief	Constructor of class TimeHourMinControl
 *		\details	Actually, it just call the constructor of GeneralHourMinControl
 */
TimeHourMinControl::TimeHourMinControl( BRect frame,
													 const char* name,
													 const BString& label,
													 BMessage* message )
	:
	GeneralHourMinControl( frame, name, label, BString( "PM" ), message )
{
	// We need to find out if the user uses 24 hours clock
	TimePreferences* time_preferences =	pref_GetTimePreferences();
	if ( !time_preferences ) {
		// By default, use 24h clock
		bUse24hClock = false;
	}
	else
	{
		// Found the preferences successfully
		bUse24hClock = time_preferences->Get24hClock();
	}
	
	
	if ( bUse24hClock ) {		
		// If we're using 24h clock, the "PM" checkbox should be disabled
		if ( this->fCheckBox ) {
			this->fCheckBox->SetEnabled( false );
		}
	}
	
	// Set the limit on hours - it's always 23
	GeneralHourMinControl::SetHoursLimit( 23 );
	// Set the limit of minutes - it's always 55.
	GeneralHourMinControl::SetMinutesLimit( 55 );
	
}	// <-- end of constructor for TimeHourMinControl



/*!	\brief		Sets current time
 */
void		TimeHourMinControl::UpdateSelectedValuesInMenus( void )
{
	
	// Firstly, the checkbox value changes according to selected hour
	if ( fRepresentedTime.tm_hour >= 12 ) {
		if ( fCheckBox ) {
			fCheckBox->SetValue( 1 );
		}
	}
	else
	{
		if ( fCheckBox ) {
			fCheckBox->SetValue( 0 );
		}
	}
	
	// If the clock is 24-hours, then...
	if ( bUse24hClock ) {
		// ..the base class version is good enough - it deals the matrix menu good
		return GeneralHourMinControl::UpdateSelectedValuesInMenus();
	}
	
	// Else, we should write another implemenatation.
	// It's basically the same as the base class implementation, just the label of
	// the item to search for is created using a calculated value, and not the hour directly.
	BMenuItem* 	toSelect = NULL;
	int			menuItemLabel;
	BString	 	sb;
	
	if ( fRepresentedTime.tm_hour > 12 ) {
		menuItemLabel = fRepresentedTime.tm_hour - 12 ;
	}
	else
	{
		menuItemLabel = fRepresentedTime.tm_hour;
	}
	
	if ( menuItemLabel < 10 ) {
		sb << '0';
	}
	sb << menuItemLabel;
	if ( fHoursMenu &&
		  ( toSelect = fHoursMenu->FindItem( sb.String() ) ) != NULL )
	{
		toSelect->SetMarked( true );
	}
	
	// No changes in the minutes section
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
};	// <-- end of function TimeHourMinControl::UpdateSelectedValuesInMenus



/*!	\brief		This function sets the control as enabled or disabled.
 *		\details		Actually, the only difference from the base class function is
 *						enabling of the PM checkbox. If the clock is 24-hours, the checkbox
 *						should remain disabled no matter what.
 *		\param[in]		newState		New state (true = enabled)
 */
void		TimeHourMinControl::SetEnabled( bool newState )
{
	if ( this->IsEnabled() == newState ) {
		return;
	}
	
	BControl::SetEnabled( newState );
	
	if ( fSelectorMenuBar )
		fSelectorMenuBar->SetEnabled( newState );
		
	if ( fHoursMenu )
		fHoursMenu->SetEnabled( newState );
		
	if ( fMinutesMenu )
		fMinutesMenu->SetEnabled( newState );
		
	if ( fCheckBox && !bUse24hClock )
		fCheckBox->SetEnabled( newState );

}	// <-- end of function TimeHourMinControl::SetEnabled



/*!	
 *	\brief		Create the hours menu corresponding to AM / PM differentiation
 *	\param[in]	pbPM	Pointer to boolean. It allows to pass three values:
 *						"true", "false" and NULL. If NULL (default), the menu
 *						to be built is 24-hours. If "false", the menu represents
 *						AM. If "true", the menu represents PM.
 */
BMenu* TimeHourMinControl::CreateHoursMenu( void )
{
	BMenu*	toReturn = NULL;
	BString	hourLabel;
	BMenuItem* toAdd;
	BMessage* toSend;
	
	if ( bUse24hClock )	// 24-hours menu
	{
		toReturn = new BMenu( "HoursMenu", B_ITEMS_IN_MATRIX );
	
		if ( ! toReturn ) {
			/* Panic! */
			exit( 1 );
		}
		toReturn->SetLabelFromMarked( true );
		toReturn->SetRadioMode( true );
		
		GeneralHourMinControl::BuildMatrixMenu( toReturn );
		
	}	// <-- end of "if (the required menu is for 24H clock)"
	
	else	// 12-hours clock
	{
		/* The solution is building 12-rows menu from 0 or 12 through 1 to 11. */
		
		// Prepare the menu
		toReturn = new BMenu( "HoursMenu", B_ITEMS_IN_COLUMN );
		if ( !toReturn ) { /* Panic! */ exit(1); }
		toReturn->SetRadioMode( true );
		toReturn->SetLabelFromMarked( true );
		
		for ( int hourVariable = 0; hourVariable <= 11; hourVariable++ )
		{
			// Prepare the label
			hourLabel.Truncate( 0 );	// Clear the previously used label
			if ( ( fRepresentedTime.tm_hour > 12 ) && ( hourVariable == 0 ) )
			{
				// PM part starts from 12
				hourLabel << "12";
			} else if ( hourVariable < 10 ) {
				hourLabel << '0' << hourVariable;
			} else {
				hourLabel << hourVariable;
			}				
			
			// Prepare the message
			toSend = new BMessage( kHoursUpdated );
			if ( ! toSend ) { 	/* Panic! */ exit (1); }
			if ( fRepresentedTime.tm_hour < 12 ) {
				toSend->AddInt32( "SelectedHour", ( int32 )hourVariable );
			} else {
				toSend->AddInt32( "SelectedHour", ( int32 )( hourVariable + 12 ) );
			}
			
			// Prepare the menu item
			toAdd = new BMenuItem( hourLabel.String(), toSend );
			if ( ! toAdd ) { /* Panic! */ exit(1); }

			// Add the menu item to the menu
			toReturn->AddItem( toAdd );
			
			// Maybe this item should be selected?
			if ( ( hourVariable == fRepresentedTime.tm_hour ) ||
			     ( hourVariable + 12 == fRepresentedTime.tm_hour ) )
			{
				toAdd->SetMarked( true );
			}
			
		}	// <-- end of "for (hours from 0 to 11)"
		
	}	// <-- end of 12-hours clock
	
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
	
}	// <-- end of function "HourMinControl::CreateAMHoursMenu"



/*!	\brief		Main function of the control.
 */
void		TimeHourMinControl::MessageReceived( BMessage* in ) {
	uint32 command = in->what;
	bool bUnlockIsNeeded = false;
	BMessage* toSend = NULL;
	
	
	if ( this->Message() ) {
		toSend = new BMessage( *this->Message() );	// Got copy
	} else {
		toSend = new BMessage( kTimeControlUpdated );	// Created new message
	}
	
	switch( command )
	{
		case kCheckBoxToggled:
			// Assumption: The checkbox can't be toggled if 24h clock is used.
			
			if ( ( fCheckBox ) && ( fCheckBox->Value() == 0 ) ) {
				// Checkbox was reset
				if ( fRepresentedTime.tm_hour >= 12 ) {
					fRepresentedTime.tm_hour -= 12;
				}
			}
			else 	// Checkbox was set
			{
				if ( fRepresentedTime.tm_hour < 12 ) {
					fRepresentedTime.tm_hour += 12;
				}
			}
			
			// If the clock is AM/PM type, we need to update the menu too.
			if ( !bUse24hClock )
			{
				if ( fSelectorMenuBar )
				{
					if ( this->Window() && this->Window()->Lock() ) 
						bUnlockIsNeeded = true;

					GeneralHourMinControl::ClearUI();	// Deleted both hours and minutes menu and the menubar
					
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
	
					if ( bUnlockIsNeeded )
						this->Window()->Unlock();

				}	// <-- end of "if ( menu bar exists )"
				
			}	// <-- end of "if ( clock is am/pm )"
			
			SendInvocationMessage( toSend );
			
			break;
			
		default:
			GeneralHourMinControl::MessageReceived( in );
			break;
	};

	// Updating the checkbox value	
	if ( fRepresentedTime.tm_hour >= 12 ) {
		if ( fCheckBox ) {
			fCheckBox->SetValue( 1 );
		}
	}
	else
	{
		if ( fCheckBox ) {
			fCheckBox->SetValue( 0 );
		}
	}
	Invalidate();

	return;
}	/* <-- end of function HourMinControl::MessageReceived */

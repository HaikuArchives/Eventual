/*
 * Copyright 2010-2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

// OS includes
#include <InterfaceDefs.h>
 
// Project includes
#include "DayItem.h"



rgb_color		kDefaultFrontColor				= ui_color( B_MENU_ITEM_TEXT_COLOR );
rgb_color		kDefaultBackColor					= ui_color( B_MENU_BACKGROUND_COLOR );
rgb_color		kDefaultServiceItemFrontColor	= { 0, 0, 128, 255 };	// Solid dark blue

/*---------------------------------------------------------------------------
 *							Implementation of class	DayItem
 *--------------------------------------------------------------------------*/


/*!	\brief					Constructor from BString
 *		\param[in]	label		Label to be displayed
 *		\param[in]	message	Message to be sent when this item is invoked.
 */
DayItem::DayItem( const BString& label, BMessage* message )
	:
	BMenuItem( label.String(), message ),
	bIsToday( false ),
	bIsServiceItem( false ),
	fFrontColor( kDefaultFrontColor ),
	fBackColor( kDefaultBackColor )
{ }	// <-- end of constructor from BString



/*!	\brief					Constructor from array of chars
 *		\details					Not too different from the previous constructor.
 *		\param[in]	label		Label to be displayed
 *		\param[in]	message	Message to be sent when this item is invoked.
 */
DayItem::DayItem( const char* label, BMessage* message )
	:
	BMenuItem( label, message ),
	bIsToday( false ),
	bIsServiceItem( false ),
	fFrontColor( kDefaultFrontColor ),
	fBackColor( kDefaultBackColor )
{ }	// <-- end of constructor from array of chars



/*!	\brief			Draw the item's content.
 *		\attention		The item uses its high and low colors even if it is disabled.
 *							There's no way to tell if an item is enabled or disabled from its look.
 */
void DayItem::DrawContent( void )
{
	BMenu* menu = this->Menu();
	if ( !menu ) { return; }	// Nothing to do if not attached to menu
	
	// Save the current colors of the menu
	rgb_color 	backColor	= menu->LowColor(),
					frontColor 	= menu->HighColor();				
	
	// Change the colors to 
	menu->SetLowColor( fBackColor );
	
	if ( IsEnabled() ) {
		menu->SetHighColor( fFrontColor );
	} else {
		menu->SetHighColor( tint_color( fFrontColor, B_LIGHTEN_1_TINT ) );
	}
	
	BRect frame = this->Frame(); 
//	frame.InsetBySelf( 1, 1 );
//	frame.OffsetBySelf( -1, -1 );
	
	menu->StrokeRect( frame, B_SOLID_LOW );
	
	BMenuItem::DrawContent();
	
	// Restoring the original colors
	menu->SetLowColor( backColor );
	menu->SetHighColor( frontColor );
}	// <-- end of function DayItem::DrawContent

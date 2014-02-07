/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _DAY_ITEM_H_
#define _DAY_ITEM_H_

// OS includes
#include <Menu.h>
#include <MenuItem.h>
#include <Rect.h>
#include <String.h>
#include <SupportDefs.h>



/* Colors to be used as defaults */
extern rgb_color		kDefaultFrontColor;
extern rgb_color		kDefaultBackColor;
extern rgb_color		kDefaultServiceItemFrontColor;



/*!	\brief		Represents a single day or a helping item.
 *		\details		Used by the CalendarControl as an item associated with a day.
 *						It is also used as a helping or service item.
 *		\attention	Please note that when an item changes state (e. g., it's notified
 *			that it represents a selected date - "today"), it automatically changes
 *			colors that are used to display it on screen. <br>
 *			There are two ways to control the colors which an item uses to display itself. <br>
 *			The first is to tweak the colors after \em each change of state. Every change
 *			will require subsequent tweak of the colors. <br>
 *			The second \em (preferred) is to tweak the default colors \em before setting any
 *			special state of any item, and let the item's automatic color setup do
 *			the job. <br>
 *			The advantage of the second way is uniform look of the controls.
 */
class DayItem
	:
	public BMenuItem
{
	friend class BMenu;
	
	protected:
		bool bIsToday;
		bool bIsServiceItem;
		rgb_color	fFrontColor;
		rgb_color	fBackColor;	
	
	public:
		DayItem( const BString& label, BMessage* message );
		DayItem( const char* label, BMessage* message );
		virtual ~DayItem() {};
		
		// Boolean getters and setters
		inline virtual void	SetToday( bool in = false ) {
			bIsToday = in;
			in ?  ( fBackColor = kDefaultServiceItemFrontColor ) :
					( fBackColor = kDefaultBackColor );
		}
		inline virtual bool	IsToday( void ) const { return bIsToday; }
		inline virtual void	SetServiceItem( bool in = false ) {
			bIsServiceItem = in;
			in ?  ( fFrontColor = kDefaultServiceItemFrontColor ) : 
					( fFrontColor = kDefaultFrontColor );
		}
		inline virtual bool	IsServiceItem( void ) const { return bIsServiceItem; }

		// Colors getters and setters
		inline virtual rgb_color	GetFrontColor( void ) const { return fFrontColor; }
		inline virtual void 			SetFrontColor( rgb_color in ) { fFrontColor = in; }
		inline virtual rgb_color 	GetBackColor( void ) const { return fBackColor; }
		inline virtual void 			SetBackColor( rgb_color in ) { fBackColor = in; }
		
		// The only interesting function
		virtual void DrawContent();

};	// <-- end of class DayItem

#endif // _DAY_ITEM_H_

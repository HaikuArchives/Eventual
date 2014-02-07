/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CATEGORY_ITEM_H_
#define _CATEGORY_ITEM_H_

#include <iostream>
#include <ostream>
#include <stdlib.h>

#include <InterfaceKit.h>
#include <GraphicsDefs.h>
#include <SupportDefs.h>
#include <Handler.h>
#include <Looper.h>

#include "Category.h"
#include "ColorUpdateWindow.h"

const uint32	kCategorySelected	= 'CATS';
const uint32	kCategoryInvoked	= 'CATI';



/*!	\class		CategoryListItem
 *	\brief		Represents a category in BListView and BOutlineListView.
 *	\details	Based on "IconListItem" sample class by Fabien Fulhaber.
 */
class CategoryListItem
	:
	public BListItem
{
	public:
		CategoryListItem( rgb_color color, BString& label, int level = 0, bool expanded = false );
		CategoryListItem( const Category &cat, int level = 0, bool expanded = false );
		CategoryListItem( const Category *cat, int level = 0, bool expanded = false );
		virtual ~CategoryListItem( );
		
		/*!	\name		Graphic output related functions */
		///@{
		virtual void DrawItem( BView *owner, BRect frame, bool touchEverything = false );
		virtual void Update( BView *owner, const BFont *font );	
		virtual inline void UpdateColor( rgb_color newColor ) {
			currentColor = newColor;
			icon = CreateIcon( newColor, icon );
		}		
		///@}

		/*!	\name		Information functions		*/
		///@{
		virtual inline rgb_color GetColor() const { return currentColor; }
		virtual inline BString GetLabel() const { return currentLabel; }
		virtual float	GetItemWidth( void );
		///@}
		
	protected:
		rgb_color	currentColor;
		BRect 		bounds;
		BString 	currentLabel;
		BBitmap*	icon;
		
		/*!	\name		Service functions		*/
		///@{
		BBitmap*	CreateIcon( const rgb_color color, BBitmap* toChange = NULL );
		///@}
};



/*!	\class 		CategoryListView
 *	\brief		Extention of the BListView, especially built for handling categories.
 *	\details	Internally manages the scrollbars. Provides support for invocation
 *				on an item (which represent a Category), opening an edit window.
 */
class CategoryListView
	:
	public BListView
{
	public:
		CategoryListView( BRect frame, const char* name );
		virtual ~CategoryListView();
		
		virtual bool AddItem( BListItem* toAdd ) {
			bool toReturn = BListView::AddItem( toAdd );
			FixupScrollbars();
			return toReturn;
		}
		
		virtual bool AddItem( BListItem* toAdd, int index ) {
			bool toReturn = BListView::AddItem( toAdd, index );
			FixupScrollbars();
			return toReturn;
		}
		
		virtual bool AddItem( CategoryListItem* toAdd );
		virtual void FrameResized( float width, float height );
		virtual void GetPreferredSize( float* width, float* height );
		
		virtual void RefreshList( BMessage* preferences = NULL );
		virtual void TargetedByScrollView( BScrollView* scroller );
		
		virtual BScrollView* GetScroller() const;
		
	protected:
		BScrollView*	scrollView;
		
		virtual void FixupScrollbars();
};	// <-- end of class CategoryListView



/*!	\class		CategoryMenuItem
 *	\brief		Represents a category in the menu.
 *	\details	Based on IconMenuItem by Fabien Fulhaber.
 */
class	CategoryMenuItem
	:
	public BMenuItem
{
	public:
		CategoryMenuItem( const BString& label,
						  const rgb_color color,
						  BMessage* message );
		CategoryMenuItem( const Category &categoryIn, BMessage *message );
		CategoryMenuItem( const Category *categoryIn, BMessage *message );
		virtual ~CategoryMenuItem();
		
		virtual void   UpdateColor( rgb_color newColor );
		inline virtual BString GetLabel() const { return BString( Label() ); }
		inline virtual rgb_color GetColor() const { return currentColor; }
		
		inline virtual void SetShortcut( char shortcut,
										 uint32 modifiers ) { }
		virtual void GetContentSize( float *width, float* height );
		virtual void DrawContent( void );
		
	protected:
		BBitmap* icon;
		rgb_color currentColor;
		
		virtual BBitmap*	CreateIcon( const rgb_color color, BBitmap* toChange = NULL );
};


/*!	\class		CategoryMenu
 *	\brief		Represents a menu filled with categories.
 *	\details	The only difference from ordinary BMenu with "items in column" layout
 *				is that this menu populates itself with pre-defined Categories.
 */
class	CategoryMenu
	:
	public BMenu
{
	public:
		CategoryMenu( const char* name,
					  bool withSeparator = false,
					  BMessage* templateMessage = NULL,
					  BMessage* preferences = NULL );
		virtual ~CategoryMenu();
	
		virtual void RefreshMenu( BMessage* preferences );
		virtual void UpdateItem( BString& oldLabel, BString& newLabel, rgb_color newColor ) {};
		virtual bool AddItem( CategoryMenuItem* item );

		virtual bool GetWithSeparator( void ) const {
			return bWithSeparator;
		}
		virtual void SetWithSeparator( bool ws ) {
			bWithSeparator = ws;
		}
	
	protected:
	
		/*!	\details		If this variable is "true", the menu upon creation or refresh
		 *				will contain a separator item which will be selected by default.
		 *				If this variable is "false", the "Default" category will be
		 *				selected, or the first one, if there's no "Default" category.
		 */
		bool bWithSeparator;
		
		/*!	\details		This message is the template message to be sent by every
		 *						invoked item. If item does not have a message, a copy of
		 *						template message is assigned to it. Additional data assigned
		 *						to message of every item in the menu is its Category Name.
		 */
		BMessage* fTemplateMessage;
};


#endif // _CATEGORY_ITEM_H_

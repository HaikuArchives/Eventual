/*
 * Copyright 2010 Alexey Burshtein	<aburst02@campus.haifa.ac.il>
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
#include "Utilities.h"

#ifndef SPACING
	#define SPACING 2
#endif

 

/*****************************************************************************************
 * 		Implementation of class CategoryListItem
 *---------------------------------------------------------------------------------------
 * Inspired by IconListItem by Fabien Fulhaber
 ****************************************************************************************/

/*!	
 *	\brief			Constructor
 *	\details		Includes preparations for multilevel categories
 *	\param[in]	color	The color associated with a category
 *	\param[in]	label	Reference to a BString that contains name of the category
 *	\param[in]	level	Currently unused
 *	\param[in]	expanded	Currently unused
 */
CategoryListItem::CategoryListItem( rgb_color color, BString& label, int level, bool expanded )
	:
	BListItem( level, expanded ),
	currentColor( color ),
	currentLabel( label ),
	icon( NULL )
{
	// Creating of the icon depends on height of the item, thus calculate the height first.
	font_height	fh;
	BFont font(be_plain_font);
	font.GetHeight( &fh );
	SetHeight( ceilf( fh.ascent + fh.descent + fh.leading ) );
	
	// Creating the icon. It's a constant, therefore may be created 
	//	immediately after creation of the item. On color update it will be updated too.
	this->icon = CreateIcon( color, NULL );

	GetItemWidth();
	
}	// <-- end of default constructor for CategoryListItem


/*!
 *	\brief			Constructor from a Category
 *	\details		Includes preparations for multilevel categories
 *	\param[in]	cat		The reference to a category
 *	\param[in]	level	Currently unused
 *	\param[in]	expanded	Currently unused
 */
CategoryListItem::CategoryListItem( const Category &cat, int level, bool expanded )
	:
	BListItem( level, expanded ),
	currentColor( cat.categoryColor ),
	currentLabel( cat.categoryName ),
	icon( NULL )
{
	// Creating of the icon depends on height of the item, thus calculate the height first.
	font_height	fh;
	BFont font(be_plain_font);
	font.GetHeight( &fh );
	SetHeight( ceilf( fh.ascent + fh.descent + fh.leading ) );
	
	GetItemWidth();
	
	// Creating the icon. It's a constant, therefore may be created 
	//	immediately after creation of the item. On color update it will be updated too.
	this->icon = CreateIcon( cat.categoryColor, NULL );
	
}	// <-- end of default constructor for CategoryListItem


/*!	\brief	Sets the Item's width.
 *		\details	Called by the functions that want to know the width of this ListItem.
 *					As an additional benefit, it also sets the width of this ListItem.
 *		\returns	The item's width in pixels with subpixel presicion.
 */
float		CategoryListItem::GetItemWidth()
{
	float toReturn = 0;	
	BFont font( be_plain_font );
	
	toReturn = font.StringWidth( currentLabel.String() );
	SetWidth( toReturn );
	return toReturn;

}	// <-- end of function CategoryListItem::GetItemWidth()


/*!
 *	\brief			Constructor from a Category
 *	\details		Includes preparations for multilevel categories
 *	\param[in]	cat			The pointer to a category
 *	\param[in]	level		Currently unused
 *	\param[in]	expanded	Currently unused
 */
CategoryListItem::CategoryListItem( const Category *cat, int level, bool expanded )
	:
	BListItem( level, expanded ),
	icon( NULL )
{
	if ( cat == NULL ) { return; }
	this->currentColor = cat->categoryColor;
	this->currentLabel = cat->categoryName;
	
	// Creating of the icon depends on height of the item, thus calculate the height first.
	font_height	fh;
	BFont font( be_plain_font );
	font.GetHeight( &fh );
	SetHeight( ceilf( fh.ascent + fh.descent + fh.leading ) );
	
	// Creating the icon. It's a constant, therefore may be created 
	//	immediately after creation of the item. On color update it will be updated too.
	this->icon = CreateIcon( cat->categoryColor, NULL );

	GetItemWidth();
	
}	// <-- end of default constructor for CategoryListItem



/*!	
 *	\brief		This function creates a black square filled with submitted color.
 *	\param[in]	colorIn		The color to be used inside the square (black is Ok).
 *	\param[in]	toChange	In case the BBitmap was already allocated, it's more efficient
 *							to draw on it then to deallocate it an reallocate it anew.
 *							A new BBitmap is allocated if "toChange" is NULL (default).
 *							If dimensions of the icon don't suit, it's reallocated anyway.
 *							The new pointer is written into "toChange" and also returned.
 *	\note		
 *				Validity of parameter "toChange" is NOT checked!
 *	\note	Colors usage:
 *				This function draws the background with B_DOCUMENT_BACKGROUND_COLOR
 *				(which is by default white), and the square frame around the requested
 *				color with B_DOCUMENT_TEXT_COLOR (which is by default black).
 *
 *	\note	Icon size:
 *				The size of the icon is derived from the height of the CategoryListItem
 *				object, which depends on selected system font. Whatever the height is,
 *				icon will be square, leaving one pixel of free space above it and one
 *				pixel below. Hence, the dimensions of the square with side "a" are:
 *					a = this->Height() - 2.
 *		
 *	\returns	Created BBitmap or NULL in case of error.
 */
BBitmap* CategoryListItem::CreateIcon(const rgb_color colorIn, BBitmap* toChange )
{
	int squareSize = ceilf( this->Height() - 2 );	//!< Side of the square.
	BRect rect(0, 0, squareSize, squareSize );
	BBitmap* toReturn = NULL;
	if ( !toChange )	// Checking availability of the input
	{
		toReturn = new BBitmap(rect, B_RGB32, true);
	} else {
		// It would be a good idea to check also the color space,
		// but it may be changed by the BBitmap itself, so...
		if ( ceilf ( ( toChange->Bounds() ).Width() ) != squareSize )
		{
			delete toChange;
			toChange = new BBitmap(rect, B_RGB32, true);
			if ( !toChange )
			{
				/* Panic! */
				exit(1);
			}			
		}
		toReturn = toChange;
	}
	
	BView* drawing = new BView( rect, 
								"Drawer", 
								B_FOLLOW_LEFT | B_FOLLOW_TOP,
								B_WILL_DRAW);
	if (!drawing || !toReturn) { return NULL; }	
	toReturn->AddChild(drawing);
	if (toReturn->Lock()) {
		
		// Clean the area
		drawing->SetHighColor( ui_color( B_DOCUMENT_BACKGROUND_COLOR ) );
		drawing->FillRect(rect);
		
		// Draw the black square
		drawing->SetHighColor( ui_color( B_DOCUMENT_TEXT_COLOR ) );
		drawing->SetPenSize(1);
		drawing->StrokeRect(rect);
		
		// Fill the inside of the square
		drawing->SetHighColor( colorIn );
		drawing->FillRect(rect.InsetBySelf(1, 1));
		
		// Flush the actions to BBitmap
		drawing->Sync();
		toReturn->Unlock();
	}
	toReturn->RemoveChild(drawing);			// Cleanup
	delete drawing;
	return toReturn;
}	// <-- end of function CategoryListItem::CreateIcon


/*!	
 *	\brief			Performs actual drawing into the ListView.
 *	\param[in]	owner	Pointer to the view that owns this item.
 *	\param[in]	frame	Frame of the item
 *	\param[in]	touchEverything		Every pixel in the "frame" should be touched.	
 */
void	CategoryListItem::DrawItem( BView* owner, BRect frame, bool touchEverything )
{
	BRect iconRect = frame, labelRect = frame;
	int squareSize = ceilf( this->Height() - 2 );	//!< Side of the icon's square.
	
	if ( !owner ) { return; }	// Nothing to do
	
	/* Filling with background color */
	if ( touchEverything ) {
		owner->SetLowColor( ui_color( B_DOCUMENT_BACKGROUND_COLOR ) );
		owner->FillRect( frame, B_SOLID_LOW );
	}
	
	/* If the item is selected, its background must be drawn in another color */
	if ( this->IsSelected() )
	{
		owner->SetLowColor( ui_color( B_MENU_SELECTED_BACKGROUND_COLOR ) );
		owner->FillRect( frame, B_SOLID_LOW );
	}	// <-- end of "if ( this item is selected )"

	/*!	\note	What happens if there's no icon?
	 *			First of all, it's extremely unlikely. Default category has an icon,
	 *			and there's no way to add a category with no color. If a new category
	 *			is detected, it is assigned a random color.
	 *			The icon, of course, will not be drawn. However, the label will still
	 *			be written at some offset, like if the icon were there.
	 *			In short, all labels are left-aligned and start vertically at the same
	 *			offset, regardless if they have icons or not.
	 */
	owner->MovePenTo( 0, frame.top );
	iconRect.left = frame.left + SPACING;
	iconRect.top = frame.top + 1;
	iconRect.bottom = frame.bottom - 1;
	iconRect.right = iconRect.left + squareSize;

	/* Drawing the icon */
	if ( this->icon )
	{
		owner->SetDrawingMode( B_OP_OVER );
		owner->DrawBitmap( this->icon, iconRect );
	}	// <-- end of "if ( icon exists )"
	
	/* Making space for the label */
	labelRect.left = iconRect.right + 2 * SPACING;
	labelRect.top = frame.top;
	labelRect.bottom = frame.bottom;
	labelRect.right = frame.right;
	
	/* Drawing the label */
	font_height fh;
	owner->GetFontHeight( &fh );
	owner->MovePenTo( labelRect.left, labelRect.bottom - SPACING );
// owner->MovePenTo( labelRect.left, labelRect.bottom - fh.descent );
	
	if ( this->IsSelected() )
	{
		owner->SetHighColor( ui_color( B_DOCUMENT_TEXT_COLOR ) );
	} else {
		owner->SetHighColor( ui_color( B_MENU_SELECTED_ITEM_TEXT_COLOR ) );
	}
	owner->DrawString( this->currentLabel.String() );
	
}	// <-- end of function CategoryListItem::DrawItem



/*!	
 *	\brief			Calculates the new height of the item upon font update.
 *	\details		Also recreates the icon.
 */
void 	CategoryListItem::Update( BView* owner, const BFont* font )
{
	// Update the height and width of the item.
	BListItem::Update( owner, font );
	
	// The width of the item may require update.
	int squareSize = ceilf( this->Height() - 2 );	//!< Side of the icon's square.
	int labelSize = font->StringWidth( this->currentLabel.String() );
	if ( this->Width() < ( labelSize + squareSize + SPACING * 4 ) )
	{
		this->SetWidth( labelSize + squareSize + SPACING * 4 );
	}

	// The icon may require an update too.
	this->icon = CreateIcon( currentColor, this->icon );
	
}	// <-- end of function CategoryListItem::Update


/*!	
 *	\brief			Default destructor
 */
CategoryListItem::~CategoryListItem( )
{
	if ( icon )
	{
		delete icon;
	}
	icon = NULL;
}


/***************************************************************************************
 *		Implementation of class CategoryListView
 **************************************************************************************/

/*!	
 *	\brief		Constructor for the category list
 *	\details	The constructor sets the selection message and invocation message
 *				templates. It also defines the scrollbars.
 *	\note		Scroller support
 *				It is assumed that this class is scrolled. It even resizes itself
 *				to make place for the scrollbars in the same frame.
 */
CategoryListView::CategoryListView( BRect frame, const char *name )
	:
	BListView( BRect( frame.left,
					  frame.top,
					  frame.right - ( B_V_SCROLL_BAR_WIDTH * 2 ) - 10,
					  frame.bottom - ( B_H_SCROLL_BAR_HEIGHT ) ),
			   name ),
	scrollView( NULL )
{
	BMessage* templateMessage = NULL;

	templateMessage = new BMessage( kCategorySelected );
	if ( !templateMessage ) {
		/* Panic! */
		exit(1);
	}
	this->SetSelectionMessage( templateMessage );
	
	templateMessage = new BMessage( kCategoryInvoked );
	if ( !templateMessage ) {
		/* Panic! */
		exit(1);
	}
	this->SetInvocationMessage( templateMessage );
	
}	// <-- end of constructor for CategoryListView


/*!	
 *	\brief			Returns preferred width and height for this CategoryListView
 *	\param[out]		width		Preferred width
 *	\param[out]		height	Preferred height
 */
void		CategoryListView::GetPreferredSize( float* width, float* height )
{
	int numOfItems = this->CountItems();
	
	font_height fh;
	BFont font;
	this->GetFont( &font );
	font.GetHeight( &fh );
	
	int heightOfSingleItem = fh.leading + fh.ascent + fh.descent;
	
	if ( height )
	{
		*height = ( numOfItems == 0 ? heightOfSingleItem : ( heightOfSingleItem * numOfItems ) );
	}
	
	if ( width )
	{
		float currentWidth = 0;
		*width = ( this->Bounds() ).Width();
		
		for ( int i = 0; i < numOfItems; ++i )
		{
			currentWidth = ( ( CategoryListItem* )( this->ItemAt( i ) ))->GetItemWidth() + ( heightOfSingleItem - 2 ) + 4 * SPACING;
			if ( currentWidth > *width )
			{
				*width = currentWidth;
			}
		}
	}
	
}	// <-- end of function "CategoryListView::GetPreferredSize"



/*!	
 *	\brief			Obtain the currently defined scroller for this CategoryListView.
 *	\returns		The BScrollView previously set as the scroller for this view
 *					using function CategoryListView::TargetedByScrollView. May be NULL!
 */
BScrollView*	CategoryListView::GetScroller( void ) const
{
	return this->scrollView;	
}	// <-- end of function CategoryListView::GetScroller



/*!
 *	\brief			Called when the parent view is resized.
 *	\details		Main usage of this function is to fix the scrollbars.
 *	\param[in]	width	New width of the view.
 *	\param[in]	height	New height of the view.
 *	\note			Note on the scrollbars:
 *					It's assumed that this view is a target of a BScrollView. It's
 *					resized in the way which makes space for the scrollbars. If it
 *					knows about a BScrollView that targets this view, it resizes that
 *					BScrollView accordingly (thus the owner shouldn't do it).
 */
void	CategoryListView::FrameResized( float width, float height )
{
	// Firstly, resize the base class.
	BView::FrameResized( width - B_V_SCROLL_BAR_WIDTH,
						 height - B_H_SCROLL_BAR_HEIGHT );

	// Now resize the BScrollView
	if ( this->scrollView )
	{
		FixupScrollbars();
	}
}	// <-- end of function "CategoryListView::FrameResized"



/*!	\brief		Function that adds an item to the list.
 *	\details	If the item is "Default", it will be added first.
 *				If it's an update of an existing item, it will be updated.
 *				If it's a new item, it will be added in alphabetical order.
 */
bool	CategoryListView::AddItem( CategoryListItem *toAdd )
{
	if ( !toAdd ) { return false; }
	
	BString testString("Default"), toAddLabel( toAdd->GetLabel() );
	CategoryListItem *testItem;
	bool toReturn = false;
	int index = 0, limit = this->CountItems();
	
	// Is it the "Default" item?
	if ( toAddLabel == testString )
	{
		// Well, yes, it is.
		testItem = dynamic_cast< CategoryListItem* >( ItemAt( 0 ) );
		
		if ( ( limit == 0 ) || ( testItem && ( testItem->GetLabel() != testString ) ) )
		{
			// Either adding the first item, 
			//	or the first item is not a CategoryListItem,
			//	or it is a CategoryListItem, but not "Default".
			
			toReturn = BListView::AddItem( toAdd, 0 );
			FixupScrollbars();
			return toReturn;
		}
		else if ( !testItem )
		{
			// The first item is not a CategoryListItem.
			toReturn = BListView::AddItem( toAdd, 0 );
			FixupScrollbars();
			return toReturn;
		}
		else 
		{
			// The first item is a "Default" item. Update its color.
			testItem->UpdateColor( toAdd->GetColor() );
			delete toAdd;
			toAdd = NULL;
			InvalidateItem( 0 );
			return true;
		}		
	}
	
	// Well, it's not a "Default" item.
	index = 0;
	while ( index < limit )
	{
		testItem = 	dynamic_cast< CategoryListItem* >( ItemAt( index ) );
		
		if ( ! testItem )
		{
			++index;
			continue;
		}
		
		testString = testItem->GetLabel();
		
		// Jump over "Default" leaving it at the top.
		if ( testString == "Default" )
		{
			++index;
			continue;
		}
		
		// Jump over items that are alphabetically lower.
		if ( testString < toAddLabel )
		{
			++index;
			continue;
		}
		
		// If encountered item with the same label, update its color.
		if ( testString == toAddLabel )
		{
			testItem->UpdateColor( toAdd->GetColor() );
			delete toAdd;
			toAdd = NULL;
			InvalidateItem( index );
			return true;
		}
		
		// Found the place to insert current item
		if ( testString > toAddLabel )
		{
			toReturn = BListView::AddItem( toAdd, index );
			FixupScrollbars();
			return toReturn;
		}
		
		++index;
	}
	
	// If we got here, then all items are alphabetically less.
	toReturn = BListView::AddItem( toAdd );
	FixupScrollbars();
	return toReturn;

}	// <-- end of function CategoryListView::AddItem



/*!	\brief		This function is called when there's a scroller around CategoryListView.
 *	\details	Does nothing special; just adjusts the scrollbars' size to current contents
 *				and enables / disables them if needed.
 * 	\param[in]	scroller	The BScrollView that targets current object.
 *	\note		Note on ownership of the BScrollView:
 *				This class takes care of updating the scrollbars etc., but
 *				it doesn't own the BScrollView. The creator of CategoryListView
 *				is responsible for deleting the BScrollView when it's not needed.
 */
void CategoryListView::TargetedByScrollView( BScrollView* scroller )
{
	BScrollBar* sb = NULL;
	this->scrollView = scroller;
	
	if ( scroller ) {
		sb = scroller->ScrollBar( B_VERTICAL );
		if ( sb )
		{
			sb->SetSteps( 5, 20 );
		}
		
		sb = scroller->ScrollBar( B_HORIZONTAL );
		if ( sb )
		{
			sb->SetSteps( 5, 20 );
		}
		
		BListView::TargetedByScrollView( scroller );
		
		FixupScrollbars();
	}
}	// <-- end of function CategoryListView::TargetedByScrollView


/*!	
 *	\brief			Resizes the scrollbars for correct size reflection
 */
void CategoryListView::FixupScrollbars( void )
{
	BRect bounds = this->Bounds();
	BScrollBar *sb;

	float ratio=1, realRectWidth=1, realRectHeight=1;
	
	// Sanity check
	if ( !scrollView ) {
		/* Nothing to fix */
		return;
	}

	// What is the actual size occupied by the items?
	this->GetPreferredSize( &realRectWidth, &realRectHeight );

	// Obtain info on current font
	font_height fh;
	BFont font( be_plain_font );
	font.GetHeight( &fh );
	
	/* Calculate the height occupied by all items *
	realRectHeight = this->CountItems() * ( ceilf( fh.ascent + fh.descent + fh.leading ) );

	* For calculating the width, we must find the item with longest label *
	realRectHeight = listView->CountItems() * ;
	realRectWidth += 15 + B_V_SCROLL_BAR_WIDTH;	
	*/
	
	// Fixing up horizontal scrollbar.
	sb = scrollView->ScrollBar( B_HORIZONTAL );
	if ( sb ) {
		ratio = bounds.Width() / ( (float)realRectWidth + 1 );

		sb->SetRange( 0, realRectWidth - bounds.Width() );
		if ( ratio >= 1 ) {
			sb->SetProportion( 1 );
		} else {
			sb->SetProportion( ratio );
		}
	}

	// Fixing up vertical scrollbar.
	sb = scrollView->ScrollBar( B_VERTICAL );
	if ( sb ) {
		ratio = bounds.Height() / ( ( float )realRectHeight + 1 );
		
		sb->SetRange( 0, realRectHeight + 2 );
		if ( ratio >= 1 ) {
			sb->SetProportion( 1 );
		} else {
			sb->SetProportion( ratio );
		}
	}
}	// <-- end of function CategoryListView::FixupScrollbars



/*!	
 *	\brief			Destructor
 */
CategoryListView::~CategoryListView( )
{
	// Remove the items.
	CategoryListItem* listItem;
	while ( ( listItem = ( CategoryListItem* )this->ItemAt( 0 ) ) != NULL )
	{
		this->RemoveItem( listItem );
		delete listItem;		
	}
}	// <-- end of "destructor for class CategoryListView"



/*!
 *	\brief			Populate the list with categories
 *	\param[in]	preferences		Message with list of categories and corresponding colors
 *	\note			How the list is populated?
 *					If the preferences message is not NULL, the categories are taken
 *					from there. If it is NULL, the categories are taken from the
 *					global list of categories.
 */
void	CategoryListView::RefreshList( BMessage* preferences )
{
	int index;
	bool bLocked = false;
	CategoryListItem *listItem = NULL;
	Category* pCat = NULL;
	
	/* Part 1.	Firstly, we need to clear the current list.
	 */
	 
	// Lock the parent window.
	if ( this->Window() && ( this->Window() )->Lock() )
	{
		bLocked = true;
	}
	
	// Remove the items.
	while ( ( listItem = ( CategoryListItem* )this->ItemAt( 0 ) ) != NULL )
	{
		this->RemoveItem( listItem );
		delete listItem;		
	}
	
	// At this point the list is empty. Let's fill it!
	
	/* Part 2.	If the message is NULL, take the items from global list of categories. 
	 */
	if ( preferences == NULL )
	{
		index = 0;
		while ( ( pCat = ( Category* )global_ListOfCategories.ItemAt( index ) ) != NULL )	
		{
			listItem = new CategoryListItem( pCat );
			if ( !listItem )
			{
				/* Panic! */
				exit( 1 );
			}
			this->AddItem( listItem );
		}		
	}
	
	/* Part 3.	The message is not NULL, so take the categories from the message.
	 */
	else
	{
		rgb_color catColor;
		BString sbCategory, sbColor, catName;
		status_t	status;
		
		index = 0;
		while ( index < USHRT_MAX )	// Almost infinite loop
		{
			sbCategory.Truncate( 0 );	// Clear the previous string.
			sbCategory << "Category" << index;
			
			sbColor.Truncate( 0 );		// Clear the previous string.
			sbColor << "Color" << index;
			
			// Now we have proper strings "CategoryX" and "ColorX" both
			// sharing the same number X. Let's find out if the message
			// contains the data on category with this ID.
			status = preferences->FindString( sbCategory.String(),
								     		  &catName );
			if ( status != B_OK )
			{
				// Didn't find name of the category
				break;	// Move on to part 2
			}
			else
			{
				// Name of the category was found successfully. Fetch the color:
				status = preferences->FindInt32( sbColor.String(),
											 	 ( int32* )&catColor );
				if ( status != B_OK )
				{
					// The name was found, but the color wasn't... Define a random color!
					catColor = CreateRandomColor();
				}
			}
			
			++index;	// Don't forget to move to the next placeholder in the message
			
			listItem = new CategoryListItem( catColor, catName );
			if ( ! listItem ) 
			{
				/* Panic! */
				exit( 1 );
			}
			this->AddItem( listItem );
		
		}	// <-- end of "while ( there are items in the message )"
	}
	
	/* Part 4.	Sort the items.
	 */
	this->SortItems( CategoriesCompareFunction );
	
	/* Unlock the parent if needed. */
	if ( bLocked )
	{
		( this->Window() )->Unlock();
	}
	
}	// <-- end of function CategoryListView::RefreshList



/***************************************************************************************
 *		Implementation of class CategoryMenuItem
 **************************************************************************************/

/*!
 *	\brief		Constructor.
 *	\param[in]	labelIn 		Reference to BString that defines the label of the Category.
 *	\param[in]	color		Color associated with the current Category.
 *	\param[in]	message		Message sent on invocation of this item.
 *	\note		Differences from standard BMenuItem constructor
 *				CategoryMenuItem actively rejects the shortcuts and modifiers, therefore
 *				they're not accepted even in standard constructor. Additionally,
 *				function "SetShortcut" is overloaded with empty body.
 */
CategoryMenuItem::CategoryMenuItem( const BString& labelIn, const rgb_color color, BMessage* message )
	:
	BMenuItem( labelIn.String(), message ),
	icon( NULL ),
	currentColor( color )
{
	this->icon = CreateIcon( color, NULL );
	if ( !this->icon )
	{
		/* Panic! */
		exit(1);
	}

}	// <-- end of constructor of CategoryMenuItem


/*!
 *	\brief		Constructor.
 *	\details	Constructor from reference to category.
 *	\param[in]	categoryIn	The category
 *	\param[in]	message		The message to be sent
 */
CategoryMenuItem::CategoryMenuItem( const Category &categoryIn,
									BMessage* message )
	:
	BMenuItem( categoryIn.categoryName.String(), message ),
	icon( NULL ),
	currentColor( categoryIn.categoryColor )
{
	this->icon = CreateIcon( categoryIn.categoryColor, NULL );
	if ( !this->icon )
	{
		/* Panic! */
		exit(1);
	}	
}	// <-- end of constructor from reference to Category.


/*!
 *	\brief		Constructor.
 *	\details	Constructor from pointer to category.
 *	\param[in]	categoryIn	The category
 *	\param[in]	message		The message to be sent
 */
CategoryMenuItem::CategoryMenuItem( const Category *categoryIn,
									BMessage* message )
	:
	BMenuItem( "", message ),
	icon( NULL )
{
	if ( !categoryIn )
	{
		/* Received NULL instead of category - can't continue. */
		return;
	}
	BMenuItem::SetLabel( ( categoryIn->categoryName ).String() );
	currentColor = categoryIn->categoryColor;
	this->icon = CreateIcon( categoryIn->categoryColor, NULL );
	if ( !this->icon )
	{
		/* Panic! */
		exit(1);
	}	
}	// <-- end of constructor from pointer to Category.


/*!
 *	\brief		Create the square icon filled with submitted color.
 *	\param[in]	color		The color of the requested icon.
 *	\param[in]	toChange	If there is an allocated item, it may be changed.
 *							If the submitted pointer is not NULL (which is default),
 *							this BBitmap is tested for dimensions match, and if dimensions
 *							allow, its contents are replaced with new icon. Else, old icon
 *							is deleted, and a new is created. In this case, both the
 *							"toChange" pointer and returned pointer point to the same
 *							BBitmap. 
 */
BBitmap* CategoryMenuItem::CreateIcon(const rgb_color colorIn, BBitmap* toChange )
{
	font_height fh;
	BFont plainFont( be_plain_font );
	plainFont.GetHeight( &fh );
	
	int squareSize = ceilf( fh.ascent + fh.descent + fh.leading - 2 );	//!< Side of the square.
	BRect rect(0, 0, squareSize, squareSize );
	BBitmap* toReturn = NULL;
	if ( !toChange )	// Checking availability of the input
	{
		toReturn = new BBitmap(rect, B_RGB32, true);
	} else {
		// It would be a good idea to check also the color space,
		// but it may be changed by the BBitmap itself, so...
		if ( ceilf ( ( toChange->Bounds() ).Width() ) != squareSize )
		{
			delete toChange;
			toChange = new BBitmap(rect, B_RGB32, true);
			if ( !toChange )
			{
				/* Panic! */
				exit(1);
			}			
		}
		toReturn = toChange;
	}
	
	BView* drawing = new BView( rect, 
								"Drawer", 
								B_FOLLOW_LEFT | B_FOLLOW_TOP,
								B_WILL_DRAW);
	if (!drawing || !toReturn) { return NULL; }	
	toReturn->AddChild(drawing);
	if (toReturn->Lock()) {
		
		// Clean the area
		drawing->SetHighColor( ui_color( B_DOCUMENT_BACKGROUND_COLOR ) );
		drawing->FillRect(rect);
		
		// Draw the black square
		drawing->SetHighColor( ui_color( B_DOCUMENT_TEXT_COLOR ) );
		drawing->SetPenSize(1);
		drawing->StrokeRect(rect);
		
		// Fill the inside of the square
		drawing->SetHighColor( colorIn );
		drawing->FillRect(rect.InsetBySelf(1, 1));
		
		// Flush the actions to BBitmap
		drawing->Sync();
		toReturn->Unlock();
	}
	toReturn->RemoveChild(drawing);			// Cleanup
	delete drawing;
	return toReturn;
}	// <-- end of function CategoryMenuItem::CreateIcon

// 
//BBitmap*	CategoryMenuItem::CreateIcon( const rgb_color color,
//										  BBitmap* toChange )
//{
//	BBitmap* toReturn = NULL;	//!< This is the value to be returned.
//	BRect tempRect;
//	float width, height, squareSide;
//	
//	// Sanity check
//	if ( ( color == currentColor ) && ( this->icon ) )
//	{
//		toChange = icon;
//		return icon;
//	}
//	
//	// Get size of the square
//	this->GetContentSize( &width, &height );
//	squareSide = ceilf( height ) - 2;
//	
//	// Compare submitted bitmap to calculated size
//	if ( toChange )
//	{
//		tempRect = toChange->Bounds();
//		if ( ( tempRect.Width() != squareSide ) ||
//			 ( tempRect.Height() != squareSide ) )
//		{
//			// Dimensions don't match - need to delete the bitmap and reallocate it
//			delete toChange;
//			tempRect.Set( 0, 0, squareSide, squareSide );
//			toChange = new BBitmap( tempRect, B_RGB32, true );
//			if ( !toChange )
//			{
//				/* Panic! */
//				exit(1);
//			}
//			
//			toReturn = toChange;
//		}
//		else
//		{
//			/*!	\note	Note about color spaces
//			 *			Actually, even if the dimensions are correct, the existing
//			 *			BBitmap may be not suitable due to incorrect color space.
//			 *			However, BBitmap may change the color space on its own, (and
//			 *			probably will, since there's no much sense in having 32 bits
//			 *			per pixel for bitmap with only 2 colors - black for the frame
//			 *			and Category's color for the inside). Therefore, color space is
//			 *			not checked. It's assumed that existing color space is good enough.
//			 */
//			// Dimensions match, color space is not checked - continuing
//			toReturn = toChange;
//		}
//	}
//	else	// No bitmap is submitted
//	{
//		toReturn = new BBitmap( tempRect, B_CMAP8, true );
//		if ( !toReturn )
//		{
//			/* Panic! */
//			exit(1);
//		}
//	}
//	
//	/* Here toReturn is already set. */
//	
//	// Add the drawing view to the bitmap
//	tempRect.Set( 0, 0, squareSide, squareSide );
//	BView* drawing = new BView (tempRect,
//								"Drawer", 
//								B_FOLLOW_LEFT | B_FOLLOW_TOP,
//								B_WILL_DRAW);
//	if (!drawing || !toReturn) {
//		/* Panic! */
//		return NULL;
//	}	
//	toReturn->AddChild(drawing);
//	if (toReturn->Lock()) {
//
//		// Clean the area
//		drawing->SetHighColor( ui_color( B_MENU_BACKGROUND_COLOR ) );
//		drawing->FillRect( tempRect );
//		
//		// Draw the black square
//		drawing->SetHighColor( ui_color( B_MENU_ITEM_TEXT_COLOR ) );
//		drawing->SetPenSize( 1 );
//		drawing->StrokeRect( tempRect );
//		
//		// Fill the inside of the square
//		drawing->SetHighColor( color );
//		drawing->FillRect( tempRect.InsetBySelf( 1, 1 ) );
//		
//		// Flush the actions to BBitmap
//		drawing->Sync();
//		toReturn->Unlock();	
//	}
//
//	toReturn->RemoveChild( drawing );
//	delete drawing;
//
//	return toReturn;
//}	// <-- end of function "CategoryMenuItem::CreateIcon"


/*!	
 *	\brief			Returns the size for this item, including possible icon and label.
 *	\param[out]		width	Width of rectangle which encloses the item.
 *	\param[out]		height	Height of rectangle which encloses the item.
 */
void 	CategoryMenuItem::GetContentSize( float* width, float* height )
{
	BFont plainFont(be_plain_font);
	font_height fh;
	plainFont.GetHeight( &fh );
	int fontHeight = ceilf( fh.leading + fh.ascent + fh.descent );
	
	// Get width and height for label only
//	if ( Menu() ) {
//		BMenuItem::GetContentSize( width, height );
//	} else {
	if ( height ) {
		*height = fontHeight;
	}
	if ( width )
	{
		// 2*SPACING between icon and label, size of icon is "height-2"
		*width = ceilf( plainFont.StringWidth( this->Label() ) ) +
			SPACING * 2 + ( fontHeight - 2 );;
	}

	
}	// <-- end of function CategoryMenuItem::GetContentSize



/*!	
 *	\brief			Performs actual drawing of the item.
 */
void	CategoryMenuItem::DrawContent( void )
{
	float height;
	this->GetContentSize( NULL, &height );
	
	if ( this->icon != NULL )
	{
		// Drawing icon	
		BPoint drawPoint( ContentLocation() );
		Menu()->SetDrawingMode( B_OP_OVER );
		Menu()->SetLowColor( B_TRANSPARENT_32_BIT );
		
		// If the item is enabled, draw its icon semi-transparent
		if ( this->IsEnabled() == false )
		{
			Menu()->SetDrawingMode( B_OP_BLEND );
			Menu()->DrawBitmap( this->icon, drawPoint );
			Menu()->SetDrawingMode( B_OP_OVER );
		}
		else	// else, use original color
		{
			Menu()->DrawBitmap( this->icon, drawPoint );
		}	
		// Move the pen to starting position of the label
		drawPoint.x += 2 * SPACING + ceilf( height - 2 );
		Menu()->MovePenTo( drawPoint );
	}

	// Draw the label using BMenuItem's function
	BMenuItem::DrawContent();
	
}	// <-- end of function "CategoryMenuItem::DrawContent"


/*!	
 *	\brief			Destructor.
 *	\details		Deallocates the icon.
 */
CategoryMenuItem::~CategoryMenuItem()
{
	if ( this->icon )
	{
		delete this->icon;
		this->icon = NULL;
	}
}	// <-- end of function "CategoryMenuItem::~CategoryMenuItem"


/*!	\brief		Update color of the existing item.
 *	\param[in]	newColor	The new color.
 */
void	CategoryMenuItem::UpdateColor( rgb_color newColor )
{
	icon = CreateIcon( newColor, icon );	
}	// <-- end of function CategoryMenuItem::UpdateColor



/***************************************************************************************
 *		Implementation of class CategoryMenu
 **************************************************************************************/

/*!	
 *	\brief			Constructor for CategoryMenu
 *	\details		This menu populates itself with Categories found in message with
 *					preferences. If this parameter is NULL, empty menu is created.
 *	\param[in]	name		Name of the menu
 *	\param[in]	preferences	Message with all preferences. It is parsed, and categories'
 *							info is used to populate the menu with categories.
 */
CategoryMenu::CategoryMenu( const char *name,
							bool withSeparator,
							BMessage* templateMessage,
							BMessage* preferences )
	:
	BMenu( name, B_ITEMS_IN_COLUMN ),
	bWithSeparator( withSeparator ),
	fTemplateMessage( templateMessage )
{
	RefreshMenu( preferences );	// Populate the menu.	
	BMenuItem* defaultItem = NULL;
	
	if ( withSeparator )
	{
		BSeparatorItem* separator = new BSeparatorItem();
		if ( !separator )
		{
			/* Panic! */
			exit( 1 );
		}
		
		defaultItem = ( BMenuItem* )this->FindItem( "Default" );
		
		if ( NULL == defaultItem ) {
			BMenu::AddItem( separator, 1 );
		} else {
			int i = 0;
			while ( defaultItem != ( BMenuItem* )this->ItemAt( i ) ) {
				++i;
			}
			BMenu::AddItem( separator, ++i );
		}
			
		this->SetLabelFromMarked( false );
		this->SetRadioMode( false );
	}
	else
	{
		CategoryMenuItem* defaultItem = ( CategoryMenuItem* )this->FindItem( "Default" );
		if ( defaultItem )
		{
			defaultItem->SetMarked( true );
		}
		else if ( this->CountItems() > 0 )
		{
			( this->ItemAt( 0 ) )->SetMarked( true );
		}
		this->SetRadioMode( true );
		this->SetLabelFromMarked( true );
	}
}	// <-- end of constructor for CategoryMenu


/*!	
 *	\brief			Destructor for CategoryMenu
 *	\details		The menu doesn't have internal data, therefore the destructor does
 *					nothing.
 */
CategoryMenu::~CategoryMenu()
{
	
}	// <-- end of destructor for CategoryMenu


/*!	
 *	\details		This function removes all items from the menu and populates it
 *					again according to the data found in the preferences message.
 *	\param[in]	preferences		The message with categories used to populate the menu.
 *					If "preferences" is NULL, items are taken from the global list of
 *					categories.
 */
void		CategoryMenu::RefreshMenu( BMessage* preferences )
{
	Category *pCat;
	CategoryMenuItem *menuItem = NULL;
	DebuggerPrintout *deb = NULL;
	BMessage* toSend = NULL;
	bool bLocked = false;
	int index, limit = global_ListOfCategories.CountItems();
	status_t status;
	
	/* Part 1. 	Clean the old menu.
	 *			We should lock the window during the whole process.
	 */
	 
	if ( this->Window() && ( this->Window() )->Lock() )
	{
		bLocked = true;
	}

	while ( ( menuItem = ( CategoryMenuItem* )this->ItemAt( 0 ) ) != NULL )
	{
		if ( ! this->RemoveItem( menuItem ) )
		{
			// Something wrong went with the item's deletion.	
			deb = new DebuggerPrintout( "Didn't succeed to remove item from menu!" );			
		}
		else 	// Everything went good; item was removed - let's delete it!
		{
			delete( menuItem );
		}
	}
	
	// At this point, the menu is clear. Let's populate it!
	
	/* Part 2.	If the preferences message was not submitted, build the menu from the
	 *			global list of categories.
	 *			I assume the items in list are sorted alphabetically.
	 */
	if ( !preferences ) 
	{
		for ( index = 0; index < limit; index++ )
		{
			pCat = ( Category* )global_ListOfCategories.ItemAt( index );
			if ( pCat )
			{
				if ( fTemplateMessage ) {
					toSend = new BMessage( *fTemplateMessage );	
				} else {
					toSend = new BMessage( kCategorySelected );
				}
				if ( !toSend )
				{
					/* Panic! */
					exit( 1 );
				}
				toSend->AddString( "Category", pCat->categoryName );
				menuItem = new CategoryMenuItem( pCat, toSend );
				if ( ! menuItem )
				{
					/* Panic! */
					exit( 1 );
				}
				
				this->AddItem( menuItem );
			}
		}
	}
	/* Part 3.	If the preferences message was submitted, parse it and build the
	 *			menu from items in the message.
	 *			Utilizind the code from PopulateListOfCategories.
	 */
	else
	{
		index = 0;
		BString sbCategory, sbColor, catName;
		rgb_color catColor;
		
		while ( index < USHRT_MAX )	// Almost infinite loop
		{
			sbCategory.Truncate( 0 );	// Clear the previous string.
			sbCategory << "Category" << index;
			
			sbColor.Truncate( 0 );		// Clear the previous string.
			sbColor << "Color" << index;
			
			// Now we have proper strings "CategoryX" and "ColorX" both
			// sharing the same number X. Let's find out if the message
			// contains the data on category with this ID.
			status = preferences->FindString( sbCategory.String(),
								     		  &catName );
			if ( status != B_OK )
			{
				// Didn't find name of the category
				break;	// Move on to part 2
			}
			else
			{
				// Name of the categ ory was found successfully. Fetch the color:
				status = preferences->FindInt32( sbColor.String(),
											 	 ( int32* )&catColor );
				if ( status != B_OK )
				{
					// The name was found, but the color wasn't... Define a random color!
					catColor = CreateRandomColor();
				}
			}
			
			++index;	// Don't forget to move to the next placeholder in the message
			
			// Create the menu item from given data.
			toSend = new BMessage( kCategorySelected );
			if ( !toSend )
			{
				/* Panic! */
				exit( 1 );
			}
			toSend->AddString( "Category", catName );
			menuItem = new CategoryMenuItem( catName, catColor, toSend );
			if ( ! menuItem )
			{
				/* Panic! */
				exit( 1 );
			}
			
			this->AddItem( menuItem );			
			
		}	// <-- end of "while (there are categories in the message)"
	}	// <-- end of "the message with categories was submitted".
	
	// Unlock the window if it was locked.
	if ( bLocked )
	{
		( this->Window() )->Unlock();
	}
	
}	// <-- end of function CategoryMenu::RefreshMenu


/*!	\brief		Insert new item into the menu in alphabetical order.
 *	\param[in]	item	The CategoryMenuItem to insert.
 *	\note		\"Default\" item and separator
 *				The item with label "Default" is always the first in list.
 *				The separator, if it exists, is always the second.
 *				All other items are sorted alphabetically.
 */
bool	CategoryMenu::AddItem( CategoryMenuItem* item )
{
	int index, limit = this->CountItems();
	BString label( item->Label() ), testString("Default");
	BMenuItem* itemToTest = NULL;
	CategoryMenuItem *catItem = NULL;
	
	/* Maybe we're adding default item? */
	if ( label == testString ) 
	{
		// If current label is "Default"
		if ( ( limit == 0 ) || ( BString( ItemAt( 0 )->Label() ) != testString ) )
		{			
			// And the menu is empty or the first item has another label
			// Adding "Default" at the first place.
			return BMenu::AddItem( item, 0 );
		}
		else
		{
			// The menu is not empty, and the first item is already "Default".
			catItem = dynamic_cast<CategoryMenuItem*>( ItemAt( 0 ) );
			if ( catItem ) {
				catItem->UpdateColor( item->GetColor() );
			}
			delete item;
			return true;	// Exit - nothing to do
		}
	}
	// Else, the item should not be added at all - there must be only one! (c) Kurgan
	
	/* Well, the category to be added was not a "Default". 
	 * It's not a separator either; the separator is added by BMenu::AddItem(). 
	 */
	index = 0;
	while ( index < limit ) {
		itemToTest = ItemAt( index );
		catItem = dynamic_cast< CategoryMenuItem* >( itemToTest );
		testString.SetTo( itemToTest->Label() );
		
		// Jumping over "Default" item
		if ( testString == "Default" )
		{
			++index;
			continue;
		}
		
		// Jumping over non-CategoryMenuItem items
		if ( !catItem )
		{
			++index;
			continue;
		}
		
		/* Now the tough part starts */
			// Jump over items that are alphabetically lower
		if ( testString < label )
		{
			++index;
			continue;
		}
			// If same item was found - update its color
		else if ( testString == label )
		{
			if ( catItem ) {
				catItem->UpdateColor( item->GetColor() );
			}
			delete item;
			return true;	// Exit - nothing to do
		}
			// Found first item that's greater alphabetically
		else
		{
			// Verify the message
			if ( ! item->Message() )
			{
				BMessage* toSend = NULL;
				if ( fTemplateMessage ) {
					toSend = new BMessage( *fTemplateMessage );
				} else {
					toSend = new BMessage( kCategorySelected );
				}
				if ( !toSend ) { return false; }
				toSend->AddString( "Category", item->Label() );
				item->SetMessage( toSend );
			}
			return BMenu::AddItem( item, index );
		}
		
		++index;
	}
	
	// If we got here, then it should be the last item.
	if ( ! item->Message() )
	{
		BMessage* toSend = NULL;
		if ( fTemplateMessage ) {
			toSend = new BMessage( *fTemplateMessage );
		} else {
			toSend = new BMessage( kCategorySelected );
		}
		if ( !toSend ) { return false; }
		toSend->AddString( "Category", item->Label() );
		item->SetMessage( toSend );
	}
	return BMenu::AddItem( item );
	
}	//	<-- end of function CategoryMenu::AddItem

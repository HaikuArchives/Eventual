/*
 * Copyright 2011 AlexeyB Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// C++ includes
#include <stdio.h>
#include <stdlib.h>
 
// System includes

#include <GridLayout.h>
#include <GraphicsDefs.h>
#include <GroupLayout.h>
#include <Layout.h>
#include <LayoutItem.h>
#include <InterfaceDefs.h>
#include <Rect.h>
#include <SeparatorItem.h>
#include <View.h>
 
// Local includes
#include "Category.h" 
#include "CategoryItem.h"
#include "CategoryPreferencesView.h"
#include "Utilities.h"

/*==============================================================================
 		Implementation of class CategoryPreferencesView
==============================================================================*/

/*!	\brief			Constructor of CategoryPreferencesView
 *	\details		It's a descendant of BView.
 *	\param[in]	frame	The frame rectangle of the view.
 */
CategoryPreferencesView::CategoryPreferencesView( BRect frame )
	:
	BView( frame,
		   "Category Preferences",
		   B_FOLLOW_ALL_SIDES,
		   B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE )
{
	BLayoutItem* layoutItem = NULL;
	BMessage* toSend = NULL;
	menuField = NULL;
	
	this->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	
	/*!	\note	Layout of the view
	 *			The view has a grid layout. It's arranged in the following way:
	 *			1)	Left column - list of Categories (CategoryList) that
	 *				contains all categories currently available.
	 *			2) 	Right column - three buttons, from top to bottom:
	 *				2a)	Edit currently selected category - guess what it's doing
	 *				2b) Add a new category
	 *				2c)	Merge a current directory into another one + menu with
	 *					all categories. The category selected in the list is disabled.
	 * \note	Restrictions:
	 *			a) The list of categories is scrolled.
	 *			b) If no category is selected, then
	 *				i) 	"Edit" button is disabled
	 *				ii)	"Merge to" field is disabled
	 *			
	 */
	BGridLayout* gridLayout = new BGridLayout();
	if ( !gridLayout )
	{
		/* Panic! */
		exit( 1 );
	}
	// Margins from the sides of the view and spacing between the elements
	gridLayout->SetInsets( 5, 5, 5, 5 );
	gridLayout->SetHorizontalSpacing( 10 );
	gridLayout->SetVerticalSpacing( 10 );
	this->SetLayout( gridLayout );
	gridLayout->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_USE_FULL_HEIGHT ) );
	gridLayout->SetExplicitMinSize( BSize( (this->Bounds()).Width(), (this->Bounds()).Height() ) );
	

	BRect rect = gridLayout->Frame();
	printf ( "The frame is %d pixels wide and %d pixels high.\n",
			 (int )rect.Width(),
			 (int )rect.Height() );
	
	
	/* Creating the CategoryListView with its scroller */
	BRect r( this->Bounds() );
	r.InsetBySelf( 5, 10 );	// Margins near the border of the view
	r.right = (int)( r.right / 2 ) - B_V_SCROLL_BAR_WIDTH + 3;
	r.bottom -= 0;
	
	listView = new CategoryListView( r, "List View" );
	if ( ! listView ) {
		/* Panic! */
		exit( 1 );
	}
	BLooper* looper = this->Looper();
	if ( looper && looper->LockLooper() )
	{
		looper->AddHandler( ( BHandler* )this );
		looper->UnlockLooper();	
	}
	
	scroller = new BScrollView( "Scroller",
								listView,
								B_FOLLOW_LEFT | B_FOLLOW_TOP,
								0, 	// Flags
								true,
								true );
	if ( !scroller )
	{
		/* Panic! */
		exit( 1 );
	}
	layoutItem = gridLayout->AddView( scroller, 0, 0, 1, 3 );
	if ( !layoutItem ) {
		/* Panic! */
		exit( 1 );
	}
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT,
												  B_ALIGN_USE_FULL_HEIGHT ) );
	toSend = new BMessage( kCategoryInvoked );
	if ( !toSend )
	{
		/* Panic! */
		exit( 1 );
	}
	listView->SetInvocationMessage( toSend );
	toSend = new BMessage( kCategorySelected );
	if ( !toSend )
	{
		/* Panic! */
		exit( 1 );
	}
	listView->SetSelectionMessage( toSend );
	
	r = listView->Bounds();	
	r.bottom += B_H_SCROLL_BAR_HEIGHT + 5;
	r.right -= ( B_V_SCROLL_BAR_WIDTH + 5 );
	
	layoutItem->SetExplicitMinSize( BSize( ( B_V_SCROLL_BAR_WIDTH * 2 ), r.Height() ) );
	gridLayout->SetMaxColumnWidth( 0, r.Width()-70 );
	gridLayout->SetMaxColumnWidth( 1, r.Width()-66 );
	
	// Add categories to the list
	PopulateCategoriesView();
	
	/* Creating the buttons */
	// Add new category button
	toSend = new BMessage( kAddNewCategory );
	addButton = new BButton( BRect( 0, 0, 1, 1), 
							 "Add category",
							 "Add category",
							 toSend,
							 B_FOLLOW_H_CENTER | B_FOLLOW_V_CENTER );
	if ( !toSend || !addButton ) {
		/* Panic! */
		exit( 1 );
	}
	addButton->ResizeToPreferred();
	addButton->SetTarget( this );
	layoutItem = gridLayout->AddView( addButton, 1, 0, 1, 1 );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER ) );
	
	// Edit old category button
	toSend = new BMessage( kCategoryInvoked );
	editButton = new BButton( BRect( 0, 0, 1, 1), 
							 "Edit category",
							 "Edit category",
							 toSend,
							 B_FOLLOW_H_CENTER | B_FOLLOW_V_CENTER );
	if ( !toSend || !editButton ) {
		/* Panic! */
		exit( 1 );
	}
	editButton->ResizeToPreferred();
	editButton->SetTarget( this );
	layoutItem = gridLayout->AddView( editButton, 1, 1, 1, 1 );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER ) );
	// Edit category button is disabled by default; 
	// it's enabled when user chooses a category in the list.
	editButton->SetEnabled( false );
	
	/* Creating the menu of merging a category */
	// Create a label
	BGroupLayout* groupLayout = new BGroupLayout( B_VERTICAL );
	if ( !groupLayout )
	{
		/* Panic! */
		exit( 1 );
	}
	gridLayout->AddItem( groupLayout, 1, 2, 1, 1 );
	groupLayout->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	
	mergeToLabel = new BStringView( BRect( 0, 0, 1, 1 ),
								"Merge to label",
								"Merge selected category into:" );
	if ( !mergeToLabel ) {
		/* Panic! */
		exit( 1 );
	}
	mergeToLabel->ResizeToPreferred();
	layoutItem = groupLayout->AddView( mergeToLabel );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	
	// Create the menu
	BMessage templateMessage( kMergeIntoCategory );
	listMenu = new CategoryMenu( "Select category to merge to:", true, &templateMessage );
	if ( !listMenu )
	{
		/* Panic! */
		exit( 1 );
	}	
	
	menuField = new BMenuField( mergeToLabel->Bounds(),
								"Merge to field",
								NULL,
								listMenu );
	if ( !menuField ) {
		/* Panic! */
		exit( 1 );
	}
	menuField->SetDivider( 0 );
	// Just like the "Edit" button above, the menu is initially disabled.
	menuField->SetEnabled( false );
	
	layoutItem = groupLayout->AddView( menuField );
	layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );
	
	for ( int index = 0; index < gridLayout->CountColumns(); ++index )
	{
		gridLayout->SetColumnWeight( index, 1 );
	}
	for ( int index = 0; index < gridLayout->CountRows(); ++index )
	{
		gridLayout->SetRowWeight( index, 1 );	
	}
	gridLayout->InvalidateLayout();
	
}	// <-- end of constructor of CategoryPreferencesView


/*!	\brief		Populate the Categories view with currently existing categories.
 *		\details		This function accesses the global list of categories and adds
 *						a category from the global list to the categories view.
 */
void		CategoryPreferencesView::PopulateCategoriesView( void )
{
	// Sanity check
	if ( ( global_ListOfCategories.IsEmpty() ) ||
		  ( !listView ) )
	{
		return;
	}
	
	CategoryListItem* toAdd = NULL;
	int index, limit = global_ListOfCategories.CountItems();
	
	for ( index = 0; index < limit; ++index )
	{
		toAdd = new CategoryListItem( ( Category* )global_ListOfCategories.ItemAt( index ) );
		if ( toAdd ) {
			listView->AddItem( toAdd );
		}
	}
	
}	// <-- end of function CategoryPreferencesView::PopulateCategoriesView



/*!	\brief		Destructor of CategoryPreferencesView
 */
CategoryPreferencesView::~CategoryPreferencesView()
{
	if ( addButton ) {
		RemoveChild( addButton );
		delete( addButton );
		addButton = NULL;
	}
	if ( editButton ) {
		RemoveChild( editButton );
		delete( editButton );
		editButton = NULL;
	}
	if ( menuField ) {
		RemoveChild( menuField );
		delete menuField;
		menuField = NULL;
	}
	if ( scroller ) {
		RemoveChild( scroller );
		delete( scroller );
		scroller = NULL;
	}
	if ( mergeToLabel ) {
		RemoveChild( mergeToLabel );
		delete( mergeToLabel );
		mergeToLabel = NULL;
	}
}	// <-- end of destructor of CategoryPreferencesView



/*!	\brief		Setting the view as preferred handler
 */
void	CategoryPreferencesView::AttachedToWindow( void )
{
	BLooper* looper = this->Looper();
	if ( looper && looper->LockLooper() )
	{
		looper->AddHandler( ( BHandler* )this );
		looper->UnlockLooper();	
	}
	
	addButton->SetTarget( this );
	editButton->SetTarget( this );
	listView->SetTarget( this );
	
	for ( int i = 0; i < listMenu->CountItems(); ++i )
	{
		( listMenu->ItemAt( i ) )->SetTarget( this );
	}
	
	BView::AttachedToWindow();
	
}	// <-- end of function CategoryPreferencesView::AttachedToWindow


/*!	\brief		Main function of the class
 *	\param[in]	in	The received message.
 */
void	CategoryPreferencesView::MessageReceived( BMessage *in )
{
	if ( !in )
	{
		return;
	}
	
	BMessage* toSend = NULL;
	BString sb;

	Category 	stub( BString("") ),
				receivedFromUpdate( BString("") );
	ColorUpdateWindow* cuWindow;
	CategoryListItem* listItem = NULL;
	CategoryMenuItem* menuItem = NULL;
	uint32		tempUint32 = 0;
	int			tempInt = 0;
	status_t	errorCode = B_OK;
	bool		tempBool = false;
	
	switch ( in->what )
	{
		case ( kAddNewCategory ):
		
			// Creating a stub of category without a name and with random color.
			stub.categoryName.SetTo( "" );
			stub.categoryColor = CreateRandomColor();
			
			cuWindow = new ColorUpdateWindow( stub,
											 true,
											 "Add new category",
											 ( BHandler* )this,
											 this->Looper(),
											 NULL );	// No message
			break;
		
		case ( kColorSelected ):
			in->FindString( "Original string", &stub.categoryName );
			in->FindInt32( "Original color", ( int32* )&tempUint32 );
			stub.categoryColor = RepresentUint32AsColor( tempUint32 );
			in->FindString( "New string", &receivedFromUpdate.categoryName );
			in->FindInt32( "New color", ( int32* )&tempUint32 );
			receivedFromUpdate.categoryColor = RepresentUint32AsColor( tempUint32 );
			
			// If the received category name is empty, don't change anything.
			if ( receivedFromUpdate.categoryName == "" )
			{
				return;	
			}
			
			listItem = new CategoryListItem( receivedFromUpdate );
			if ( ! listItem )
			{
				/* Panic! */
				exit( 1 );	
			}
			if ( listView->AddItem( listItem ) ) {
				AddCategoryToGlobalList( receivedFromUpdate );
			}
			
			// Adding category to the Menu
			toSend = new BMessage( kMergeIntoCategory );
			if ( ! toSend ) {
				/* Panic! */
				exit( 1 );
			}
			toSend->AddString( "Category", receivedFromUpdate.categoryName );
			menuItem = new CategoryMenuItem( receivedFromUpdate, toSend );
			if ( ! menuItem )
			{
				/* Panic! */
				exit( 1 );
			}
			menuItem->SetTarget( this );
			listMenu->AddItem( menuItem );

			break;
			
		case ( kColorReverted ):
			/* Nothing should be done. */
			break;
		
		case ( kCategorySelected ):
			/* Enabling the "Edit" button and the "Merge to..." menu. */
			tempInt = listView->CurrentSelection();
			if ( tempInt < 0 || ( ( ( CategoryListItem* )listView->ItemAt( tempInt ))->GetLabel() == BString( "Default" ) ) )
			{
				editButton->SetEnabled( false );
				menuField->SetEnabled( false );
			} else {
				editButton->SetEnabled( true );
				menuField->SetEnabled( true );
			}
			break;	
		
		case ( kMergeIntoCategory ):
			// Here "stub" is the category selected in the listView - the source of merge.
			// "receivedFromUpdate" is the category selected in the menu - the target of merge.
			// Actually, only the names are interesting.
			
			tempInt = listView->CurrentSelection();
			if ( tempInt < 0 || ( ( listItem = ( CategoryListItem* )listView->ItemAt( tempInt ))->GetLabel() == "Default" ) )
			{
				// "Default" category can't be merged.
				break;	
			}
			stub.categoryName = listItem->GetLabel();
			errorCode = in->FindString( "Category", &( receivedFromUpdate.categoryName ) );
			if ( ( errorCode != B_OK ) || ( stub.categoryName == receivedFromUpdate.categoryName ) )
			{
				// Can't merge category.
				utl_Deb = new DebuggerPrintout( "You're probably trying to merge a category into itself." );
				break;
			}
			
			tempBool = MergeCategories( stub.categoryName, receivedFromUpdate.categoryName );
			
			if ( tempBool )
			{
				// The merge was successful. Remove old category both from list and from menu.
				listView->DeselectAll();	// This also disables the menu.
				listView->RemoveItem( tempInt );
				listMenu->RemoveItem( listMenu->FindItem( stub.categoryName ) );
				
			}
			
			break;
		
		case ( kCategoryInvoked ):
		
			// Modifying currently existing category
			tempInt = listView->CurrentSelection();
			if ( tempInt < 0 )
			{
				break;
			}
			listItem = ( CategoryListItem* )listView->ItemAt( tempInt );
			cuWindow = new ColorUpdateWindow( Category( listItem->GetLabel(), listItem->GetColor() ),
											 false,		// Name shouldn't be edited
											 "Edit category",
											 ( BHandler* )this,
											 this->Looper(),
											 NULL );	// No message
			break;
		
		default:
			BView::MessageReceived( in );	
	};	
}	// <-- end of CategoryPreferencesView::MessageReceived




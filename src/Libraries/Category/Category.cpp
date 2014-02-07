/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "Category.h"

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

class clsMainWindow;



/*======================================================================
 * 		Implementation of global functions
 *=====================================================================*/


/*!	\brief		Add a new category to the global list of categories.
 *	\details	If a category with such name already exists, update its color.
 *	\param[in]	toAdd		The category to be added
 *	\note		Note on memory consumption:
 *				The category that's added is a copy, not an original. The original may
 *				be safely deleted.
 */
void	AddCategoryToGlobalList( const Category *toAdd )
{
	int32 index, limit = global_ListOfCategories.CountItems();
	bool categoryWithSameNameWasFound = false;
	Category* underTesting = NULL;
	
	if ( ! toAdd ) { return; }
	
	// Passing on all items currently in the list
	for ( index = 0; index < limit; ++index )
	{
		underTesting = ( Category* )global_ListOfCategories.ItemAt( index );
		
		// If found the name, check the color
		if ( underTesting && ( toAdd->operator== ( *underTesting ) ) )
		{
			// If the colors don't match,	
			if ( underTesting->categoryColor != toAdd->categoryColor )
			{
				// Need to update the color.
				underTesting->categoryColor = toAdd->categoryColor;
			}
			
			// In any case, exitting.
			return;
		}
	}	// <-- end of "loop on all items currently in list"
	
	// If the item wasn't found, add a new one to the list.
	Category* toBeAdded = new Category( toAdd );
	if ( !toBeAdded )
	{
		// Didn't succeed to create a category. Sad, but there's no need to panic.
		return;
	}
	
	global_ListOfCategories.AddItem( toBeAdded );
}	// <-- end of function AddCategoryToGlobalList



/*!	\brief		Finds a Category
 *		\details		Search the global list of categories for a category with given
 *						name. If such category is found, a \b copy of it is made and
 *						returned to the caller. Otherwise, \c NULL is returned.
 *						If several categories with the same name exist, (which is
 *						extremely unlikely), first is returned.
 *		\attention	The caller is responsible for freeing the returned object.
 *		\param[in]	name		Name of the category to search for.
 */
Category*		FindCategory( const BString& name )
{
	Category* toReturn = NULL;
	Category*  toCheck = NULL;
	
	int i, limit = global_ListOfCategories.CountItems();
	
	for ( i = 0; i < limit; ++i ) {
		toCheck = ( Category* )global_ListOfCategories.ItemAt( i );
		if ( !toCheck ) { continue; }
		
		if ( name == toCheck->categoryName ) {
			// Found the needed category
			toReturn = new Category( toCheck );
			break;	
		}
	}	// <-- end of pass on all categories in the global list
	
	return toReturn;
}	// <-- end of function FindCategory



/*!	\brief		Finds default category
 *		\details		Calls FindCategory() with argument \c "Default". Returs
 *						whatever value it gets. It may return \c NULL !
 */
Category*		FindDefaultCategory()
{
	return FindCategory( "Default" );
}	// <-- end of function FindDefaultCategory



/*!	\brief		MergeCategories
 *	\details	Move all items from one category to another.
 *	\param[in]	source		The source category
 *	\param[in]	target		The target category
 *	\returns	"true" if the merge succeeded, "false" otherwise.
 */
bool	MergeCategories( BString& source, BString& target )
{
	BQuery*	categoryQuery = NULL;
	BFile*	file = NULL;
	entry_ref	fileToReadAttributesFrom;
	attr_info	attribute_info;	//!< Information about the attribute.
	BAlert*	alert = NULL;
	BString 	sb;
	int		tempInt;
	bool 		toReturn = false;
	status_t	status;
	
	/* Ask the user if he really wants to perform the merge */
	sb << "You are going to move all items currently related to category ";
	sb << source;
	sb << " to the new category: " << target;
	sb << ". This action can't be reverted. Are you sure?";
	
	alert = new BAlert( "Merge categories?",
						sb.String(),
						"Yes, sure!",
						"No way!",
						NULL,
						B_WIDTH_AS_USUAL,
						B_OFFSET_SPACING,
						B_STOP_ALERT );
	tempInt = alert->Go();
	
		// The user has denied the offer.
	if ( ( tempInt < 0 ) || ( tempInt >= 1 ) )
	{
		return false;
	}
	
	/* The user has accepted the offer. Miserable human... */
	categoryQuery = new BQuery();
	if ( ! categoryQuery ) {
		// Can't make any change
		return false;
	}
	BVolumeRoster volumeRoster;
	BVolume bootVolume;
	volumeRoster.GetBootVolume( &bootVolume );
	
		// Setting the query to look in the boot volume
	categoryQuery->SetVolume( &bootVolume );
	
		// Check the category attribute type's name.
	int i = 0;
	BString categoryAttributeInternalName;
	while ( AttributesArray[ i ].internalName != 0 )
	{
		if ( strcmp( AttributesArray[ i ].humanReadableName, "Category" ) == 0 )
		{
			// Found the correct attribute! Now, let's take its internal name...
			break;
		}
		++i;
	}
	
	if ( AttributesArray[ i ].internalName == NULL )
	{
		utl_Deb = new DebuggerPrintout( "Didn't succeed to find internal name for Category attribute. Play with attributes you did, yound padavan?" );
		return false;
	}
	
		// Construct the predicate
	categoryQuery->PushAttr( AttributesArray[ i ].internalName );
	categoryQuery->PushString( source.String() );
	categoryQuery->PushOp( B_EQ );
	
		// Another item of the predicate is the type of the file.
	categoryQuery->PushAttr( "BEOS:TYPE" );
	categoryQuery->PushString( kEventFileMIMEType );
	categoryQuery->PushOp( B_EQ );
	categoryQuery->PushOp( B_AND );
	
		// Fire the query!
	categoryQuery->Fetch();
	
		// Fetching the results
	while ( ( status = categoryQuery->GetNextRef( &fileToReadAttributesFrom ) ) == B_OK )
	{
		// Successfully retrieved next entry
		
		file = new BFile( &fileToReadAttributesFrom, B_READ_ONLY );
		if ( !file || file->InitCheck() != B_OK )
			continue;
		
		status = file->GetAttrInfo( AttributesArray[ i ].internalName,
											 &attribute_info );
		if ( status != B_OK )
			continue;
	
			// Update the category attribute	
		file->WriteAttr( AttributesArray[ i ].internalName,
							  AttributesArray[ i ].type,	// Supposedly, B_STRING_TYPE
							  0,
							  target.String(),
							  target.Length() );
		
		/* Actually, the return value is not needed.
		 * In any case we're continuing to a next item.
		 */
		 
		 	// Clear the file descriptor.
		delete file;
	}
	
	delete categoryQuery;
	
		// Remove category from global list of categories.
	DeleteCategoryFromGlobalList( source );
	
	return true;	
}	// <-- end of function MergeCategories



/*!	\brief		Removes category from global list of categories.
 */
void		DeleteCategoryFromGlobalList( const BString& toDelete )
{
	int i, limit = global_ListOfCategories.CountItems();
	Category* underTesting = NULL;
	BString sb;
	
	for ( i = 0; i < limit; ++i )
	{
		underTesting = ( Category* )global_ListOfCategories.ItemAt( i );
		if ( underTesting == NULL )
			continue;
		
		if ( toDelete == underTesting->categoryName ) {
			global_ListOfCategories.RemoveItem( i );
			delete underTesting;
			break;
		}
	}	
}	// <-- end of function DeleteCategoryFromGlobalList






/*======================================================================
 * 		Implementation of struct Category
 *=====================================================================*/

/*!	\brief		Constructor without a color - assigns random color.
 *	\param[in]	nameIn	Name of the category.
 */
Category::Category( const BString& nameIn )
	:
	categoryName( nameIn )
{
	categoryColor = CreateRandomColor();
}	// <-- end of constructor of Category



/*!	\brief		Constructor from name and color
 *	\param[in]		name	Reference to name of the category
 *	\param[in]		color	Color of the category
 */
Category::Category( const BString& nameIn, rgb_color colorIn )
	:
	categoryName( nameIn ),
	categoryColor( colorIn )
{
}



/*!	\brief		Copy constructor.
 */
Category::Category( const Category& in )
	:
	categoryName( in.categoryName ),
	categoryColor( in.categoryColor )
{
}


/*!	\brief		Copy constructor from pointer.
 */
Category::Category( const Category* in )
{
	if ( in )
	{
		this->categoryName.SetTo( in->categoryName ),
		this->categoryColor = in->categoryColor;
	} else {
		this->categoryName.SetTo( "Default" );
		this->categoryColor = ui_color( B_WINDOW_TAB_COLOR );
	}
}

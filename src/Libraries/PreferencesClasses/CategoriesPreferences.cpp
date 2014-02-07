/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include	"Utilities.h"
#include "Category.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Entry.h>
#include <File.h>
#include <List.h>
#include <Node.h>
#include <Query.h>
#include <Roster.h>
#include <String.h>
#include <VolumeRoster.h>

#include <fs_attr.h>

/*****************************************************************************
 *				Definitions of global variables
 ****************************************************************************/


/*****************************************************************************
 *				Declarations of static functions
 ****************************************************************************/

	/* Get categories from the message and add them into the global list. */
static
void			ReadCategoriesFromMessage( BMessage* in = NULL );

	/* Search the filesystem for the categories. */
static
void			SearchFilesystemForAdditionalCategories( void );
	


/*****************************************************************************
 *				Definitions of global functions
 ****************************************************************************/

/*!	\brief		Obtain Categories from the preferences message.
 *		\param[in]	in		BMessage with the preferences. It may be NULL!
 *		\details		In case of any error, defaults are used.
 *		\note			Return value
 *						This function does not return anything. It modifies in-place
 *						the global list ::global_ListOfCategories .
 *		\note			Few notes on implementation
 *			The function performs several tasks:
 *					1)	If the input message is <b>not</b> NULL, the function parses
 *						the message and compares current list of categories with the
 *						data found in the message. 
 *						If there are inconsistencies (which can be only in color),
 *						the data from message wins.
 *					2)	In any case, the function performs a filesystem query and adds
 *						to the list any categories it finds from the disk data.
 *						The colors assigned to these categories is random.
 *					3) 	It sorts the categories alphabetically
 *					4)	If the list is still empty, it defines a "Default" category.
 */
status_t		pref_PopulateCategories( BMessage* in ) {
	
	/* Preface. Clear the old categories.
	 */
	if ( !global_ListOfCategories.IsEmpty() ) {
		global_ListOfCategories.MakeEmpty();
	}
	
	/* Part 1. Read categories from the message.
	 */
	ReadCategoriesFromMessage( in );
	
	/* Part 2. Search for other categories in the file system.
	 */
	SearchFilesystemForAdditionalCategories();
	
	/* Part 3. Sort the list.
	 */
	if ( !global_ListOfCategories.IsEmpty() )
	{
		global_ListOfCategories.SortItems( CategoriesCompareFunction );	
	}
	else
	{
		/* Part 4. Add the "Default" category.
		 */
		AddCategoryToGlobalList( BString( "Default" ), ui_color( B_WINDOW_TAB_COLOR ) );
	}

	return B_OK;
	
}	// <-- end of function	pref_PopulateCategories



/*!	\brief		Saves the categories into a message.
 *		\param[out]		out		The BMessage to which the categories should be added.
 */
status_t		pref_SaveCategories( BMessage* out )
{
	// Sanity check
	if ( !out ) { return B_ERROR; }
	
	int index = 0, limit = global_ListOfCategories.CountItems();
	Category* item = NULL;
	BString 	sbCategoryName;
	BString 	sbCategoryColor;
	
	while ( ( index < USHRT_MAX ) && ( index < limit ) )
	{
		item = ( Category* )global_ListOfCategories.ItemAt( index );
		if ( item == NULL ) {
			++index;
			continue;
		}
		
		sbCategoryName.SetTo( "Category" );
		sbCategoryName << index;
		
		sbCategoryColor.SetTo( "Color" );
		sbCategoryColor << index;

		if ( out->HasString( sbCategoryName.String() ) ) {
			out->ReplaceString( sbCategoryName.String(), item->categoryName.String() );
		} else {
			out->AddString( sbCategoryName.String(), item->categoryName.String() );
		}
		
		if ( out->HasInt32( sbCategoryColor.String() ) ) {
			out->ReplaceInt32( sbCategoryColor.String(), ( int32 )RepresentColorAsUint32( item->categoryColor ) );
		} else {
			out->AddInt32( sbCategoryColor.String(), ( int32 )RepresentColorAsUint32( item->categoryColor ) );
		}
		
		++index;	
	}
	
	// Deleting leftovers of previously existed categories
	// Index is intentionally not touched
	do {
		sbCategoryName.SetTo( "Category" );
		sbCategoryName << index;
		
		sbCategoryColor.SetTo( "Color" );
		sbCategoryColor << index;
		
		if ( out->HasString( sbCategoryName.String() ) ) {
			out->RemoveName( sbCategoryName.String() );
		}
		if ( out->HasInt32( sbCategoryColor.String() ) ) {
			out->RemoveName( sbCategoryColor.String() );
		}
		
		++index;
	} while ( out->HasString( sbCategoryName.String() ) || 
				 out->HasInt32( sbCategoryColor.String() ) );

	return B_OK;
	
}	// <-- end of function pref_SaveEmailPreferences
 
 

/*****************************************************************************
 *				Definitions of static functions
 ****************************************************************************/

/*!	\brief		Reads categories from message.
 *		\details		Finds all categories in the message and adds them into global
 *						list of categories.
 *		\param[in]	in		The BMessage to read categories from.
 *		\returns 	B_OK if everything went good.
 *		\note			Assumptions
 *						Categories' names and corresponding colors are located in
 *						the message under the names "Category0", "Color0" until
 *						"CategoryX", "ColorX" for X+1 categories. There are no repetitions
 *						and no skipped items.
 */
static
void			ReadCategoriesFromMessage( BMessage* in )
{
	BString 	sbCategory,		//!< That will be identifier of category name
				sbColor;			//!< That will be identifier of the corresponding color
	
	BString catName;		//!< This is the name of the category read from message or from hard disk.
	rgb_color catColor;		//!< This is the color of the category read from message or created randomly.
	uint32		tempUint32 = 0;	//!< Used to read the color.
	status_t		status = B_OK;
	
	if ( !in )
		return;		// Nothing to parse.

	unsigned int index = 0;
	while ( index < USHRT_MAX )	// Almost infinite loop
	{
		sbCategory.SetTo( "Category" );	// Clear the previous string.
		sbCategory << index;
		
		sbColor.SetTo( "Color" );		// Clear the previous string.
		sbColor << index;
		
		// Now we have proper strings "CategoryX" and "ColorX" both
		// sharing the same number X. Let's find out if the message
		// contains the data on category with this ID.
		status = in->FindString( sbCategory.String(),
							     		 &catName );
		if ( status != B_OK )
		{
			return;		// Didn't find name of the category
		}
		else
		{
			// Name of the category was found successfully. Fetch the color:
			status = in->FindInt32( sbColor.String(),
											( int32* )&tempUint32 );
			if ( status != B_OK )
			{
				// The name was found, but the color wasn't... Define a random color!
				catColor = CreateRandomColor();
			}
			else
			{
				catColor = RepresentUint32AsColor( tempUint32 );	
			}
		}
		
		++index;	// Don't forget to move to the next placeholder in the message
		
		// Actually add the category.
		AddCategoryToGlobalList( catName, catColor );
		
	}	// <-- end of "while (there are categories in the message)"
		
	return;
}	// <-- end of function ReadCategoriesFromMessage



/*!	\brief		Search the filesystem for additional categories.
 *		\details		Starts a whole-filesystem query for the Event files with
 *						categories which may be copied to the system and not appear
 *						in the Categories' database.
 */
static
void			SearchFilesystemForAdditionalCategories( void )
{
	BQuery* categoryQuery = NULL;	//!< The way to fill the previously-uncatched categories.
	Category*	pCategory = NULL;	//!< Used to traverse the list of categories	
	status_t	status;					//!< Result of the last action.
	ssize_t		bytesTransferred;	//!< Used in I/O operations
	entry_ref	fileToReadAttributesFrom;	//!< This is the reference to file with unknown category.
	BFile*		file = NULL;		//!< This file will be initialized with fileToGetTheAttributesFrom.
	attr_info	attribute_info;	//!< Information about the attribute.
	rgb_color	catColor;			//!< Category color
	char			buffer[ 255 ];		//!< I'll use this buffer to read Categories from files
	
	categoryQuery = new BQuery();
	if ( !categoryQuery ) {
		/* Nothing to do */
		return;
	}
	
		// For initialization of the BQuery, we need to find the Volume with user's data.
	BVolumeRoster volumeRoster;
	BVolume bootVolume;
	volumeRoster.GetBootVolume( &bootVolume );
	
		// Setting the query to look in the boot volume
	categoryQuery->SetVolume( &bootVolume );
	
	/* First item of the predicate is the type of the file.
	 */
	categoryQuery->PushAttr( "BEOS:TYPE" );
	categoryQuery->PushString( kEventFileMIMEType );
	categoryQuery->PushOp( B_EQ );
	
	/* Check the category attribute type's name.
	 */
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
	
	/* Build the query predicate.
	 * This is meaningful only if global list of categories contains any items,
	 *	and if we succeeded to find the attribute with human-readable name "Category".
	 */
	if ( ! global_ListOfCategories.IsEmpty() &&
		  ( AttributesArray[ i ].internalName != NULL ) )
	{
		for ( int j = 0, limit = global_ListOfCategories.CountItems();
				j < limit;
				++j )
		{
			pCategory = ( Category* )global_ListOfCategories.ItemAt( j );
			if ( !pCategory )
				continue;
				
			categoryQuery->PushAttr( AttributesArray[ i ].internalName );
			categoryQuery->PushString( pCategory->categoryName.String(), true );
			categoryQuery->PushOp( B_NE );
			
			categoryQuery->PushOp( B_AND );
		}	// <-- end of "for ( all currently known categories )"
		
	}	// <-- end of "if ( there are any items in the list of known categories )"
	
	/* The predicate that we currently have looks like this:
	 * ((( type is Eventual ) && ( category != "Cat1" )) && ( category != "Cat2" )) && ...
	 * The order does not matter, since we're using "AND".
	 *
	 * Well, let's fire and see what comes...
	 */
	categoryQuery->Fetch();
	
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
		
		status = file->ReadAttr( AttributesArray[ i ].internalName,
										 attribute_info.type,
										 0,
										 buffer,
										 ( attribute_info.size > 255 ) ? 255 : attribute_info.size );
		if ( status != B_OK )
			continue;
		
		// Succeeded to read the category name, it's in "buffer". Create the color...
		catColor = CreateRandomColor();
		
		// ...and add the category to the list of categories.
		AddCategoryToGlobalList( BString( buffer ), catColor );
		
		// We don't need the file anymore.
		delete file;
	}
	
	delete categoryQuery;
	
}	// <-- end of function SearchFilesystemForAdditionalCategories

/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CATEGORY_H_
#define _CATEGORY_H_

#include <iostream>
#include <ostream>
#include <stdlib.h>

#include <InterfaceKit.h>
#include <GraphicsDefs.h>
#include <SupportDefs.h>
#include <Handler.h>
#include <Looper.h>


/*!	\struct 	Category
 *	\brief		Represents a category with its name and color.
 *	\details	
 */
struct Category {
	rgb_color	categoryColor;
	BString 	categoryName;
	
	//! \brief		Constructor without a color - assigns random color.
	Category( const BString &nameIn );
	Category( const BString &nameIn, rgb_color colorIn );
	Category( const Category &in );
	Category( const Category *in );
	
	/*!	\note	Operators on categories
	 *			In all of the following operators, except for output and assignment,
	 *			the color is not important. Only the name matters.
	 */
	inline bool operator== ( const Category& in ) const {
		return ( this->categoryName == in.categoryName );
	}
	inline bool operator!= ( const Category& in ) const {
		return ( this->categoryName != in.categoryName );
	}
	inline bool operator< ( const Category& in ) const {
		return ( this->categoryName < in.categoryName );
	}
	inline bool operator> ( const Category& in ) const {
		return ( this->categoryName > in.categoryName );
	}
	inline bool operator<= ( const Category& in ) const {
		return ( ! ( this->categoryName > in.categoryName ) );
	}
	inline bool operator>= ( const Category& in ) const {
		return ( ! ( this->categoryName < in.categoryName ) );
	}
	inline Category operator= ( const Category& in ) {
		this->categoryName = in.categoryName;
		this->categoryColor = in.categoryColor;
		return *this;
	}
	
};



extern BList global_ListOfCategories;	//!< List that holds all categories in the system.

Category*	FindCategory( const BString& name );
Category*	FindDefaultCategory();
void	AddCategoryToGlobalList( const Category *toAdd );
bool	MergeCategories( BString& source, BString& target );

/*!	\brief		Just a shortcut for another function.
 *	\details	Creates a temporary object of type Category and calls the other function
 *				synchronously.
 */
inline void	AddCategoryToGlobalList( const BString &name, rgb_color color )
{
	Category* toAdd = new Category( name, color );
	AddCategoryToGlobalList( toAdd );
}

/*!	\brief		Just a shortcut for another function.
 */
inline void AddCategoryToGlobalList( const Category &toAdd )
{
	AddCategoryToGlobalList( &toAdd );
}

/*!	\brief		Just a shortcut for another function.
 *	\details	Creates a temporary object of type Category, assigns it a random color
 *				and calls the other function synchronously.
 */
inline void AddCategoryToGlobalList( const BString &name )
{
	Category toAdd( name );
	AddCategoryToGlobalList( toAdd );	
}


void		DeleteCategoryFromGlobalList( const BString& toDelete );


/*!	\brief		Compares two categories using BString's operator<.
 */
inline int		CategoriesCompareFunction( const void* in1, const void* in2 )
{
	return ( ( (const Category*)in1)->categoryName < ( ( const Category* )in2)->categoryName );
}



/*!
 *	\brief		Creates a random color.
 *	\note		What color can be used?
 *				Actually, all colors except for black (0, 0, 0) are allowed.
 *				The color is fully opaque. Values for Red, Green and Blue
 *				are selected randomly.
 */
inline rgb_color	CreateRandomColor()
{
	rgb_color toReturn;
	toReturn.red	= rand() % 0xFF;
	toReturn.green	= rand() % 0xFF;
	toReturn.blue	= rand() % 0xFF;
	toReturn.alpha  = 0xFF;		// The color is always opaque!
	
	// If the color is black, we'll modify it to become not black.
	if ( !toReturn.red && !toReturn.blue && !toReturn.green )
	{
		toReturn.blue = 1;
	}
	
	return toReturn;
}	// <-- end of function CreateRandomColor.



#endif // _CATEGORY_H_

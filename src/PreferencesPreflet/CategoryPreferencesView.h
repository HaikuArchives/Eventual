/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
/*!	\file		CategoriesPreferencesView
 *	\brief		The central view for managing categories.
 */
#ifndef _CATEGORY_PREFERENCES_VIEW_H_
#define _CATEGORY_PREFERENCES_VIEW_H_

#include <Button.h>
#include <GraphicsDefs.h>
#include <InterfaceDefs.h>
#include <MenuField.h>
#include <Message.h>
#include <Rect.h>
#include <ScrollView.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <View.h>

#include "Category.h"
#include "CategoryItem.h"

/* Message constants */
const uint32	kAddNewCategory 	= 'ADDC';
const uint32	kEditOldCategory	= 'EDIC';
const uint32	kMergeIntoCategory	= 'MERC';

class CategoryPreferencesView
	:
	public BView
{
	public:
		CategoryPreferencesView( BRect frame );
		virtual ~CategoryPreferencesView();
		
		virtual void MessageReceived( BMessage* in );
		virtual void AttachedToWindow( void );
		
	private:
		BButton* addButton;
		BButton* editButton;
		BMenuField* menuField;
		CategoryMenu* listMenu;
		BMenuField* mergeTo;
		CategoryListView* listView;
		BScrollView* scroller;
		BStringView* mergeToLabel;

		void	PopulateCategoriesView( void );
};

#endif // _CATEGORY_PREFERENCES_VIEW_H_

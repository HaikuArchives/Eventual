/*
 * Copyright 2011 AlexeyB Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// C++ includes
#include <stdio.h>
#include <stdlib.h>
 
// System includes

#include <Application.h>
#include <GridLayout.h>
#include <GraphicsDefs.h>
#include <GroupLayout.h>
#include <Layout.h>
#include <LayoutItem.h>
#include <Mime.h>
#include <InterfaceDefs.h>
#include <Rect.h>
#include <Resources.h>
#include <Roster.h>
#include <SeparatorItem.h>
#include <TranslationUtils.h>
#include <View.h>
 
// Local includes
//#include "University logo.h"
#include "AboutView.h"
#include "URLView.h"
#include "Utilities.h"

// POSIX includes
#include	<stdio.h>

/*!	\brief		Constructor.
 */
AboutView::AboutView( BRect frame )
	:
	BView( frame,
			 "About",
			 B_FOLLOW_LEFT | B_FOLLOW_TOP,
			 B_WILL_DRAW | B_FRAME_EVENTS )
{
	BLayoutItem* layoutItem;
	BFont			 font;
	BStringView* stringViewToAdd = NULL;
	URLView*		 urlViewToAdd = NULL;
	bool			 needToUnlockLooper = false;
	unsigned int lineNumber = 0;

	// Obtain the Haifa University logo from resources file
	app_info info;
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, B_READ_ONLY);
	size_t	length;
	BResources res;
	if ( res.SetTo(&file) == B_OK )
	{
		BMessage* message = new BMessage();
		message->Unflatten( ( const char* )res.LoadResource( B_MESSAGE_TYPE, 100, &length ) );
		if ( ! message ) {
			printf( "Did not succeed to load resource\n" );
		} else {
			printf( "Loaded resource of length %u successfully.\n", ( unsigned int )length );
		}
		logo = new BBitmap( message );
		delete message;
	} else {
		logo = NULL;
	}

	this->SetViewColor( ui_color( B_PANEL_BACKGROUND_COLOR ) );
	
	BGridLayout* gridLayout = new BGridLayout( 5, 2 );
	if ( ! gridLayout )	/* Panic! */
		return;
		
	this->SetLayout( gridLayout );
	gridLayout->SetInsets( 60, 5, 5, 5 );
	
	needToUnlockLooper = false;
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "Haifa University link",
										 "Haifa University, Haifa, Israel",
										 "http://www.haifa.ac.il" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( false );
		urlViewToAdd->AddAttribute( "META:name", "Haifa University" );
		urlViewToAdd->AddAttribute( "META:company", "Haifa University" );
		urlViewToAdd->AddAttribute( "META:url", "http://www.haifa.ac.il" );
		urlViewToAdd->AddAttribute( "META:country", "Israel" );
		urlViewToAdd->SetIconSize( B_LARGE_ICON );
		urlViewToAdd->GetFont( &font );
		font.SetFace( B_BOLD_FACE );
		font.SetSize( 15 );
		urlViewToAdd->SetFont( &font, B_FONT_SIZE | B_FONT_FACE );
		urlViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( urlViewToAdd, 0, lineNumber++, 3, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
			layoutItem->SetExplicitMaxSize( BSize( ( int )Bounds().Width(), (int )urlViewToAdd->Bounds().Height()) );
		}
		gridLayout->SetMaxRowHeight( 0, ( int )urlViewToAdd->Bounds().Height() );
		urlViewToAdd->SetUnderlineThickness( 0 );
		urlViewToAdd->SetColor( 0, 0, 80 );
//		urlViewToAdd->SetHoverColor( 0, 0, 255 );
	}
	
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"main2",
												"Department of Computer Sciences, Project course (203.3199)" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->GetFont( &font );
		font.SetFace( B_BOLD_FACE );
		font.SetSize( 14 );
		stringViewToAdd->SetFont( &font, B_FONT_SIZE | B_FONT_FACE );
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber++, 3, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"program",
												"Eventual - personal time organizer for Haiku OS." );
	if ( stringViewToAdd )
	{
		stringViewToAdd->GetFont( &font );
		font.SetFace( B_BOLD_FACE );
		font.SetSize( 16 );
		stringViewToAdd->SetFont( &font, B_FONT_SIZE | B_FONT_FACE );
		stringViewToAdd->SetHighColor( 50, 102, 152 );
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber++, 3, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
		}
	}
	
	
	// Data about me
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"author",
												"Idea and implementation:" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "author1",
										 "Alexey \"hitech\" Burshtein",
										 "aburst02@campus.haifa.ac.il" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( true );
		urlViewToAdd->AddAttribute( "META:name", "Alexey Burshtein" );
		urlViewToAdd->AddAttribute( "META:nickname", "hitech" );
		urlViewToAdd->AddAttribute( "META:company", "Haifa University" );
		urlViewToAdd->AddAttribute( "META:url", "http://cs.haifa.ac.il" );
		urlViewToAdd->AddAttribute( "META:country", "Israel" );
		layoutItem = gridLayout->AddView( urlViewToAdd, 1, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}

	// Data about mentors
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"mentor1 - label",
												"Project administration coordinator:" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "mentor1",
										 "Hananel Hazan",
										 "http://hananel.wordpress.com" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( true );
		urlViewToAdd->AddAttribute( "META:name", "Hananel Hazan" );
		urlViewToAdd->AddAttribute( "META:company", "Haifa University" );
		urlViewToAdd->AddAttribute( "META:url", "http://hananel.wordpress.com" );
		urlViewToAdd->AddAttribute( "META:country", "Israel" );
		layoutItem = gridLayout->AddView( urlViewToAdd, 1, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}

	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"mentor2 - label",
												"Mentor from Haiku side:" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "mentor1",
										 "Michael Pfeiffer (\"laplace\")",
										 "michael.w.pfeiffer@gmail.com" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( true );
		urlViewToAdd->AddAttribute( "META:name", "Michael Pfeiffer" );
		urlViewToAdd->AddAttribute( "META:nickname", "laplace" );
		layoutItem = gridLayout->AddView( urlViewToAdd, 1, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}

	// Additional thanks
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks label",
												"Also thanks to:" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber++, 3, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_CENTER, B_ALIGN_TOP ) );
		}
	}
	
	// 1st additional thanks
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 1",
												"My wife Shelly Cohen ―" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 1 - for",
												"for support and understanding while I coded" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 1, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	
	// 2nd additional thanks
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 2",
												"Kevin H. Patterson ―" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 3 - for",
												"for his sample code \"Skeleton\"" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 1, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "link1",
										 "here",
										 "http://www.bebits.com/app/2669" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( true );
		
		urlViewToAdd->AddAttribute( "META:url", "http://www.bebits.com/app/2669" );
		layoutItem = gridLayout->AddView( urlViewToAdd, 2, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	
	// 3rd additional thanks
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 3",
												"Fabien Fulhaber, Brendan Allen ―" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 3 - for",
												"for \"IconListItem\" class" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 1, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "link - iconlistitem",
										 "here",
										 "http://haikuware.com/directory/view-details/development/class-libraries/iconlistitem" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( true );
		
		urlViewToAdd->AddAttribute( "META:url", "http://haikuware.com/directory/view-details/development/class-libraries/iconlistitem" );
		layoutItem = gridLayout->AddView( urlViewToAdd, 2, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	
	// 4th additional thanks
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 3",
												"Atsushi Takamatsu ―" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 3 - for",
												"for \"IconMenuItem\" class" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 1, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}

	// 5th additional thanks
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 4",
												"William Kakes ―" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 4 - for",
												"for \"URLView\" class" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 1, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "link4",
										 "here",
										 "http://haikuware.com/directory/view-details/development/class-libraries/urlview" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( true );
		
		urlViewToAdd->AddAttribute( "META:url", "http://haikuware.com/directory/view-details/development/class-libraries/urlview" );
		layoutItem = gridLayout->AddView( urlViewToAdd, 2, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}

	// 6th additional thanks
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 5",
												"Scott Mc and Pulko Mandy ―" );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 0, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_TOP ) );
		}
	}
	stringViewToAdd = new BStringView( BRect( 0, 0, 1, 1 ),
												"additional thanks 5 - for",
												"for help and assistance." );
	if ( stringViewToAdd )
	{
		stringViewToAdd->ResizeToPreferred();
		layoutItem = gridLayout->AddView( stringViewToAdd, 1, lineNumber, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}
	urlViewToAdd = new URLView( BRect( 0, 0, 1, 1 ),
										 "link5",
										 ":-)",
										 "http://www.haiku-os.org" );
	if ( urlViewToAdd )
	{
		urlViewToAdd->SetHoverEnabled( true );
		urlViewToAdd->SetUnderlineThickness( 0 );
		
		urlViewToAdd->AddAttribute( "META:url", "http://www.haiku-os.org" );
		layoutItem = gridLayout->AddView( urlViewToAdd, 2, lineNumber++, 1, 1 );
		if ( layoutItem ) {
			layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		}
	}

	// Logo	
	if ( logo ) {
		this->DrawBitmap( logo,
								BPoint( 5, 5 ) );
	}
	
	file.Unset();
	res.Unset();
	
}


/*!	\brief		Destructor.
 */
AboutView::~AboutView()
{
	BView* toRemove = NULL;
	
	while ( ( toRemove = this->ChildAt( 0 ) ) != NULL )
	{
		toRemove->RemoveSelf();
		delete toRemove;		
	}	// <-- end of "While ( there are any children )"

}	// <-- end of "desctuctor for About view"


/*!	\brief		Handles the view updates.
 */
void AboutView::Draw( BRect rect )
{
	if ( logo ) {
		this->DrawBitmap( logo,
								BPoint( 5, 5 ) );
	} else {
//		utl_Deb = new DebuggerPrintout( "Logo wasn't found!" );
	}
	
	BView::Draw( rect );
	
}	// <-- end of function  AboutView::Draw()

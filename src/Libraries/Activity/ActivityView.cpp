/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// Prohject includes
#include	"ActivityView.h"
#include "Utilities.h"

// OS includes
#include <GroupLayout.h>
#include <LayoutItem.h>

 
// POSIX includes 
#include <stdio.h>
#include <math.h>

 
/*!	\brief		Constructor for ActivityView
 */
ActivityView::ActivityView( BRect frame,
									 const char* name,
									 ActivityData* toEdit )
	:
	BView( frame,
			 name,
			 B_FOLLOW_ALL_SIDES, 
			 B_WILL_DRAW | B_FRAME_EVENTS | B_PULSE_NEEDED ),
	fNotView( NULL ),
	fSoundView( NULL ),
	fProgView( NULL ),
	fLastError( B_OK ),
	bIsEnabled( true )
{
	BLayoutItem* layoutItem = NULL;
	BGroupLayout* groupLayout = new BGroupLayout( B_VERTICAL );
	if ( !groupLayout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	groupLayout->SetInsets( 5, 5, 5, 5 );
	this->SetLayout( groupLayout );
	
	fNotView = new NotificationView( BRect( 0, 0, 1, 1 ),
												"Notification setup",
												"Notification text:",
												toEdit );
	if ( !fNotView || ( fNotView->InitCheck() != B_OK ) ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fNotView->ResizeToPreferred();
	layoutItem = groupLayout->AddView( fNotView );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );
	} else {
		utl_Deb = new DebuggerPrintout( "Could not add notification setup view!" );
	}
	
	fSoundView = new SoundSetupView( BRect( 0, 0, 1, 1 ),
												"Sound file setup",
												toEdit );
	if ( !fSoundView || ( fSoundView->InitCheck() != B_OK ) ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fSoundView->ResizeToPreferred();
	
	layoutItem = groupLayout->AddView( fSoundView );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );
	} else {
		utl_Deb = new DebuggerPrintout( "Could not add sound setup view!" );
	}

	fProgView = new ProgramSetupView( BRect( 0, 0, 1, 1 ),
												 "ProgramFileSetup",
												 toEdit );
	if ( !fProgView || ( fProgView->InitCheck() != B_OK ) ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fProgView->ResizeToPreferred();
	
	layoutItem = groupLayout->AddView( fProgView );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );
	} else {
		utl_Deb = new DebuggerPrintout( "Could not add program setup view!" );
	}

	
}	// <-- end of constructor for ActivityView



/*!	\brief		Called when the view is attached to window.
 */
void		ActivityView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if ( Parent() ) {
		this->SetViewColor( Parent()->ViewColor() );
	}
}	// <-- end of function ActivityView::AttachedToWindow



/*!	\brief		Destructor for ActivityView
 */
ActivityView::~ActivityView()
{
	if ( fNotView ) {
		fNotView->RemoveSelf();
		delete fNotView;
	}
	
	if ( fSoundView ) {
		fSoundView->RemoveSelf();
		delete fSoundView;
	}
	
	if ( fProgView ) {
		fProgView->RemoveSelf();
		delete fProgView;
	}
}	// <-- end of destructor for Activity View



/*!	\brief			Enable or disable the view.
 */
void 			ActivityView::SetEnabled( bool toSet )
{
	if ( toSet == IsEnabled() ) { return; }
	bIsEnabled = toSet;
	
	if ( fNotView ) fNotView->SetEnabled( toSet );
	if ( fSoundView ) fSoundView->SetEnabled( toSet );
	if ( fProgView ) fProgView->SetEnabled( toSet );
	
}	// <-- end of function ActivityView::SetEnabled

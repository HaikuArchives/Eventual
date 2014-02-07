/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// Project includes
#include "EventEditorNoteView.h"

// OS includes
#include <GroupLayout.h>
#include <LayoutItem.h>
#include <Size.h>

/*!	\brief	Constructor
 *		\details	After the construction the user should always check the
 *					\c InitCheck() output. If it's not B_OK, the class is unusable.
 */
EventEditor_NoteView::EventEditor_NoteView( BRect frame, EventData* data )
	:
	BView( frame, 
			 "Note", 
			 B_FOLLOW_ALL_SIDES,
			 B_WILL_DRAW | B_PULSE_NEEDED | B_FRAME_EVENTS ),
	fData( data ),
	fLastError( B_OK ),
	fScroller( NULL ),
	fExplanation1( NULL ),
	fExplanation2( NULL ),
	fTextView( NULL )
{
	if ( !data ) {
		/* Panic! */
		fLastError = B_BAD_VALUE;
		return;
	}
	
	BGroupLayout* layout = new BGroupLayout( B_VERTICAL );
	if ( !layout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	
	this->SetLayout( layout );
	layout->SetInsets( 5, 5, 10, 10 );
	
	fExplanation1 = new BStringView( BRect( 0, 0, 1, 1 ),
											  "Explanation",
											  "You may write here additional information" );
	fExplanation2 = new BStringView( BRect( 0, 0, 1, 1 ),
											  "Explanation 2",
											  "about this Event." );
	if ( !fExplanation1 || !fExplanation2 ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	
	fExplanation1->ResizeToPreferred();
	BRect tempRect = fExplanation1->Bounds();	
	BSize size( tempRect.Width(), tempRect.Height() );
	BLayoutItem* layoutItem = layout->AddView( fExplanation1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitMaxSize( size );
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_BOTTOM ) );
	}
	
	fExplanation2->ResizeToPreferred();
	tempRect = fExplanation2->Bounds();	
	size.Set( tempRect.Width(), tempRect.Height() );
	layoutItem = layout->AddView( fExplanation2 );
	if ( layoutItem ) {
		layoutItem->SetExplicitMaxSize( size );
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
	}
	
	fTextView = new BTextView( BRect( 0, 0, 100, 100 ),
										"Note contents",
										( frame.InsetByCopy( 20, 0 ).OffsetBySelf( -20, -2 ) ),
										B_FOLLOW_ALL_SIDES,
										B_WILL_DRAW );
	if ( !fTextView ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fTextView->SetText( fData->GetNoteText().String() );

	fScroller = new BScrollView( "Scroller",
										  fTextView,
										  B_FOLLOW_ALL_SIDES,
										  B_WILL_DRAW,
										  false,
										  true );
	if ( !fScroller ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	layoutItem = layout->AddView( fScroller );
	
}	// <-- end of constructor for class EventEditor_NoteView
			


/*!	\brief		Destructor for class EventEditor_NoteView
 */
EventEditor_NoteView::~EventEditor_NoteView()
{
	if ( fTextView ) {
//		if ( fData )
//			fData->SetNoteText( fTextView->Text() );
			
		fTextView->RemoveSelf();
		delete fTextView;
		fTextView = NULL;
	}
	if ( fScroller ) {
		fScroller->RemoveSelf();
		delete fScroller;
		fScroller = NULL;
	}
	if ( fExplanation1 ) {
		fExplanation1->RemoveSelf();
		delete fExplanation1;
		fExplanation1 = NULL;
	}
	if ( fExplanation2 ) {
		fExplanation2->RemoveSelf();
		delete fExplanation2;
		fExplanation2 = NULL;
	}
}	// <-- end of destructor for EventEditor_NoteView



/*!	\brief		This function saves current note's text into Event.
 *		\details		To ease the load, the data is saved only once in 4 seconds.
 */
void		EventEditor_NoteView::Pulse() {
	static uint counter = 1;
	if ( ( counter % 4 ) == 0 ) {
//		SaveText();
		counter = 1;
	}
	else
	{
		++counter;
	}
}	// <-- end of function EventEditor_NoteView::Pulse



/*!	\brief		Actually save the data into the Event.
 */
void		EventEditor_NoteView::SaveText()
{
	if ( fTextView && fData ) {
		fData->SetNoteText( fTextView->Text() );
	}
}	// <-- end of function EventEditor_NoteView::SaveText


/*!	\brief		Updating the window to the new size.
 *		\details		Main task is to change the text rectangle size in the text view.
 */
void		EventEditor_NoteView::FrameResized( float newWidth, float newHeight )
{
	BView::FrameResized( newWidth, newHeight );
	
	if ( fTextView ) {
		BRect textViewRect = fTextView->Bounds();
		textViewRect.InsetBy( 3, 3 );
		fTextView->SetTextRect( textViewRect );
	}	
	
}	// <-- end of EventEditor_NoteView::FrameResized

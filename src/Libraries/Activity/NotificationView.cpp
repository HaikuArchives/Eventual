/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

// Project includes
#include "NotificationView.h"
#include "Utilities.h"

// OS includes
#include <GridLayout.h>
#include <GroupLayout.h>
#include <InterfaceDefs.h>
#include <LayoutItem.h>
#include <Looper.h>

/*---------------------------------------------------------------------------
 *						Implementation of class NotificationView
 *--------------------------------------------------------------------------*/




/*!	\brief		Constructor.
 */
NotificationView::NotificationView( BRect frame,
												const char* name,
												const char* label,
												ActivityData* toEdit )
	:
	BBox( frame, 
			name, 
			B_FOLLOW_LEFT | B_FOLLOW_TOP, 
			B_WILL_DRAW | B_FRAME_EVENTS | B_PULSE_NEEDED ),
	fData( toEdit ),
	fLastError( B_OK ),
	bIsEnabled( true ),
	fCheckBox( NULL ),
	fLabel( NULL ),
	fTextView( NULL ),
	fScroller( NULL ),
	fLabelLayoutItem( NULL ),
	fTextViewLayoutItem( NULL )
{
	BRect textViewRect;		//!< Rectangle that defines text area inside of the TextView.
	BSize	size;
	
	// Creating the checkbox
	BMessage* toSend = new BMessage( kNotificationActivityCheckBoxToggled );
	if ( !toSend ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fCheckBox = new BCheckBox( BRect( 0, 0, 1, 1 ),
										"Notification Enabler",
										"Display notification",
										toSend );
	if ( !fCheckBox ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fCheckBox->ResizeToPreferred();
	
	// Check / uncheck the checkbox and load current text
	if ( fData ) {
		fCheckBox->SetValue( fData->GetNotification( &fNotificationText ) );
	}
	
	BBox::SetLabel( fCheckBox );
	
	// Build internal layout
	BGridLayout* gridLayout = new BGridLayout();
	if ( ! gridLayout ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	gridLayout->SetInsets( 10, ( fCheckBox->Bounds() ).Height(), 10, 10 );
	gridLayout->SetSpacing( 5, 2 );
	this->SetLayout( gridLayout );
	
	// Create label
	fLabel = new BStringView( BRect( 0, 0, 1, 1 ),
									  "Notification Label",	
									  label );
	if ( !fLabel ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fLabel->ResizeToPreferred();
	BRect labelRect = fLabel->Bounds();

	// Place the label
	fLabelLayoutItem = gridLayout->AddView( fLabel, 0, 1, 1, 1 );
	if ( fLabelLayoutItem ) {
		fLabelLayoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_TOP ) );
		fLabelLayoutItem->SetExplicitMinSize( BSize( labelRect.Width(), labelRect.Height() ) );
		fLabelLayoutItem->SetExplicitPreferredSize( BSize( labelRect.Width(), labelRect.Height() ) );
	}
	
	// Create the text view and the scroller
//	textViewRect.right -= ( 10 + B_V_SCROLL_BAR_WIDTH );
//	textViewRect.bottom -= ( 10 + B_H_SCROLL_BAR_HEIGHT );
	fTextView = new BTextView( textViewRect,
										"Notification Text",
										BRect( 0, 0, 0, 0 ),
//										BRect( 0, 0, textViewRect.right - textViewRect.left, textViewRect.bottom - textViewRect.top ),
										B_FOLLOW_ALL_SIDES,
										B_NAVIGABLE | B_WILL_DRAW );
	if ( !fTextView ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fTextView->SetStylable( false );
	size.SetWidth( 120 );
	size.SetHeight( 3 * fTextView->LineHeight() );
/*	textViewRect.SetLeftTop( BPoint( 0, 0 ) );
	textViewRect.right = 120;
	textViewRect.bottom = size.Height(); */
	fTextView->ResizeTo( size.Width(), size.Height() );
	fTextView->SetTextRect( BRect( 1, 1, size.Width() - 1, size.Height() - 1 ) );
	
	fScroller = new BScrollView( "Notification Text Scroller",
										  fTextView,
										  B_FOLLOW_ALL_SIDES,
										  0,
										  false,
										  true );
	if ( !fScroller ) {
		/* Panic! */
		fLastError = B_NO_MEMORY;
		return;
	}
	fTextView->SetText( fNotificationText.String() );
	
	fTextViewLayoutItem = gridLayout->AddView( fScroller, 1, 1, 1, 1 );
	if ( fTextViewLayoutItem ) {
		fTextViewLayoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT,
																	 			 B_ALIGN_USE_FULL_HEIGHT ) );
		fTextViewLayoutItem->SetExplicitMinSize( BSize( size.Width() + B_V_SCROLL_BAR_WIDTH,
																		size.Height() + B_H_SCROLL_BAR_HEIGHT ) );
		fTextViewLayoutItem->SetExplicitPreferredSize( BSize( size.Width() + B_V_SCROLL_BAR_WIDTH,
																				size.Height() + B_H_SCROLL_BAR_HEIGHT ) );
	}
	if ( fCheckBox->Value() == 0 ) {
		EnableTextView( false );
	}
	
}	// <-- end of constructor



/*!	\brief		Destructor
 */
NotificationView::~NotificationView()
{
	if ( fScroller ) {
		fScroller->RemoveSelf();
		delete fScroller;
	}
	if ( fLabel ) {
		fLabel->RemoveSelf();
		delete fLabel;
	}
}	// <-- end of destructor



/*!	\brief		Saves data set by the current element into Event's database.
 */
void		NotificationView::SaveData()
{
	bool checkBoxValue;
	
	if ( !fCheckBox ) { checkBoxValue = false; } 
	else { checkBoxValue = ( fCheckBox->Value() != 0 ); }
	
	if ( fData && fTextView ) {
		fData->SetNotification( checkBoxValue,
										( fNotificationText.SetTo( fTextView->Text() ) ) );
	}
}	// <-- end of function NotificationView::SaveData



/*!	\brief		Update the label of the text.
 */
void		NotificationView::SetLabel( const char* toSet )
{
	BSize size;
	fLabelText.SetTo( toSet );
	if ( fLabel ) {
		fLabel->SetText( toSet );
		fLabel->ResizeToPreferred();
		size.SetWidth( fLabel->Bounds().Width() );
		size.SetHeight( fLabel->Bounds().Height() );
		if ( fLabelLayoutItem ) {
			fLabelLayoutItem->SetExplicitMinSize( size );
			fLabelLayoutItem->SetExplicitPreferredSize( size );
		}
	}	
	InvalidateLayout();
	Invalidate();
	if ( this->Window() ) {
		this->Window()->UpdateIfNeeded();
	}
}	// <-- end of function NotificationView::SetLabel



/*!	\brief		This function gets called when the view becomes a window's child.
 */
void		NotificationView::AttachedToWindow()
{
	if ( this->Looper() && this->Looper()->Lock() ) {
		this->Looper()->AddHandler( this );
		this->Looper()->Unlock();
	}
	BView::AttachedToWindow();
	fCheckBox->SetTarget( this );
}	// <-- end of function NotificationView::AttachedToWindow


/*!	\brief		Update the ActionData upon every keypress
 */
void		NotificationView::Pulse( void )
{
	if ( fData ) {
		
	}
	BView::Pulse();
}	// <-- end of function NotificationView::Draw



/*!	\brief		Respond to the notification messages.
 */
void		NotificationView::MessageReceived( BMessage* in )
{
	BCheckBox* toCheck = NULL;
	
	switch( in->what )
	{
		case kNotificationActivityCheckBoxToggled:
			if ( in->FindPointer( "source", ( void** )&toCheck ) != B_OK )
			{
				toCheck = fCheckBox;
			}
		
			if ( toCheck != NULL ) {
				
				if ( this->fCheckBox->Value() == 0 ) {
					if ( this->fTextView ) {
						EnableTextView( false );
					}
					if ( this->fData ) {
						this->fData->SetNotification( false,
																this->fTextView->Text() );
					}
				}
				else
				{
					if ( this->fTextView ) {
						EnableTextView( true );
					}
					if ( this->fData ) {
						this->fData->SetNotification( true,
																this->fTextView->Text() );
					}
				}
			}

			break;
		default:
			BView::MessageReceived( in );
	};
	
}	// <-- end of function NotificationView::MessageReceived



/*!	\brief		Compute the preferred size for this View.
 *		\param[out]	width		Pointer to the preferred width
 *		\param[out]	height	Pointer to the preferred height
 */
void		NotificationView::GetPreferredSize( float* width, float* height )
{
	BGridLayout* layout = ( BGridLayout* )this->GetLayout();
	BSize size( layout->PreferredSize() );
	if ( width )
		*width = size.Width() + 10;
	if ( height )
		*height = size.Height() + 30;
	
}	// <-- end of function NotificationView::GetPreferredSize



/*!	\brief		When this view is resized, the text rectangle should be resized as well.
 */
void		NotificationView::FrameResized( float width, float height )
{
	BBox::FrameResized( width, height );
	
	if ( fTextView ) {
		BRect rect = fTextView->Bounds();
		rect.InsetBySelf( 2, 2 );
		fTextView->SetTextRect( rect );
	}
	
	this->Invalidate();
	if ( this->Window() )
		this->Window()->UpdateIfNeeded();
	
}	// <-- end of function NotificationView::FrameResized



/*!	\brief		Update colors of the TextView according to view being enabled or not.
 *		\details		Based on function "BTextControl::_UpdateTextViewColors( bool )".
 *		\param[in]	enabled		\c true if this view is going to be enabled.
 *										\c false if not.
 */
void 		NotificationView::EnableTextView( bool enabled )
{
	rgb_color	textColor;	//!< Color for displaying the text
	rgb_color	bgColor;		//!< Color for displaying the background
	BFont			font;
	
	if ( !fTextView ) {
		return;	// Nothing to do
	}
	
		// Only single style is used, therefore it doesn't matter what offset to use.
		// However, offsets other then 0 are not guaranteed to exits.
	fTextView->GetFontAndColor( 0, &font );
	
	if ( enabled ) {
		textColor = ui_color( B_DOCUMENT_TEXT_COLOR );
		bgColor = ui_color( B_DOCUMENT_BACKGROUND_COLOR );
		fTextView->SetFlags( fTextView->Flags() | B_NAVIGABLE );
		fTextView->MakeEditable( true );
	} else {
		textColor = tint_color( ui_color( B_PANEL_BACKGROUND_COLOR ),
										B_DISABLED_LABEL_TINT );
		bgColor = tint_color( ui_color( B_PANEL_BACKGROUND_COLOR ),
									 B_LIGHTEN_2_TINT );
		fTextView->SetFlags( fTextView->Flags() & ~B_NAVIGABLE );
		fTextView->MakeEditable( false );
	}
	
	fTextView->SetFontAndColor( &font, B_FONT_ALL, &textColor );
	fTextView->SetViewColor( bgColor );
	fTextView->SetLowColor( bgColor );
	
	// Redisplay
	fTextView->Invalidate();
	if ( this->Window() )
		Window()->UpdateIfNeeded();
}	// <-- end of function NotificationView::UpdateTextViewColors



/*!	\brief		Enable or disable the view.
 */
void		NotificationView::SetEnabled( bool toSet ) {
	if ( toSet == IsEnabled() ) { return; }

	bIsEnabled = toSet;
	if ( fCheckBox ) {
		fCheckBox->SetEnabled( toSet );
		if ( fCheckBox->Value() != 0 ) {
			EnableTextView( toSet );
		}
	}
}	// <-- end of function NotificationView::SetEnabled

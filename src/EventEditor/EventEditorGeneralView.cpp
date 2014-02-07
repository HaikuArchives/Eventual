/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
 
// Project includes
#include "EventEditorGeneralView.h"

// OS includes
#include <Alignment.h>
#include <GridLayout.h>
#include <GroupLayout.h>
#include <LayoutItem.h>
#include <Size.h>
#include <StringView.h>

// POSIX includes
#include <stdio.h>
#include <math.h>

/*----------------------------------------------------------------------------
 *								Messages constants
 *---------------------------------------------------------------------------*/
 
const uint32		kEventNameUpdated				= 'ENup';
const uint32		kEventLocationUpdated		= 'ELup';
const uint32		kEventCategoryUpdated		= 'ECup';
const uint32		kStartTimeUpdated				= 'STup';
const uint32		kStartDateUpdated				= 'SDup';
const uint32		kEndTimeUpdated				= 'ETup';
const uint32		kEndDateUpdated				= 'EDup';
const	uint32		kEventLastsWholeDays			= 'ELWD';
const uint32		kEventNoDurationToggled		= 'ENoD';
const uint32		kNewCalendarModuleSelected = 'ECMS';
const uint32		kEventTypeSet					= 'ETyS';
const uint32		kPrivateToggled				= 'EPrv';

/*----------------------------------------------------------------------------
 *						Implementation of class EventEditor_GeneralView
 *---------------------------------------------------------------------------*/

/*!	\brief		Default constructor
 */
EventEditor_GeneralView::EventEditor_GeneralView( BRect frame, EventData* data )
	: 
	BView( frame,
			"General Event Editor tab",
			B_FOLLOW_LEFT | B_FOLLOW_TOP,
			B_WILL_DRAW | B_FRAME_EVENTS ),
	fData( data ),
	_EventName( NULL ),
	_Location( NULL ),
	_CategoryMenu( NULL ),
	_CategoryMenuField( NULL ),
	_StartMomentSelector( NULL ),
	_StartTimeHourMinControl( NULL ),
	_StartDateControl( NULL ),
	_EndMomentSelector( NULL ),
	_EndTimeEnabled( NULL ),
	_EndTimeHourMinControl( NULL ),
	_EndDateControl( NULL )
{
	BMessage* toSend = NULL;
	BLayoutItem* layoutItem = NULL;
	
	
	// Sanity check
	if ( !data ) {
		_LastError = B_BAD_VALUE;
		return;
	}

	
	fStartTime = fData->GetStartTime();
	fCalModule = utl_FindCalendarModule( fStartTime.GetCalendarModule() );
	
	if ( !fCalModule ) {
		/* Panic! */
		_LastError = B_BAD_VALUE;
		return;
	}
	fPreviousDuration = fDuration = fData->GetDuration();
	fEndTime = fCalModule->FromTimeTToLocalCalendar( fDuration +
					+ fCalModule->FromLocalCalendarToTimeT( fStartTime ) );
	
	// Create layout
	BGridLayout* gridLayout = new BGridLayout();
	if ( ! gridLayout ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	this->SetLayout( gridLayout );
	gridLayout->SetInsets( 5, 5, 10, 5 );
	
	/*----------------------------
	 *   Event Name text field
	 *---------------------------*/
	
	// Message
	toSend = new BMessage( kEventNameUpdated );
	if ( !toSend ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	
	// Create field for Event name label
	BStringView* eventNameLabel = new BStringView( BRect( 0, 0, 1, 1 ),
																  "Event name label",
																  "Event name:",
																  B_FOLLOW_LEFT | B_FOLLOW_TOP );	// Label
	if ( !eventNameLabel ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	eventNameLabel->ResizeToPreferred();
	layoutItem = gridLayout->AddView( eventNameLabel, 0, 0, 1, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ));
	}
	
	// Create the text control
	_EventName = new BTextControl( BRect( 0, 0, 1, 1 ),
											 "Event name text control",
											 NULL,		// Label
											 fData->GetEventName().String(),
											 toSend, 
											 B_FOLLOW_LEFT | B_FOLLOW_TOP );
	if ( !_EventName ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	_EventName->ResizeToPreferred();
	_EventName->SetToolTip( "Identifier of the Event.\n"
								  "May be not unique, or even empty.\n"
								  "Limited to 255 symbols." );
	_EventName->SetDivider( 0 );
	_EventName->TextView()->SetMaxBytes( 255 );
	layoutItem = gridLayout->AddView( _EventName, 1, 0, 2, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_MIDDLE ) );
	}
	
	/*----------------------------
	 *   Location text field
	 *---------------------------*/
	
	// Message
	toSend = new BMessage( kEventLocationUpdated );
	if ( !toSend ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	
	// Create label for the Event location
	BStringView* locationLabel = new BStringView( BRect( 0, 0, 1, 1 ),
																 "Event location label",
																 "Event location:",	// Label
																 B_FOLLOW_LEFT | B_FOLLOW_TOP );
	if ( ! locationLabel ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	locationLabel->ResizeToPreferred();
	layoutItem = gridLayout->AddView( locationLabel, 0, 1, 1, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );
	}
	
	// Create field for Event location
	_Location = new BTextControl( BRect( 0, 0, 1, 1 ),
											 "Event location text control",
											 NULL,
											 fData->GetEventLocation().String(),
											 toSend,
											 B_FOLLOW_LEFT | B_FOLLOW_TOP );
	if ( !_Location ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	_Location->ResizeToPreferred();
	_Location->TextView()->SetMaxBytes( 255 );
	_Location->SetToolTip( "Where will the Event occur?\n"
								  "Limited to 255 symbols." );
	layoutItem = gridLayout->AddView( _Location, 1, 1, 2, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_MIDDLE ) );
	}
	
	/*------------------------------
	 * 		Categories menu
	 *-----------------------------*/
	toSend = new BMessage( kEventCategoryUpdated );
	if ( !toSend ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}

	_CategoryMenu = new CategoryMenu( "Catrgories menu",
												 false,		// Separator is not needed
												 toSend,		// Template message
												 NULL );		// Categories from the global list
	if ( !_CategoryMenu ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	
	// Mark the category representing current item
	CategoryMenuItem* item = ( CategoryMenuItem* )_CategoryMenu->FindItem( fData->GetCategory().String() );
	if ( !item ) {
		item = ( CategoryMenuItem* )_CategoryMenu->FindItem( "Default" );
		fData->SetCategory( "Default" );
	}
	if ( !item ) {
		_LastError = B_BAD_VALUE;
		return;
	}
	item->SetMarked( true );

	/*------------------------------
	 *  	Categories menu field
	 *-----------------------------*/
	BStringView* menuStringView = new BStringView( BRect( 0, 0, 1, 1 ),
																  "Category menu explanation",
																  "Category:",
																  B_FOLLOW_LEFT | B_FOLLOW_TOP );
	if ( !menuStringView ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	menuStringView->ResizeToPreferred();
	layoutItem = gridLayout->AddView( menuStringView, 0, 2, 1, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );
	}
	
	BMenuBar* categoryMenuBar = new BMenuBar( BRect( 0, 0, 1, 1 ),
															"Category selector",
															B_FOLLOW_LEFT | B_FOLLOW_TOP,
															B_ITEMS_IN_ROW,
															true );
	if ( !categoryMenuBar ) {
		/* Panic! */
		_LastError = B_NO_MEMORY;
		return;
	}
	categoryMenuBar->AddItem( _CategoryMenu );
	
	BMenuItem* tempMenuItem = new BMenuItem( "|", NULL );
	tempMenuItem->SetEnabled( false );
	categoryMenuBar->AddItem( tempMenuItem );
	categoryMenuBar->SetBorder( B_BORDER_EACH_ITEM );
	layoutItem = gridLayout->AddView( categoryMenuBar, 1, 2, 2, 1 );
	if ( layoutItem ) {
		float maxLength = 0, tempLength;
		int limit = global_ListOfCategories.CountItems();
		Category* cat;
		for ( int i = 0; i < limit; ++i ) {
			cat = ( Category* )global_ListOfCategories.ItemAt( i );
			if ( ( tempLength = be_plain_font->StringWidth( cat->categoryName.String() ) ) > maxLength ) {
				maxLength = tempLength;
			}
		}
		BSize tempSize( maxLength + 20, menuStringView->Bounds().Height() + 3 );
		layoutItem->SetExplicitMaxSize( tempSize );
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_MIDDLE ) );
	}

	/*------------------------------
	 *  	Separator field
	 *-----------------------------*/
	BBox* separatorBox = new BBox( BRect( 0, 0, Bounds().Width() - 15, 1 ), 
											 "Separator Box",
											 B_FOLLOW_LEFT | B_FOLLOW_TOP );
	if ( separatorBox ) {
		layoutItem = gridLayout->AddView( separatorBox, 0, 3, 3, 1 );
		BSize size( Bounds().Width() - 15, 1 );
		layoutItem->SetExplicitMaxSize( size );
		layoutItem->SetExplicitMinSize( size );
	}
	
	
	/*------------------------------
	 *  	Start date part
	 *-----------------------------*/
	_StartMomentSelector = CreateStartMomentSelector();
	if ( !_StartMomentSelector ) {
		_LastError = B_NO_MEMORY;
		return;
	}
	_StartMomentSelector->ResizeToPreferred();
	layoutItem = gridLayout->AddView( _StartMomentSelector, 0, 4, 3, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_TOP ) );
		layoutItem->SetExplicitPreferredSize( BSize( Bounds().Width()-15, _StartMomentSelector->Bounds().Height() ) );
//		layoutItem->SetExplicitMinSize( BSize( Bounds().Width()-15, startMomentSelector->Bounds().Height() ) );
	}

	/*------------------------------
	 *  	Event takes whole day
	 *-----------------------------*/
	toSend = new BMessage( kEventLastsWholeDays );
	if ( !toSend ) {
		_LastError = B_NO_MEMORY;
		return;
	}
	
	_EventLastsWholeDays = new BCheckBox( BRect( 0, 0, 1, 1 ),
													  "Event lasts whole days",
													  "All day",
													  toSend );
	if ( !_EventLastsWholeDays ) {
		_LastError = B_NO_MEMORY;
		return;
	}
	_EventLastsWholeDays->ResizeToPreferred();
	_EventLastsWholeDays->SetToolTip( "If this checkbox is selected, Event\n"
												 "will start at midnight and end on\n"
												 "midnight. It will occupy enough days\n"
												 "to include all previously set Duration." );
	_EventLastsWholeDays->SetValue( fData->GetLastsWholeDays() == true );
	
	layoutItem = gridLayout->AddView( _EventLastsWholeDays, 0, 5, 1, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_MIDDLE ) );
	}
	
	
	/*------------------------------
	 *  		Event is private
	 *-----------------------------*/
	toSend = new BMessage( kPrivateToggled );
	if ( !toSend ) {
		_LastError = B_NO_MEMORY;
		return;
	}
	
	_EventIsPrivate = new BCheckBox( BRect( 0, 0, 1, 1 ),
													  "Event is private",
													  "Private",
													  toSend );
	if ( !_EventIsPrivate ) {
		_LastError = B_NO_MEMORY;
		return;
	}
	_EventIsPrivate->ResizeToPreferred();
	_EventIsPrivate->SetValue( fData->GetPrivate() == true );
	_EventIsPrivate->SetToolTip( "This option does nothing. It's intented\n"
										  "for the glorious times when Haiku is real\n"
										  "multiuser. Then it will hid Events of one\n"
										  "user from the suspecting eyes of another.\n" );
	
	layoutItem = gridLayout->AddView( _EventIsPrivate, 1, 5, 1, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );
	}


	/*------------------------------
	 *  		End date and time
	 *-----------------------------*/
	_EndMomentSelector = CreateEndMomentSelector();
	if ( !_EndMomentSelector ) {
		_LastError = B_NO_MEMORY;
		return;
	}
	layoutItem = gridLayout->AddView( _EndMomentSelector, 0, 6, 3, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_MIDDLE ) );
	}
	
	/*------------------------------
	 *  		Duration label
	 *-----------------------------*/
	 _DurationLabel = new BStringView( BRect( 0, 0, 1, 1 ),
	 											  "Duration label",
	 											  "Duration:" );
	 if ( !_DurationLabel ) {
	 	_LastError = B_NO_MEMORY;
		return;
	}
	_DurationLabel->ResizeToPreferred();
	layoutItem = gridLayout->AddView( _DurationLabel, 0, 7, 1, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_RIGHT, B_ALIGN_MIDDLE ) );
	}
	
	/*------------------------------
	 *  		Duration text
	 *-----------------------------*/
	 _DurationLength = new BStringView( BRect( 0, 0, 1, 1 ),
	 											  "Duration length",
	 											  NULL );	// Label will be updated later
	 if ( !_DurationLength ) {
	 	_LastError = B_NO_MEMORY;
		return;
	}
	BFont font( be_bold_font );
	font.SetSize( font.Size() + 2 );
	font.SetFace( B_BOLD_FACE | B_UNDERSCORE_FACE );
	_DurationLength->SetFont( &font );
	layoutItem = gridLayout->AddView( _DurationLength, 1, 7, 2, 1 );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_MIDDLE ) );
	}
	
	// Updating the label
	UpdateDurationLengthLabel();
	

	_LastError = B_OK;
	
}	// <-- end of EventEditor_GeneralView::EventEditor_GeneralView



/*!	\brief 		Destructor.
 */
EventEditor_GeneralView::~EventEditor_GeneralView()
{
	if ( _EventName ) {
		_EventName->RemoveSelf();
		delete _EventName;
		_EventName = NULL;
	}
	
	if ( _Location ) {
		_Location->RemoveSelf();
		delete _Location;
		_Location = NULL;
	}
	
	if ( _CategoryMenuField ) {
		_CategoryMenuField->RemoveSelf();
		delete _CategoryMenuField;
		_CategoryMenuField = NULL;
	}
	
	
	
	
}	// <-- end of destructor for class EventEditor_GeneralView



/*!	\brief		Creates the box that manages start time.
 */
BBox*		EventEditor_GeneralView::CreateStartMomentSelector()
{
	BBox* toReturn;
	BGroupLayout* layout;
	BLayoutItem* layoutItem;
	BMessage* toSend;
	
	toReturn = new BBox( BRect( 5, 5, this->Bounds().right - 5, 30 ),
								"Start moment selector" );
	if ( !toReturn ) {
		return NULL;
	}
	toReturn->SetLabel( "Start day and time" );
	
	layout = new BGroupLayout( B_VERTICAL );
	if ( !layout ) {
		delete toReturn;
		return NULL;
	}
	toReturn->SetLayout( layout );
	layout->SetInsets( 10, 15, 10, 10 );
	
	// Start date selector
	toSend = new BMessage( kStartDateUpdated );
	if ( !toSend ) {
		/* Panic! */
		return NULL;
	}
	_StartDateControl = new CalendarControl( BRect( 0, 0, 1, 1 ),
														  "Start date selector",
														  "Start date:",
														  fStartTime.GetCalendarModule(),
														  0,
														  toSend );
	if ( !_StartDateControl ) {
		/* Panic! */
		delete toReturn;
		return NULL;
	}
	_StartDateControl->InitTimeRepresentation( fStartTime );
	layoutItem = layout->AddView( _StartDateControl );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_USE_FULL_WIDTH, B_ALIGN_MIDDLE ) );
	}
	
	
	// Start time selector													  
	toSend = new BMessage( kStartTimeUpdated );
	if ( !toSend ) {
		/* Panic! */
		delete toReturn;
		return NULL;
	}
	
	_StartTimeHourMinControl = new TimeHourMinControl( BRect( 0, 0, 1, 1 ),
																		"Start time selector",
																		"Start time:",
																		toSend );
	if ( !_StartTimeHourMinControl ) {
		/* Panic! */
		delete toReturn;
		return NULL;
	}	
	layout->AddView( _StartTimeHourMinControl );
	_StartTimeHourMinControl->SetCurrentTime( fStartTime );
	
	return toReturn;
}	// <-- end of function EventEditor_GeneralView::CreateStartMomentSelector()



/*!	\brief		Main function of the class.
 */
void		EventEditor_GeneralView::MessageReceived( BMessage* in )
{
	BString tempString;
	TimeRepresentation tempRepresentation;
	uint32	tempUint32_1, tempUint32_2;
	bool	tempBool;
	CalendarModule* startCM, *endCM;
	CategoryMenuItem* item;
	
	switch( in->what )
	{
		case kEventNameUpdated:
			fData->SetEventName( _EventName->Text() );
			break;
		
		case kEventLocationUpdated:
			fData->SetEventLocation( _Location->Text() );
			break;
		
		case kEventCategoryUpdated:
			if ( B_OK != in->FindString( "Category", &tempString ) ) {
				tempString.SetTo( "Default" );
			}
			fData->SetCategory( tempString );
			break;

		case kPrivateToggled:
			if ( _EventIsPrivate ) {
				tempBool = ( _EventIsPrivate->Value() != 0 );
				fData->SetPrivate( tempBool );
			}
			break;
			
		case kEventLastsWholeDays:
			if ( _EventLastsWholeDays ) {
				
					// Notify the Event that it will (or will not) last whole days.
				tempBool = ( _EventLastsWholeDays->Value() != 0 );
				fData->SetLastsWholeDays( tempBool );
				
					// Access the Calendar Modules to calculate proper duration
				startCM = utl_FindCalendarModule( fStartTime.GetCalendarModule() );
				endCM = utl_FindCalendarModule( fEndTime.GetCalendarModule() );
				
					// Disable or enable the Hour-Min controls
				if ( _StartTimeHourMinControl ) _StartTimeHourMinControl->SetEnabled( !tempBool );
				if ( _EndTimeHourMinControl &&
					  ( _EndTimeEnabled && _EndTimeEnabled->Value() == 0 ) )	{
					_EndTimeHourMinControl->SetEnabled( !tempBool );
				}
				
					// Calculate the new duration
				startCM = utl_FindCalendarModule( fStartTime.GetCalendarModule() );
				endCM = utl_FindCalendarModule( fEndTime.GetCalendarModule() );
				if ( !startCM || !endCM ) { break; }
					
				if ( tempBool ) {
					// The event will last whole days
					fDuration = CalculateNumberOfDaysForDuration( fDuration );					
				} else {
					fDuration = endCM->FromLocalCalendarToTimeT( fEndTime ) - startCM->FromLocalCalendarToTimeT( fStartTime );
				}
				
				fData->SetDuration( fDuration );
				
				UpdateDurationLengthLabel( false );
			}			
			break;
			
		case kEventNoDurationToggled:

			if ( _EndTimeEnabled ) {
				
				startCM = utl_FindCalendarModule( fStartTime.GetCalendarModule() );
				endCM = utl_FindCalendarModule( fEndTime.GetCalendarModule() );
				if ( !startCM || !endCM ) { break; }
				
				tempBool = ( _EndTimeEnabled->Value() != 0 );
				
				// Enabled the control - the duration will be 0
				if ( tempBool ) {
					
					if ( !_EventLastsWholeDays || ( _EventLastsWholeDays->Value() == 0 ) ) {
						// Storing current duration for later possible uncheck of this control
						fPreviousDuration = fDuration;
						// Duration must be 0						
						fData->SetDuration( ( fDuration = 0 ) );
						// End time moment is translated to match start time moment
						fEndTime = endCM->FromTimeTToLocalCalendar( startCM->FromLocalCalendarToTimeT( fStartTime ) );
					} else {
						fDuration = CalculateNumberOfDaysForDuration( fDuration );
						fEndTime = endCM->FromTimeTToLocalCalendar( fDuration + startCM->FromLocalCalendarToTimeT( fStartTime ) - 24*60*60 );
					}
					
				}
				
				// Disabling the control - restoring previous duration
				else
				{
					// Update duration to number of days if needed
					if ( _EventLastsWholeDays && ( _EventLastsWholeDays->Value() != 0 ) ) {
						fDuration = CalculateNumberOfDaysForDuration( fDuration );
						fEndTime = endCM->FromTimeTToLocalCalendar( fDuration + startCM->FromLocalCalendarToTimeT( fStartTime ) - 24*60*60 );
					} else {
						fDuration = fPreviousDuration;
						fEndTime = endCM->FromTimeTToLocalCalendar( fDuration + startCM->FromLocalCalendarToTimeT( fStartTime ) );
					}
					
				}
				
				fData->SetDuration( fDuration );
				_EndDateControl->InitTimeRepresentation( fEndTime );
				_EndTimeHourMinControl->SetCurrentTime( fEndTime );
				
				SetEnabledStateOfEndTimeBox();
				UpdateDurationLengthLabel( false );
			}
			break;
			
		case kCalendarControlInvoked:
			utl_Deb = new DebuggerPrintout( "Calendar control was invoked." );
			break;
		
		case kStartDateUpdated:
			tempRepresentation = fStartTime;
			
			in->FindInt32( "Day", ( int32* )&tempRepresentation.tm_mday );
			in->FindInt32( "Month", ( int32* )&tempRepresentation.tm_mon );
			in->FindInt32( "Year", ( int32* )&tempRepresentation.tm_year );
			if ( B_OK == in->FindString( "Calendar Module", &tempString ) ) {
				tempRepresentation.SetCalendarModule( tempString );
			}
			
			// If the Event lasts whole days
			if ( ( _EventLastsWholeDays && ( _EventLastsWholeDays->Value() != 0 ) ) ||
					 fDuration == 0 )
			{
				// Move end time along with the start time
				fStartTime = tempRepresentation;
				fData->SetStartDate( fStartTime );
				
				startCM = utl_FindCalendarModule( fStartTime.GetCalendarModule() );
				endCM = utl_FindCalendarModule( fEndTime.GetCalendarModule() );
				
				fEndTime = endCM->FromTimeTToLocalCalendar( fDuration + startCM->FromLocalCalendarToTimeT( fStartTime ) );
				
				_EndDateControl->InitTimeRepresentation( fEndTime );
				_EndTimeHourMinControl->SetCurrentTime( fEndTime );
			}
			// Event does not last whole days, but the change is legal
			else if ( VerifyEndTimeIsGreaterThenStart( tempRepresentation, fEndTime, &fDuration ) ) {
				fStartTime = tempRepresentation;
				fData->SetStartDate( fStartTime );
				fData->SetDuration( fDuration );
			// Event does not last whole days and the change is illegal
			} else {
				_StartDateControl->InitTimeRepresentation( fStartTime );
				_StartDateControl->Invalidate();
			}
			
			UpdateDurationLengthLabel();
			break;
		
		case kEndDateUpdated:
			tempRepresentation = fEndTime;
			
			in->FindInt32( "Day", ( int32* )&tempRepresentation.tm_mday );
			in->FindInt32( "Month", ( int32* )&tempRepresentation.tm_mon );
			in->FindInt32( "Year", ( int32* )&tempRepresentation.tm_year );
			if ( B_OK == in->FindString( "Calendar Module", &tempString ) ) {
				tempRepresentation.SetCalendarModule( tempString );
			}
			
			if ( VerifyEndTimeIsGreaterThenStart( fStartTime, tempRepresentation, &fDuration ) ) {
				fEndTime = tempRepresentation;
				
				if ( _EventLastsWholeDays && ( _EventLastsWholeDays->Value() != 0 ) ) {
					fDuration = CalculateNumberOfDaysForDuration( fDuration );
				}
				
				fData->SetDuration( fDuration );
			} else {
				_EndDateControl->InitTimeRepresentation( fEndTime );
			}
			UpdateDurationLengthLabel();
			break;
			
		case kEndTimeUpdated:
			tempRepresentation = fEndTime;
			in->FindInt32( kHoursValueKey.String(), ( int32* )&tempRepresentation.tm_hour );
			in->FindInt32( kMinutesValueKey.String(), ( int32* )&tempRepresentation.tm_min );
			
			SetCorrectDuration();
			
			if ( VerifyEndTimeIsGreaterThenStart( fStartTime, tempRepresentation, &fDuration ) ) {
				fEndTime = tempRepresentation;
				fData->SetDuration( fDuration );
			} else {
				_EndTimeHourMinControl->SetCurrentTime( fEndTime );
			}
			UpdateDurationLengthLabel();
			break;
			
		case kStartTimeUpdated:
			tempRepresentation = fStartTime;
			in->FindInt32( kHoursValueKey.String(), ( int32* )&tempRepresentation.tm_hour );
			in->FindInt32( kMinutesValueKey.String(), ( int32* )&tempRepresentation.tm_min );
			
			startCM = utl_FindCalendarModule( fStartTime.GetCalendarModule() );
			endCM = utl_FindCalendarModule( fEndTime.GetCalendarModule() );
			if ( !startCM || !endCM ) { break; }
			
			// If the Event lasts whole days
			if ( ( _EventLastsWholeDays && ( _EventLastsWholeDays->Value() != 0 ) ) ||
					 fDuration == 0 )
			{
				// Move end time along with the start time
				fStartTime = tempRepresentation;
				fData->SetStartTime( fStartTime );				
				
				fEndTime = endCM->FromTimeTToLocalCalendar( fDuration + startCM->FromLocalCalendarToTimeT( fStartTime ) );
				
				_EndDateControl->InitTimeRepresentation( fEndTime );
				_EndTimeHourMinControl->SetCurrentTime( fEndTime );
			}
			// Event does not last whole days, but the change is legal
			else if ( VerifyEndTimeIsGreaterThenStart( tempRepresentation, fEndTime, &fDuration ) ) {
				fStartTime = tempRepresentation;
				fData->SetStartTime( fStartTime );
				fDuration = endCM->FromLocalCalendarToTimeT( fEndTime ) - startCM->FromLocalCalendarToTimeT( fStartTime );
				fData->SetDuration( fDuration );
			// Event does not last whole days and the change is illegal
			} else {
				_StartTimeHourMinControl->SetCurrentTime( fStartTime );
				_StartTimeHourMinControl->Invalidate();
			}
			UpdateDurationLengthLabel();
			
			break;
		
		case kSaveRequested:
			// Main data parts required to save are:
			// Name, location and category.
			// Other data parts are updated constantly anyway.
			if ( _EventName ) fData->SetEventName( _EventName->Text() );
			if ( _Location )  fData->SetEventLocation( _Location->Text() );
			if ( _CategoryMenu ) {
				if ( ( item = ( CategoryMenuItem* )_CategoryMenu->FindMarked() ) == NULL )
				{
					fData->SetCategory( "Default" );
				}
				else
				{
					fData->SetCategory( item->GetLabel() );
				}
			}
			
			break;
		
		default:
			BView::MessageReceived( in );	
	}
	
}	// <-- end of function EventEditor_GeneralView::MessageReceived



/*!	\brief		Adding this view to the messaging loop
 */
void		EventEditor_GeneralView::AttachedToWindow() {
	BView::AttachedToWindow();
	
	if ( Parent() )
		SetViewColor( Parent()->ViewColor() );
	
	if ( this->Looper() && this->Looper()->Lock() ) {
		Looper()->AddHandler( this );
		Looper()->Unlock();	
	}
	
	// Update targets
	if ( _EventName ) _EventName->SetTarget( this );
	if ( _Location ) _Location->SetTarget( this );
	if ( _EventLastsWholeDays ) _EventLastsWholeDays->SetTarget( this );
	if ( _EventIsPrivate ) _EventIsPrivate->SetTarget( this );
	if ( _StartTimeHourMinControl ) _StartTimeHourMinControl->SetTarget( this );
	if ( _StartDateControl ) _StartDateControl->SetTarget( this );
	if ( _EndDateControl ) _EndDateControl->SetTarget( this );
	if ( _EndTimeHourMinControl ) _EndTimeHourMinControl->SetTarget( this );
	if ( _EndTimeEnabled ) _EndTimeEnabled->SetTarget( this );
}	// <-- end of function EventEditor_GeneralView::AttachedToWindow



/*!	\brief		Create the box for the end time.
 *		\attention	I assume that \c fEndTime data member is already calculated.
 */
BBox*			EventEditor_GeneralView::CreateEndMomentSelector()
{
	BFont font( be_plain_font );
	font_height fh;
	font.GetHeight( &fh );
	BBox* toReturn = new BBox( BRect( 0, 0, 1, 1 ), "End moment selector" );
	BGroupLayout* groupLayout = new BGroupLayout( B_VERTICAL );
	if ( !toReturn || !groupLayout ) { return NULL; }
	toReturn->SetLayout( groupLayout );
	
	toReturn->SetLabel( "End day and time" );
	
	groupLayout->SetInsets( 10, fh.leading + fh.ascent + fh.descent + 3, 10, 10 );
	
	// Filling contents
	BMessage* toSend = new BMessage( kEventNoDurationToggled );
	if ( !toSend ) {
		/* Panic! */
		delete toReturn;
		return NULL;	
	}
	_EndTimeEnabled = new BCheckBox( BRect( 0, 0, 1, 1 ),
												"Duration toggler",
												"Set duration to 0",
												toSend );
	if ( !_EndTimeEnabled ) {
		/* Panic! */
		delete toReturn;
		return NULL;	
	}
	_EndTimeEnabled->ResizeToPreferred();
	_EndTimeEnabled->SetValue( ( fData->GetDuration() == 0 ) );
	_EndTimeEnabled->SetToolTip( "If this box is checked, Event becomes\n"
											"a momentary occurrence - without end\n"
											"time and with duration of zero." );
	BLayoutItem* layoutItem = groupLayout->AddView( _EndTimeEnabled );
	if ( layoutItem ) {
		layoutItem->SetExplicitAlignment( BAlignment( B_ALIGN_LEFT, B_ALIGN_BOTTOM ) );
	}
	
	// Date control
		// End date selector
	toSend = new BMessage( kEndDateUpdated );
	if ( !toSend ) {
		/* Panic! */
		return NULL;
	}	
	_EndDateControl = new CalendarControl( BRect( 0, 0, 1, 1 ),
														"End date selector",
														"End date:",
														fEndTime.GetCalendarModule(),
														0,
														toSend );
	if ( !_EndDateControl ) {
		delete toReturn;
		return NULL;
	}
	_EndDateControl->InitTimeRepresentation( fEndTime );
	_EndDateControl->SetToolTip( "Remember: you can't select a date\n"
											"earlier then Start date." );
//	_EndDateControl->ResizeToPreferred();
	groupLayout->AddView( _EndDateControl );
	

	// End time selector													  
	toSend = new BMessage( kEndTimeUpdated );
	if ( !toSend ) {
		/* Panic! */
		delete toReturn;
		return NULL;
	}
	
	_EndTimeHourMinControl = new TimeHourMinControl( BRect( 0, 0, 1, 1 ),
																	"End time selector",
																	"End time:",
																	toSend );
	if ( !_EndTimeHourMinControl ) {
		/* Panic! */
		delete toReturn;
		return NULL;
	}	
	groupLayout->AddView( _EndTimeHourMinControl );
	_EndTimeHourMinControl->SetCurrentTime( fEndTime );
	
	// Enable or disable the controls according to the value of the checkbox
	SetEnabledStateOfEndTimeBox();
//	SetCorrectDuration();
	
	return toReturn;
}	// <-- end of function EventEditor_GeneralView::CreateEndMomentSelector



/*!	\brief		This function enables or disables the End Time box
 *		\details		It checks the value of the checkbox \c _EndTimeEnabled and
 *						sets the "enabled" state of the elements to strict accordance
 *						with that value.
 */
void		EventEditor_GeneralView::SetEnabledStateOfEndTimeBox()
{
	if ( !_EndTimeEnabled ) {
		return;
	}
	
	// Controls are enabled when checkbox is UNCHECKED
	bool toSet = ( _EndTimeEnabled->Value() == 0 );
	
	if ( _EndTimeHourMinControl ) {
			// Don't enable the control if event lasts whole days
		_EndTimeHourMinControl->SetEnabled( ( _EventLastsWholeDays->Value() == 0 ) && toSet );
	}
	if ( _EndDateControl ) {
		_EndDateControl->SetEnabled( toSet );
	}
	
}	// <-- end of function EventEditor_GeneralView::SetEnabledStateOfEndTimeBox()



/*!	\brief		Verify end time is still greater then start time.
 *		\details		This function also counts the new difference between start
 *						and end times.
 *		\param[in]	start		Reference to the time suspected to be Start time.
 *		\param[in]	end		Reference to the time suspected to be End time.
 *		\param[out]	newDuration	Pointer to placeholder for new duration.
 *		\returns		\c true if end time is greater then or equal to the start time,
 *						\c false if not.
 *						\c newDuration is filled only if the function returns \c true !
 *						In case of any error function returns \c false .
 */
bool		EventEditor_GeneralView::VerifyEndTimeIsGreaterThenStart( const TimeRepresentation& start,
																						 const TimeRepresentation& end,
																						 time_t* newDuration )
{
	// Get the calendar modules for start and end time representations.
	CalendarModule* startModule = utl_FindCalendarModule( start.GetCalendarModule() ),
						 *endModule = utl_FindCalendarModule( end.GetCalendarModule() );
	if ( !startModule || !endModule ) {
		return false;
	}
	
	// Convert the time representations to the time_t format
	time_t	startTimeT = startModule->FromLocalCalendarToTimeT( start ),
				endTimeT = endModule->FromLocalCalendarToTimeT( end );
	
	// Result
	if ( endTimeT < startTimeT ) {
		return false;
	}
	
	// Compute duration
	if ( newDuration ) {
		*newDuration = endTimeT - startTimeT;
		
		if ( _EventLastsWholeDays && ( _EventLastsWholeDays->Value() != 0 ) ) {
			*newDuration = CalculateNumberOfDaysForDuration( fDuration );
		}
	}
	return true;
	
}	// <-- end of function EventEditor_GeneralView::VerifyEndTimeIsGreaterThenStart



/*!	\brief		Updates the information on current duration.
 *		\param[in]	mayUpdate		If \c true (default), this function is allowed to
 *											change state of the checkbox.
 */
void 		EventEditor_GeneralView::UpdateDurationLengthLabel( bool mayUpdate ) {
	if ( !_DurationLength ) { return; }
	
	bool dataPrinted = false;
	BString sb;
	time_t duration = fDuration, seconds, minutes, hours, days;
	
	if ( duration == 0 ) {
		if ( mayUpdate && _EndTimeEnabled && ( _EndTimeEnabled->Value() == 0 ) ) {
//			_EndTimeEnabled->SetValue( 1 );
			SetEnabledStateOfEndTimeBox();
		}
		_DurationLength->SetText( "no duration." );
		return;
	}
	
	seconds = duration % 60;
	
	duration /= 60;		// Duration is now in minutes
	
	minutes = duration % 60;
	
	duration /= 60;		// Duration is now in hours
	
	hours = duration % 24;
	
	duration /= 24;		// Duration is now in days	
	days = duration;
	
	if ( days != 0 ) {
		sb << days;
		if ( days == 1 ) { sb << " day"; } else { sb << " days"; }
		dataPrinted = true;
	}
	
	if ( hours != 0 ) {
		if ( dataPrinted ) sb << ' ';
		sb << hours;
		if ( hours == 1 ) { sb << " hour"; } else { sb << " hours"; }
		dataPrinted = true;
	}
	
	if ( minutes != 0 ) {
		if ( dataPrinted ) sb << ' ';
		sb << minutes;
		if ( minutes == 1 ) { sb << " min"; } else { sb << " mins"; }
		dataPrinted = true;
	}
	
	if ( seconds != 0 ) {
		if ( dataPrinted ) sb << ' ';
		sb << seconds;
		sb << " sec";
	}
	sb << '.';
	
	_DurationLength->SetText( sb.String() );
}	// <-- end of function EventEditor_GeneralView::UpdateDurationLengthLabel()



/*!	\brief		Calculate how many whole days a given duration lasts.
 *		\param[in]	durIn			The duration to be checked.
 *		\param[in]	inSecs		If \c true (default), the output will be in seconds.
 *										Else, the output is in number of days.
 */
time_t		EventEditor_GeneralView::CalculateNumberOfDaysForDuration( time_t durIn,
																							  bool inSec )
{
	uint32 days = 0;
	
	if ( durIn != 0 ) { --durIn; }
	
	do {
		++days;
		durIn -= 60 * 60 * 24;
	} while ( durIn >= 0 );
	
	if ( inSec ) { 
		return ( time_t )( days * 60 * 60 * 24 );
	} else {
		return ( time_t )days;
	}
}	// <-- end of function EventEditor_GeneralView::CalculateNumberOfDaysForDuration



/*!	\brief		This function sets the time controls as enabled or disabled.
 *		\details		It finds out what the settings are according to checkbox values.
 *						It also sets the correct duration.
 */
void			EventEditor_GeneralView::SetCorrectDuration()
{
	bool	bIsAllDay = false;
	bool 	bIsEndDisabled = false;		//!< No duration
	
	if ( _EndTimeEnabled ) bIsEndDisabled = ( _EndTimeEnabled->Value() != 0 );
	if ( _EventLastsWholeDays ) bIsAllDay = ( _EventLastsWholeDays->Value() != 0 );
	
	CalendarModule* startCM = utl_FindCalendarModule( fStartTime.GetCalendarModule() );
	CalendarModule* endCM = utl_FindCalendarModule( fEndTime.GetCalendarModule() );
	
	if ( !startCM || !endCM ) { return; }
	
	if ( bIsAllDay )
	{
		// The minimal length of the Event is 1 day. Even if no end time is set,
		// the Event still lasts one day.
		if ( bIsEndDisabled ) {
			fDuration = 60 * 60 * 24;		// One day!
		} else {
			fDuration = CalculateNumberOfDaysForDuration( fDuration );
		}
	}
	else
	{
		if ( bIsEndDisabled ) {
			fDuration = 0;
		} else {
			fDuration = fPreviousDuration;
		}
	}
	fData->SetDuration( fDuration );
	if ( _DurationLength ) UpdateDurationLengthLabel();
}	// <-- end of function EventEditor_GeneralView::SetEnabledStateOfTimeControls

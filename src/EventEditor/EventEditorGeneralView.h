/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_EDITOR_GENERAL_VIEW_H_
#define _EVENT_EDITOR_GENERAL_VIEW_H_

// OS includes
#include <Box.h>
#include <SupportDefs.h>
#include <TextControl.h>
#include <View.h>

// Project includes
#include "CalendarControl.h"
#include "Category.h"
#include "CategoryItem.h"
#include "Event.h"
#include "TimeHourMinControl.h"
#include "Utilities.h"

/*!	\brief	Provides the way to editing some options of the Event data.
 */
class EventEditor_GeneralView
	:
	public BView
{
	public:
		EventEditor_GeneralView( BRect frame, EventData* data );
		~EventEditor_GeneralView();
		
		virtual	status_t		InitCheck() const { return _LastError; }

		virtual	void			MessageReceived( BMessage* in );
		virtual 	void			AttachedToWindow();

	protected:
		// Data holders
		EventData* 	fData;
		status_t		_LastError;
		
		TimeRepresentation	fStartTime;
		TimeRepresentation	fEndTime;
		CalendarModule*		fCalModule;
		time_t					fDuration;
		time_t					fPreviousDuration;
		
		// UI elements
		BTextControl*	_EventName;
		BTextControl*	_Location;
		CategoryMenu*	_CategoryMenu;
		BMenuField*		_CategoryMenuField;
		BCheckBox*		_EventLastsWholeDays;
		BCheckBox*		_EventIsPrivate;
		BStringView*	_DurationLabel;
		BStringView*	_DurationLength;
		
		// Start time selector
		BBox*				_StartMomentSelector;
		TimeHourMinControl*		_StartTimeHourMinControl;
		CalendarControl*			_StartDateControl;
		
		// End time selector
		BBox*				_EndMomentSelector;
		BCheckBox*					_EndTimeEnabled;
		TimeHourMinControl*		_EndTimeHourMinControl;
		CalendarControl*			_EndDateControl;
		
		// Service functions
		virtual BBox*	CreateStartMomentSelector();
		virtual BBox*	CreateEndMomentSelector();
		virtual void	SetEnabledStateOfEndTimeBox();
		virtual void	SetCorrectDuration();
		virtual void	UpdateDurationLengthLabel( bool mayUpdate = true );
		virtual bool	VerifyEndTimeIsGreaterThenStart( const TimeRepresentation& start,
																		const TimeRepresentation& end,
																		time_t* newDuration );
		virtual time_t	CalculateNumberOfDaysForDuration( time_t durIn,
																		 bool inSecs = true );
		
};	// <-- end of class EventEditor_GeneralView

#endif // _EVENT_EDITOR_GENERAL_VIEW_H_

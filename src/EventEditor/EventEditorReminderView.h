/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_EDITOR_REMINDER_VIEW_H_
#define _EVENT_EDITOR_REMINDER_VIEW_H_


// OS includes
#include <CheckBox.h>
#include <Message.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <View.h>

// Project includes
#include "ActivityView.h"
#include "Event.h"
#include "GeneralHourMinControl.h"


/*!	\brief		Allows the user to set up and modify the reminder.
 */
class EventEditor_ReminderView 
	:
	public BView
{
public:
	EventEditor_ReminderView( BRect frame, EventData* data );
	~EventEditor_ReminderView();
	
	virtual void		MessageReceived( BMessage* in );
	virtual void		AttachedToWindow();
	virtual status_t	InitCheck() const { return fLastError; }
private:

	// Data holders
	bool bIsReminderEnabled;
	uint32				 fHours, fMinutes;
	EventData*			 fData;
	status_t				 fLastError;
	
	// UI elements
	BCheckBox*			 fReminderEnabler;
	BStringView*		 fExplanation;
	BStringView*		 fExplanation2;
	GeneralHourMinControl*	fHourMinControl;
	ActivityView*		 fActivityView;
	
	// Service functions
//	virtual	void		CheckboxToggled();

};	// <-- end of class EventEditor_ReminderView

#endif // _EVENT_EDITOR_REMINDER_VIEW_H_

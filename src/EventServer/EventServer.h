/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_SERVER_H_
#define _EVENT_SERVER_H_

// OS includes
#include <Application.h>
#include <Entry.h>
#include <Message.h>
#include <Query.h>
#include <SupportDefs.h>


extern uint32	global_toReturn;

/*!	\brief		Class that polls the events and finds what events should fire.
 */
class EventServer :
	public BApplication
{
public:
	EventServer();
	~EventServer();
	
	virtual void ReadyToRun();
	virtual void MessageReceived( BMessage* in );
	virtual void AboutRequested();
	
	virtual void Pulse();
	
protected:
	//!	\name		Data members
	///@{
	BQuery fEventQuery;		//!< Query working with events' start time
	BQuery fReminderQuery;	//!< Query working with reminders' start time
	time_t fCurrentTime;		//!< Current time
	BMessenger*	fCurrentMessenger;	//!< Way to send messages to the current application.
	///@}
	
	//!	\name		Service functions
	///@{
	virtual void		PerformEventQuery();
	virtual void		PrepareEventQuery();
	
	virtual void		PerformReminderQuery();
	virtual void		PrepareReminderQuery();

	virtual void		DealWithEntries( BQuery* in, bool bReminder = false );
	
	static  void		SnoozeActivity( entry_ref ref, bool bReminder,
												 int32 hours, int32 minutes );
	
	virtual void		UpdateCategories();
	///@}
};



#endif // _EVENT_SERVER_H_

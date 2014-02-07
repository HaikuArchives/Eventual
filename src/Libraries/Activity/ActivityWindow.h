/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ACTIVITY_WINDOW_H_
#define _ACTIVITY_WINDOW_H_

// OS includes
#include <Message.h>
#include <Messenger.h>
#include <ScrollView.h>
#include <String.h>
#include <SupportDefs.h>
#include <Window.h>


// Project includes
#include "ActivityData.h"
#include "Category.h"
#include "GeneralHourMinControl.h"
#include "Utilities.h"
#include "Preferences.h"


const	uint32		kActitivyWindowRepsonceMessage		= 'AWRM';


/*!	\brief		This window is used for displaying Activity.
 *		\note			Construction may fail!
 *						The caller should always check output of \c InitCheck() function
 *						before trying to \c Show() an object of this class. However, if
 *						there's no activity to perform, the \c InitCheck() returns
 *						\c B_NO_INIT - but this is \b not an error.
 */
class ActivityWindow 
	:
	public BWindow
{
public:
	ActivityWindow( ActivityData* data,
					  BMessenger* target,
					  BString	  name,
					  Category*	  category,
					  BMessage* templateMessage = NULL,
					  bool reminder = false );
	virtual 	~ActivityWindow();
	
	virtual 	void	MessageReceived( BMessage* in );

	virtual	BMessage*	Message() const { return fTemplateMessage; }
	virtual	void			SetMessage( BMessage* in ) {
		if ( fTemplateMessage ) { delete fTemplateMessage; }
		fTemplateMessage = in;
	}
	
	virtual	status_t		InitCheck() const { return fLastError; }
	

protected:

	//!	\name		Data placeholders
	///@{ 
	BMessenger*		fTarget;
	ActivityData*	fData;
	BMessage*		fTemplateMessage;
	bool				bIsReminder;
	status_t			fLastError;
	BString			fEventName;
	Category			fCategory;
	uint32			fSnoozeHours;
	uint32			fSnoozeMins;
	///@}
	
	//!	\name  	UI elements.
	///@{
	BStringView					*fTitle;
	BStringView					*fEventNameView;
	BStringView					*fCategoryView;
	BScrollView					*fTextScroller;
	GeneralHourMinControl	*fSnoozeTime;
	BTextView					*fNoteText;
	BButton						*fSnooze;
	BButton						*fOk;
	BView							*fBackground;
	//@}
	
	//!	\name		Service functions
	///@{
//	virtual void	UpdateGeneralHourMinControlSettings();
	///@}
};

#endif // _ACTION_WINDOW_H_

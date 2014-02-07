/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ACTIVITY_VIEW_H_
#define _ACTIVITY_VIEW_H_

#include "NotificationView.h"
#include "SoundSetupView.h"
#include "ProgramSetupView.h"


/*---------------------------------------------------------------------------
 *						Declaration of class ActivityView
 *--------------------------------------------------------------------------*/

/*!	\brief		This class provides options to edit the activities.
 */
class ActivityView
	:
	public BView
{
public:
	// Constructor and destructor
	ActivityView( BRect frame,
					  const char* name,
					  ActivityData* toEdit );
	virtual ~ActivityView();
	
	// UI-related functions
	virtual void	AttachedToWindow();
	
	// Save the Activity Data
	virtual void	SaveData() {
		if ( fNotView ) { fNotView->SaveData(); }
		if ( fSoundView ) { fSoundView->SaveData(); }
		if ( fProgView ) { fProgView->SaveData(); }	
	}
	
	// Get the Activity Data
	inline virtual ActivityData*	GetActivityData( void ) const { return fData; }
	
	inline virtual status_t		InitCheck() const { return fLastError; }
	
	inline virtual bool IsEnabled() const { return bIsEnabled; }
	virtual void	SetEnabled( bool toSet );
	
protected:
	// Data placeholders
	ActivityData* 		fData;
	NotificationView* fNotView;
	SoundSetupView*	fSoundView;
	ProgramSetupView*	fProgView;
	status_t				fLastError;
	bool					bIsEnabled;
	
	// Service functions
	virtual void 	BuildUI( void ) {};

};	// <-- end of class ActivityView


#endif // _ACTIVITY_VIEW_H_

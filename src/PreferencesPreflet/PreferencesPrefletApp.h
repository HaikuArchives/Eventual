/*! \file	PreferencesPrefletApp.h
 *	\brief	Declarations of the main application class of the Preflet.
 *	\details	Based on Skeleton application class by Kevin H. Patterson.
 */

//Title: Skeleton Application Class
//Platform: BeOS 5
//Version: 2001.12.18
//Description:
//	The application class.
//	This class inherits from BApplication, and runs the application's message loop.
//Copyright (C) 2001, 2002 Kevin H. Patterson

#ifndef _PREFERENCES_PREFLET_APP_H_
#define _PREFERENCES_PREFLET_APP_H_
	
#include <Application.h>

#include "PreferencesPrefletMainWindow.h"
// #include "globals.h"
#include "Utilities.h"

/*!	\class		PreferencesPrefletApp
 *	\brief		Implements the application.
 */
class PreferencesPrefletApp
	:
	public BApplication
{
public:
	PreferencesPrefletApp();
	~PreferencesPrefletApp();
	virtual void ReadyToRun();
	virtual void Pulse();
	inline virtual void Quit( uint32 status = B_NO_MEMORY ) {
		AppReturnValue = status;
		be_app->PostMessage(B_QUIT_REQUESTED);
	};
private:
	PreferencesPrefletMainWindow *fMainWindow;
};	// <-- end of class PreferencesPrefletApp
	
#endif	// _PREFERENCES_PREFLET_APP_H_

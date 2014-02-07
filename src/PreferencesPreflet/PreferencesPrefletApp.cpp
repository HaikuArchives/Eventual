/*! \file	PreferencesPrefletApp.cpp
 *	\brief	Implementation of the main application class of the Preflet.
 *	\details	Based on Skeleton application class by Kevin H. Patterson.
 */


//Title: Skeleton Application Class
//Platform: BeOS 5
//Version: 2001.12.18
//Description:
//	The application class.
//	This class inherits from BApplication, and runs the application's message loop.

//Copyright (C) 2001, 2002 Kevin H. Patterson

#include "PreferencesPrefletApp.h"


int AppReturnValue = 0;

/*!	
 *	\brief		Main function of the application. 
 */
int main(int, char **)
{
	AppReturnValue = B_ERROR;

	new PreferencesPrefletApp();	//A class derived from BApplication is automatically stored
								//in the global variable be_app upon instantiation.

	if (be_app != NULL)
	{
		AppReturnValue = B_OK;
		be_app->Run();
	}
	else
		AppReturnValue = B_NO_MEMORY;

	delete be_app;
	return AppReturnValue;
}	// <-- end of function "main"



/*!	
 *	\brief			Constructor for the application.
 */
PreferencesPrefletApp::PreferencesPrefletApp()
:
	BApplication( kPreferencesPrefletApplicationSignature ),
	fMainWindow(NULL)
{
	fMainWindow = new PreferencesPrefletMainWindow();
	if (fMainWindow != NULL)
	{
		fMainWindow->Show();
	}
	else
	{
		AppReturnValue = B_NO_MEMORY;
		be_app->PostMessage(B_QUIT_REQUESTED);
	}
}	// <-- end of constructor



/*!	
 *	\brief		Application's destructor.
 */
PreferencesPrefletApp::~PreferencesPrefletApp()
{
	if (fMainWindow != NULL)
		if (fMainWindow->LockWithTimeout(30000000) == B_OK)	// Must Lock() before calling Quit()
			fMainWindow->Quit();	// Never delete a BWindow, call Quit() instead.
}	// <-- end of application's destructor



void PreferencesPrefletApp::ReadyToRun()
{
	//A good place to set up windows, etc., just before your application is about to Run().
}

void PreferencesPrefletApp::Pulse()
{
	//Provides a "heartbeat" for your application; a good place to blink cursors, etc.
	//You set the pulse rate in BApplication::SetPulseRate().
}

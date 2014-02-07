/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef _EVENT_EDITOR_APPLICATION_H_
#define _EVENT_EDITOR_APPLICATION_H_
	
#define ApplicationSignature "application/x-vnd.generic-SkeletonApplication"

#include <Application.h>

#include "EventEditorMainWindow.h"


extern uint32 global_toReturn;


class EventEditorApplication
	:
	public BApplication
{
public:
	EventEditorApplication( int argc, char** argv );
	~EventEditorApplication();
	
	virtual void	ArgvReceived( int32 argc, char* argv[] );
	virtual void	AboutRequested();
	virtual void ReadyToRun();
	virtual void Pulse();
private:
	EventEditorMainWindow *fMainWindow;
};
	
#endif	// _EVENT_EDITOR_APPLICATION_H_

/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EventEditorApp.h"

// Global variable for return value
uint32		global_toReturn = B_OK;

int main( int argc, char **argv )
{
	global_toReturn = B_ERROR;

	new EventEditorApplication( argc, argv );
	
	if ( be_app != NULL )
	{
		global_toReturn = B_OK;
		be_app->Run();
	}
	else
	{
		global_toReturn = B_NO_MEMORY;
	}

	delete be_app;
	return global_toReturn;
}

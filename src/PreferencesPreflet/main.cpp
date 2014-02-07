//Title: Skeleton Main Module
//Platform: BeOS 5
//Version: 2001.12.18
//Description:
//	The main program module.
//	Includes global variables, global functions, and the main() function.

//Copyright (C) 2001, 2002 Kevin H. Patterson
//Available for use under the terms of the BSD license agreement:
//See the associated file "License.txt" for details.

#include "globals.h"
#include "clsApp.h"

int AppReturnValue(0);

int main(int, char **)
{
	AppReturnValue = B_ERROR;

	new clsApp();	//A class derived from BApplication is automatically stored
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
}

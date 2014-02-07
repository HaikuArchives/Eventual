/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <Alert.h>
#include <GraphicsDefs.h>
#include <SupportDefs.h>

//  #include "TimeRepresentation.h"
// #include "CalendarModule.h"

/*---------------------------------------------------------------------------------
 *			Applications' signatures and MIME types section
 *--------------------------------------------------------------------------------*/

extern const char*		kPreferencesPrefletApplicationSignature;

extern const char*		kEventEditorApplicationSignature;

extern const char*		kEventServerApplicationSignature;
 
extern const char*		kEventViewerApplicationSignature;

extern const char*		kEventFileMIMEType;

/*---------------------------------------------------------------------------------
 *			Calendar modules section
 *--------------------------------------------------------------------------------*/
class CalendarModule;
 
#define		NUMBER_OF_CALENDAR_MODULES		1

extern BList global_ListOfCalendarModules;	//!< List of all calendar modules in the system.

CalendarModule*		utl_FindCalendarModule( const BString& id );

/*---------------------------------------------------------------------------------
 *			Categories section
 *--------------------------------------------------------------------------------*/

extern BList global_ListOfCategories;	//!< List that holds all categories in the system.


/*---------------------------------------------------------------------------------
 *			Preferences section
 *--------------------------------------------------------------------------------*/

const uint32 kGlobalPreferences = 'PREF';	//!< ID of the global preferences.
extern BMessage global_Preferences;		//!< Message with all existing preferences.


/*---------------------------------------------------------------------------------
 *			Service routines section
 *--------------------------------------------------------------------------------*/

	/* Register the application's filetype if it isn't already registered. */
void	utl_RegisterFileType( void );

	/* Check if the string is valid */
bool 	utl_CheckStringValidity( BString& input );

	/* Syntactical verification of Email address */
bool	utl_VerifyEmailAddress( const char* );

	/* Syntactical verification of server address */
bool	utl_VerifySeverAddress( const char* address );

/*!	\brief		Allows easy convertion of uint32 to a set of chars and vice versa.
 */
union UintToChar {
	char    chars[4];
	uint32	integer;	
};


/*!
 *	\brief		Allows to refer to rgb_color to as uint32
 */
uint32			RepresentColorAsUint32( rgb_color color );
rgb_color		RepresentUint32AsColor( uint32 set );

/*!
 *	\brief		A short debugging message
 *	\details	Based on BAlert class
 */
class DebuggerPrintout
	:
	public BAlert
{
public:
	inline DebuggerPrintout(const char* message)
		:
		BAlert("Printout", message, "Ok")
	{
		this->Go();
	}				
private:

};

	/* Pre-created debugger printout. */
extern DebuggerPrintout* utl_Deb;


/*----------------------------------------------------------------
 *				MIME type setup section
 *---------------------------------------------------------------*/
 
/*!	\brief		This structure helps to define the attributes of the Event files.
 */
struct DefaultAttribute {
	const char*	internalName;
	const char* humanReadableName;
	uint32		type;
	bool			publicAttr;
	bool			editableAttr;
	bool 			indexedAttr;
	int			fieldWidth;
};


extern		struct DefaultAttribute		AttributesArray[];




#endif // _UTILITIES_H_

/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

// System includes
#include <ctype.h> 
#include <stdio.h>
#include <string.h>

// OS includes
#include <Application.h>
#include <Bitmap.h>
#include <List.h>
#include <Message.h>
#include <MimeType.h>
#include <Roster.h>
#include <Size.h>
#include <String.h>
#include <Resources.h>
#include <TypeConstants.h>
#include <Volume.h>
#include <VolumeRoster.h>

#include <fs_index.h>

// Project includes
#include "Utilities.h"
#include "CalendarModule.h"

/*---------------------------------------------------------------------------------
 *			Applications' signatures and MIME types section
 *--------------------------------------------------------------------------------*/

const char*		kPreferencesPrefletApplicationSignature = "application/x-vnd.Hitech.Eventual.Preferences";

const char*		kEventEditorApplicationSignature = "application/x-vnd.Hitech.Eventual.EventEditor";

const char*		kEventServerApplicationSignature = "application/x-vnd.Hitech.Eventual.EventServer";

const char*		kEventViewerApplicationSignature = "application/x-vnd.Hitech.Eventual.EventViewer";

const char*		kEventFileMIMEType = "text/x-vnd.Hitech-event";


/*!	\brief		Debugging printout - defined once and globally.
 */
DebuggerPrintout		*utl_Deb = NULL;


/*!
 *	\brief		List that holds all categories in the system. 
 *	\details	It's part of the utilities since it's included in all parts of the
 *				program. Every part of the program initializes this object for itself.
 *				Sample initialization is in CategoryItem file.
 *	\sa			::PopulateListOfCategories( BMessage* in )
 */
BList global_ListOfCategories;


BList global_ListOfCalendarModules( NUMBER_OF_CALENDAR_MODULES );


/*!	\function 	utl_CheckStringValidity
 *	\brief		Verify the string submitted by the user is valid.
 *	\details	Perform also some adjustments, described below.
 *	\returns	"true" if the string is valid, "false" if user must correct it.
 *	\param[in]	toCheck	Reference to BString which should be checked. This is also output.
 */
bool utl_CheckStringValidity( BString& toCheck )
{
	/*!	\par	Rules of validity for strings
	 *			1) A string lasts from beginning to first "newline" character.
	 *			   Everything after "newline" character is trimmed.
	 *			2) All whitespaces are replaced with "space" char.
	 *			3) All sequences of more then one "space" are replaced with a single "space".
	 *			4) All leading and trailing space are trimmed.
	 *			5) If the string is empty, it's invalid.
	 *			String that consists of punctuation mark only is allowed!
	 */
	int index;
	
	if ( toCheck.CountChars() == 0 )
	{
		return false;
	}
	
	BString workingCopy( toCheck );
	
	/* Trim everything after first newline character. */
	if ( ( index = workingCopy.FindFirst("\n") ) != B_ERROR )
	{
		// index is the position of the first newline character
		if ( index > 0 )
		{
			workingCopy.Truncate( index - 1 );	
		} else {
			workingCopy.Truncate( 0 );
		}
	}
	
	/* Remove leading and trailing whitespaces. */
	workingCopy = workingCopy.Trim();
	
	/* Replace every whitespace with a "space" character. */
	workingCopy = workingCopy.ReplaceSet( " \t\a\n\r\v", ' ');
	
	/* Replace every two sequentive whitespaces with a single space. */
	while ( ( index = workingCopy.FindFirst("  ") ) != B_ERROR )
	{
		workingCopy = workingCopy.IReplaceAll("  ", " ");
	}	// <-- end of "while ( there are two sequentive spaces )"
	
	if ( workingCopy.CountChars() == 0 ) {
		return false;
	} else {
		toCheck = workingCopy;
		return true;
	}

}	// <-- end of function "utl_CheckStringValidity"


/*!	\brief		Find calendar module based on its ID.
 *	\param[in]	id	The identifier of the Calendar Module.
 *	\returns	Valid pointer to calendar module object, if it is found.
 *				NULL, if it's not.
 */
CalendarModule* 	utl_FindCalendarModule( const BString& id )
{
	int index, limit;
	CalendarModule* testing;
	
	limit = global_ListOfCalendarModules.CountItems();
	
	for ( index = 0; index < limit; ++index )
	{
		testing = ( CalendarModule* )global_ListOfCalendarModules.ItemAt( index );
		if ( testing->Identify() == id )
		{
			return testing;	
		}
	}
	
	return NULL;	
}	// <-- end of function FindCalendarModule



uint32	RepresentColorAsUint32( rgb_color color )
{
	uint32 toReturn, red, green, blue, alpha;
	
	red   = color.red   << 24;
	green = color.green << 16;
	blue  = color.blue  << 8;
	alpha = color.alpha;
	
	toReturn =  ( red   & 0xFF000000 ) 	|
				( green & 0x00FF0000 )  |
				( blue  & 0x0000FF00 )  |
				( alpha & 0x000000FF );
	
	return toReturn;
}


rgb_color RepresentUint32AsColor( uint32 in )
{
	rgb_color toReturn;
	toReturn.red 	=	( in & 0xFF000000 ) >> 	24;
	toReturn.green = 	( in & 0x00FF0000 ) >>  16;
	toReturn.blue 	= 	( in & 0x0000FF00 ) >>  8;
	toReturn.alpha = 	( in & 0x000000FF );
	return toReturn;
}


/*!	\brief		Syntactically verify a string for being an Email address.
 *		\details		This recipe is an adopted code from the book
 *						"Secure Programming Cookbook for C and C++" by 
 *						John Viega and Matt Messier. It can be found as Recipe 3.9
 *						beginning on page 101.
 *		\returns		"true" if the string looks like an Email address, "false" otherwise.
 */
bool utl_VerifyEmailAddress( const char *address )
{
  int        count = 0;
  const char *c, *domain;
  static char *rfc822_specials = "()<>@,;:\\\"[]";
  
  if ( strlen( address ) == 0 ) {
		// Empty string
		return true;
  }

  /* first we validate the name portion (name@domain) */
  for ( c = address;  *c;  c++ ) 
  {
    if ( ( *c == '\"' ) && 
    	   ( c == address || *(c - 1) == '.' || *(c - 1) == '\"' ) )
   {
      while (*++c) {
        if (*c == '\"') break;
        if (*c == '\\' && (*++c == ' ')) continue;
        if (*c < ' ' || *c >= 127) return false;
      }
      if (!*c++) return false;
      if (*c == '@') break;
      if (*c != '.') return false;
      continue;
    }
    if (*c == '@') break;
    if (*c <= ' ' || *c >= 127) return false;
    if ( strchr( rfc822_specials, *c ) ) return false;
  }
  if (c == address || *(c - 1) == '.') return false;

  /* next we validate the domain portion (name@domain) */
  if (!*(domain = ++c)) return false;
  do {
    if (*c == '.') {
      if (c == domain || *(c - 1) == '.') return false;
      count++;
    }
    if (*c <= ' ' || *c >= 127) return false;
    if (strchr(rfc822_specials, *c)) return false;
  } while (*++c);

  return (count >= 1);
}	// <-- end of function utl_VerifyEmailAddress


/*!	\brief		Verify a server address or IP.
 *		\details		Based on the function utl_VerifyEmailAddress.
 *		\note			Implementation details
 *						This function does not test DNS or pings the server. It just
 *						checks the address looks correct.
 *		\returns		"true" if the address looks Ok, "false" otherwise.
 */
bool		utl_VerifySeverAddress( const char* address )
{
  int        count = 0;
  const char *c = address, *domain;
  static char *rfc822_specials = "()<>@,;:\\\"[]";

	if ( strlen( address ) == 0 ) { return true; }

  if (!*(domain = ++c)) return false;
  do {
    if (*c == '.') {
      if (c == domain || *(c - 1) == '.') return false;
      count++;
    }
    if (*c <= ' ' || *c >= 127) return false;
    if (strchr(rfc822_specials, *c)) return false;
  } while (*++c);

  return (count >= 1);
}	// <-- end of function utl_VerifySeverAddress



/* This message loads upon startup and is saved by the Preferences preflet.
 */
BMessage global_Preferences( kGlobalPreferences );



/*---------------------------------------------------------------------------------
 *							MIME type setup section
 *--------------------------------------------------------------------------------*/

/*!	\brief		This array is the array of the defined attributes.
 */
struct DefaultAttribute AttributesArray[] = {
	// internalName				humanReadable			type					public	editable	indexed		width
	{	"EVNT:name",				"Event name",			B_STRING_TYPE,		true,		true,		true,			255	},
	{	"EVNT:category",			"Category",				B_STRING_TYPE,		true,		false,	true,			255	},
	{	"EVNT:where",				"Location",				B_STRING_TYPE,		true,		true,		true,			255	},
	{	"EVNT:private",			"Private",				B_UINT32_TYPE,		false,	false,	false,		70		},
	{	"EVNT:verified",			"Settings verified",	B_UINT32_TYPE,		false,	false,	false,		70		},
	{  "EVNT:whole_day",			"Lasts whole days",	B_UINT32_TYPE,		false,	false,	false,		70		},
	{	"EVNT:type",				"Event type",			B_UINT32_TYPE,		false,	false,	false,		70		},
	{	"EVNT:cal_module",		"Calendar module ID",B_STRING_TYPE,		true,		false,	false,		255	},
	{	"EVNT:duration",			"Duration",				B_UINT32_TYPE,		true,		false,	true,			50	},
	{	"EVNT:next_occurrence",	"Next occurrence",	B_UINT32_TYPE,		false,	false,	true,			70	},
	{	"EVNT:and_rules",			"Inclusion rules",	B_RAW_TYPE,			false,	false,	false,		70	},
	{	"EVNT:not_rules",			"Exclusion rules",	B_RAW_TYPE,			false,	false,	false,		70	},
	{	"EVNT:start_TR",			"Start time",			B_RAW_TYPE,			false,	false,	false,		70	},
	{	"EVNT:event_activity",	"Event Activity",		B_RAW_TYPE,			false,	false,	false,		70	},
	{	"EVNT:activity_fired",	"Activity fired",		B_UINT32_TYPE,		true,		false,	true,			70	},
	{	"EVNT:reminder_offset",	"Reminder offset",	B_UINT32_TYPE,		true,		false,	false,		70 },
	{	"EVNT:next_reminder",	"Next reminder",		B_UINT32_TYPE,		false,	false,	true,			70	},
	{	"EVNT:reminder_activity","Reminder Acitivty",B_RAW_TYPE,			false,	false,	false,		70	},
	{	"EVNT:reminder_fired",	"Reminder fired",		B_UINT32_TYPE,		true,		false,	true,			70	},
	
	{	NULL,							NULL,						B_ANY_TYPE,			false,	false,	false,		0	}
};	// <-- end of AttributesArray
 	


/*!	\brief		Registers the Event file type and additional data.
 *		\details		This function should be called every time upon startup. It does
 *						not perform any changes, if the file type is registered.
 */
void 		utl_RegisterFileType( void )
{
	// Obtain the Eventual icon from resources of the application file
	app_info info;
	be_app->GetAppInfo(&info);
	BFile file(&info.ref, B_READ_ONLY);
	size_t	length;
	BResources res;
	BMessage extensionsInfo;
	BString sb;
	status_t	error;
	const uint8* rawIcon = NULL;
	
	if ( res.SetTo(&file) == B_OK )
	{
		rawIcon = ( const uint8* )res.LoadResource( B_VECTOR_ICON_TYPE, 99, &length );
		if ( ! rawIcon ) {
			printf( "Did not succeed to load vector icon for filetype.\n" );
		} else {
			printf( "Loaded vector icon for filetype of length %u successfully.\n", ( unsigned int )length );
		}
	} else {
		rawIcon = NULL; 
	}
	
	BMimeType mimeType( kEventFileMIMEType );
	if ( mimeType.InitCheck() != B_OK )
		utl_Deb = new DebuggerPrintout( "Initialization was unsuccessful." );
	
		// Making the changes to the MIME database
	if ( mimeType.IsInstalled() ) {
		if ( mimeType.Delete() != B_OK )
			utl_Deb = new DebuggerPrintout( "Didn't succeed to delete the file type." );
	}
	
	if ( mimeType.Install() != B_OK )
	{
		utl_Deb = new DebuggerPrintout( "Didn't succeed to register the file type." );
	}
	else
	{
//		Message on successful installation.
//		utl_Deb = new DebuggerPrintout( "Registered the file type successfully." );
	}

	if ( mimeType.SetShortDescription( "\"Eventual\" event file" ) != B_OK )
		utl_Deb = new DebuggerPrintout( "Didn't succeed to set short description of Event file." );
		
	if ( mimeType.SetLongDescription( "File containing information about single Event." ) != B_OK )
		utl_Deb = new DebuggerPrintout( "Didn't succeed to set short description of Event file." );
	
	if ( mimeType.SetPreferredApp( kEventEditorApplicationSignature ) != B_OK )
		utl_Deb = new DebuggerPrintout( "Didn't succeed to set preferred application signature." );
		
	if ( rawIcon ) {
		mimeType.SetIcon( ( const uint8* )rawIcon, length );
/*****************************************************************************
		The following code is used to set the old BeOS-style icons
		----------------------------------------------------------

		BBitmap *smallIconRGBA, *largeIconRGBA,
			  *smallIconCMAP, *largeIconCMAP;
		
		// Making placeholders for the icons
			// Mini icon
		smallIconRGBA = new BBitmap( BRect( 0, 0, 15, 15 ),
											 B_RGBA32,
											 false );
		smallIconCMAP = new BBitmap( BRect( 0, 0, 15, 15 ),
											 B_CMAP8,
											 false );
			// Large icon
		largeIconRGBA = new BBitmap( BRect( 0, 0, 31, 31 ),
											 B_RGBA32,
											 false );
		largeIconCMAP = new BBitmap( BRect( 0, 0, 31, 31 ),
											 B_CMAP8,
											 false );
		// Set the small icon
		if ( smallIconRGBA && smallIconCMAP ) {
			if ( ( BIconUtils::GetVectorIcon( rawIcon, length, smallIconRGBA ) == B_OK ) &&
				  ( BIconUtils::ConvertToCMAP8( smallIconRGBA, smallIconCMAP ) == B_OK ) )
			{
				// Setting the mini icon for type
				mimeType.SetIcon( smallIconCMAP, B_MINI_ICON );
			} else {
				// Did not succeed to read or convert the icon
				delete smallIconCMAP;
				delete smallIconRGBA;
				smallIconCMAP = smallIconRGBA = NULL;
			}
		}
		else
		{
			// Something went bad; need to deallocate both icons
			if ( smallIconRGBA ) {
				delete smallIconRGBA; smallIconRGBA = NULL;
			}
			if ( smallIconCMAP ) {
				delete smallIconCMAP; smallIconCMAP = NULL;
			}
		}
		
		// Set the large icon
		if ( largeIconRGBA && largeIconCMAP ) {
			if ( ( BIconUtils::GetVectorIcon( rawIcon, length, largeIconRGBA ) == B_OK ) &&
			 	  ( BIconUtils::ConvertToCMAP8( largeIconRGBA, largeIconCMAP ) == B_OK ) )
			{
				// Setting the icon for type
				mimeType.SetIcon( largeIconCMAP, B_ICON_LARGE );
			} else {
				// Did not succeed to read or to convert the icon
				delete largeIconRGBA;
				delete largeIconCMAP;
				largeIconRGBA = largeIconCMAP = NULL;
			}
		}
		else
		{
			// Something went bad; need to deallocate both icons
			if ( largeIconRGBA ) {
				delete largeIconRGBA; largeIconRGBA = NULL;
			}
			if ( largeIconCMAP ) {
				delete largeIconCMAP; largeIconCMAP = NULL;
			}
		}
		
		END of section of setting the icons in 
***********************************************************************************/
	}
	
	// Working on extentions
	extensionsInfo.AddString( "extensions", "event" );
		/* The information on other supported extentions is clobberred on purpose!
		 */
	if ( mimeType.SetFileExtensions( &extensionsInfo ) != B_OK )
		utl_Deb = new DebuggerPrintout( "Didn't succeed to install the extensions." );

	int i = 0;
	BMessage attributes;
	while ( AttributesArray[ i ].internalName != NULL )
	{
		attributes.AddString( "attr:name", AttributesArray[ i ].internalName );
		attributes.AddString( "attr:public_name", AttributesArray[ i ].humanReadableName );
		attributes.AddInt32( "attr:type", AttributesArray[ i ].type );
		attributes.AddBool( "attr:public", AttributesArray[ i ].publicAttr );
		if ( ( AttributesArray[ i ].publicAttr ) &&
			  ( AttributesArray[ i ].fieldWidth != 0 ) )
		{
			attributes.AddBool( "attr:viewable", AttributesArray[ i ].publicAttr );
			attributes.AddInt32( "attr:width", AttributesArray[ i ].fieldWidth );
			attributes.AddInt32( "attr:alignment", B_ALIGN_LEFT );
		}
		else
		{
			attributes.AddBool( "attr:viewable", false );	
		}
		attributes.AddBool( "attr:editable", AttributesArray[ i ].editableAttr );
		attributes.AddBool( "attr:indexed", AttributesArray[ i ].indexedAttr );
			
		++i;
	}
	
	mimeType.SetAttrInfo( &attributes );
	
	// Index all indexable attributes on boot volume only
	BVolume bootVolume;
	BVolumeRoster volumeRoster;
	volumeRoster.GetBootVolume( &bootVolume );
	i = 0;
	index_info		checkIfIndexIsInstalled;
	while ( AttributesArray[ i ].internalName != NULL )
	{
		if ( AttributesArray[ i ].indexedAttr )
		{
			// Check - if this index is already installed, no need to install it again.
			error = fs_stat_index( bootVolume.Device(),
							 			  AttributesArray[ i ].internalName,
							 			  &checkIfIndexIsInstalled	);
			if ( error == B_OK ) {
				++i;
				continue;
			}

			// The index was not installed, so try to install it
			if ( 0 != ( error = fs_create_index( bootVolume.Device(),
															 AttributesArray[ i ].internalName,
															 AttributesArray[ i ].type,
															 0 ) ) ) 		// Flags are always 0
			{
				sb.SetTo( "Could not create index! Error is " );
				switch ( error ) {
					case B_BAD_VALUE:
						sb << "Device does not exist, or Name is reserved."; break;
					case B_NOT_ALLOWED:
						sb << "Device is read-only."; break;
					case B_NO_MEMORY:
						sb << "Not enough memory."; break;
					case B_FILE_EXISTS:
						sb << "This index already exists."; break;
					case B_DEVICE_FULL:
						sb << "Not enough space."; break;
					case B_FILE_ERROR:
						sb << "Invalid directory reference."; break;
					case B_ERROR:
						sb << "This index type isn't supported."; break;
					default:
						sb << "Unknown error has occurred."; break;
				};
				utl_Deb = new DebuggerPrintout( sb.String() );
			}
		}
		++i;
	}

}	// <-- end of function utl_RegisterFileType

/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _GENERAL_HOUR_MIN_CONTROL_H
#define _GENERAL_HOUR_MIN_CONTROL_H

// System includes
#include <CheckBox.h>
#include <Control.h>
#include <LayoutItem.h>
#include <Menu.h>
#include <MenuBar.h>
#include <Message.h>
#include <Rect.h>
#include <String.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <View.h>

// Local includes
#include "TimeRepresentation.h"


/*------------------------------------------------------------------------
 *						Message constants
 *-----------------------------------------------------------------------*/

	// Getting data on internal components
const uint32		kHoursUpdated						= 'HoUP';
const	uint32		kMinutesUpdated					= 'MiUP';
const uint32		kCheckBoxToggled					= 'CbUP';

	// Default message variable sent to outside world
const uint32		kGeneralHourMinControlUpdated	= 'GHMC';

extern const BString		kHoursValueKey;
extern const BString		kMinutesValueKey;
extern const BString		kCheckBoxExistsKey;
extern const BString		kCheckBoxValueKey;


/*------------------------------------------------------------------------
 *						Masks and macros
 *-----------------------------------------------------------------------*/

/* Defines */
#define		GeneralHourMinControl_HOURS_MASK					( 0x0FFF0000 )
#define		GeneralHourMinControl_MINUTES_MASK				( 0x0000FFFF )
#define		GeneralHourMinControl_CHECKBOX_EXISTS_MASK	( 0x80000000 )
#define		GeneralHourMinControl_CHECKBOX_VALUE_MASK		( 0x40000000 )

/* Macros */

/*!	\brief		Returns the hours value from submitted variable, according to mask.
 */
#define		GeneralHourMinControl_GET_HOURS( _uint32_value_ )	\
	( ( _uint32_value_ & GeneralHourMinControl_HOURS_MASK ) >> 16 )

/*!	\brief		Returns the minutes value from submitted variable, according to mask.
 */
#define		GeneralHourMinControl_GET_MINUTES( _uint32_value_ )	\
	( _uint32_value_ & GeneralHourMinControl_MINUTES_MASK )

/*!	\brief		Returns "true" if the checkbox exists, according to mask.
 */
#define		GeneralHourMinControl_GET_CHECK_BOX_EXISTS( _uint32_value_ )	\
	( ( _uint32_value_ & GeneralHourMinControl_CHECKBOX_EXISTS_MASK ) >> 31 )

/*!	\brief		Returns "true" if the checkbox is checked, according to mask.
 */
#define		GeneralHourMinControl_GET_CHECK_BOX_VALUE( _uint32_value_ )	\
	( ( _uint32_value_ & GeneralHourMinControl_CHECKBOX_VALUE_MASK ) >> 30 )


/*------------------------------------------------------------------------
 *						Definition of class GeneralHourMinControl
 *-----------------------------------------------------------------------*/

/*	\brief		This class represents UI element capable to select time.
 *	\attention	This class is NOT suitable for selecting time of day! It's
 *					good for defining time periods.
 *	\details		The UI of this class has three sections: Label, hours and minutes
 *					selector and optional checkbox. It mimics the BControl class.
 */
class GeneralHourMinControl
	:
	public BControl
{
	public:
		
		// Constructor and destructor
		GeneralHourMinControl( BRect frame,
									  const char* name,
									  const BString& label,
									  const BString& checkBoxLabel = BString( "" ),
									  BMessage* toSend = NULL );
		virtual ~GeneralHourMinControl();
		
		// Required functions
		virtual void		MessageReceived( BMessage* in );
		virtual void		AttachedToWindow();
		
		// General functions
		virtual void		SetEnabled( bool in );
		virtual void		SetCheckBoxLabel( const BString& label = BString( "" ) );
		virtual BString	GetCheckBoxLabel( void ) const;
		virtual void		SetValue( int32 value );
		virtual int32		Value( void ) const;
		virtual void		SetLabel( const char* in );
		virtual void 		SetLabel( const BString& in );
		
		// Class-specific functions
		virtual TimeRepresentation	GetCurrentTime( bool* hasCheckBox, bool* checkBoxValue ) const;
		virtual void					SetCurrentTime( const TimeRepresentation& in );
		virtual void					GetCurrentTime( int* hours,
															    int* minutes,
															    bool* hasCheckBox,
															    bool* checkBoxValue ) const;
		virtual void					SetCurrentTime( int hours, int minutes );
		virtual bool					GetCheckBoxValue( bool* hasCheckBox ) const;
		virtual void					SetCheckBoxValue( bool checked );
		virtual void					SetHoursLimit( unsigned int hoursLimit );
		virtual void					SetMinutesLimit( unsigned int minutesLimit );
		virtual unsigned int			GetHoursLimit( void ) const { return fHoursLimit; }
		virtual unsigned int			GetMinutesLimit( void ) const { return fMinutesLimit; }
	
	protected:
	
		// Data holders
		TimeRepresentation	fRepresentedTime;
		bool						bDoesCheckBoxExist;		/* "b" is for "bool" */
		BString					fLabel;
		BString					fCheckBoxLabel;
		unsigned int			fHoursLimit;
		unsigned int			fMinutesLimit;
		
		
		// UI elements
		BStringView		*fLabelView;
		BMenuBar			*fSelectorMenuBar;
		BMenu				*fHoursMenu;
		BMenu				*fMinutesMenu;
		BCheckBox		*fCheckBox;
		
		BLayoutItem*	fLabelItem;
		BLayoutItem*	fMenuBarItem;
		BLayoutItem*	fCheckBoxItem;
		
		// Internal functions
		virtual void			InitUI( void );
		virtual void			ClearUI( void );
		virtual BMenuBar*		CreateMenuBar( void );
		virtual BMenu*			CreateHoursMenu( void );
		virtual BMenu*			CreateMinutesMenu( void );
		virtual BCheckBox*	CreateCheckBox( void );
		virtual void			BuildMatrixMenu( BMenu* toEdit );
		virtual void			UpdateSelectedValuesInMenus( void );
		virtual void			SendInvocationMessage( BMessage* in );
		

};	// <-- end of class GeneralHourMinControl

#endif // _GENERAL_HOUR_MIN_CONTROL_H

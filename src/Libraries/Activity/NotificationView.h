/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _NOTIFICATION_VIEW_H_
#define _NOTIFICATION_VIEW_H_

// OS includes
#include <Box.h>
#include <CheckBox.h>
#include <Control.h>
#include <LayoutItem.h>
#include <Message.h>
#include <Rect.h>
#include <ScrollView.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <TextView.h>
#include <View.h>

// Project includes
#include "ActivityData.h"



/*---------------------------------------------------------------------------
 *								Message constants
 *--------------------------------------------------------------------------*/

	/*!	\brief	This const is used as message data in all views. */
const uint32	kNotificationActivityCheckBoxToggled		= 'ChBT';



/*---------------------------------------------------------------------------
 *						Declaration of class NotificationView
 *--------------------------------------------------------------------------*/


/*!	\brief		This class manages work on Notification area of the Activity.
 */
class	NotificationView
	:
	public BBox
{
	public:
		NotificationView( BRect frame, 
								const char* name,
								const char* label,
								ActivityData* toEdit );
		~NotificationView();

		virtual void	MessageReceived( BMessage* in );
		virtual void	AttachedToWindow( void );
		inline virtual status_t	InitCheck( void ) const { return fLastError; }
		inline virtual void		SetLabel( const char* labelIn );
			
		virtual void	GetPreferredSize( float* width, float* height );
		virtual void	FrameResized( float width, float height );
		virtual void	Pulse( void );
		
		virtual void	SetEnabled( bool toSet );
		virtual bool	IsEnabled() const { return bIsEnabled; }
		virtual void	SaveData();
	
	protected:
		// Information holders
		ActivityData*	fData;
		BString			fNotificationText;
		BString			fLabelText;
		status_t			fLastError;
		bool				bIsEnabled;

		// UI elements
		BCheckBox*		fCheckBox;
		BStringView*	fLabel;
		BTextView*		fTextView;
		BScrollView*	fScroller;
		
		BLayoutItem*	fLabelLayoutItem;
		BLayoutItem*	fTextViewLayoutItem;
		
		// Service functions
		virtual void	EnableTextView( bool enabled );

};	// <-- end of class NotificationView


#endif // _NOTIFICATION_VIEW_H_

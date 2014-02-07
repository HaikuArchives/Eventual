/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _SOUND_SETUP_VIEW_H_
#define _SOUND_SETUP_VIEW_H_

// OS includes
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <Entry.h>
#include <FilePanel.h>
#include <Message.h>
#include <Messenger.h>
#include <Node.h>
#include <Path.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <View.h>

// POSIX includes


// Project includes



class ActivityData;




/*!	\brief		This class serves as filter in open file panel.
 */
struct stat_beos;
class SoundFileFilter
	:
	public BRefFilter
{
	public:
		SoundFileFilter() {}
		virtual bool Filter( const entry_ref* ref,
									BNode* node,
									struct stat_beos* st,
									const char* filetype );
};	// <-- end of class SoundFileFilter



/*!	\brief		This class manages setting and unsetting of Sound file to be played.
 */
class SoundSetupView
	:
	public BBox
{
public:
	//!	\name		Constructor and destructor
	///@{
	SoundSetupView( BRect frame, const char* name, ActivityData* data );
	virtual ~SoundSetupView();
	///@}
	
	//!	\name		Overloaded functions related to BView class.
	///@{
	virtual void AttachedToWindow();
	virtual void MessageReceived( BMessage* in );
	virtual void FrameResized( float width, float height );
	///@}
	
	/*!	\brief	If this function doesn't return B_OK, last action failed. */
	inline virtual status_t	InitCheck() const { return fLastError; }
	
	//!	\name		Enable state getter and setter
	///@{
	inline virtual bool	IsEnabled() const { return bIsEnabled; }
	virtual void SetEnabled( bool toSet );
	///@}
	
	/*!	\details		This function does nothing - all options are fed to
	 *						EventData upon selection.
	 */
	virtual void SaveData() {};
					
protected:
	//!	\name		Data placeholders
	///@{
	ActivityData*	fData;
	BPath				fPathToDirectory;
	BPath				fPathToFile;
	status_t			fLastError;
	bool				bIsEnabled;
	///@}

	/*!	\name	 UI and UI-related elements	*/
	///@{
	BCheckBox*		fCheckBox;
	BStringView*	fLabel;
	BStringView* 	fFileName;
		//!	\name File panel-related elements
		///@{
	BButton*			fOpenFilePanel;
	bool				bFilePanelCreated;
	BFilePanel*		fFilePanel;
	BRefFilter*		fRefFilter;
	BMessenger*		fThisMessenger;	
		///@}
	
	BLayoutItem*	fLabelLayoutItem;
	
	///@}
	
	//!	\name		Service routines
	///@{
	virtual void		UpdateInitialValues( void );
	virtual status_t	CreateAndShowFilePanel( void );
	virtual void		ToggleCheckBox( bool enable );
	///@}

};	// <-- end of class SoundSetupView

#endif // _SOUND_SETUP_VIEW_H_

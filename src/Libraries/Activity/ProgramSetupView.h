/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _PROGRAM_SETUP_VIEW_H_
#define _PROGRAM_SETUP_VIEW_H_

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
#include <TextControl.h>
#include <View.h>

// POSIX includes


// Project includes


class ActivityData;



/*!	\brief		This class serves as filter in open file panel.
 */
struct stat_beos;
class ProgramFileFilter
	:
	public BRefFilter
{
	public:
		ProgramFileFilter() {}
		virtual bool Filter( const entry_ref* ref,
									BNode* node,
									struct stat_beos* st,
									const char* filetype );
};	// <-- end of class ProgramFileFilter



/*!	\brief		This class manages setting and unsetting of Program file to be run.
 */
class ProgramSetupView
	:
	public BBox
{
public:
	ProgramSetupView( BRect frame, const char* name, ActivityData* data );
	virtual ~ProgramSetupView();
	
	virtual void AttachedToWindow();
	virtual void MessageReceived( BMessage* in );
	
	/*!	\brief	If this function doesn't return B_OK, last action failed. */
	inline virtual status_t	InitCheck() const { return fLastError; }

	virtual void		FrameResized( float width, float height );
	
	inline virtual bool	IsEnabled() const { return bIsEnabled; }
	virtual void		SetEnabled( bool toSet );
	
	virtual	void		SaveData();

protected:
	// Data placeholders
	ActivityData*	fData;
	BPath				fPathToDirectory;
	BPath				fPathToFile;
	BString			fCommandLineOptions;
	status_t			fLastError;
	bool				bIsEnabled;

	// UI elements
	BCheckBox*		fCheckBox;
	BStringView*	fLabel;
	BStringView* 	fFileName;
	BButton*			fOpenFilePanel;
	BFilePanel*		fFilePanel;
	bool				bFilePanelCreated;
	BRefFilter*		fRefFilter;
	BMessenger*		fThisMessenger;
	BTextControl*	fCommandLineOptionsInput;
	
	BLayoutItem*	fLabelLayoutItem;
	
	// Service routines
	virtual void		UpdateInitialValues( void );
	virtual status_t	CreateAndShowFilePanel( void );
	virtual void		ToggleCheckBox( bool );

};	// <-- end of class ProgramSetupView

#endif // _PROGRAM_SETUP_VIEW_H_

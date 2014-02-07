/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_EDITOR_MAIN_WINDOW_H_
#define _EVENT_EDITOR_MAIN_WINDOW_H_

// OS includes
#include <Application.h>
#include <FilePanel.h>
#include <MenuBar.h>
#include <View.h>
#include <Window.h>

// Project includes
#include "EventEditorGeneralView.h"
#include "EventEditorReminderView.h"
#include "ActivityView.h"
#include "EventEditorNoteView.h"

// POSIX includes
#include <time.h>


/*----------------------------------
 * 		Messages constants
 *---------------------------------*/

const uint32	kFileNew				= 'FiNe';
const uint32	kFileOpen 			= 'FiOp';
const uint32	kFileSave			= 'FiSa';
const uint32	kFileSaveAs			= 'FiSA';
const uint32	kFileRevert			= 'FiRe';

const uint32	kFileOpenConfirmed = 'FiOC';
const uint32	kFileSaveConfirmed = 'FiSC';


extern uint32	global_toReturn;


/*--------------------------------------------------------------------
 *					Declaration of class EventFileRefFilter
 *-------------------------------------------------------------------*/

/*!	\brief		This class is used in open and save file panels.
 *		\details		It allows to view only Event files or directories.
 */
class	EventFileRefFilter
	:
	public BRefFilter
{
	virtual bool Filter( const entry_ref* ref,
								BNode* node,
								struct stat_beos* st,
								const char* filetype );
};	// <-- end of class EventFileRefFilter


/*--------------------------------------------------------------------
 *					Declaration of class EventEditorMainWindow
 *-------------------------------------------------------------------*/

class EventEditorMainWindow
	:
	public BWindow
{
protected:
	EventData fData;

	EventEditor_GeneralView* genView;
	EventEditor_ReminderView* remView;
	ActivityView*				actView;
	EventEditor_NoteView*	noteView;
	
	BRefFilter*		fRefFilter;
	BFilePanel*		fOpenFile;
	BFilePanel*		fSaveFile;
	
	BView*			MainView;
	BMenuBar*		menuBar;
	BMenu*			fileMenu;
	BMenu*			editMenu;
	BButton*			saveAndClose;
	
	/*!	\name		Service functions		*/
	///@{
	BMenuBar* 		CreateMenuBar();
	void				InitUI();
	void				ClearUI();
	void 				InitializeFilePanels( BString path = BString("") );
	///@}
	
public:
	EventEditorMainWindow( time_t startMoment = 0 );
	EventEditorMainWindow( BString path );
	~EventEditorMainWindow();
	virtual void MessageReceived(BMessage *Message);
	virtual bool QuitRequested();
};

#endif	// _EVENT_EDITOR_MAIN_WINDOW_H_

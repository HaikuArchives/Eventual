/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_EDITOR_NOTE_VIEW_H_
#define _EVENT_EDITOR_NOTE_VIEW_H_

// OS includes
#include <ScrollView.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <TextView.h>
#include <View.h>

// Project includes
#include "Event.h"

class EventEditor_NoteView
	:
	public BView
{
	public:
		EventEditor_NoteView( BRect frame, EventData* data );
		virtual 				~EventEditor_NoteView();
		inline virtual void	AttachedToWindow() {
			if ( Parent() ) { this->SetViewColor( Parent()->ViewColor() ); }
		}
		
		virtual void 		Pulse();
		virtual status_t	InitCheck() const { return fLastError; }
		virtual void		FrameResized( float width, float height );
		virtual void		SaveText();
		
	protected:
		// Information placeholders
		EventData* 	 fData;
		status_t		 fLastError;
		
		// UI elements
		BScrollView* fScroller;
		BStringView* fExplanation1;
		BStringView* fExplanation2;
		BTextView*	 fTextView;
};

#endif // _EVENT_EDITOR_NOTE_VIEW_H_

/*
 * Copyright 2011 Alexey Burshtein <aburst02@campus.haifa.ac.il>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _EVENT_H_
#define _EVENT_H_

// OS includes
#include <Entry.h>
#include <File.h>
#include <List.h>
#include <Path.h>
#include <String.h>
#include <SupportDefs.h>


// Project includes
#include "ActivityData.h"
#include "TimeRepresentation.h"
#include "Utilities.h"

const uint32	kSaveRequested = 'SAV!';

/*---------------------------------------------------------------------------
 *					Declaration of enum EventType and corresponding strings
 *--------------------------------------------------------------------------*/

#ifdef	EVENT_TYPE_CROSS_MACRO
#undef	EVENT_TYPE_CROSS_MACRO
#endif

#define	EVENT_TYPE_CROSS_MACRO			\
	PARSER( kEventType_Appointment, 	"Appointment"	)	\
	PARSER( kEventType_Anniversary, 	"Anniversary"	)	\
	PARSER( kEventType_Holiday,		"Holiday"		)	\
	PARSER( kEventType_Note,			"Note"			)

#ifdef	PARSER
#undef	PARSER
#endif

#define	PARSER(a, b)	a,

/*!	\brief		This defines the type of the Event.
 *		\details		It is merely a matter of convenience. Some options are
 *						set to defaults, disabled or enabled depending on what type
 *						of the Event the user chooses.
 *		\par			kEventType_Appointment
 *						The basic type of the Event. It has start date, start time and
 *						duration that may be non-zero.
 *		\par			kEventType_Anniversary
 *						Marks a yearly anniversary. Has only start date, but no start
 *						time (starts at midnight). Always set to last 1 day.
 *		\par			kEventType_Holiday
 *						An extention to \c kEventType_Anniversary. Has start date, but
 *						no start time (starts at midnight). May last several days. The
 *						\c fDuration member of the class defines number of days: the
 *						number of days is always the smallest number that can fully
 *						accomodate \c fDuration (which is measured in seconds), and is
 *						at least 1.
 *		\par			kEventType_Note
 *						A note to the user himself. Only start date and time, but no
 *						duration ( \c fDuration = 0 ). Basically, "reminder" is an
 *						Event of type Note.
 */
enum	EventType {
	EVENT_TYPE_CROSS_MACRO
};

#undef	PARSER



/*---------------------------------------------------------------------------
 *					Declaration of class EventData
 *--------------------------------------------------------------------------*/


/*!	\brief		This class holds Event information.
 *		\details		Actually, it just provides an easy way to read and store Event file
 *						attributes.
 */
class EventData
{
protected:

	// General information placeholders
	BString		fEventName;			//!<  Duh
	entry_ref*	fEventFile;			/*!<  May be NULL if a new Event is constructed.
											 *		Used for "Save" and "Revert" features. */
	CalendarModule*	fCalModule;	//!< Calendar module used for creation of this Event.
	BString		fCategory;			//!< Category of this Event. It \b must be set!
	BString		fLocation;			//!< Where this Event will occur?
	uint32		bPrivate;			/*!< If \c true - this Event is private. Not used for now,
											 *	  since Haiku is single-user.										*/
	uint32		bVerified;			/*!< If \c true - user has checked this Event's program.
											 *	  Designed to prevent maluse of program running ability.
											 *	  Not used for now.													*/
	EventType	fEventType;			//!< See \c enum \c EventType.
	uint32		bLastsWholeDays;	/*!< If \c true, only start day matters, not time. \c fDuration
											 *		defines how many whole days the Event lasts, rounding \b up
											 *		to smallest number of days that can accomodate \c fDuration. */
	BString		fNote;				//!< User's note (contents of the file).

	time_t		startTime;			//!< Used for "revert" feature

	// When this Event starts, for how long does it last and what does it do.	
	TimeRepresentation	fStart;	//!< Start time of the first occurrence of the Event.
	time_t				fDuration;	//!< Duration of the Event in seconds. May be 0.
	uint32	fActivitySnoozedTime;	//!< For Snooze activity only. When the Activity is schedulled to start, from UNIX epoch.
	bool		bEventActivityWasFired;	//!< \c true if Event's activity was fired.
	ActivityData		fEventActivity;
	
	// When the Reminder starts, and what does it do.
	ActivityData		fReminderActivity;		//!< Activity of the Reminder
	bool		bReminderActivityWasFired;			//!< \c true if Reminder activity was fired
	uint32	bReminderIsFiredBeforeEvent;		//!< \c true if Reminder starts before Event's start time.
	uint32	fReminderSnoozedTime;				//!< For Snooze feature only. Seconds in UNIX time epoch until Reminder fires
	time_t	fOffsetBetweenReminderAndEvent;	/*!< Difference in seconds between Reminder and Event.
															 *   Offset of 0 means Reminder is disabled.		*/
			
	// Data about recurrency.	
	BList		fAndRules;				//!< Recurrence rules that define when this Event \b IS repeated.
	BList		fNotRules;				//!< Recurrence rules that define when this Event is \b NOT repeated.
	
	time_t	fNextOccurrence;		//!< When is the closest occurrence of this Event?

	// Service functions
	virtual void		_InitDefaults( void );
	virtual status_t	_SaveToFile( BFile* file );

	
public:

	/*!	\name		Constructors and destructor
	 */
	///@{
	EventData( time_t in = 0 );	// Constructor from seconds
	EventData( const entry_ref& fileIn );	// Constructor from file
	virtual ~EventData();			// Destructor
	///@}
	
	/*!	\name		Work with filesystem
	 */
	///@{
	virtual void 		InitFromFile( const entry_ref& fileIn );		// Read the object data from file
	virtual status_t	SaveToFile( entry_ref* fileIn = NULL );
	virtual status_t	SaveToFile( BFile* fileIn );
	virtual void		Revert();
	virtual entry_ref*	GetRef() { return fEventFile; }
	///@}

	/* Setting and getting Event general data */
	virtual BString	GetCategory() const { return fCategory; }
	virtual void		SetCategory( const BString& toSet ) { fCategory.SetTo( toSet ); }
	virtual void		SetCategory( const char* toSet ) { if ( toSet ) fCategory.SetTo( toSet ); }
	
	virtual bool		GetPrivate() const { return ( bPrivate != 0 ); }
	virtual void		SetPrivate( bool toSet ) { bPrivate = ( toSet ? 1 : 0 ); }
	
	virtual EventType	GetEventType() const { return fEventType; }
	virtual status_t	SetEventType( EventType etIn );
	
	virtual BString	GetEventName() const { return fEventName; }
	virtual status_t	SetEventName( const BString& toSet ) { fEventName.SetTo( toSet ); return B_OK; }
	virtual status_t	SetEventName( const char* toSet ) { if ( toSet ) return SetEventName( BString( toSet ) ); return B_ERROR; }
	
	//!	\name		Where the event will occur?
	///@{
	virtual BString	GetEventLocation() const { return fLocation; }
	virtual status_t	SetEventLocation( const BString& toSet ) { fLocation.SetTo( toSet ); }
	virtual status_t	SetEventLocation( const char* toSet ) { if ( toSet ) fLocation.SetTo( toSet ); }
	///@}
	
	//!	\name		File reference manipulations
	///@{
	virtual entry_ref*	GetFileRef() const { return fEventFile; }
	virtual status_t		SetFileRef( const entry_ref& in ) { fEventFile = new entry_ref( in ); }
	///@}
	
	//!	\name		Start date and time manipulations
	///@{
	virtual TimeRepresentation	GetStartTime() const { return fStart; }
	virtual void		GetStartTime( int* hour, int* min ) const {
		if ( hour ) *hour = fStart.tm_hour;
		if ( min ) *min = fStart.tm_min;
	}
	virtual void		GetStartTime( int* day, int* month, int* year ) {
		if ( day ) *day = fStart.tm_mday;
		if ( month ) *month = fStart.tm_mon;
		if ( year ) *year = fStart.tm_year;
	}
	virtual status_t	SetStartTime( const TimeRepresentation& trIn ) {
		fStart = trIn;
	}	
	virtual status_t	SetStartTime( int hour, int min );
	virtual status_t	SetStartDate( int day, int month, int year, BString calendar = BString( "Gregorian" ) );
	virtual status_t	SetStartDate( const TimeRepresentation& trIn ) {
		return SetStartDate( trIn.tm_mday, trIn.tm_mon, trIn.tm_year, trIn.GetCalendarModule() );
	}
	///@}
	
	//!	\name		Duration getter and setter
	///@{
	virtual time_t		GetDuration() const { return fDuration; }
	virtual status_t	SetDuration( time_t durIn, EventType* newType = NULL );
	///@}
	
	/*!	\name			Activity data access
	 *		\attention	The following functions return pointers to
	 *						the live data on purpose!
	 */
	///@{
	virtual ActivityData*	GetEventActivity() { return &fEventActivity; }
	virtual ActivityData*	GetReminderActivity() { return &fReminderActivity; }
	
	virtual	bool		WasEventActivityFired() const	{ return bEventActivityWasFired; }
	virtual	void		SetEventActivityFired( bool toSet ) { bEventActivityWasFired = toSet; }
	
	virtual	bool		WasReminderActivityFired() const	{ return bReminderActivityWasFired; }
	virtual	void		SetReminderActivityFired( bool toSet ) { bReminderActivityWasFired = toSet; }
	
	virtual uint32		GetReminderSnoozeTime() const { return fReminderSnoozedTime; }
	virtual void		SetReminderSnoozeTime( uint32 toSet ) {
		fReminderSnoozedTime = toSet;
		bReminderActivityWasFired = false;
	}
	
	virtual uint32		GetActivtiySnoozeTime() const { return fActivitySnoozedTime; }
	virtual void		SetActivitySnoozeTime( uint32 toSet ) {
		fActivitySnoozedTime = toSet;
		bEventActivityWasFired = false;
	}
	///@}
	
	/*!	\name 		Get and set reminder offset.
	 *		\details		If the reminder offset is 0, the reminder is totally disabled.
	 */
	///@{
	virtual time_t		GetReminderOffset( bool* beforeEventOut ) {
		if ( beforeEventOut ) { *beforeEventOut = ( bReminderIsFiredBeforeEvent != 0 ); }
		return fOffsetBetweenReminderAndEvent;
	}
	virtual status_t	SetReminderOffset( time_t newOffset, bool beforeEvent ) {
		bReminderIsFiredBeforeEvent = ( beforeEvent ? 1 : 0 );
		fOffsetBetweenReminderAndEvent = newOffset;
	}
	///@}
	
	virtual bool		GetLastsWholeDays() const { return ( bLastsWholeDays != 0 ); }
	virtual void		SetLastsWholeDays( bool toSet ) { bLastsWholeDays = ( toSet ? 1 : 0 ); }
	
	/*!	\name			Note text manipulations.
	 *		\note
	 *				Since Note text may be quite long, I provide additional method
	 *				that returns reference (and doesn't allocate another object).
	 *				Hence this method is not \c const.
	 */
	///@{
	virtual BString& 	GetNoteTextReference() { return fNote; }
	virtual BString	GetNoteText() const { return fNote; }
	virtual void		SetNoteText( const BString& toSet ) { fNote.SetTo( toSet ); }
	virtual void 		SetNoteText( const char* toSet ) { if ( toSet ) fNote.SetTo( toSet ); }
	///@}
	
};	// <-- end of class EventData

#endif // _EVENT_H_

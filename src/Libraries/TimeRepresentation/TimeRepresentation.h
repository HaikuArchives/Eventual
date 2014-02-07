#ifndef	__TIME_REPRESENTATION_H__
#define __TIME_REPRESENTATION_H__

// OS includes
#include <Message.h>
#include <String.h>
#include <List.h>

// POSIX includes
#include <time.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

extern BList listOfCalendarModules;

/*! 
	\brief	The Calendar Module allows to check what day of week is a local date.
	\details	Used also in the Rule for defining a weekly repeating pattern.
*/

const uint32	kSunday		= 1;
const uint32	kMonday		= 2;
const uint32	kTuesday	= 3;
const uint32	kWednesday	= 4;
const uint32	kThursday	= 5;
const uint32	kFriday		= 6;
const uint32	kSaturday	= 7;
const uint32	k8thDay		= 8;
const uint32	k9thDay		= 9;
const uint32	k10thDay	= 10;
const uint32	k7DaysWeek	= 20;
const uint32	kInvalid	= 0xFF;



class CalendarModule;

/*!
	\brief	This class is an expansion of the struct tm.

	\details	TimeRepresentation is an extention of struct tm. However, since the TimeRepresentation
				is calendar-independent, all fields in its struct are disjointed from Gregorian calendar.
				As such, tm_year is NOT years since 1900, but the year itself. The tm_mon is not the
				months since January, but number of the month in current year (for Gregorian calendar,
				January is 1st month, and December is 12th, but Gregorian calendar is not the only option).
	\sa			struct tm
*/
class TimeRepresentation
	:
	public tm
{
protected:
	BString fCalendarModule;	//!< String that defines the Calendar module used for the representation
	bool fIsRepresentingRealDate;	//!< This variable is "true", if current object represents an actual date.	
public:
	TimeRepresentation();
	TimeRepresentation(struct tm &in, BString calModule = BString("Gregorian"));
	TimeRepresentation(const TimeRepresentation &in);
	virtual ~TimeRepresentation();

	inline virtual void SetIsRepresentingRealDate(bool in) { this->fIsRepresentingRealDate = in; }
	inline virtual const bool GetIsRepresentingRealDate (void) const { return this->fIsRepresentingRealDate; }
	inline virtual const BString GetCalendarModule (void) const { BString a(this->fCalendarModule); return a; }
	inline virtual void SetCalendarModule(const BString &module) { this->fCalendarModule.SetTo(module); }

	virtual const tm GetRepresentedTime (void) const ;
	
	void Archive( BMessage* in );
	void Unarchive( BMessage* in );

	// Operators
	virtual TimeRepresentation& operator= (const TimeRepresentation& in);
	virtual bool operator== (const TimeRepresentation &in) const;
//	virtual TimeRepresentation operator+ (const TimeRepresentation& op1, const TimeRepresentation &op2);
//	virtual TimeRepresentation& operator+= (const TimeRepresentation& op1);
//	virtual TimeRepresentation operator- (const TimeRepresentation& op1, const TimeRepresentation &op2);
//	virtual TimeRepresentation& operator-= (const TimeRepresentation& op1);
	virtual bool operator<(const TimeRepresentation& in) const;
	virtual inline bool operator<= (const TimeRepresentation &in) const { return (this->operator<(in) || this->operator==(in)); }
	virtual inline bool operator> (const TimeRepresentation &in) const { return (!this->operator<=(in)); }
	virtual inline bool operator>= (const TimeRepresentation &in) const { return (!this->operator<(in)); }
};

#endif	// __TIME_REPRESENTATION_H__

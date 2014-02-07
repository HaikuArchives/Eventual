#include <string.h>
#include <stdlib.h>

#include "TimeRepresentation.h"

BList listOfCalendarModules;

/*! 
 *	\brief			Constructor from struct tm and BString for calendar module name.
 *	\details		It is a really bad practice to construct a TimeRepresentation
 *					object without explicitly referencing a CalendarModule.
 *	\param[in]	in			The struct tm to be initialized from.
 *	\param[in]	calModule	The calendar module to be used.
 *	\sa				CalendarModule, struct tm
 */
TimeRepresentation::TimeRepresentation(struct tm& in, BString calModule ) {
	int length = 0;		// Used later for getting size of the time zone string

	// Since we got the struct tm as the input, it's suggested that a real date is represented.
	this->fIsRepresentingRealDate = true;

	// Hour and minute (and irrelevant second)
	this->tm_hour = in.tm_hour;
	this->tm_min = in.tm_min;
	this->tm_sec = in.tm_sec;

	// Day, month, year
	this->tm_mday = in.tm_mday;
	this->tm_mon = in.tm_mon;
	this->tm_year = in.tm_year;

	// Other irrelevant stuff	
	this->tm_wday = in.tm_wday;
	this->tm_yday = in.tm_yday;
	
	// Copying time zone information
	this->tm_gmtoff = in.tm_gmtoff;
	this->tm_isdst = in.tm_isdst;
	if (in.tm_zone && (length=strlen(in.tm_zone)) != 0) {
		this->tm_zone = new char[length+1];	// +1 is for NULL character
		if (!this->tm_zone) {
			// Panic!
			exit(1);
		}
		strcpy(this->tm_zone, in.tm_zone);
	} else {
		this->tm_zone = NULL;
	}

	// Copying calendar module information
	this->fCalendarModule.SetTo(calModule);
}
// <-- end of constructor of TimeRepresentation

/*!	
 *	\brief			Empty constructor.
 */
TimeRepresentation::TimeRepresentation() {
	tm_hour = tm_min = tm_sec = tm_mon = tm_year = 0;
	tm_wday = kSunday;
	this->fCalendarModule.Truncate(0);
	this->fIsRepresentingRealDate = false;
	this->tm_zone = NULL;
}
// <-- end of empty constructor of TimeRepresentation

/*! 
 *	\brief			Copy constructor
 *	\param[in]	in	Reference to TimeRepresentation from which the copy should be performed.
 *	\remarks		The created copy is deep copy. The time zone is created anew.
 */
TimeRepresentation::TimeRepresentation(const TimeRepresentation &in) {
	int limit=0;
	// Simply copying the submitted struct, field by field.
	this->fCalendarModule.SetTo(in.fCalendarModule);
	this->tm_hour = in.tm_hour;
	this->tm_min = in.tm_min;
	this->tm_sec = in.tm_sec;
	this->tm_mday = in.tm_mday;
	this->tm_mon = in.tm_mon;
	this->tm_year = in.tm_year;
	this->tm_wday = in.tm_wday;
	this->tm_yday = in.tm_yday;
	this->tm_isdst = in.tm_isdst;
	this->tm_gmtoff = in.tm_gmtoff;
	this->fIsRepresentingRealDate = in.fIsRepresentingRealDate;
	// Copy the time zone information
	if (in.tm_zone && (limit=strlen(in.tm_zone))) {
		this->tm_zone = new char[limit+1];
		strcpy(this->tm_zone, in.tm_zone);		
	} else {
		this->tm_zone = NULL;
	}
}
// <-- end of copy constructor of TimeRepresentation

/*!	
 *	\brief			Returns the time represetned by the TimeRepresentation.
 *	\details		The created representation is a deep copy. That is, the
 *					tm_zone string is cloned. All other fields are copied
 *					"as is", without any sanity checks of any kind.
 *	\returns		constant Struct tm of the represented time.
 *	\sa				TimeRepresentation constructor, struct tm.
 */
const tm TimeRepresentation::GetRepresentedTime() const {
	tm out;
	int length;	
	
	// Hour and minute (and second)
	out.tm_hour = this->tm_hour;
	out.tm_min = this->tm_min;
	out.tm_sec = this->tm_sec;
	
	// Day, month and year
	out.tm_year = this->tm_year;
	out.tm_mon = this->tm_mon;
	out.tm_mday = this->tm_mday;

	// Irrelevant stuff
	out.tm_wday = this->tm_wday;
	out.tm_yday = this->tm_yday;

	// Copying time zone information
	out.tm_gmtoff = this->tm_gmtoff;
	out.tm_isdst = this->tm_isdst;
	if (this->tm_zone && (length=strlen(this->tm_zone))) {
		out.tm_zone = new char[length+1];	// +1 is for NULL character
		if (!out.tm_zone) {
			// Panic!
			exit(1);
		}
		strcpy(out.tm_zone, this->tm_zone);
	} else {
		out.tm_zone = NULL;
	}
	return out;
}
// <-- end of function TimeRepresentation::GetRepresentedTime

/*! 
 *	\brief			Default destructor of the TimeRepresentation
 *	\details		Recycles the memory for the dynamically allocated
 *					element of the structure - the time zone name. Also,
 *					removes the calendar module's name.
 *
 */
TimeRepresentation::~TimeRepresentation() {
	if (this->tm_zone) {
		delete this->tm_zone;
		this->tm_zone = NULL;
	}
	this->fCalendarModule.Truncate(0);
}
// <-- end of destructor of TimeRepresentation

/*! 
 *	\brief			Duh?
 *	\param[in]	in	The value to which the assignment should be performed.
 *	\returns		Reference to the object overgone assignment.
 */
TimeRepresentation& TimeRepresentation::operator=(const TimeRepresentation &in) {
	int limit=0;
	// Simply copying the submitted struct, field by field.
	this->fCalendarModule.SetTo(in.fCalendarModule);
	this->tm_hour = in.tm_hour;
	this->tm_min = in.tm_min;
	this->tm_sec = in.tm_sec;
	this->tm_mday = in.tm_mday;
	this->tm_mon = in.tm_mon;
	this->tm_year = in.tm_year;
	this->tm_wday = in.tm_wday;
	this->tm_yday = in.tm_yday;
	this->tm_isdst = in.tm_isdst;
	this->tm_gmtoff = in.tm_gmtoff;
	this->fIsRepresentingRealDate = in.fIsRepresentingRealDate;
	// Copy the time zone information
	if (this->tm_zone != NULL) {
		delete this->tm_zone;
	}
	if (in.tm_zone && (limit=strlen(in.tm_zone))) {
		this->tm_zone = new char[limit+1];
		strcpy(this->tm_zone, in.tm_zone);		
	} else {
		this->tm_zone = NULL;
	}
	return *this;
}
// <-- end of TimeRepresentation::operator=

/*!	
 *	\brief			Allows field-by-field comparison of two TimeRepresentations
 *	\param[in]	in	The TimeRepresentation object to be compared with.
 *	\returns		true if the objects are equal, else false.
 */
bool TimeRepresentation::operator== (const TimeRepresentation& in) const {
	if ((this->fCalendarModule == in.fCalendarModule)	&&
		(this->tm_year == in.tm_year)					&&
		(this->tm_mon == in.tm_mon)						&&
		(this->tm_mday == in.tm_mday)					&&
		(this->tm_hour == in.tm_hour)					&&
		(this->tm_min == in.tm_min)						&&
		(this->tm_sec == in.tm_sec)						&&
		(this->tm_wday == in.tm_wday)					&&
		(this->tm_yday == in.tm_yday)					&&
		(this->tm_isdst == in.tm_isdst)					&&
		(this->fIsRepresentingRealDate == in.fIsRepresentingRealDate) &&
		(this->tm_gmtoff == in.tm_gmtoff))
	{
		if (((this->tm_zone != NULL) && (in.tm_zone != NULL) && (strcmp(this->tm_zone, in.tm_zone) == 0)) || 
			((this->tm_zone == NULL) && (in.tm_zone == NULL)))
		{
			return true;
		}
	}
	return false;
}
// <-- end of TimeRepresentation::operator==

/*! 
 *	\brief			Sums two dates
 *	\details		 
 *	\param[in]	op		Const reference to the second operand
 *	\returns		The sum of *this and op.
 *	\remarks		If the dates are set in differennt time zones, the result will belong
 *					to the time zone of the 1st operand.
 *
TimeRepresentation TimeRepresentation::operator+ (const TimeRepresentation &op) 
{
//	BString calModule1 = this->GetCalendarModule(), calModule2 = in.GetCalendarModule();
	bool real1 = this->GetIsRepresentingRealDate(), real2 = op.GetIsRepresentingRealDate();
	if (!real1 && real2) {		// Only the second operand is meaningful
		TimeRepresentation toReturn(op);
		toReturn += *this;
		return toReturn;
	} 
	TimeRepresentation toReturn1(*this);
	toReturn1 += op;
	return toReturn1;
}
// <-- end of function TimeRepresentation::operator+

 *! 
 *	\brief			Sums two dates and put the result into "this"
 *	\details		
 *	\param[in]	in		Const reference to the date to be added 
 *	\returns		Reference to "this"
 *	\remarks		If the dates are set in differennt time zones, the result will belong
 *					to the time zone of the 1st operand.
 */ /*
TimeRepresentation& TimeRepresentation::operator+= (const TimeRepresentation &in) 
{
	bool real1 = this->GetIsRepresentingRealDate(), real2 = in.GetIsRepresentingRealDate();
	if (real1 && real2)	// Both of the dates are real dates 
	{
		BString calIn = in.GetCalendarModule(), calThis = this->GetCalendarModule();
		CalendarModule* calModuleIn = NULL, calModuleThis = NULL;
		
		for (int i = 0; i < listOfCalendarModules.CountItems(); i++) {
			if (calIn == (CalendarModule*)(listOfCalendarModules.ItemAt(i))->Identify()) {
				calModuleIn = (CalendarModule*)(listOfCalendarModules.ItemAt(i));				
			}
			if (calThis == (CalendarModule*)(listOfCalendarModules.ItemAt(i))->Identify()) {
				calModuleThis = (CalendarModule*)(listOfCalendarModules.ItemAt(i));				``
			}
			// If both modules are found - no need to continue looping
			if (calModuleIn && calModuleThis) { break; }
		}

		// If one of the requested modules does not exist, exitting immediately.
		// This is because both TimeRepresentations represent a real data, therefore, they both
		//   need to be treated using a calendar module. But one of them isn't found; the result
		//	 of this operation is undefined; we have no choise but to exit.
		if (!calModuleIn || !calModuleThis) {
			// Panic!
			exit(2);
		}

		// Move both time representations into Gregorian calendar
		TimeRepresentation tempIn = calModuleIn->fromLocalCalendarToGregorian(in);
		TimeRepresentation tempThis = calModuleThis->fromLocalCalendarToGregorian(*this);

		// Get the seconds representation for every one of the additives.
		tm tempInTm = tempIn.GetRepresentedTime();
		tm tempThisTm = tempThis.GetRepresentedTime();
		
		--tempIn.tm_mon

			// Perform addition
		time_t tempTimeTIn;

		return *this;
	}
	// Current time may be real, the other operand is surely not
	
	// Sum up times and dates
	this->tm_sec += in.tm_sec;
	this->tm_min += in.tm_min;
	this->tm_hour += in.tm_hour;
	this->tm_mday += in.tm_mday;
	this->tm_mon += in.tm_mon;
	this->tm_year += in.tm_year;
	return *this;
}
// <-- end of function TimeRepresentation::operator+=

*/

/*!	
 *	\brief		Comparing two TimeRepresentation objects.
 *	\remarks	The objects must be of the same CalendarModule!
 */
bool TimeRepresentation::operator<(const TimeRepresentation& in) const {
	if (this->GetCalendarModule() != in.GetCalendarModule()) { return false; }
//	TimeRepresentation thisTimeRep(*this);
	tm thisTime = this->GetRepresentedTime();
	tm inTime = in.GetRepresentedTime();
	--thisTime.tm_mon; --inTime.tm_mon;
	thisTime.tm_year -= 1900; inTime.tm_year -= 1900;
	time_t time1 = mktime(&thisTime), time2 = mktime(&inTime);
	return (time1<time2);
}
// <-- end of TimeRepresentation::operator<


/*!	\brief 		Archiving into a BMessage
 *		\param[in]	in		The BMessage to archive into.
 */
void		TimeRepresentation::Archive( BMessage* in )
{
	if ( !in ) { return; }
	
	in->MakeEmpty();
	
	in->AddString( "Calendar Module", fCalendarModule );
	in->AddBool( "Representing Real Date", fIsRepresentingRealDate );
	
	in->AddInt32( "Year", ( int32 )tm_year );
	in->AddInt32( "Month", ( int32 )tm_mon );
	in->AddInt32( "Day", ( int32 )tm_mday );
	in->AddInt32( "Hour", ( int32 )tm_hour );
	in->AddInt32( "Min", ( int32 )tm_min );
	in->AddInt32( "Sec", ( int32 )tm_sec );
	in->AddInt32( "Wday", ( int32 )tm_wday );
	in->AddInt32( "Yday", ( int32 )tm_yday );
	in->AddInt32( "IsDST", ( int32 )tm_isdst );
	in->AddInt32( "GMToff", ( int32 )tm_gmtoff );
	if ( tm_zone != NULL )
		in->AddString( "TimeZone", tm_zone );
}	// <-- end of function TimeRepresentation::Archive



/*!	\brief		Unarchiving from BMessage
 *		\param[in]	in		The BMessage to instantiate from.
 */
void		TimeRepresentation::Unarchive( BMessage* in ) {
	if ( !in ) { return; }
	
	in->FindString( "Calendar Module", &fCalendarModule );
	in->FindBool( "Representing Real Date", &fIsRepresentingRealDate );
	
	in->FindInt32( "Year", ( int32* )&tm_year );
	in->FindInt32( "Month", ( int32* )&tm_mon );
	in->FindInt32( "Day", ( int32* )&tm_mday );
	in->FindInt32( "Hour", ( int32* )&tm_hour );
	in->FindInt32( "Min", ( int32* )&tm_min );
	in->FindInt32( "Sec", ( int32* )&tm_sec );
	in->FindInt32( "Wday", ( int32* )&tm_wday );
	in->FindInt32( "Yday", ( int32* )&tm_yday );
	in->FindInt32( "IsDST", ( int32* )&tm_isdst );
	in->FindInt32( "GMToff", ( int32* )&tm_gmtoff );
	
	// Time zones are currently not supported
	tm_zone = NULL;
}	// <-- end of function TimeRepresentation::Unarchive

#include <map>
#include <stdlib.h>
#include <stdio.h>

#include "GregorianCalendarModule.h"
#include "CalendarModule.h"
#include "TimeRepresentation.h"


GregorianCalendar::GregorianCalendar()
{
	this->id.SetTo("Gregorian" );
	this->fDaysInLongestMonth = 31;
	int i=1;
	BString builder;

	// Fill in the dates
	for (; i<32; ++i) {
		builder << (uint32)i;
		this->fDaysNames[i] = builder;
		builder.Truncate(0);		// Remove the contents of the string
	}
	
	struct DoubleNames names;

	names.longName = BString("January");
	names.shortName = BString("Jan");
	this->fMonthsNames[1] = names;

	names.longName = BString("February");
	names.shortName = BString("Feb");
	this->fMonthsNames[2] = names;

	names.longName = BString("March");
	names.shortName = BString("Mar");
	this->fMonthsNames[3] = names;

	names.longName = BString("April");
	names.shortName = BString("Apr");
	this->fMonthsNames[4] = names;

	names.longName = BString("May");
	names.shortName = BString("May");
	this->fMonthsNames[5] = names;

	names.longName = BString("June");
	names.shortName = BString("Jun");
	this->fMonthsNames[6] = names;

	names.longName = BString("July");
	names.shortName = BString("Jul");
	this->fMonthsNames[7] = names;

	names.longName = BString("August");
	names.shortName = BString("Aug");
	this->fMonthsNames[8] = names;

	names.longName = BString("September");
	names.shortName = BString("Sep");
	this->fMonthsNames[9] = names;

	names.longName = BString("October");
	names.shortName = BString("Oct");
	this->fMonthsNames[10] = names;

	names.longName = BString("November");
	names.shortName = BString("Nov");
	this->fMonthsNames[11] = names;
	names.longName = BString("December");
	names.shortName = BString("Dec");
	this->fMonthsNames[12] = names;

	this->fDaysInWeek = 7;

	uint32 counter = kSunday;	// 0x01
	names.longName.SetTo("Sunday");
	names.shortName.SetTo("Su");
	this->fWeekdaysNames[counter] = names;

	counter = kMonday;
	names.longName.SetTo("Monday");
	names.shortName.SetTo("Mo");
	this->fWeekdaysNames[counter] = names;

	counter = kTuesday;
	names.longName.SetTo("Tuesday");
	names.shortName.SetTo("Tu");
	this->fWeekdaysNames[counter] = names;

	counter = kWednesday;
	names.longName.SetTo("Wednesday");
	names.shortName.SetTo("We");
	this->fWeekdaysNames[counter] = names;

	counter = kThursday;
	names.longName.SetTo("Thursday");
	names.shortName.SetTo("Th");
	this->fWeekdaysNames[counter] = names;

	counter = kFriday;
	names.longName.SetTo("Friday");
	names.shortName.SetTo("Fr");
	this->fWeekdaysNames[counter] = names;

	counter = kSaturday;
	names.longName.SetTo("Saturday");
	names.shortName.SetTo("Sa");
	this->fWeekdaysNames[counter] = names;
}

GregorianCalendar::GregorianCalendar(const GregorianCalendar &in) 
{ 
	this->fDaysInWeek = in.fDaysInWeek;
	this->fDaysNames = in.fDaysNames;
	this->fMonthsNames = in.fMonthsNames;
	this->fWeekdaysNames = in.fWeekdaysNames;
	this->id.SetTo(in.id);
}

int GregorianCalendar::FromGregorianToLocalYear(int year) { return year; }

int GregorianCalendar::FromLocalToGregorianYear(int year) { return year; }

map<int, BString> GregorianCalendar::GetDayNamesForLocalYearMonth(int localYear, int month) {
	return GetDayNamesForGregorianYearMonth(localYear, month);
}

map<int, BString> GregorianCalendar::GetDayNamesForGregorianYearMonth(int gregorianYear, int month)
{
	map<int, BString> toReturn;
	while (month < 1) {
		month += 12;
	}
	if ((month-1) >= 12) {
		month = ((month-1) % 12)+1;
	}
	// Now month is between 1 (January) and 12 (December)

	int i = 0, limit = 0;
	switch (month) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			// 31 days in month
			limit = 32;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			// 30 days in month
			limit = 31;
			break;
		case 2:
			// 28 or 29 days in month
			if (GregorianCalendar::IsYearLeap(gregorianYear)) {
				limit = 30;
			} else {
				limit = 29;
			}
			break;
		default:
			// Panic!
			break;
	};
	for (i=1; i<limit; ++i) {
		toReturn[i] = this->fDaysNames[i];
	}
	return toReturn;
}

map<int, DoubleNames> GregorianCalendar::GetMonthNamesForLocalYear(int localYear) {
	return (this->fMonthsNames);
}

map<int, DoubleNames> GregorianCalendar::GetMonthNamesForGregorianYear(int localYear) {
	return (this->fMonthsNames);
}

bool GregorianCalendar::IsYearLeap(TimeRepresentation &date) {
	return IsYearLeap(date.tm_year);
}

bool GregorianCalendar::IsYearLeap(int year) {
	if (year % 400 == 0) { return true; }
	if (year % 100 == 0) { return false; }
	if (year % 4 == 0) { return true; }
	return false;
}

/*! \function 		GregorianCalendar::IsDateValid
 *	\brief			Checks if the date represents a valid date in Gregorian calendar.
 *	\details		Gregorian calendar was proposed at 1582. It was adopted by 
 *					different countries at different years; there is no way to determine 
 *					the correct date of adoption on-the fly for every current user. 
 *					However, it may be generally assumed that the adoption did not 
 *					happen before 1600.
 *					Gregorian calendar is defined as 12 months, from 1 (January) to 12 
 *					(December). Any other month is illegal.
 *					Every month is from 1 to 28, 29, 30 or 31 days long.
 *					Hours and minutes are between -24:-59 and 24:59.
 */
bool GregorianCalendar::IsDateValid(TimeRepresentation& date) {	
	int limit = 0;
	if (date.tm_year < 1600) { return false; }		// 

	if (date.tm_mon <= 0 || date.tm_mon > 12) { return false; }		// 

	if (date.tm_mday <= 0) { return false; }
	switch (date.tm_mon) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			// 31 days in month
			limit = 32;
			break;
		case 4:
		case 6:
		case 9:
		case 11:
			// 30 days in month
			limit = 31;
			break;
		case 2:
			// 28 or 29 days in month
			if (IsYearLeap(date.tm_year)) {
				limit = 30;
			} else {
				limit = 29;
			}
			break;
		default:
			// Panic!
			break;
	};
	if (date.tm_mday >= limit) { return false; }
	if (date.tm_hour > 24 || date.tm_hour < -24) { return false; }
	if (date.tm_min > 59 || date.tm_min < -59) { return false; }
	return true;
}

/*! \function		GregorianCalendar::GetWeekDayForLocalDate
 *	\brief			Calculate the day of week for a given date
 *	\details		The only useful fields are "year", "month" and "day",
 *					since it's assumed that the week day is unknown.
 *					Working according to the Zeller's congruence
 *					(Wikipedia: http://en.wikipedia.org/wiki/Zeller's_congruence)
 *	\return			The corresponding day of week - as defined by the uint32.
 *	\param[in]	date	Struct tm describing the date for which the day 
 *						of week is needed.
 *	\param[out]	wday	Pointer to an integer. If it's not NULL, the pointer will be
 *						set to integer which describes the difference between
 *						day of week of TimeRepresentation submitted in the first
 *						parameter and previous Sunday.
 *	\sa		struct tm
 */
uint32 GregorianCalendar::GetWeekDayForLocalDate( const TimeRepresentation& date,
													int* wdayToReturn)
{
	uint32 toReturn = kInvalid;

	tm time = date.GetRepresentedTime();
	time.tm_mon -= 1;
	time.tm_year -= 1900;
	if (mktime(&time) < 0) { return kInvalid; }
	if (wdayToReturn != NULL) {
		*wdayToReturn = time.tm_wday+1;		// Sunday is day 0, but in this program, it's 1.
	}
	toReturn = 0x01;	// Sunday
	--time.tm_wday;		// Difference from Sunday was taken into account
	toReturn <<= time.tm_wday;
	return toReturn;
/*	 
//	int dayOfWeek, year = date.tm_year;	
	int tempDate = 0;
	int q = date.tm_year, k = q % 100, j = (int)q/100, h;
	int m = date.tm_mon; if (m < 3) { m += 12; }	// Month is from 3 (March) to 14 (February)

	if (! date.GetIsRepresentingRealDate()) { return kInvalid; }

	h = ((int)((int )((q + (int)(((m+1)*26)/10) + k + (int)(k/4) + (int)(j/4) + 5*j)%7) + 5) % 7) + 1;

	// Now h is from 1 - which is Monday - to 7 - which is Sunday.
	if (h < 1 || h > 7) {
		toReturn = kInvalid;
	} else {
		tempDate = (int)kMonday; --h;	// h is between 0 and 6 including
		tempDate <<= h;				// shifting from 0 to 6 bytes left.
	}
	toReturn = (enum WEEKDAYS)tempDate;

	return toReturn;
*/
}
// <-- end of function GregorianCalendar::GetWeekDayForLocalDate

/*!	\function		GregorianCalendar::DayFromBeginningOfTheYear
 *	\brief			Calculate the difference in days between the submitted day and Jan 1st.
 *	\details		If the TimeRepresentation does not represent a real date, this function
 *					indicates an error by returning a negative value. The function modifies
 *					the submitted time representation by setting its tm_yday member!
 *	\param[in]	timeIn		Reference to the time object.
 *	\returns		A positive int for a day of the year, a -1 in case of error.
 */
int GregorianCalendar::DayFromBeginningOfTheYear(TimeRepresentation& date)
{
	if (!date.GetIsRepresentingRealDate()) { return -1; }
	tm time = date.GetRepresentedTime();
	time.tm_mon -= 1;
	time.tm_year -= 1900;
	if (mktime(&time) < 0) { return -1; }
	return (date.tm_yday = time.tm_yday);
}
// <-- end of function GregorianCalendar::DayFromBeginningOfTheYear

map<uint32, DoubleNames> GregorianCalendar::GetWeekdayNames(void) {
	return fWeekdaysNames;	
}

TimeRepresentation GregorianCalendar::FromGregorianCalendarToLocal(TimeRepresentation &in) {
	TimeRepresentation toReturn	(in);
	return toReturn;
}

/*!	\function		GregorianCalendar::FromLocalCalendarToTimeT
 *	\brief			Converts a local date into a time_t object.
 *	\param[in]	timeIn		Reference to the time object.
 *	\returns		The time_t representation of the object.
 */
time_t GregorianCalendar::FromLocalCalendarToTimeT(const TimeRepresentation &timeIn) {
	time_t toReturn = 0;
	tm theTime = timeIn.GetRepresentedTime();
	theTime.tm_year -= 1900;
	theTime.tm_mon -= 1;	// Month is from 0 to 11
	toReturn = mktime(&theTime);	// Calculating the time_t from the submitted representation
	return toReturn;		// Even if mktime returns -1, it's probably Ok.
}
// <-- end of function FromLocalCalendarToTimeT

/*!	\function		GregorianCalendar::FromTimeTToLocalCalendar
 *	\brief			Converts a time_t into a TimeRepresentation object.
 *	\details		Since time_t does not bear any information about time zone, the resulting
 *					time is set to GMT. The caller should modify the result accordingly. The
 *					same goes for daylight saving time.
 *	\param[in]	timeIn		Time object.
 *	\returns		The TimeRepresentation of the object - in GMT time.
 */
TimeRepresentation GregorianCalendar::FromTimeTToLocalCalendar(const time_t timeIn) {
	tm temp;
	if ( localtime_r( &timeIn, &temp ) == NULL )
	{
		// Panic!
		exit(3);
	}
//	int tempDay = 0x01;	// tempDay is 0x01
	temp.tm_year += 1900;
	++(temp.tm_mon);
	
	TimeRepresentation toReturn( temp, "Gregorian");

	// Calculating and filling the week day

	// If the resulting day is not Sunday, we need to calculate it
	if (temp.tm_wday >= 0) {
		toReturn.tm_wday = temp.tm_wday;
	} else {
		toReturn.tm_wday = -1;	// Invalid week day
	}
	
	
//	// Setting the time zone to GMT
//	toReturn.tm_gmtoff = 0;
//	toReturn.tm_zone = new char[4];
//	if (! toReturn.tm_zone ) { // Panic!
//		exit(1);
//	}
//	strcpy(toReturn.tm_zone, "GMT");

	return toReturn;
}
// <-- end of function GregorianCalendar::FromTimeTToLocalCalendar

TimeRepresentation GregorianCalendar::AddTime(const TimeRepresentation &op1, const TimeRepresentation &op2) {
	TimeRepresentation toReturn;
	// Sanity check is performed inside of function AddTimeTo1stOperand
	if (op1.GetCalendarModule() == "") {
		toReturn = op2;
		this->AddTimeTo1stOperand(toReturn, op1);
	} else {
		toReturn = op1;
		this->AddTimeTo1stOperand(toReturn, op2);
	}
	return toReturn;
}
// <-- end of function GregorianCalendar::AddTime

TimeRepresentation& GregorianCalendar::AddTimeTo1stOperand(TimeRepresentation &op1, const TimeRepresentation &op2) {
	
	BString nameOfModule1 = op1.GetCalendarModule(), nameOfModule2 = op2.GetCalendarModule();
	BString ident = this->Identify();
	bool atLeastOneDateIsReal = false;

	// The operation is correct if one or both of the additives belonds to GregorianCalendar.
	// If only one of the additives belongs to GregorianCalendar, then other must not represent a real date.
	if (op1.GetIsRepresentingRealDate() && op2.GetIsRepresentingRealDate()) {		
		if ((nameOfModule1 != ident) && (nameOfModule2 != ident)) 
		{
			// Panic!
			exit(1);
		}		
	} else {	// At least one of the operands does not represent a real date.
		if ((op1.GetIsRepresentingRealDate() && nameOfModule1 != ident) ||
			(op2.GetIsRepresentingRealDate() && nameOfModule2 != ident)) 
		{
			// Panic!
			exit(1);
		}
	}
	atLeastOneDateIsReal = op1.GetIsRepresentingRealDate() || op2.GetIsRepresentingRealDate();

	// Adding hour, minute and second
	op1.tm_hour += op2.tm_hour;
	op1.tm_min  += op2.tm_min;
	op1.tm_sec	+= op2.tm_sec;

	// Adding day of month, month and year
	op1.tm_mon  += op2.tm_mon;
	op1.tm_mday += op2.tm_mday;
	op1.tm_year += op2.tm_year;

	// Normalize the date
	if (atLeastOneDateIsReal) {
		op1 = NormalizeDate(op1);
	}
	
	return op1;
}

/*!	\function		GregorianCalendar::NormalizeDate
 *	\brief			Verify the represented date is a legal date.
 *	\details		Verify the seconds, hours, minutes are in legal diapason.
 *					Verify the day, month and year are in legal diapason as well.
 *					Recalculate day of the year and day of the week.
 *					The isdst, time zone and GMT offset are not touched.
 *	\param[in]	in		TimeRepresentation that represents the date to be normalized.
 *						It is not changed during the normalization, hence it's const.
 *	\returns		The normalized TimeRepresentation.
 *	\remarks		In case of any errors, this function will just exit. It's up to
 *					the caller to call this function only when needed.
 */
 TimeRepresentation GregorianCalendar::NormalizeDate(const TimeRepresentation &in) {
	TimeRepresentation tR(in);	// Here we save time zone information.
	map<int, BString> daysInMonth;

	// If the represented time is not a date, no need to do anything.
	if (!in.GetIsRepresentingRealDate()) { return tR; }

	// Correct seconds
	while (tR.tm_sec < 0) {
		tR.tm_sec += 60;
		--tR.tm_min;
	}
	while (tR.tm_sec >= 60) {
		tR.tm_sec -= 60;
		++tR.tm_min;
	}

	// Correct minutes
	while (tR.tm_min < 0) {
		tR.tm_min += 60;
		--tR.tm_hour;
	}
	while (tR.tm_min >= 60) {
		tR.tm_min -= 60;
		++tR.tm_hour;
	}

	// Correct hours
	while (tR.tm_hour < 0) {
		tR.tm_hour += 24;
		--tR.tm_mday;
	}
	while (tR.tm_hour >= 24) {;
		tR.tm_hour -= 24;
		++tR.tm_mday;
	}

	// Correct days
	int limit = 0;
	while (tR.tm_mday < 0) {
		daysInMonth = this->GetDayNamesForGregorianYearMonth(tR.tm_year, tR.tm_mon-1);
		limit = (int )daysInMonth.size();
		tR.tm_mday += limit;
		--tR.tm_mon;
	}
	daysInMonth = this->GetDayNamesForGregorianYearMonth(tR.tm_year, tR.tm_mon);
	while (tR.tm_mday > (limit = (int)(daysInMonth.size()))) {
		tR.tm_mday -= limit;
		++tR.tm_mon;
		daysInMonth = this->GetDayNamesForGregorianYearMonth(tR.tm_year, tR.tm_mon);
	}

	// Correct months
	while (tR.tm_mon <= 0) {
		tR.tm_mon += 12;
		--tR.tm_year;
	}
	while (tR.tm_mon > 12) {
		tR.tm_mon -= 12;
		++tR.tm_year;
	}

	// Correct day of the week
	GetWeekDayForLocalDate(tR, &tR.tm_wday);

	// Correct day of the year
	TimeRepresentation Jan1st(tR), difference;	
	Jan1st.tm_hour = 12;
	Jan1st.tm_min = 0;
	difference = GetDifference(tR, Jan1st, true);
	tR.tm_yday = difference.tm_yday;

	return (tR);
}
// <-- end of function GregorianCalendar::NormalizeDate

/*!	\function	GregorianCalendar::GetDifference
 *	\brief		This function calculates time difference between two dates.
 *	\details	Level of details is defined by the third (optional) parameter.
 *				If it's true, only days matter - the difference is calculated between
 *				middays of the given days. Else, the difference is calculated to the
 *				level of seconds.
 *	\remarks	The result is NOT a valid date representation, it's a time period representation!
 *	\param[in]		op1		TimeRepresentation of the first date.
 *	\param[in]		op2		TimeRepresentation of the second date.
 *	\param[in]	daysOnly	If "true", only days difference means.
 *	\returns	The time representation of the difference.
 */
TimeRepresentation GregorianCalendar::GetDifference(const TimeRepresentation& op1, const TimeRepresentation& op2, bool daysOnly) {
	TimeRepresentation in1, in2, toReturn;
	// The difference will always be positive.
	if (op1 < op2) {
		in1 = op2;
		in2 = op1;
	} else {
		in1 = op1;
		in2 = op2;
	}
	// If we're interested only in days, then equify hours, minutes and seconds.
	if (daysOnly) {
		in1.tm_hour = 12;
		in1.tm_min = 0;
		in1.tm_sec = 0;
		in2.tm_hour = 12;
		in2.tm_min = 0;
		in2.tm_sec = 0;
	}
	tm inTm1 = in1.GetRepresentedTime(), inTm2 = in2.GetRepresentedTime();
	--inTm1.tm_mon; --inTm2.tm_mon;
	inTm1.tm_year -= 1900; inTm2.tm_year -= 1900;
	time_t time1 = mktime(&inTm1), time2 = mktime(&inTm2), differenceTime = time1 - time2;
	toReturn.tm_sec = differenceTime % 60; differenceTime = (int)(differenceTime / 60);
	toReturn.tm_min = differenceTime % 60; differenceTime = (int)(differenceTime / 60);
	toReturn.tm_hour = differenceTime % 24; differenceTime = (int)(differenceTime / 24);
	toReturn.tm_yday = differenceTime;
	toReturn.tm_mon = toReturn.tm_year = toReturn.tm_isdst = toReturn.tm_gmtoff = 0;
	toReturn.tm_wday = -1;
	toReturn.tm_zone = NULL;
	toReturn.SetIsRepresentingRealDate(false);
	return toReturn;
}
// <-- end of function GregorianCalendar::GetDifference


int GregorianCalendar::GetWeekDayForLocalDateAsInt(const TimeRepresentation& date) {
	int toReturn;
	this->GetWeekDayForLocalDate(date, &toReturn);
	return toReturn;
}


int GregorianCalendar::GetWeekDayForLocalDateAsInt(const uint32 in) {	
	if (in == kInvalid) { return -1; }
	return FromWeekDaysToInt(in);
}

int GregorianCalendar::FromWeekDaysToInt(const uint32 in) const {
	uint32 temp = in;
	if (in == 0) { return -1; }
	uint32 a = in-1;
	uint32 b = a;	 
	b <<= 1;
	b += 1;		// b is a string of 1 in length of in
	if ((temp|a) - b != 0) { return -1; }
	
	int toReturn = 1;
	while (temp != 1) {
		++toReturn;
		temp >>= 1;
	}
	return toReturn;
}

BList* GregorianCalendar::GetDefaultWeekend( void ) const
{
		// There's no need to make more room then we actually need
	BList* toReturn = new BList( this->GetDaysInWeek() );
	if ( !toReturn )
	{
		// Panic!
		exit(1);
	}
	
	toReturn->AddItem( (void*)kSaturday );
	toReturn->AddItem( (void*)kSunday );
	
	return toReturn;
}	// <-- end of function GregorianCalendar::GetDefaultWeekend


uint32	GregorianCalendar::GetDefaultStartingDayOfWeek( void ) const
{
	return kSunday;	
}	// <-- end of function GregorianCalendar::GetDefaultStartingDayOfWeek

#ifndef __GREGORIAN_CALENDAR_H__
#define __GREGORIAN_CALENDAR_H__

#include "TimeRepresentation.h"
#include "CalendarModule.h"

class CalendarModule;

class GregorianCalendar : 
	public CalendarModule
{
private: 

	/*! \brief	This function allows to calculate leap years.
	*			A year is leap if it divides by 4, but not by 100, but yes by 400.
	*/
	virtual bool IsYearLeap(int year);
	virtual bool IsYearLeap(TimeRepresentation &date);	
	virtual int FromWeekDaysToInt(const uint32 in) const;
	
public:
	GregorianCalendar();
	GregorianCalendar(const GregorianCalendar& in);
	
		//! These functions translate the times from one format to another.
	inline virtual TimeRepresentation FromLocalCalendarToGregorian(const TimeRepresentation& timeIn) { TimeRepresentation toReturn(timeIn); return toReturn; }
	virtual TimeRepresentation FromGregorianCalendarToLocal(TimeRepresentation& timeIn);
	virtual time_t FromLocalCalendarToTimeT(const TimeRepresentation& timeIn);
	virtual TimeRepresentation FromTimeTToLocalCalendar(const time_t timeIn);

	//! These functions translate the years to and from the local calendar.
	virtual int FromLocalToGregorianYear(int year);
	virtual int FromGregorianToLocalYear(int year);
	
	//! These functions return the names of the months in given year.
	virtual map<int, DoubleNames> GetMonthNamesForGregorianYear(int gregorianYear);
	virtual map<int, DoubleNames> GetMonthNamesForLocalYear(int localYear);

	//! These functions return the localized names of the days in given year and month.
	virtual map<int, BString> GetDayNamesForGregorianYearMonth(int gregoryanYear, int month);
	virtual map<int, BString> GetDayNamesForLocalYearMonth(int localYear, int month);

	/*! The following function returns map where each weekday's name is mapped to corresponding
	 *	uint32 from the WEEKDAYS consts.
	 */
	virtual map<uint32, DoubleNames> GetWeekdayNames(void);

	/*!	\brief	This way the caller can place a given date at a specific place in the grid.
	 */
	virtual uint32 GetWeekDayForLocalDate(const TimeRepresentation& date, int* wday = NULL);
	virtual int GetWeekDayForLocalDateAsInt(const TimeRepresentation& date);
	virtual int GetWeekDayForLocalDateAsInt(const uint32 in);
	virtual int DayFromBeginningOfTheYear(TimeRepresentation& date);

	inline virtual ~GregorianCalendar(void) {};

	//! Date legality verification.
	virtual TimeRepresentation NormalizeDate(const TimeRepresentation &in);
	/*! \brief	This function accepts a date and answers the question if it's valid or not.
	 *			It's the caller's responcibility to call this function only when 
	 *			the constructed struct tm represents really a date.
	 */
	virtual bool IsDateValid(TimeRepresentation& in);

	/*! \brief	This function calculates time difference between two dates.
	 */
	virtual TimeRepresentation GetDifference(const TimeRepresentation& op1, const TimeRepresentation& op2, bool daysOnly = false);

	//! Date manipulation routines
	virtual TimeRepresentation AddTime(const TimeRepresentation &op1, const TimeRepresentation &op2);
	virtual TimeRepresentation& AddTimeTo1stOperand(TimeRepresentation &op1, const TimeRepresentation &op2);
	
	inline virtual void SetLongestMonthLength(const unsigned char length=31) { fDaysInLongestMonth = length; }
	inline virtual void SetDaysInWeek(const unsigned char length=7) { fDaysInWeek = length; } 
	
	virtual BList* GetDefaultWeekend() const;
	virtual uint32  GetDefaultStartingDayOfWeek() const;
};

#endif	// __GREGORIAN_CALENDAR_H__

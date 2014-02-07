#ifndef __CALENDAR_MODULE_H__
#define __CALENDAR_MODULE_H__

#include <time.h>
#include <ctime>
#include <stdlib.h>
#include <cpp/map>
#include <support/SupportDefs.h>
#include <support/List.h>
#include <support/String.h>

#include "TimeRepresentation.h"

extern BList listOfCalendarModules;

class TimeRepresentation;

/*! \struct		DoubleNames
 *	\brief		A structure which consists of two strings.
 *	\details	Used for holding 
 */
struct DoubleNames {
	BString shortName;
	BString longName;
};

/*! \class	CalendarModule
	\brief	An abstract class that represents a calendar.
*/
class CalendarModule
{
protected:		// Constants for the calendar calculation
	unsigned char	fDaysInWeek;		//!< Usually it's 7. The range is from 0 to 255.
	map<int, DoubleNames> fMonthsNames;		//!< Names of the months, localized, in short and long form.
	map<int, BString> fDaysNames;			//!< Names of the days, localized.
	map<uint32, DoubleNames> fWeekdaysNames;	//!< Names of the weekdays, localized, in short and long form.
	BString id;							//!< Identifier of the module.
	unsigned char	fDaysInLongestMonth;	//!< How many days the longest month has?

public:
	//! These functions translate the times from one format to another.
	virtual TimeRepresentation FromLocalCalendarToGregorian(const TimeRepresentation& timeIn) = 0;
	virtual TimeRepresentation FromGregorianCalendarToLocal(TimeRepresentation& timeIn) = 0;
	virtual time_t FromLocalCalendarToTimeT(const TimeRepresentation& timeIn) = 0;
	virtual TimeRepresentation FromTimeTToLocalCalendar(const time_t timeIn) = 0;

	//! These functions translate the years to and from the local calendar.
	virtual int FromLocalToGregorianYear(int year) = 0;
	virtual int FromGregorianToLocalYear(int year) = 0;
	
	//! These functions return the names of the months in given year.
	virtual map<int, DoubleNames> GetMonthNamesForGregorianYear(int gregorianYear) = 0;
	virtual map<int, DoubleNames> GetMonthNamesForLocalYear(int localYear) = 0;

	//! These functions return the localized names of the days in given year and month.
	virtual map<int, BString> GetDayNamesForGregorianYearMonth(int gregoryanYear, int month) = 0;
	virtual map<int, BString> GetDayNamesForLocalYearMonth(int localYear, int month) = 0;

	/*! The following function returns map where each weekday's name is mapped to corresponding
	 *	int from the enum WEEKDAYS.
	 */
	virtual map<uint32, DoubleNames> GetWeekdayNames(void) = 0;

	/*!	\brief	This way the caller can place a given date at a specific place in the grid.
	 */
	virtual uint32 GetWeekDayForLocalDate(const TimeRepresentation& date, int *wday = NULL) = 0;
	virtual int GetWeekDayForLocalDateAsInt(const TimeRepresentation& date) = 0;
	virtual int GetWeekDayForLocalDateAsInt(const uint32 in) = 0;
	virtual int DayFromBeginningOfTheYear(TimeRepresentation& date) = 0;

	//! Identification
	virtual const BString Identify(void);	
	
	//! Construction and destruction
/*	CalendarModule();
	CalendarModule(const BString& );
	CalendarModule(const CalendarModule& in);
*/
	virtual ~CalendarModule(void);

	//! Date legality verification.
	virtual TimeRepresentation NormalizeDate(const TimeRepresentation &in) = 0;
	/*! \brief	This function accepts a date and answers the question if it's valid or not.
	 *			It's the caller's responcibility to call this function only when 
	 *			the constructed struct tm represents really a date.
	 */
	virtual bool IsDateValid(TimeRepresentation& in) = 0;

	/*! \brief	This function calculates time difference between two dates.
	 */
	virtual TimeRepresentation GetDifference(const TimeRepresentation& op1, const TimeRepresentation& op2, bool daysOnly = false) = 0;

	//! Date manipulation routines
	virtual TimeRepresentation AddTime(const TimeRepresentation &op1, const TimeRepresentation &op2) = 0;
	virtual TimeRepresentation& AddTimeTo1stOperand(TimeRepresentation &op1, const TimeRepresentation &op2) = 0;
/*	virtual TimeRepresentation SubTime(const TimeRepresentation &op1, const TimeRepresentation &op2) = 0;
	virtual TimeRepresentation& SubTimeFrom1stOperand(TimeRepresentation &op1, const TimeRepresentation &op2) = 0;
*/	
	//! To ease the calculation of the rectangle to display the month
	virtual void SetLongestMonthLength(const unsigned char length) = 0;
	virtual unsigned char GetLongestMonthLength(void) const;
	virtual void SetDaysInWeek(const unsigned char length) = 0;
	virtual unsigned char GetDaysInWeek(void) const;
	
	virtual BList* GetDefaultWeekend( void ) const = 0;
	virtual uint32	GetDefaultStartingDayOfWeek( void ) const = 0;
};

#endif		// __CALENDAR_MODULE_H__

#include "TimeRepresentation.h"
#include "CalendarModule.h"

CalendarModule::~CalendarModule(void)
{
	this->id.Truncate(0);
}

const BString CalendarModule::Identify() {
	return this->id;
}

unsigned char CalendarModule::GetLongestMonthLength(void) const {
	return fDaysInLongestMonth;	
}

unsigned char CalendarModule::GetDaysInWeek(void) const {
	return fDaysInWeek;
}

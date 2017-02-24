#include "time.h"

uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define TIME_FLAG_A		0xABCD
#define TIME_FLAG_B		0xEF01
#define TIME_FLAG_B_GPS	0xEF02

volatile uint16_t unix_time_set_flag_a __attribute__ ((section (".noinit")));
volatile uint32_t unix_time __attribute__ ((section (".noinit")));
volatile uint16_t unix_time_set_flag_b __attribute__ ((section (".noinit")));

#define LEAP_YEAR(_year) ((_year%4)==0)

#define TIME_MIN_DATE	(1420113600) //1.1.2015 12:00.00

bool time_is_set()
{
	if (unix_time_set_flag_a == TIME_FLAG_A && unix_time_set_flag_b == TIME_FLAG_B)
		return true;
	else
		return false;
}

bool time_need_set()
{
	if (unix_time_set_flag_a == TIME_FLAG_A)
		if (unix_time_set_flag_b == TIME_FLAG_B || unix_time_set_flag_b == TIME_FLAG_B_GPS)
			return false;

	return true;
}

void time_wait_for_gps()
{
	unix_time_set_flag_a = TIME_FLAG_A;
	unix_time_set_flag_b = TIME_FLAG_B_GPS;
}

void time_set_default()
{
	unix_time = TIME_MIN_DATE;
}

uint32_t datetime_to_epoch(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year)
{
	uint16_t i;
	uint32_t timestamp;

	// seconds from 1970 till 1 jan 00:00:00 this year
	timestamp = (year - 1970) * (60 * 60 * 24L * 365);

	// add extra days for leap years
	for (i = 1970; i < year; i++) {
		if (LEAP_YEAR(i)) {
			timestamp += 60 * 60 * 24L;
		}
	}
	// add days for this year
	for (i = 0; i < (uint16_t)(month - 1); i++) {
		if (i == 1 && LEAP_YEAR(year)) {
			timestamp += 60 * 60 * 24L * 29;
		} else {
			timestamp += 60 * 60 * 24L * monthDays[i];
		}
	}

	timestamp += (day - 1) * 3600 * 24L;
	timestamp += hour * 3600L;
	timestamp += min * 60L;
	timestamp += sec;
	return timestamp;
}


void time_from_epoch(uint32_t epoch, uint8_t * psec, uint8_t * pmin, uint8_t * phour)
{
	*psec=epoch%60;
	epoch/=60; // now it is minutes
	*pmin=epoch%60;
	epoch/=60; // now it is hours
	*phour=epoch%24;
}

void datetime_from_epoch(uint32_t epoch, uint8_t * psec, uint8_t * pmin, uint8_t * phour, uint8_t * pday, uint8_t * pwday, uint8_t * pmonth, uint16_t * pyear)
{
	uint8_t year;
	uint8_t month, monthLength;
	uint32_t days;

	*psec=epoch%60;
	epoch/=60; // now it is minutes
	*pmin=epoch%60;
	epoch/=60; // now it is hours
	*phour=epoch%24;
	epoch/=24; // now it is days

	*pwday=(epoch+4)%7;

	year=70;
	days=0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch) {
		year++;
	}
	*pyear=year + 1900; // *pyear is returned as years from 1900

	days -= LEAP_YEAR(year) ? 366 : 365;
	epoch -= days; // now it is days in this year, starting at 0
	//*pdayofyear=epoch;  // days since jan 1 this year

	days=0;
	month=0;
	monthLength=0;
	for (month=0; month<12; month++) {
		if (month==1) { // february
			if (LEAP_YEAR(year)) {
				monthLength=29;
			} else {
				monthLength=28;
			}
		} else {
			monthLength = monthDays[month];
		}

		if (epoch>=monthLength) {
			epoch-=monthLength;
		} else {
			break;
		}
	}
	*pmonth=month + 1;  // jan is month 1
	*pday=epoch+1;  // day of month
}

void print_datetime(uint32_t epoch)
{
	uint8_t sec, min, hour, day, wday, month;
	uint16_t year;

	datetime_from_epoch(epoch, &sec, &min, &hour, &day, &wday, &month, &year);

	DEBUG("%02d.%02d.%04d %02d:%02d.%02d\n", day, month, year, hour, min, sec);
}

volatile bool time_rtc_irq;

ISR(rtc_overflow_interrupt)
{
	time_rtc_irq = true;
	unix_time += 1;
}

void time_init()
{
	RTC_PWR_ON;

	unix_time += 1;

	RtcSetPeriod(32767); //do not forget -1 , since 0 count!
	RtcInit(rtc_32kHz_tosc, rtc_div1); //f == 1024Hz
	RtcEnableInterrupts(rtc_overflow); //ovf every sec

	if (time_get_local() < TIME_MIN_DATE)
		unix_time = TIME_MIN_DATE;
}

void time_set_flags()
{
	unix_time_set_flag_a = TIME_FLAG_A;
	unix_time_set_flag_b = TIME_FLAG_B;
}

void time_set_local(uint32_t t)
{
	unix_time = t;
}

void time_set_utc(uint32_t t)
{
	time_set_local(t + config.system.time_zone * 1800ul);
}


uint32_t time_get_local()
{
	return unix_time;
}

uint32_t time_get_utc()
{
	return unix_time - (config.system.time_zone * 1800ul);
}

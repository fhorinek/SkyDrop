
#ifndef TIME_H_
#define TIME_H_

#include "../skydrop.h"

uint32_t datetime_to_epoch(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t month, uint16_t year);

void time_from_epoch(uint32_t epoch, uint8_t * psec, uint8_t * pmin, uint8_t * phour);
void datetime_from_epoch(uint32_t epoch, uint8_t * psec, uint8_t * pmin, uint8_t * phour, uint8_t * pday, uint8_t * pwday, uint8_t * pmonth, uint16_t * pyear);

void print_datetime(uint32_t epoch);

uint32_t time_get_local();
void time_set_local(uint32_t t);

void time_set_utc(uint32_t t);
uint32_t time_get_utc();

void time_init();
void time_set_flags();
void time_set_default();
bool time_is_set();
bool time_need_set();
void time_wait_for_gps();

uint16_t time_get_rtc_value();

extern uint8_t monthDays[12];

#endif

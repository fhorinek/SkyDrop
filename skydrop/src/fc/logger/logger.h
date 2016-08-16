/*
 * logger.h
 *
 *  Created on: 5.10.2015
 *      Author: horinek
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#define LOG_DIR		PSTR("logs")
#define LOG_MID		"XSB"
#define LOG_MID_P	PSTR(LOG_MID)


#include "../../common.h"
#include "../../drivers/storage/storage.h"

extern FIL log_fil;
extern uint8_t logger_flight_number;

void logger_init();
void logger_step();
void logger_start();
void logger_stop();
void logger_comment(char * text);
bool logger_active();
bool logger_error();

#endif /* LOGGER_H_ */

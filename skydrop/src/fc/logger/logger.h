/*
 * logger.h
 *
 *  Created on: 5.10.2015
 *      Author: horinek
 */

#ifndef LOGGER_H_
#define LOGGER_H_


#define LOG_DIR		"logs"
#define LOG_DIR_P	PSTR("logs")
#define LOG_DIR_ABS	 "/" LOG_DIR
#define LOG_MID		"XSB"
#define LOG_MID_P	PSTR("XSB")


#include "../../common.h"
#include "../../drivers/storage/storage.h"

extern FIL log_file;
extern uint8_t logger_flight_number;

void logger_init();
void logger_step();
void logger_start();
void logger_stop();
void logger_comment(const char *format, ...);
bool logger_active();
bool logger_error();

#endif /* LOGGER_H_ */

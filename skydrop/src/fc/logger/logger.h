/*
 * logger.h
 *
 *  Created on: 5.10.2015
 *      Author: horinek
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#define LOG_DIR		"logs"
#define LOG_DIR_P	PSTR(LOG_DIR)
#define LOG_MID		"XSB"
#define LOG_MID_P	PSTR(LOG_MID)


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

/**
 * Return how many entries we have in the LOG_DIR directory.
 *
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the number of path entries found.
 */
uint16_t logger_count(bool onlyDir);

/**
 * Return how many entries we have.
 *
 * @param dirname The name of the directory to search through all subdirs
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the number of path entries found.
 */
uint16_t logger_count(const char *dirname, bool dirOnly);

/**
 * Traverse LOG_DIR and return the path entry number "count" in fname.
 * If the entry is found, then "0" is returned. Otherwise the remaining
 * value of count is returned. E.g. if the current directory holds 3
 * files and we look for count=5, then 2 is returned.
 *
 * @param count   The number of the entry, which we are searching, start with "1"
 * @param fname   A pointer to a memory area allocated by caller to take full filename
 *                (including directory) of the found filename
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the remaining count.
 */
uint16_t logger_fileno(uint16_t count, char *fname, bool onlyDir);

/**
 * Traverse dirname and return the path entry number "count" in fname.
 * If the entry is found, then "0" is returned. Otherwise the remaining
 * value of count is returned. E.g. if the current directory holds 3
 * files and we look for count=5, then 2 is returned.
 *
 * @param dirname The name of the directory, where we start searching
 * @param count   The number of the entry, which we are searching, start with "1"
 * @param fname   A pointer to a memory area allocated by caller to take full filename
 *                (including directory) of the found filename
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the remaining count.
 */
uint16_t logger_fileno(const char *dirname, uint16_t count, char *fname, bool onlyDir);

/**
 * Return how many entries we have.
 *
 * @param dirname The name of the directory to search through all subdirs
 * @param dirOnly Set to "true", if we only look for directories.
 *                "false" means, to look for files only.
 *
 * @return the number of path entries found.
 */
uint16_t logger_count();

#endif /* LOGGER_H_ */

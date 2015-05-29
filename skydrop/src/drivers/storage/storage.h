/*
 * storage.h
 *
 *  Created on: 23.7.2014
 *      Author: horinek
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include "../../skydrop.h"

#include "FatFs/ff.h"
#include "FatFs/diskio.h"

//Glue for LUFA
#define DATAFLASH_PAGE_SIZE		512

bool storage_init();
void storage_deinit();
bool storage_selftest();
void storage_step();

#endif /* STORAGE_H_ */

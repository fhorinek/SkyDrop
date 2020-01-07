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

bool storage_card_in();
bool storage_ready();
void storage_step();

//flags for storage_dir_list
#define STORAGE_IS_DIR	0x01

uint8_t storage_get_files();
bool storage_dir_open(char * path);
void storage_dir_close();
void storage_dir_rewind();
bool storage_dir_list(char * fname, uint8_t * flags);

bool storage_file_exist(char * path);
bool storage_file_exist_P(const char * path);


#endif /* STORAGE_H_ */

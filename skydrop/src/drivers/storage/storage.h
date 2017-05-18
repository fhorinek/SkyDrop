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

uint8_t storage_dir_get_count();
bool storage_dir_open(char * path);
void storage_dir_close();
void storage_dir_rewind();
bool storage_dir_list(char * fname, uint8_t * flags);

/**
 * Read the "no" directory entry. If count is "1" this is
 * the next entry. For "2" it is the second entry ....
 *
 * @param fname a pointer to memory where the name of the
 *              directory entry is returned to the caller.
 * @param flags a pointer to the flags returned to the caller.
 *              Entries describing a sub directory the flags
 *              contain STORAGE_IS_DIR.
 * @param no    the number of the directory entry to read.
 *
 * @return the number of remaining entries, after hitting
 *         the end of the directory. If the entry could be
 *         read successfully, this is "0". Otherwise something
 *         between 0 and the number given in. E.g. if the
 *         directory holds 500 entries and you try to read
 *         number 600. It will return 100.
 */
int16_t storage_dir_list_n(char * fname, uint8_t * flags, int16_t no);

#endif /* STORAGE_H_ */

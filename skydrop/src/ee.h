/*
 * ee.h
 *
 *  Created on: 6. 4. 2020
 *      Author: horinek
 */

#ifndef EE_H_
#define EE_H_

#include "common.h"

#define ee_update_byte(PTR, VAL) ee_update_block((void *)&VAL, (void *)PTR, 1);
#define ee_update_word(PTR, VAL) ee_update_block((void *)&VAL, (void *)PTR, 2);
#define ee_update_dword(PTR, VAL) ee_update_block((void *)&VAL, (void *)PTR, 4);
#define ee_update_float(PTR, VAL) ee_update_block((void *)&VAL, (void *)PTR, 4);

#define ee_read_byte(PTR, VAL) ee_read_block((void *)&VAL, (void *)PTR, 1);
#define ee_read_word(PTR, VAL) ee_read_block((void *)&VAL, (void *)PTR, 2);
#define ee_read_dword(PTR, VAL) ee_read_block((void *)&VAL, (void *)PTR, 4);


void ee_update_block(void * ram_ptr, void * ee_ptr, uint16_t size);
void ee_read_block(void * ram_ptr, void * ee_ptr, uint16_t size);


#endif /* EE_H_ */

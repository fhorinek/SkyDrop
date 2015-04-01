/*
 * cfg.h
 *
 *  Created on: 22.10.2014
 *      Author: horinek
 */

#ifndef CFG_H_
#define CFG_H_

#include "FatFs/ff.h"
#include "../../common.h"
#include <string.h>

bool cfg_get_str(FIL * fl, const char * section, const char * label, char * value);

int cfg_get_int(FIL * fl, const char * section, const char * label, int def);
bool cfg_have_section(FIL * fl, const char * section);

#endif /* CFG_H_ */

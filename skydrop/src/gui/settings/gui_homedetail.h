/*
 * gui_homedetail.h
 *
 *  Created on: 11.05.2017
 *      Author: tilmann@bubecks.de
 */


#ifndef GUI_HOMEDETAIL_H_
#define GUI_HOMEDETAIL_H_

#include "../gui.h"
#include "../../fc/fc.h"

#define gui_homedetail_stop gui_dummy
#define gui_homedetail_loop gui_dummy
#define gui_homedetail_irqh gui_dummy

void gui_homedetail_init();

void read_homefile(const char *filename, cfg_home * home);

#endif /* GUI_HOMEDETAIL_H_ */

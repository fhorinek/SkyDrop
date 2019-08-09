/*
 * factory_test.h
 *
 *  Created on: 26.5.2015
 *      Author: horinek
 */

#ifndef GUI_FACTORY_TEST_H_
#define GUI_FACTORY_TEST_H_

#include "gui.h"

void gui_factory_test_init();
void gui_factory_test_stop();
void gui_factory_test_loop();
void gui_factory_test_irqh(uint8_t type, uint8_t * buff);

#endif /* GUI_FACTORY_TEST_H_ */

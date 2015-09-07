
#ifndef BUZZER_H_
#define BUZZER_H_


#include "../../common.h"

void buzzer_init();
void buzzer_set_freq(uint16_t freq_hz);
void buzzer_set_vol(uint8_t vol);


#endif /* BUZZER_H_ */

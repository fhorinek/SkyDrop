#ifndef DAC_H_
#define DAC_H_

#include "../common.h"
#include "gpio.h"

typedef enum xlib_core_dac_outputs_e
{
	dac_internal = 0b00000001,
	dac_ch0 = 0b00000010,
	dac_ch1 = 0b00000100
} xlib_core_dac_outputs;

typedef enum xlib_core_dac_reference_e
{
	dac_int1V = DAC_REFSEL_INT1V_gc,
	dac_avcc = DAC_REFSEL_AVCC_gc,
	dac_refA = DAC_REFSEL_AREFA_gc,
	dac_refB = DAC_REFSEL_AREFB_gc,
} xlib_core_dac_reference;

void DacInit(uint8_t outputs);
void DacInit();
void DacSetReference(xlib_core_dac_reference ref);

//more speed
#define DacCh0Ready() (DACB.STATUS & DAC_CH0DRE_bm)
#define DacCh1Ready() (DACB.STATUS & DAC_CH1DRE_bm)
#define DacSetCh0(A) {while (!DacCh0Ready()); DACB.CH0DATA = A;}
#define DacSetCh1(A) {while (!DacCh1Ready()); DACB.CH1DATA = A;}

#endif /* DAC_H_ */

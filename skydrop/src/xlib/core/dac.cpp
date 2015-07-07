#include "dac.h"

extern uint32_t freq_cpu;

void DacInit()
{
	DacInit(dac_ch0 | dac_ch1 | dac_internal);
}

void DacInit(uint8_t enable)
{
	uint8_t ctrla = DAC_ENABLE_bm;
	uint8_t ctrlb = 0;


	if (enable & dac_internal)
		ctrla |= DAC_IDOEN_bm;

	if (enable & dac_ch0)
	{
		ctrla |= DAC_CH0EN_bm;
		GpioWrite(dac0, OUTPUT);
		ctrlb = DAC_CHSEL_SINGLE_gc;
	}

	if (enable & dac_ch1)
	{
		ctrla |= DAC_CH1EN_bm;
		GpioWrite(dac1, OUTPUT);
		ctrlb = DAC_CHSEL_SINGLE1_gc;
	}

	if (enable & dac_ch0 || enable & dac_ch1)
		ctrlb = DAC_CHSEL_DUAL_gc;

	DACB.CTRLA = ctrla;
	DACB.CTRLB = ctrlb;
}

void DacSetReference(xlib_core_dac_reference ref)
{
	DACB.CTRLC |= ref;
}

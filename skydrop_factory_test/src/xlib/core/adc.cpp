#include "adc.h"

extern uint32_t freq_cpu;

#define ADCACAL0_offset 0x20
#define ADCACAL1_offset 0x21
#define ADCBCAL0_offset 0x24
#define ADCBCAL1_offset 0x25

uint16_t xlib_core_adc_pow(int8_t exp)
{
    uint16_t res = 1;

    while (exp > 0)
    {
		exp--;
		res = res * 2;
	}

    return res;
}

/**
 * Initialize ADCA subsytem with selected voltage reference
 *
 * \param ref Voltage reference
 *
 * \note this method should be after setting system clock
 */
void AdcAInit(xlib_core_adc_reference ref)
{
	//load callibration
	ADCA.CALL = SP_ReadCalibrationByte(PROD_SIGNATURES_START + ADCACAL0_offset);
	ADCA.CALH = SP_ReadCalibrationByte(PROD_SIGNATURES_START + ADCACAL1_offset);

	//enable 0 1 2 3 adc ch, enable adc
	ADCA.CTRLA = ADC_ENABLE_bm;

	//set resolution 12bit & unsigned mode
	ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;

	//prepare ref for bandgap & temp
	ADCA.REFCTRL = ADC_BANDGAP_bm | ADC_TEMPREF_bm;

	//Set max freq (2Mhz)
	uint8_t i;
	for (i=2; i<10; i++)
		if ((uint32_t)(freq_cpu >> i) <= 2000000UL) break;

	ADCA.PRESCALER = 0b00000011 & (i-2);

	AdcASetReference(ref);
}

/**
 * Initialize ADCB subsytem with selected voltage reference
 *
 * \param ref Voltage reference
 *
 * \note this method should be after setting system clock
 */
void AdcBInit(xlib_core_adc_reference ref)
{
	//load callibration
	ADCB.CALL = SP_ReadCalibrationByte(PROD_SIGNATURES_START + ADCBCAL0_offset);
	ADCB.CALH = SP_ReadCalibrationByte(PROD_SIGNATURES_START + ADCBCAL1_offset);

	//enable 0 1 2 3 adc ch, enable adc
	ADCB.CTRLA = ADC_ENABLE_bm;

	//set resolution 12bit & unsigned mode
	ADCB.CTRLB = ADC_RESOLUTION_12BIT_gc;

	//prepare ref for bandgap & temp
	//not needed for bioprobe
	//ADCB.REFCTRL = ADC_BANDGAP_bm | ADC_TEMPREF_bm;

	//Set max freq (2Mhz)
	uint8_t i;
	for (i=2; i<10; i++)
		if ((uint32_t)(freq_cpu >> i) <= 2000000UL) break;

	ADCB.PRESCALER = 0b00000011 & (i-2);

	AdcBSetReference(ref);
}

/**
 * Set source for ADC pipe
 *
 * \param pipe ADC pipe
 * \param mux Source for adc pipe
 *
 * \note pipea0-3 is physically connected to PORTA on atxmega128a3 and is able to use only porta0-7 as external sources
 * \note pipeb0-3 is physically connected to PORTB on atxmega128a3 and is able to use only portb0-7 as external sources
 */
void AdcPipeSetSource(ADC_CH_t * pipe, xlib_core_adc_source mux)
{
	if (mux >= 0xA0)
	{
		pipe->CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc | (pipe->CTRL & 0b11111100);
		pipe->MUXCTRL = ((mux - 0xA0) << 3) | (pipe->MUXCTRL & 0b10000111);
	}
	else
	{
		pipe->CTRL = ADC_CH_INPUTMODE_INTERNAL_gc | (pipe->CTRL & 0b11111100);
		pipe->MUXCTRL = (mux << 3) | (pipe->MUXCTRL & 0b10000111);
	}
}

/**
 * Set differential source for ADC pipe
 *
 * \param pipe ADC pipe
 * \param gain Gain
 * \param s1 Positive source
 * \param s2 Negative source
 *
 * \note ADC must be in unsigned mode
 * \note pipea0-3 is physically connected to PORTA on atxmega128a3 and is able to use only porta0-7 as positive source, porta0-7 as negative source without gain and porta4-7 as negative source using gain
 * \note pipeb0-3 is physically connected to PORTB on atxmega128a3 and is able to use only portb0-7 as positive source, portb0-7 as negative source without gain and portb4-7 as negative source using gain
 */
void AdcPipeSetDiffSource(ADC_CH_t * pipe, xlib_core_adc_gain gain, xlib_core_adc_source s1, xlib_core_adc_source s2)
{
	pipe->MUXCTRL = ((s1 - 0xA0) << 3) | (pipe->MUXCTRL & 0b10000111);

	if (gain == adc_gain1)
	{
		if (s2 > 0xA3)
			pipe->CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc;
		else
			pipe->CTRL = ADC_CH_INPUTMODE_DIFF_gc;
	}
	else
		pipe->CTRL = ADC_CH_INPUTMODE_DIFFWGAIN_gc | gain;


	pipe->MUXCTRL = ((s2 - 0xA0) % 4) | (pipe->MUXCTRL & 0b11111100);
}

/**
 * Set ADC pipe interrupt
 *
 * \param pipe ADC Pipe
 * \param interrupt Interrupt type
 */
void AdcPipeSetInterrupt(ADC_CH_t * pipe, xlib_core_adc_interrupt interrupt)
{
	if (interrupt == adc_none)
		pipe->INTCTRL = 0;
	else
		pipe->INTCTRL = interrupt | ADC_CH_INTLVL_LO_gc;
}

/**
 * Start ADC pipe conversion
 *
 * \param pipe ADC pipe
 */
void AdcPipeStart(ADC_CH_t * pipe)
{
	pipe->CTRL |= ADC_CH_START_bm;
}

/**
 * Get ADC pipe value
 *
 * \param pipe ADC pipe
 * \return 16-bit value of ADC pipe
 */
uint16_t AdcPipeValue(ADC_CH_t * pipe)
{
	return pipe->RES;
}

/**
 * Read ADC value from input
 *
 * \param pipe ADC pipe used for conversion
 * \param mux Source
 *
 * \note pipea0-3 is physically connected to PORTA on atxmega128a3 and is able to use only porta0-7 as external sources
 * \note pipeb0-3 is physically connected to PORTB on atxmega128a3 and is able to use only portb0-7 as external sources
 *
 * \return 16-bit value representing source voltage
 *
 */
uint16_t AdcPipeRead(ADC_CH_t * pipe, xlib_core_adc_source mux)
{
	//set mux
	AdcPipeSetSource(pipe, mux);
	//intflag when complete
	pipe->INTCTRL = ADC_CH_INTMODE_COMPLETE_gc;
	//start
	AdcPipeStart(pipe);
	//wait until conversion is done
	while (pipe->INTFLAGS & ADC_CH_CHIF_bm);
	//clear intflag
	pipe->INTFLAGS |= ADC_CH_CHIF_bm;

	//retutn value
	return AdcPipeValue(pipe);
}

bool AdcPipeReady(ADC_CH_t * pipe)
{
	bool tmp = pipe->INTFLAGS & ADC_CH_CHIF_bm;

	//clear intflag
	pipe->INTFLAGS |= ADC_CH_CHIF_bm;

	return tmp;
}
//TODO: freerun + events + temperature
//evctrl.sweep + fr bit


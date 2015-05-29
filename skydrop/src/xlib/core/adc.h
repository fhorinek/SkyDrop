#ifndef ADC_H_
#define ADC_H_

#include "../common.h"

typedef enum xlib_core_adc_reference_e
{
	adc_int1V = ADC_REFSEL_INT1V_gc,	//!< Accurate internal 1.00V reference
	adc_avcc = ADC_REFSEL_INTVCC_gc,	//!< Internal VCC/1.6V reference
	adc_refA = ADC_REFSEL_AREFA_gc,		//!< External reference on porta0
	adc_refB = ADC_REFSEL_AREFB_gc,		//!< External reference on portb0
	adc_avcc2 = ADC_REFSEL_INTVCC2_gc	//!< Internal VCC/1.6V reference
} xlib_core_adc_reference;

typedef enum xlib_core_adc_mode_e
{
	adc_signed = ADC_CONMODE_bm,		//!< Signed mode operation
	adc_unsigned = 0					//!< Unsigned mode operation
} xlib_core_adc_mode;

typedef enum xlib_core_adc_source_e
{
	//internal
	int_temp = 0x00,					//!< Internal temperature sensor
	int_bandgap = 0x01,					//!< Internal source for voltage references
	int_vcc = 0x02,						//!< Internal VCC scaled down by 10
	int_dac = 0x03,						//!< Output from DAC
	//external
	ext_porta0 = 0xA0,					//!< External source on porta0
	ext_porta1 = 0xA1,					//!< External source on porta1
	ext_porta2 = 0xA2,					//!< External source on porta2
	ext_porta3 = 0xA3,					//!< External source on porta3
	ext_porta4 = 0xA4,					//!< External source on porta4
	ext_porta5 = 0xA5,					//!< External source on porta5
	ext_porta6 = 0xA6,					//!< External source on porta6
	ext_porta7 = 0xA7,					//!< External source on porta7
	ext_portb0 = 0xA0,					//!< External source on portb0
	ext_portb1 = 0xA1,					//!< External source on portb1
	ext_portb2 = 0xA2,					//!< External source on portb2
	ext_portb3 = 0xA3,					//!< External source on portb3
	ext_portb4 = 0xA4,					//!< External source on portb4
	ext_portb5 = 0xA5,					//!< External source on portb5
	ext_portb6 = 0xA6,					//!< External source on portb6
	ext_portb7 = 0xA7,					//!< External source on portb7
} xlib_core_adc_source;

typedef enum xlib_core_adc_interrupt_e
{
	adc_none = 0x1,								//!< Clear Interrupt
	adc_complete = ADC_CH_INTMODE_COMPLETE_gc,	//!< Set interrupt when ADC conversion is complete
	adc_below = ADC_CH_INTMODE_BELOW_gc,		//!< Set interrupt when ADC result is below threshold
	adc_above = ADC_CH_INTMODE_ABOVE_gc			//!< Set interrupt when ADC result is above threshold
} xlib_core_adc_interrupt;

typedef enum xlib_core_adc_gain_e
{
	adc_gain1 =	ADC_CH_GAIN_1X_gc,				//!< No gain
	adc_gain2 =	ADC_CH_GAIN_2X_gc,				//!< Gain 2x
	adc_gain4 =	ADC_CH_GAIN_4X_gc,				//!< Gain 4x
	adc_gain8 =	ADC_CH_GAIN_8X_gc,				//!< Gain 8x
	adc_gain16 = ADC_CH_GAIN_16X_gc,			//!< Gain 16x
	adc_gain32 = ADC_CH_GAIN_32X_gc,			//!< Gain 32x
	adc_gain64 = ADC_CH_GAIN_64X_gc,			//!< Gain 64x
} xlib_core_adc_gain;

#define pipea0	&ADCA.CH0
#define pipea1	&ADCA.CH1
#define pipea2	&ADCA.CH2
#define pipea3	&ADCA.CH3

#define pipeb0	&ADCB.CH0
#define pipeb1	&ADCB.CH1
#define pipeb2	&ADCB.CH2
#define pipeb3	&ADCB.CH3


#define pipea0_interrupt	ADCA_CH0_vect
#define pipea1_interrupt	ADCA_CH1_vect
#define pipea2_interrupt	ADCA_CH2_vect
#define pipea3_interrupt	ADCA_CH3_vect

#define pipeb0_interrupt	ADCB_CH0_vect
#define pipeb1_interrupt	ADCB_CH1_vect
#define pipeb2_interrupt	ADCB_CH2_vect
#define pipeb3_interrupt	ADCB_CH3_vect

/**
 * Set reference for ADCA
 *
 * \prama ref Voltage reference
 */
#define	AdcASetReference(ref) ADCA.REFCTRL = ref | (ADCA.REFCTRL & 0b11001111);
/**
 * Set reference for ADCB
 *
 * \prama ref Voltage reference
 */
#define	AdcBSetReference(ref) ADCB.REFCTRL = ref | (ADCB.REFCTRL & 0b11001111);

/**
 * Set ADCA mode
 *
 * \prama mode ADC mode
 */
#define	AdcASetMode(mode) ADCA.CTRLB = mode | (ADCA.CTRLB & 0b11101111);
/**
 * Set ADCB mode
 *
 * \prama mode ADC mode
 */
#define	AdcBSetMode(mode) ADCB.CTRLB = mode | (ADCB.CTRLB & 0b11101111);

/**
 * Set threshold value for ACDA
 *
 * \param cmp 16-bit threshold value
 *
 * \note this function is used for below/above interrupt
 */
#define	AdcASetThreshold(cmp) ADCA.CMP = cmp;
/**
 * Set threshold value for ACDB
 *
 * \param cmp 16-bit threshold value
 *
 * \note this function is used for below/above interrupt
 */
#define	AdcBSetThreshold(cmp) ADCB.CMP = cmp;

void AdcAInit(xlib_core_adc_reference ref);
void AdcBInit(xlib_core_adc_reference ref);

void AdcPipeSetSource(ADC_CH_t * pipe, xlib_core_adc_source mux);
void AdcPipeSetDiffSource(ADC_CH_t * pipe, xlib_core_adc_gain gain, xlib_core_adc_source s1, xlib_core_adc_source s2);
void AdcPipeSetInterrupt(ADC_CH_t * pipe, xlib_core_adc_interrupt interrupt);
void AdcPipeStart(ADC_CH_t * pipe);
uint16_t AdcPipeValue(ADC_CH_t * pipe);
uint16_t AdcPipeRead(ADC_CH_t * pipe, xlib_core_adc_source mux);

bool AdcPipeReady(ADC_CH_t * pipe);

#endif /* ADC_H_ */

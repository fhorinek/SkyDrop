#include "clock.h"

/**
 * System clock frequency
 */
uint32_t freq_sys = 2000000UL;
/**
 * CPU (Peripheral x1) clock frequency
 */
uint32_t freq_cpu = 2000000UL;
/**
 * Peripheral x2 clock frequency
 */
uint32_t freq_per2 = 2000000UL;
/**
 * Peripheral x4 System clock frequency
 */
uint32_t freq_per4 = 2000000UL;


/**
 * Enable, stabilize and set clock source as system clock
 *
 *\param source Selected clock source
 */
void ClockSetSource(xlib_core_clock_source source)
{
	switch (source)
	{
		case(x32kHz_int):
			OSC.CTRL |= OSC_RC32KEN_bm;					//enable source
			while (!(OSC.STATUS & OSC_RC32KRDY_bm));	//stabilize
			CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_RC32K_gc);//set clk source
			freq_sys = 32768UL;
			break;
		case(x2MHz):
			OSC.CTRL |= OSC_RC2MEN_bm;
			while (!(OSC.STATUS & OSC_RC2MRDY_bm));
			CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_RC2M_gc);
			freq_sys = 2000000UL;
			break;
		case(x32MHz):
			OSC.CTRL |= OSC_RC32MEN_bm;
			while (!(OSC.STATUS & OSC_RC32MRDY_bm));
			CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_RC32M_gc);
			freq_sys = 32000000UL;
			break;
		case(x32kHz_ext):
			OSC.XOSCCTRL = OSC_XOSCSEL_32KHz_gc;// LP mode -> | OSC_X32KLPM_bm;
			OSC.CTRL |= OSC_XOSCEN_bm;
			while (!(OSC.STATUS & OSC_XOSCRDY_bm));
			CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_XOSC_gc);
			freq_sys = 32768UL;
			break;
		case(x16MHz):
			OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc; //set xtal opt
			OSC.CTRL |= OSC_XOSCEN_bm;
			while (!(OSC.STATUS & OSC_XOSCRDY_bm));
			CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_XOSC_gc);
			freq_sys = 16000000UL;
			break;
	}

	freq_per4 = freq_sys;
	freq_per2 = freq_sys;
	freq_cpu = freq_sys;
}

/**
 * Enable, stabilize and set PLL as system clock
 *
 * \param source Clock source for PLL
 * \param multi PLL multiplication factor (1-31)
 *
 * \f$ f_{out} = f_{clk} * multi \f$
 *
 * where \f$ f_{out} \f$ is output frequency, \f$ f_{clk} \f$ is frequency of selected clock source and \f$ multi \f$ is multiplication factor
 *
 * maximum output frequency of PLL is 10MHz and maximal is 200MHz
 *
 * \note if you use internal 32MHz RC as clock source it will be divided by 4 (\f$ f_{clk} = 8MHz \f$)
 * \note maximum cpu clock frequency is 32MHz see datasheet for ATxmega128a3 device
 */
void ClockSetPLL(xlib_core_clock_source source, uint8_t multi)
{
	uint8_t f;

	switch (source)
	{
		case(x2MHz):
			OSC.CTRL |= OSC_RC2MEN_bm;
			while (!(OSC.STATUS & OSC_RC2MRDY_bm));
			OSC.PLLCTRL = OSC_PLLSRC_RC2M_gc | (0x1F & multi);
			f = 2;
			break;
		case(x32MHz):
			OSC.CTRL |= OSC_RC32MEN_bm;
			while (!(OSC.STATUS & OSC_RC32MRDY_bm));
			OSC.PLLCTRL = OSC_PLLSRC_RC32M_gc | (0x1F & multi);
			f = 8;
			break;
		case(x16MHz):
			OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc; //set xtal opt
			OSC.CTRL |= OSC_XOSCEN_bm;
			while (!(OSC.STATUS & OSC_XOSCRDY_bm));
			OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | (0x1F & multi);
			f = 16;
			break;
		default:
			return;
	}
	//min = 10MHz max f = 200MHz max multi 31
	if ((f * multi < 10) || (f * multi > 200) || (multi > 31))
		return;

	//enable pll
	OSC.CTRL |= OSC_PLLEN_bm;
	//wait to stabilize
	while (!(OSC.STATUS & OSC_PLLRDY_bm));
	//set pll as source
	CCPIOWrite(&CLK.CTRL, CLK_SCLKSEL_PLL_gc);
	freq_sys = (uint32_t)((uint32_t)f * (uint32_t)multi * (uint32_t)1000000);

	freq_per4 = freq_sys;
	freq_per2 = freq_sys;
	freq_cpu = freq_sys;
}

/**
 * Set prescaler A
 *
 * \param pres Prescaler setting
 *
 * \note See ATxmega123a3 device datasheet for prescaler configuration (7.5)
 */
void ClockSetPrescaler(xlib_core_clock_prescaler_a pres)
{
	ClockSetPrescaler(pres, clk_div_b1_c1);
}

/**
 * Set prescaler A,B and C
 *
 * \param pres_a Prescaler group A setting
 * \param pres_b_c Prescaler group B setting
 *
 * \note See ATxmega123a3 device datasheet for prescaler configuration (7.5)
 */
void ClockSetPrescaler(xlib_core_clock_prescaler_a pres_a, xlib_core_clock_prescaler_b_c pres_bc)
{
//	CLK.PSCTRL = pres_a | (0x83 & CLK.PSCTRL);

	CCPIOWrite(&CLK.PSCTRL, pres_a | (0x83 & CLK.PSCTRL));

	switch (pres_a)
	{
		case (clk_nodiv):
			freq_per4 = freq_sys;
			break;
		case (clk_div2):
			freq_per4 = freq_sys / 2;
			break;
		case (clk_div4):
			freq_per4 = freq_sys / 4;
			break;
		case (clk_div8):
			freq_per4 = freq_sys / 8;
			break;
		case (clk_div16):
			freq_per4 = freq_sys / 16;
			break;
		case (clk_div32):
			freq_per4 = freq_sys / 32;
			break;
		case (clk_div64):
			freq_per4 = freq_sys / 64;
			break;
		case (clk_div128):
			freq_per4 = freq_sys / 128;
			break;
		case (clk_div256):
			freq_per4 = freq_sys / 256;
			break;
		case (clk_div512):
			freq_per4 = freq_sys / 512;
			break;
	}

//TODO: BUG register protection datasheet p. 80

	CCPIOWrite(&CLK.PSCTRL, pres_bc | (0xFC & CLK.PSCTRL));
//	CLK.PSCTRL = pres_bc | (0xFC & CLK.PSCTRL);

	switch (pres_bc)
	{
		case(clk_div_b1_c1):
			freq_per2 = freq_per4;
			freq_cpu = freq_per2;
			break;
		case(clk_div_b1_c2):
			freq_per2 = freq_per4;
			freq_cpu = freq_per2 / 2;
			break;
		case(clk_div_b4_c1):
			freq_per2 = freq_per4 / 4;
			freq_cpu = freq_per2;
			break;
		case(clk_div_b2_c2):
			freq_per2 = freq_per4 / 2;
			freq_cpu = freq_per2 / 2;
			break;
	}
}

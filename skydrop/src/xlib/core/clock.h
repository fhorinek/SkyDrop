#ifndef CLOCK_H_
#define CLOCK_H_

#include "../common.h"

/**
 * Available clock sources
 */
typedef enum xlib_core_clock_source_e
{
	x32kHz_int,	//!< 32kHz internal RC oscillator
	x2MHz,		//!< 2Mhz internal RC oscillator
	x32MHz,		//!< 32MHz internal RC oscillator
	x32kHz_ext,	//!< 32kHz external clock crystal
	x16MHz		//!< 16MHz external system clock crystal
} xlib_core_clock_source;

/**
 *
 */
typedef enum xlib_core_clock_prescaler_a_e
{
	clk_nodiv = CLK_PSADIV_1_gc,	//!< No prescaling
	clk_div2 = CLK_PSADIV_2_gc,		//!< divide system clock by 2
	clk_div4 = CLK_PSADIV_4_gc,		//!< divide system clock by 4
	clk_div8 = CLK_PSADIV_8_gc,		//!< divide system clock by 8
	clk_div16 = CLK_PSADIV_16_gc,	//!< divide system clock by 16
	clk_div32 = CLK_PSADIV_32_gc,	//!< divide system clock by 32
	clk_div64 = CLK_PSADIV_64_gc,	//!< divide system clock by 64
	clk_div128 = CLK_PSADIV_128_gc,	//!< divide system clock by 128
	clk_div256 = CLK_PSADIV_256_gc,	//!< divide system clock by 256
	clk_div512 = CLK_PSADIV_512_gc	//!< divide system clock by 512
} xlib_core_clock_prescaler_a;

//sys clk -> prescaler A -> prescaler B -> prescaler C -> CPU, per
//				-> per4			-> pre2

typedef enum xlib_core_clock_prescaler_b_c_e
{
	clk_div_b1_c1 = CLK_PSBCDIV_1_1_gc, //!< No prescaling
	clk_div_b1_c2 = CLK_PSBCDIV_1_2_gc, //!< \f$ freq\_per4 = 1*freq\_per2 = 2*freq\_cpu \f$
	clk_div_b4_c1 = CLK_PSBCDIV_4_1_gc, //!< \f$ freq\_per4 = 4*freq\_per2 = 4*freq\_cpu \f$
	clk_div_b2_c2 = CLK_PSBCDIV_2_2_gc, //!< \f$ freq\_per4 = 2*freq\_per2 = 4*'freq\_cpu' \f$
} xlib_core_clock_prescaler_b_c;

void ClockSetSource(xlib_core_clock_source source);
void ClockSetPLL(xlib_core_clock_source source, uint8_t multi);
void ClockSetPrescaler(xlib_core_clock_prescaler_a pres);
void ClockSetPrescaler(xlib_core_clock_prescaler_a pres_a, xlib_core_clock_prescaler_b_c pres_bc);

#endif /* CLOCK_H_ */

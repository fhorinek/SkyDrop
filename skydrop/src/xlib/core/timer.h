#ifndef TIMER_H_
#define TIMER_H_

#include "../common.h"
#include "gpio.h"

#define timer0	&TCC0,	&PORTC
#define timer1	&TCD0,	&PORTD
#define timer2	&TCE0,	&PORTE
#define timer3	&TCF0,	&PORTF

#define timer4	&TCC1,	&PORTC
#define timer5	&TCD1,	&PORTD
#define timer6	&TCE1,	&PORTE

#define timerc0		&TCC0,	&PORTC
#define timerc1		&TCC1,	&PORTC
#define timerd0		&TCD0,	&PORTD
#define timerd1		&TCD1,	&PORTD
#define timere0		&TCE0,	&PORTE
#define timere1		&TCE1,	&PORTE
#define timerf0		&TCF0,	&PORTF

#define rgb_timer	timer1

#define timer_A	0x10
#define timer_B	0x20
#define timer_C	0x40
#define timer_D	0x80

#define timer0_overflow_interrupt	TCC0_OVF_vect
#define timer0_error_interrupt		TCC0_ERR_vect
#define timer0_compareA_interrupt	TCC0_CCA_vect
#define timer0_compareB_interrupt	TCC0_CCB_vect
#define timer0_compareC_interrupt	TCC0_CCC_vect
#define timer0_compareD_interrupt	TCC0_CCD_vect

#define timer1_overflow_interrupt	TCD0_OVF_vect
#define timer1_error_interrupt		TCD0_ERR_vect
#define timer1_compareA_interrupt	TCD0_CCA_vect
#define timer1_compareB_interrupt	TCD0_CCB_vect
#define timer1_compareC_interrupt	TCD0_CCC_vect
#define timer1_compareD_interrupt	TCD0_CCD_vect

#define timer2_overflow_interrupt	TCE0_OVF_vect
#define timer2_error_interrupt		TCE0_ERR_vect
#define timer2_compareA_interrupt	TCE0_CCA_vect
#define timer2_compareB_interrupt	TCE0_CCB_vect
#define timer2_compareC_interrupt	TCE0_CCC_vect
#define timer2_compareD_interrupt	TCE0_CCD_vect

#define timer3_overflow_interrupt	TCF0_OVF_vect
#define timer3_error_interrupt		TCF0_ERR_vect
#define timer3_compareA_interrupt	TCF0_CCA_vect
#define timer3_compareB_interrupt	TCF0_CCB_vect
#define timer3_compareC_interrupt	TCF0_CCC_vect
#define timer3_compareD_interrupt	TCF0_CCD_vect

#define timer4_overflow_interrupt	TCC1_OVF_vect
#define timer4_error_interrupt		TCC1_ERR_vect
#define timer4_compareA_interrupt	TCC1_CCA_vect
#define timer4_compareB_interrupt	TCC1_CCB_vect

#define timer5_overflow_interrupt	TCD1_OVF_vect
#define timer5_error_interrupt		TCD1_ERR_vect
#define timer5_compareA_interrupt	TCD1_CCA_vect
#define timer5_compareB_interrupt	TCD1_CCB_vect

#define timer6_overflow_interrupt	TCE1_OVF_vect
#define timer6_error_interrupt		TCE1_ERR_vect
#define timer6_compareA_interrupt	TCE1_CCA_vect
#define timer6_compareB_interrupt	TCE1_CCB_vect

#define timerc0_overflow_interrupt	TCC0_OVF_vect
#define timerc0_error_interrupt		TCC0_ERR_vect
#define timerc0_compareA_interrupt	TCC0_CCA_vect
#define timerc0_compareB_interrupt	TCC0_CCB_vect
#define timerc0_compareC_interrupt	TCC0_CCC_vect
#define timerc0_compareD_interrupt	TCC0_CCD_vect

#define timerd0_overflow_interrupt	TCD0_OVF_vect
#define timerd0_error_interrupt		TCD0_ERR_vect
#define timerd0_compareA_interrupt	TCD0_CCA_vect
#define timerd0_compareB_interrupt	TCD0_CCB_vect
#define timerd0_compareC_interrupt	TCD0_CCC_vect
#define timerd0_compareD_interrupt	TCD0_CCD_vect

#define timere0_overflow_interrupt	TCE0_OVF_vect
#define timere0_error_interrupt		TCE0_ERR_vect
#define timere0_compareA_interrupt	TCE0_CCA_vect
#define timere0_compareB_interrupt	TCE0_CCB_vect
#define timere0_compareC_interrupt	TCE0_CCC_vect
#define timere0_compareD_interrupt	TCE0_CCD_vect

#define timerf0_overflow_interrupt	TCF0_OVF_vect
#define timerf0_error_interrupt		TCF0_ERR_vect
#define timerf0_compareA_interrupt	TCF0_CCA_vect
#define timerf0_compareB_interrupt	TCF0_CCB_vect
#define timerf0_compareC_interrupt	TCF0_CCC_vect
#define timerf0_compareD_interrupt	TCF0_CCD_vect

#define timerc1_overflow_interrupt	TCC1_OVF_vect
#define timerc1_error_interrupt		TCC1_ERR_vect
#define timerc1_compareA_interrupt	TCC1_CCA_vect
#define timerc1_compareB_interrupt	TCC1_CCB_vect

#define timerd1_overflow_interrupt	TCD1_OVF_vect
#define timerd1_error_interrupt		TCD1_ERR_vect
#define timerd1_compareA_interrupt	TCD1_CCA_vect
#define timerd1_compareB_interrupt	TCD1_CCB_vect

#define timere1_overflow_interrupt	TCE1_OVF_vect
#define timere1_error_interrupt		TCE1_ERR_vect
#define timere1_compareA_interrupt	TCE1_CCA_vect
#define timere1_compareB_interrupt	TCE1_CCB_vect


typedef enum xlib_core_timer_prescaler_e
{
	timer_div1 		= 0b0001,
	timer_div2 		= 0b0010,
	timer_div4 		= 0b0011,
	timer_div8 		= 0b0100,
	timer_div64		= 0b0101,
	timer_div256	= 0b0110,
	timer_div1024	= 0b0111,

	timer_event0	= 0b1000,
	timer_event1	= 0b1001,
	timer_event2	= 0b1010,
	timer_event3	= 0b1011,
	timer_event4	= 0b1100,
	timer_event5	= 0b1101,
	timer_event6	= 0b1110,
	timer_event7	= 0b1111,
} xlib_core_timer_prescaler;

typedef enum xlib_core_timer_mode_e
{
	timer_normal		= 0b000,
	timer_freq			= 0b001,
	timer_pwm			= 0b011,
	timer_ds_pwm_t		= 0b101,
	timer_ds_pwm_tb		= 0b110,
	timer_ds_pwm_b		= 0b111
} xlib_core_timer_mode;

typedef enum xlib_core_timer_interrupt_e
{
	timer_compareA = 0b00010000,
	timer_compareB = 0b00100000,
	timer_compareC = 0b01000000,
	timer_compareD = 0b10000000,
	timer_overflow = 0b00000001,
	timer_error	   = 0b00000010
} xlib_core_timer_interrupt;


class Timer
{
private:
	TC0_t * tc0;
	TC1_t * tc1;


	PORT_t * port;
	uint16_t pres;

	xlib_core_timer_prescaler prescaler;

	void Init(PORT_t * port, xlib_core_timer_prescaler pres);

	uint8_t irq_priority;

public:

	void Init(TC0_t * tc0, PORT_t * port, xlib_core_timer_prescaler pres);
	void Init(TC1_t * tc1, PORT_t * port, xlib_core_timer_prescaler pres);

	void SetMode(xlib_core_timer_mode mode);
	void EnableOutputs(uint8_t flags);
	void DisableOutputs(uint8_t flags);
	void EnableInterrupts(uint8_t inter);
	void DisableInterrupts(uint8_t inter);

	void Start();
	void Stop();
	uint16_t GetValue();
	void SetValue(uint16_t val);

	void SetTop(uint16_t top);
	void SetCompare(uint8_t flag, uint16_t compare);

	void SetPrescaler(xlib_core_timer_prescaler pres);

	void SetInterruptPriority(uint8_t p);

	bool IrqPending(uint8_t inter);
	void ClearIrqFlag(uint8_t inter);

	//TODO: not implemented
	float MsPerFs();
	uint16_t StepsPerMs();

};


#endif /* TIMER_H_ */

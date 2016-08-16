#ifndef GPIO_H_
#define GPIO_H_

#include "../common.h"

//PORTA
#define porta0	&PORTA, 0
#define porta1	&PORTA, 1
#define porta2	&PORTA, 2
#define porta3	&PORTA, 3
#define porta4	&PORTA, 4
#define porta5	&PORTA, 5
#define porta6	&PORTA, 6
#define porta7	&PORTA, 7

//PORTB
#define portb0	&PORTB, 0
#define portb1	&PORTB, 1
#define portb2	&PORTB, 2
#define portb3	&PORTB, 3
#define portb4	&PORTB, 4
#define portb5	&PORTB, 5
#define portb6	&PORTB, 6
#define portb7	&PORTB, 7

//DAC
#define dac0	portb2
#define dac1	portb3

//PORTC 0 - 7
#define portc0	&PORTC, 0
#define portc1	&PORTC, 1
#define portc2	&PORTC, 2
#define portc3	&PORTC, 3
#define portc4	&PORTC, 4
#define portc5	&PORTC, 5
#define portc6	&PORTC, 6
#define portc7	&PORTC, 7

//PORTD 0 - 7
#define portd0	&PORTD, 0
#define portd1	&PORTD, 1
#define portd2	&PORTD, 2
#define portd3	&PORTD, 3
#define portd4	&PORTD, 4
#define portd5	&PORTD, 5
#define portd6	&PORTD, 6
#define portd7	&PORTD, 7

//PORTE 0 - 7
#define porte0	&PORTE, 0
#define porte1	&PORTE, 1
#define porte2	&PORTE, 2
#define porte3	&PORTE, 3
#define porte4	&PORTE, 4
#define porte5	&PORTE, 5
#define porte6	&PORTE, 6
#define porte7	&PORTE, 7

//PORTF 0 - 7
#define portf0	&PORTF, 0
#define portf1	&PORTF, 1
#define portf2	&PORTF, 2
#define portf3	&PORTF, 3
#define portf4	&PORTF, 4
#define portf5	&PORTF, 5
#define portf6	&PORTF, 6
#define portf7	&PORTF, 7

//PORTR 0- 1
#define portr0	&PORTR, 0
#define portr1	&PORTR, 1


//LEDS
#define ledr	&PORTD, 0
#define ledg	&PORTD, 1
#define ledb	&PORTD, 3

//USART0
#define usart0_rx	&PORTE, 2
#define usart0_tx	&PORTE, 3

//USART1
#define usart1_rx	&PORTE, 6
#define usart1_tx	&PORTE, 7

//USART2
#define usart2_rx	&PORTF, 2
#define usart2_tx	&PORTF, 3

//USART3 - PORTC
#define usart3_rx	&PORTC, 2
#define usart3_tx	&PORTC, 3

//USART4 - PWM
#define usart4_rx	&PORTC, 6
#define usart4_tx	&PORTC, 7

//USART5 - SPI
#define usart5_rx	&PORTD, 6
#define usart5_tx	&PORTD, 7

//USART6 - NOT TO BE USED FOR STAX
#define usart6_rx	&PORTD, 2
#define usart6_tx	&PORTD, 3

//SPI
#define spi_ss_usb		&PORTE, 4
#define spi_ss_sdcard	&PORTE, 5
#define spi_ss_user0	&PORTF, 0
#define spi_ss_user1	&PORTF, 1
#define spi_ss_user2	&PORTF, 4
#define spi_ss_user3	&PORTF, 5

#define spi_ss		&PORTD, 4
#define spi_mosi	&PORTD, 5
#define spi_miso	&PORTD, 6
#define spi_sck		&PORTD, 7

//I2C
#define i2c_sda		&PORTE, 0
#define i2c_scl		&PORTE, 1

//MISC
#define user_button	&PORTD, 2
#define bt_reset	&PORTF, 6
#define bt_shdn		&PORTF, 7

//ISR
#define porta_interrupt0	PORTA_INT0_vect
#define porta_interrupt1	PORTA_INT1_vect

#define portb_interrupt0	PORTB_INT0_vect
#define portb_interrupt1	PORTB_INT1_vect

#define portc_interrupt0	PORTC_INT0_vect
#define portc_interrupt1	PORTC_INT1_vect

#define portd_interrupt0	PORTD_INT0_vect
#define portd_interrupt1	PORTD_INT1_vect

#define porte_interrupt0	PORTE_INT0_vect
#define porte_interrupt1	PORTE_INT1_vect

#define portf_interrupt0	PORTF_INT0_vect
#define portf_interrupt1	PORTF_INT1_vect

#define portr_interrupt0	PORTR_INT0_vect
#define portr_interrupt1	PORTR_INT1_vect

//pullup configuration
typedef enum xlib_core_gpio_pull_e
{
	gpio_totem = PORT_OPC_TOTEM_gc,
	gpio_buskeeper = PORT_OPC_BUSKEEPER_gc,
	gpio_pull_down = PORT_OPC_PULLDOWN_gc,
	gpio_pull_up = PORT_OPC_PULLUP_gc,
	gpio_wired_or = PORT_OPC_WIREDOR_gc,
	gpio_wired_and = PORT_OPC_WIREDAND_gc,
	gpio_wired_or_pull = PORT_OPC_WIREDORPULL_gc,
	gpio_wired_and_pull = PORT_OPC_WIREDANDPULL_gc
} xlib_core_gpio_pull;

//interrupts
typedef enum xlib_core_gpio_int_e
{
	gpio_bothedges = PORT_ISC_BOTHEDGES_gc,
	gpio_rising = PORT_ISC_RISING_gc,
	gpio_falling = PORT_ISC_FALLING_gc,
	gpio_level = PORT_ISC_LEVEL_gc,
	gpio_input_disable = PORT_ISC_INPUT_DISABLE_gc
} xlib_core_gpio_int;

typedef enum xlib_core_gpio_intmask_e
{
	gpio_clear,
	gpio_interrupt0,
	gpio_interrupt1
} xlib_core_gpio_intmask;

void GpioWrite(PORT_t * port, uint8_t pin, uint8_t set);
uint8_t GpioRead(PORT_t * port, uint8_t pin);
void GpioSetPull(PORT_t * port, uint8_t pin, xlib_core_gpio_pull pull);
void GpioSetInvert(PORT_t * port, uint8_t pin, uint8_t inv);
void GpioSetDirection(PORT_t * port, uint8_t pin, uint8_t dir);
void GpioSetInterrupt(PORT_t * port, uint8_t pin, xlib_core_gpio_intmask mask, xlib_core_gpio_int interrupt);
void GpioSetInterrupt(PORT_t * port, uint8_t pin, xlib_core_gpio_intmask mask);


#endif /* GPIO_H_ */

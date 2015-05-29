/*
 * common.h
 *
 *  Created on: 12.9.2014
 *      Author: horinek
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <xlib/core/clock.h>
#include <xlib/core/usart.h>
#include <xlib/core/spi.h>
#include <xlib/core/system.h>
#include <xlib/core/rtc.h>
#include <xlib/core/timer.h>
#include <xlib/core/adc.h>
#include <xlib/core/dac.h>
#include <xlib/core/i2c.h>

#include "build_defs.h"


union byte4
{
	int32_t uint32;
	uint8_t uint8[4];
};

union byte2
{
	uint16_t uint16;
	int16_t int16;
	uint8_t uint8[2];
};

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t z;
} vector_i16_t;

typedef struct
{
	float x;
	float y;
	float z;
} vector_float_t;

#define APP_INFO_EE_offset	32
#define APP_INFO_TEST_hex	0xAA
#define APP_INFO_NAME_len	(APP_INFO_EE_offset - 1)

struct app_info
{
	uint8_t app_name[APP_INFO_NAME_len];
	uint8_t test_pass;
};

extern struct app_info fw_info;
extern struct app_info ee_fw_info __attribute__ ((section(".fw_info")));



//PINOUT
//---------------- GENERAL -------------------
#define USB_PWR_ON				PR.PRGEN &= 0b10111111;
#define USB_PWR_OFF				PR.PRGEN |= 0b01000000;
#define RTC_PWR_ON				PR.PRGEN &= 0b11111011;
#define RTC_PWR_OFF				PR.PRGEN |= 0b00000100;

//---------------- PORTA ---------------------
#define GPS_EN					porta0
#define SWITCH3					porta1
#define SWITCH2					porta2
#define SWITCH1					porta3
#define GPS_TIMER				porta4
#define GPS_RESET				porta5
#define BAT_EN					porta6
#define BAT_SNS					porta7

#define SWITCH_INT				porta_interrupt0
#define GPS_TIMER_INT			porta_interrupt1

#define BAT_SNS_ADC				ext_porta7

#define BATTERY_ADC_PWR_ON		PR.PRPA &= 0b11111101;
#define BATTERY_ADC_PWR_OFF		PR.PRPA |= 0b00000010;
#define BATTERY_ADC_DISABLE		ADCA.CTRLA = 0;
#define BATTERY_ADC_ENABLE		ADCA.CTRLA = ADC_ENABLE_bm;

//---------------- PORTB ---------------------

#define SD_IN					portb0
#define IO0						portb1
#define USB_IN					portb2
#define DAC						portb3
#define BT_EN					portb4
#define BT_RESET				portb5
#define BT_RTS					portb6 //CTS from BT
#define BT_CTS					portb7 //RTS from BT

#define IO0_ADC					ext_portb1

#define BUZZER_VOL_DAC			dac_ch1
#define BUZZER_VOL_DAC_PWR_ON	PR.PRPB &= 0b11111011;
#define BUZZER_VOL_DAC_PWR_OFF	PR.PRPB |= 0b00000100;


#define IO_ADC_PWR_ON			PR.PRPB &= 0b11111101;
#define IO_ADC_PWR_OFF			PR.PRPB |= 0b00000010;
#define IO_ADC_DISABLE			ADCB.CTRLA = 0;
#define IO_ADC_ENABLE			ADCB.CTRLA = ADC_ENABLE_bm;

#define USB_CONNECTED			(GpioRead(USB_IN) == HIGH)
#define USB_CONNECTED_IRQ		portb_interrupt0
#define USB_CONNECTED_IRQ_ON	GpioSetInterrupt(USB_IN, gpio_interrupt0, gpio_bothedges);
#define USB_CONNECTED_IRQ_OFF	GpioSetInterrupt(USB_IN, gpio_clear);

#define BT_CTS_PIN_INT			portb_interrupt1

#define DAC_PWR_ON				PR.PRPB &= 0b11111011;
#define DDC_PWR_OFF				PR.PRPB |= 0b00000100;

//---------------- PORTC ---------------------
#define LCD_RST					portc0
#define LCD_CE					portc1
#define DEBUG_RXD				portc2
#define DEBUG_TXD				portc3
#define LCD_DC					portc4
#define LCD_DIN					portc5
#define CHARGING				portc6
#define LCD_CLK					portc7

#define DEBUG_UART				usartc0
#define DEBUG_UART_PWR_ON		PR.PRPC &= 0b11101111;
#define DEBUG_UART_PWR_OFF		PR.PRPC |= 0b00010000;

#define LCD_SPI					spic
#define LCD_SPI_PWR_ON			PR.PRPC &= 0b11110111;
#define LCD_SPI_PWR_OFF			PR.PRPC |= 0b00001000;

#define FC_MEAS_TIMER			timerc0
#define FC_MEAS_TIMER_OVF		timerc0_overflow_interrupt
#define FC_MEAS_TIMER_CMPA		timerc0_compareA_interrupt
#define FC_MEAS_TIMER_CMPB		timerc0_compareB_interrupt
#define FC_MEAS_TIMER_CMPC		timerc0_compareC_interrupt
#define FC_MEAS_TIMER_CMPD		timerc0_compareD_interrupt
#define FC_MEAS_TIMER_PWR_OFF	PR.PRPC |= 0b00000001
#define FC_MEAS_TIMER_PWR_ON	PR.PRPC &= 0b11111110


#define TASK_TIMER				timerc1
#define TASK_TIMER_OVF			timerc1_overflow_interrupt
#define TASK_TIMER_PWR_OFF		PR.PRPC |= 0b00000010
#define TASK_TIMER_PWR_ON		PR.PRPC &= 0b11111101



//---------------- PORTD ---------------------
#define LCD_VCC					portd0
#define IO1						portd1
#define BT_RXD					portd2
#define BT_TXT					portd3
#define LEDG					portd4
#define LEDR					portd5
//USB_N							portd6
//USB_P							portd7

#define BT_UART					usartd0
#define BT_UART_PWR_ON			PR.PRPD &= 0b11101111;
#define BT_UART_PWR_OFF			PR.PRPD |= 0b00010000;

//XXX: timerd0 should left unused so user can generate pwm.

#define LED_TIMER1				timerd1
#define LED_TIMER1_PWR_ON		PR.PRPD	&= 0b11111101;
#define LED_TIMER1_PWR_OFF		PR.PRPD	|= 0b00000010;
#define LED_TIMER1_OVF			timerd1_overflow_interrupt

//---------------- PORTE ---------------------
//SDA							porte0
//SCL							porte1
#define GPS_RX					porte2
#define GPS_TX					porte3
#define LEDB					porte4
#define BCKL					porte5
//X2 RTC TOSC2					porte6
//X1 RTC TOSC1					porte7

#define MEMS_I2C				i2ce
#define MEMS_I2C_PWR_ON			PR.PRPE &= 0b10111111
#define MEMS_I2C_PWR_OFF		PR.PRPE |= 0b01000000

#define GPS_UART				usarte0
#define GPS_UART_PWR_ON			PR.PRPE &= 0b11101111;
#define GPS_UART_PWR_OFF		PR.PRPE |= 0b00010000;

#define AUDIO_TIMER				timere0
#define AUDIO_TIMER_OVF			timere0_overflow_interrupt
#define AUDIO_TIMER_CMPA		timere0_compareA_interrupt
#define AUDIO_TIMER_CMPB		timere0_compareB_interrupt
#define AUDIO_TIMER_CMPC		timere0_compareC_interrupt
#define AUDIO_TIMER_CMPD		timere0_compareD_interrupt
#define AUDIO_TIMER_PWR_OFF		PR.PRPE |= 0b00000001
#define AUDIO_TIMER_PWR_ON		PR.PRPE &= 0b11111110

#define LED_TIMER2				timere1
#define LED_TIMER2_PWR_ON		PR.PRPE	&= 0b11111101;
#define LED_TIMER2_PWR_OFF		PR.PRPE	|= 0b00000010;
#define LED_TIMER2_OVF			timere1_overflow_interrupt


//---------------- PORTF ---------------------
#define BUZZ					portf0
//SD_SCK						portf1
//SD_MISO						portf2
//SD_MOSI						portf3
#define SD_EN					portf4
#define I2C_EN					portf5
#define MEMS_EN					portf6
#define AM_INT2					portf7

#define AM_INT2_INT				portf_interrupt0

#define SD_SPI					usartf0
#define SD_SPI_PWR_ON			PR.PRPF &= 0b11101111;
#define SD_SPI_PWR_OFF			PR.PRPF |= 0b00010000;

#define SD_EN_ON				GpioWrite(SD_EN, HIGH);
#define SD_EN_OFF				GpioWrite(SD_EN, LOW);
#define SD_EN_INIT				GpioSetDirection(SD_EN, OUTPUT); \
								SD_EN_OFF;

#define I2C_POWER_ON			GpioWrite(I2C_EN, HIGH);
#define I2C_POWER_OFF			GpioWrite(I2C_EN, LOW);
#define I2C_POWER_INIT			GpioSetDirection(I2C_EN, OUTPUT); \
								I2C_POWER_OFF;

#define MEMS_POWER_ON			GpioWrite(MEMS_EN, HIGH);
#define MEMS_POWER_OFF			GpioWrite(MEMS_EN, LOW);
#define MEMS_POWER_INIT			GpioSetDirection(MEMS_EN, OUTPUT); \
								MEMS_POWER_OFF;


#define BUZZER_TIMER			timerf0
#define BUZZER_TIMER_OVF		timerf0_overflow_interrupt
#define BUZZER_TIMER_PWR_OFF	PR.PRPF |= 0b00000001
#define BUZZER_TIMER_PWR_ON		PR.PRPF &= 0b11111110

//---------------- PORTR ---------------------
#define AM_INT1					portr0
#define SD_SS					portr1

#define AM_INT1_INT				portr_interrupt0

#define BUILD_VER	"%02d%02d%02d-%02d%02d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN

//--------------------------------------------

#define IO1_INIT				GpioSetDirection(IO1, OUTPUT);
#define IO1_HIGH				GpioWrite(IO1, HIGH);
#define IO1_LOW					GpioWrite(IO1, LOW);

#define IO0_INIT				GpioSetDirection(IO0, OUTPUT);
#define IO0_HIGH				GpioWrite(IO0, HIGH);
#define IO0_LOW					GpioWrite(IO0, LOW);

//#define IO0_INIT
//#define IO0_HIGH
//#define IO0_LOW



class DataBuffer
{
public:
	uint8_t * data;

	uint16_t size;
	uint16_t length;
	uint16_t write_index;
	uint16_t read_index;

	DataBuffer(uint16_t size);
	~DataBuffer();

	uint16_t Read(uint16_t len, uint8_t * * data);
	bool Write(uint16_t len, uint8_t * data);

	uint16_t Length();
	void Clear();
};

//--------------------------------------------

void print_cpu_usage();
void test_memory();
bool cmpn(char * s1, const char * s2, uint8_t n);
bool cmpn_p(char * s1, const char * s2, uint8_t n);
void print_fw_info();
int freeRam();
void LoadEEPROM();

void turnoff_subsystems();

void bat_en_high(uint8_t mask);
void bat_en_low(uint8_t mask);


#endif /* COMMON_H_ */

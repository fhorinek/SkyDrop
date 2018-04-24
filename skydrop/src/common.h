/*
 * common.h
 *
 *  Created on: 12.9.2014
 *      Author: horinek
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <math.h>

#include <xlib/core/clock.h>
#include <xlib/core/usart.h>
#include <xlib/core/spi.h>
#include <xlib/core/system.h>
#include <xlib/core/rtc.h>
#include <xlib/core/timer.h>
#include <xlib/core/adc.h>
#include <xlib/core/dac.h>
#include <xlib/core/i2c.h>

#include <xlib/ring.h>
#include <xlib/stream.h>

#include "build_defs.h"
#include "build_number.h"
#include "debug.h"

union byte8
{
	uint64_t uint64;
	uint32_t uint32[2];
	uint8_t uint8[4];
};


union byte4
{
	uint32_t uint32;
	int32_t int32;
	uint8_t uint8[4];
};

union byte2
{
	uint16_t uint16;
	int16_t int16;
	uint8_t uint8[2];
};

struct vector_i16_t
{
	int16_t x;
	int16_t y;
	int16_t z;
};

struct vector_i32_t
{
	int32_t x;
	int32_t y;
	int32_t z;
};

struct vector_float_t
{
	float x;
	float y;
	float z;
};

#define APP_INFO_EE_offset	32
#define APP_INFO_NAME_len	(APP_INFO_EE_offset - 1)

struct app_info
{
	uint8_t app_name[APP_INFO_NAME_len];
	uint8_t reserved;
};

extern struct app_info fw_info;
extern struct app_info ee_fw_info __attribute__ ((section(".fw_info")));

//PINOUT
//---------------- GENERAL -------------------
#define USB_PWR_ON				PR.PRGEN &= 0b10111111;
#define USB_PWR_OFF				PR.PRGEN |= 0b01000000;
#define RTC_PWR_ON				PR.PRGEN &= 0b11111011;
#define RTC_PWR_OFF				PR.PRGEN |= 0b00000100;

#define DMA_PWR_ON   			PR.PRGEN &= 0b11111110; \
								DMA.CTRL |= DMA_ENABLE_bm;

#define DMA_PWR_OFF   			DMA.CTRL &= ~DMA_ENABLE_bm; \
								PR.PRGEN |= 0b00000001;

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

//suppress and allow uart communication from bt module
#define BT_SUPRESS_TX			GpioWrite(BT_RTS, bt_module_state != BT_MOD_STATE_OFF);
#define BT_ALLOW_TX				GpioWrite(BT_RTS, LOW);

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

#define BAT_CHARGING			(GpioRead(CHARGING) == LOW)
#define BAT_FULL				(GpioRead(CHARGING) == HIGH)

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
#define TASK_TIMER_CMPA			timerc1_compareA_interrupt
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
#define BT_UART_DMA_CH			&DMA.CH1
#define BT_UART_DMA_TRIGGER		DMA_CH_TRIGSRC_USARTD0_RXC_gc

//XXX: timerd0 should left unused so user can generate pwm.
#define DEBUG_TIMER				timerd0
#define DEBUG_TIMER_PWR_ON		PR.PRPD	&= 0b11111110;
#define DEBUG_TIMER_PWR_OFF		PR.PRPD	|= 0b00000001;
#define DEBUG_TIMER_OVF			timerd0_overflow_interrupt

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
#define GPS_UART_DMA_CH			&DMA.CH0
#define GPS_UART_DMA_TRIGGER	DMA_CH_TRIGSRC_USARTE0_RXC_gc

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
#define IO4						portf5
#define MEMS_EN					portf6
#define IO3						portf7

#define SD_SPI					usartf0
#define SD_SPI_PWR_ON			PR.PRPF &= 0b11101111;
#define SD_SPI_PWR_OFF			PR.PRPF |= 0b00010000;

#define SD_EN_ON				GpioWrite(SD_EN, HIGH);
#define SD_EN_OFF				GpioWrite(SD_EN, LOW);
#define SD_EN_INIT				GpioSetDirection(SD_EN, OUTPUT); \
								SD_EN_OFF;

#define BUZZER_TIMER			timerf0
#define BUZZER_TIMER_OVF		timerf0_overflow_interrupt
#define BUZZER_TIMER_PWR_OFF	PR.PRPF |= 0b00000001
#define BUZZER_TIMER_PWR_ON		PR.PRPF &= 0b11111110

//---------------- PORTR ---------------------
#define IO2						portr0
#define SD_SS					portr1

#define BUILD_VER	"%02d%02d%02d-%02d%02d", BUILD_YEAR, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN

//--------------------------------------------

//revision specific pins
#define HW_REW_1504		0
#define HW_REW_1506		1
#define HW_REW_1512		2
#define HW_REW_UNKNOWN	0xFF

#define REV_1504_MEMS_EN_2		portb1
#define REV_1504_I2C_EN			portf5

//bat_en
#define BAT_EN_ADC	(1 << 0)
#define BAT_EN_LED	(1 << 1)
#define BAT_EN_LCD	(1 << 2)

class DataBuffer
{
public:
	uint8_t * data;

	uint16_t size;
	uint16_t length;
	uint16_t write_index;
	uint16_t read_index;

	DataBuffer(uint16_t size, uint8_t * buffer);
	~DataBuffer();

	uint16_t Read(uint16_t len, uint8_t * * data);
	bool Write(uint16_t len, uint8_t * data);

	uint16_t Length();
	void Clear();
};

//--------------------------------------------

//memory
void test_memory();
int freeRam();

//common
bool cmpn(char * s1, const char * s2, uint8_t n);
bool cmpn_p(char * s1, const char * s2, uint8_t n);
uint8_t fast_flip(uint8_t in);

float mul_to_sec(float mul);
float sec_to_mul(float sec);
uint8_t hex_to_num(uint8_t c);

uint32_t pow_ten(uint8_t pow);

uint32_t atoi_n(char * str, uint8_t n);
uint8_t atoi_c(char * str);
float atoi_f(char * str);

/**
 * Returns a pointer to the last occurrence of the character c in the string s.
 *
 * @param s the string to look through
 * @param c the character to search
 *
 * @return pointer to found character or NULL if not found.
 */
char *index(char * s, char c);

/**
 * Returns a pointer to the last occurrence of the character c in the string s.
 *
 * @param s the string to look through
 * @param c the character to search
 *
 * @return pointer to found character or NULL if not found.
 */
char *rindex(char * s, char c);

char * find_comma(char * str);
uint8_t nmea_checksum(char *s);


//settings
bool LoadEEPROM();
bool StoreEEPROM();

//system info
void print_fw_info();
void print_reset_reason();
extern uint8_t hw_revision;
extern uint8_t device_id[11];
void GetID(); //11 b
void GetID_str(char * id); //22 b

//power
void mems_power_init();
void mems_power_on();
void mems_power_off();

void turnoff_subsystems();

void bat_en_high(uint8_t mask);
void bat_en_low(uint8_t mask);

//GPIO
void io_init();
void io_write(uint8_t io, uint8_t level);

/**
 * Convert an angle given in degree to radians.
 */
inline double to_radians(double degree) {
    return degree / 180.0 * M_PI;
}

/**
 * Convert an angle given in radians to degree.
 */
inline double to_degrees(double radians) {
    return radians * (180.0 / M_PI);
}

/*
 * 0123456789 ISDIGIT
 * ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ISALPHA
 * 0123456789ABCDEF ISCAPITALHEX
 * 0123456789ABCDEFabcdef ISXDIGIT
 */
#define ISDIGIT(c) ((c) - '0' + 0U <= 9U)
#define ISALPHA(c) (((c) | 32) - 'a' + 0U <= 'z' - 'a' + 0U)
#define ISCAPITALHEX(c) ((((((c) - 48U) & 255) * 23 / 22 + 4) / 7 ^ 1) <= 2U)
#define ISXDIGIT(c) (((((((((c) - 48U) & 255) * 18 / 17 * 52 / 51 * 58 / 114 \
     * 13 / 11 * 14 / 13 * 35 + 35) / 36 * 35 / 33 * 34 / 33 * 35 / 170 ^ 4) \
     - 3) & 255) ^ 1) <= 2U)

#endif /* COMMON_H_ */

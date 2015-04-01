#include "gps_l80.h"
#include "../uart.h"

Usart gps_uart;

CreateStdOut(gps_out, gps_uart.Write);

ISR(GPS_TIMER_INT)
{
	DEBUG("GPS_TIMER_INT\n");
}

#define GPS_IDLE	0
#define GPS_DATA	1
#define GPS_CRC		2
#define GPS_END		3

#define NMEA_MAX_LEN	82

volatile uint8_t gps_parser_state;
char gps_parser_buffer[NMEA_MAX_LEN];
char * gps_parser_ptr;
volatile uint8_t gps_parser_buffer_index = 0;
volatile uint8_t gps_checksum;
volatile uint8_t gps_rx_checksum;

uint8_t hex_to_num(uint8_t c)
{
	if (c >= 'A')
		return c - 'A' + 10;
	else
		return c - '0';
}

uint32_t atoi_n(char * str, uint8_t n)
{
	uint32_t tmp = 0;

	for (uint8_t i = 0; i < n; i++)
	{
		if (str[i] == '.')
		{
			n++;
			continue;
		}

		tmp *= 10;
		tmp += str[i] - '0';
	}

	return tmp;
}

uint8_t atoi_c(char * str)
{
	uint8_t tmp = 0;
	uint8_t i = 0;

	while(str[i] != ',')
	{
		tmp *= 10;
		tmp += str[i] - '0';
		i++;
	}

	return tmp;
}

float atoi_f(char * str)
{
	float tmp = 0;
	uint8_t dot = 0;
	uint8_t i = 0;

	while(str[i] != ',')
	{
		if (str[i] == '.')
		{
			dot = i;
			i++;
		}

		if (dot == 0)
		{
			tmp *= 10;
			tmp += str[i] - '0';
		}
		else
		{
			tmp += (str[i] - '0') / pow(10, i - dot);
		}

		i++;
	}

	return tmp;
}

char * find_comma(char * str)
{
	while ((*str) != ',')
		str++;

	return (str + 1);
}


//132405.000,A,4809.2356,N,01704.4263,E,0.15,317.49,270115,,,A
//hhmmss.XXX,
void gps_parse_rmc()
{
	DEBUG("\nRMC\n");


	char * ptr = find_comma(gps_parser_buffer);

	//UTC time
	uint8_t hour = atoi_n(ptr + 0, 2);
	uint8_t min = atoi_n(ptr + 2, 2);
	uint8_t sec = atoi_n(ptr + 4, 2);

	DEBUG("%02d:%02d:%02d\n", hour, min, sec);

	ptr = find_comma(ptr);

	//Valid (A = valid)
	bool valid = (*ptr) == 'A';

	ptr = find_comma(ptr);

	//Latitude
	float latitude = atoi_n(ptr + 2, 6);
	latitude = atoi_n(ptr + 0, 2) + latitude / 600000;

	ptr = find_comma(ptr);

	//Sign
	if ((*ptr) == 'S')
		latitude *= -1;

	ptr = find_comma(ptr);

	//Longitude
	float longitude = atoi_n(ptr + 3, 6);
	longitude = atoi_n(ptr + 0, 3) + longitude / 600000;

	ptr = find_comma(ptr);

	//Sign
	if ((*ptr) == 'W')
		longitude *= -1;

	ptr = find_comma(ptr);

	DEBUG("lat+lon %0.7f %0.7f\n", latitude, longitude);

	//Ground speed in knots
	float speed = atoi_f(ptr);

	ptr = find_comma(ptr);

	//Ground course
	float course = atoi_f(ptr);

	ptr = find_comma(ptr);

	DEBUG("spd+crs %0.2f %0.2f\n", speed, course);

	//UTC date
	uint8_t day = atoi_n(ptr + 0, 2);
	uint8_t month = atoi_n(ptr + 2, 2);
	uint16_t year = atoi_n(ptr + 4, 2) + 2000;

	DEBUG("%02d.%02d.%04d\n", day, month, year);
}

//132405.000,4809.2356,N,01704.4263,E,1,6,1.95,160.3,M,42.7,M,,
void gps_parse_gga()
{
	DEBUG("\nGGA\n");

	char * ptr = find_comma(gps_parser_buffer);

	//Skip time
	ptr = find_comma(ptr);
	//Skip latitude
	ptr = find_comma(ptr);
	ptr = find_comma(ptr);
	//Skip longitude
	ptr = find_comma(ptr);
	ptr = find_comma(ptr);

	//Fix status
	uint8_t fix = (*ptr) - '0';
	ptr = find_comma(ptr);

	//Number of sat
	uint8_t sat = atoi_c(ptr);
	ptr = find_comma(ptr);

	//HDOP TODO:calc error
	float hdop = atoi_f(ptr);
	ptr = find_comma(ptr);

	//altitude
	float altitude = atoi_f(ptr);
	ptr = find_comma(ptr);
	ptr = find_comma(ptr); //skip M

	//Geo id
	float geo_id = atoi_f(ptr);

	DEBUG("fix %d (%d), hdop: %0.2f\n", fix, sat, hdop);
	DEBUG("alt %0.1fm geo: %0.1fm\n", altitude, geo_id);
}

void gps_parse_hello()
{
	DEBUG("HELLO\n");

	gps_setup();
	gps_change_baud();
}


void gps_parse(Usart * c_uart)
{
	uint8_t c = c_uart->Read();

	DEBUG("%c", c);

	switch (gps_parser_state)
	{
		case(GPS_IDLE):
			if (c == '$')
			{
				gps_parser_buffer_index = 0;
				gps_checksum = 0;
				gps_parser_state = GPS_DATA;
			}
			else
			{
				gps_parser_buffer_index++;
				if (gps_parser_buffer_index > 10)
				{
					gps_setup();
					gps_change_baud();
					gps_setup();
					gps_parser_buffer_index = 0;
				}
			}
		break;

		case(GPS_DATA):
			if (c == '*')
			{
				gps_parser_state = GPS_CRC;
				gps_parser_buffer_index = 0;
			}
			else
			{
				gps_checksum ^= c;
				gps_parser_buffer[gps_parser_buffer_index] = c;
				gps_parser_buffer_index++;
			}

			if (gps_parser_buffer_index >= NMEA_MAX_LEN)
			{
				gps_parser_state = GPS_IDLE;
				gps_parser_buffer_index = 0;
			}
		break;

		case(GPS_CRC):
			gps_rx_checksum = hex_to_num(c) << 4;
			gps_parser_state = GPS_END;
		break;

		case(GPS_END):
			gps_rx_checksum += hex_to_num(c);
			gps_parser_state = GPS_IDLE;

			if (gps_rx_checksum == gps_checksum)
			{
				if (cmpn_p(gps_parser_buffer, PSTR("GP"), 2))
				{
					gps_parser_ptr = gps_parser_buffer + 2;

					if (cmpn_p(gps_parser_ptr, PSTR("RMC"), 3))
						gps_parse_rmc();

					if (cmpn_p(gps_parser_ptr, PSTR("GGA"), 3))
						gps_parse_gga();
				}
				else if (cmpn_p(gps_parser_buffer, PSTR("PMTK"), 4))
				{
					gps_parser_ptr = gps_parser_buffer + 4;

					if (cmpn_p(gps_parser_ptr, PSTR("011"), 3))
						gps_parse_hello();
				}

			}
			else
			{
				DEBUG("GPS CHECKSUM IS WRONG! %02X %02X\n", gps_rx_checksum, gps_checksum);
			}
		break;


	}
}

void gps_setup()
{
	DEBUG("set_nmea_output\n");
	fprintf(gps_out, "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n");

	DEBUG("set_baudrate\n");
	fprintf(gps_out, "$PMTK251,115200*1F\r\n");

	DEBUG("setup done\n");
}

//for 115200 @ 32M
#define BSEL	2094
#define BSCALE	-7

void gps_change_baud()
{
	gps_uart.Stop();
	gps_uart.Init(GPS_UART, 115200);
//	gps_uart.RegisterEvent(usart_event_rxcomplete, &gps_parse);

//	gps_uart.usart->CTRLA = 0;
//	gps_uart.usart->BAUDCTRLA = 0xFF & BSEL;
//	gps_uart.usart->BAUDCTRLB = (BSCALE << 4) | (0x0F & (BSEL >> 8));
//	gps_uart.usart->CTRLC = USART_CHSIZE_8BIT_gc;
//	gps_uart.usart->CTRLB = USART_RXEN_bm | USART_TXEN_bm;
//	gps_uart.SetInterruptPriority(LOW);
}


void gps_init()
{
	DEBUG("gps init\n");

	GPS_UART_PWR_ON;
	gps_uart.InitBuffers(250, 40);
	gps_uart.Init(GPS_UART, 9600);
//	gps_uart.RegisterEvent(usart_event_rxcomplete, &gps_parse);

	GpioSetDirection(GPS_EN, OUTPUT);	 //active high
	GpioWrite(GPS_EN, LOW);

	GpioSetDirection(GPS_TIMER, INPUT);  //active low, otherwise open-drain
	GpioSetPull(GPS_TIMER, gpio_pull_up);
	GpioSetInterrupt(GPS_TIMER, gpio_interrupt1, gpio_falling);

	GpioSetDirection(GPS_RESET, OUTPUT); //active low
	GpioWrite(GPS_RESET, LOW);

	GpioWrite(GPS_EN, HIGH);
	_delay_ms(10);
	GpioWrite(GPS_RESET, LOW);
	_delay_ms(20);
	GpioWrite(GPS_RESET, HIGH);

	gps_parser_state = GPS_IDLE;
}

void gps_stop()
{
	GpioSetDirection(GPS_EN, INPUT);
	GpioSetDirection(GPS_RESET, INPUT);

	gps_uart.Stop();
	GPS_UART_PWR_OFF;
}

void gps_step()
{
	while (!gps_uart.isRxBufferEmpty())
		gps_parse(&gps_uart);
}

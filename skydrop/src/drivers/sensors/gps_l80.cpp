#include "gps_l80.h"
#include "../uart.h"

#include "../../fc/fc.h"

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
bool gps_detail_enabled = false;

bool gps_init_ok = false;

#define L80_SAT_CNT	12

uint8_t l80_sat_id[L80_SAT_CNT];
uint8_t l80_sat_snr[L80_SAT_CNT];

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
		if (str[i] == ',')
			return 0;
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
//	DEBUG("\nRMC\n");

	gps_init_ok = true;

	char * ptr = find_comma(gps_parser_buffer);

	//UTC time
	uint8_t hour = atoi_n(ptr + 0, 2);
	uint8_t min = atoi_n(ptr + 2, 2);
	uint8_t sec = atoi_n(ptr + 4, 2);

//	DEBUG("%02d:%02d:%02d\n", hour, min, sec);

	ptr = find_comma(ptr);

	//Valid (A = valid)
	fc.gps_data.valid = (*ptr) == 'A';
	if (fc.gps_data.valid)
	{
		if (fc.gps_data.fix_cnt < GPS_FIX_CNT_MAX)
			fc.gps_data.fix_cnt++;
	}
	else
		fc.gps_data.fix_cnt = 0;

	ptr = find_comma(ptr);

	//Latitude
	float latitude = atoi_n(ptr + 2, 6);
	latitude = atoi_n(ptr + 0, 2) + latitude / 600000;

	ptr = find_comma(ptr);

	//Sign
	if ((*ptr) == 'S')
		latitude *= -1;

	fc.gps_data.latitude = latitude;

	ptr = find_comma(ptr);

	//Longitude
	float longitude = atoi_n(ptr + 3, 6);
	longitude = atoi_n(ptr + 0, 3) + longitude / 600000;

	ptr = find_comma(ptr);

	//Sign
	if ((*ptr) == 'W')
		longitude *= -1;


	fc.gps_data.longtitude = longitude;

	ptr = find_comma(ptr);

//	DEBUG("lat+lon %0.7f %0.7f\n", latitude, longitude);

	fc.gps_data.groud_speed = atoi_f(ptr) * 1.852;

	ptr = find_comma(ptr);

	//Ground course
	fc.gps_data.heading = atoi_f(ptr);

	ptr = find_comma(ptr);

	//UTC date
	uint8_t day = atoi_n(ptr + 0, 2);
	uint8_t month = atoi_n(ptr + 2, 2);
	uint16_t year = atoi_n(ptr + 4, 2) + 2000;

	fc.gps_data.utc_time = datetime_to_epoch(sec, min, hour, day, month, year);

//	DEBUG("%02d.%02d.%04d\n", day, month, year);
}

//132405.000,4809.2356,N,01704.4263,E,1,6,1.95,160.3,M,42.7,M,,
void gps_parse_gga()
{
//	DEBUG("\nGGA\n");

	char * ptr = find_comma(gps_parser_buffer);

	//Skip time
	ptr = find_comma(ptr);
	//Skip latitude
	ptr = find_comma(ptr);
	ptr = find_comma(ptr);
	//Skip longitude
	ptr = find_comma(ptr);
	ptr = find_comma(ptr);

	//Skip fix status
	ptr = find_comma(ptr);

	//Number of sat
	fc.gps_data.sat_used = atoi_c(ptr);
	ptr = find_comma(ptr);

	//HDOP TODO:calc error
	fc.gps_data.hdop = atoi_f(ptr);
	ptr = find_comma(ptr);

	//altitude
	fc.gps_data.altitude = atoi_f(ptr);
	ptr = find_comma(ptr);
	ptr = find_comma(ptr); //skip M

	//Geo id
	float geo_id = atoi_f(ptr);
	fc.gps_data.altitude -= geo_id;

//	DEBUG("fix %d (%d), hdop: %0.2f\n", fix, sat, hdop);
//	DEBUG("alt %0.1fm geo: %0.1fm\n", altitude, geo_id);
}

//
void gps_parse_gsa()
{
//	DEBUG("\nGSA\n");

	char * ptr = find_comma(gps_parser_buffer);

	//Skip mode
	ptr = find_comma(ptr);
	//fix status
	fc.gps_data.fix = atoi_c(ptr);
}

void gps_parse_gsv()
{
//	DEBUG("\nGSV\n");

	char * ptr = find_comma(gps_parser_buffer);

	//Number of messages
	uint8_t msg_c = atoi_c(ptr);
	ptr = find_comma(ptr);
	//message number
	uint8_t msg_i = atoi_c(ptr);
	ptr = find_comma(ptr);
	//sat in view
	fc.gps_data.sat_total = atoi_c(ptr);
	ptr = find_comma(ptr);

	uint8_t sat_n;

	sat_n = (msg_c > msg_i || msg_c * 4 == fc.gps_data.sat_total) ? 4 : fc.gps_data.sat_total % 4;

//	DEBUG(">> %d, %d", msg_c, msg_i);

	for (uint8_t i = 0; i < 4 ; i++)
	{
		uint8_t index = (msg_i - 1) * 4 + i;

		if (i >= sat_n)
		{
			fc.gps_data.sat_id[index] = 0;
		    fc.gps_data.sat_snr[index] = 0;
		    continue;
		}
		//sat_id
		fc.gps_data.sat_id[index] = atoi_c(ptr);
		ptr = find_comma(ptr);

		//skip elevation
		ptr = find_comma(ptr);

		//skip azimut
		ptr = find_comma(ptr);

		//snr
		fc.gps_data.sat_snr[index] = atoi_c(ptr);
		ptr = find_comma(ptr);
//		DEBUG("%d %d,", fc.gps_data.sat_id[index], fc.gps_data.sat_snr[index]);
	}
//	DEBUG("\n");
}

void gps_normal()
{
	gps_detail_enabled = false;
	DEBUG("set_nmea_output - normal\n");
	fprintf_P(gps_out, PSTR("$PMTK314,0,1,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*2D\r\n"));
}

void gps_detail()
{
	gps_detail_enabled = true;
	DEBUG("set_nmea_output - detail\n");
	fprintf_P(gps_out, PSTR("$PMTK314,0,1,0,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0*2D\r\n"));
}

void gps_set_baudrate()
{
	DEBUG("set_baudrate\n");
	fprintf_P(gps_out, PSTR("$PMTK251,115200*1F\r\n"));
	gps_uart.FlushTxBuffer();
	_delay_ms(1);
}


//for 115200 @ 32M
#define BSEL	2094
#define BSCALE	-7

void gps_change_uart_baudrate()
{
	gps_uart.Stop();
	gps_uart.Init(GPS_UART, 115200);
}


void gps_parse_hello()
{
	DEBUG("HELLO\n");
}

void gps_parse_sys()
{
	DEBUG("SYS");
//	gps_set_baudrate();
//	gps_change_uart_baudrate();

	if (gps_detail_enabled)
		gps_detail();
	else
		gps_normal();
}

void gps_parse(Usart * c_uart)
{
	uint8_t c = c_uart->Read();

//	DEBUG("%c", c);

	switch (gps_parser_state)
	{
		case(GPS_IDLE):
			if (c == '$')
			{
				gps_parser_buffer_index = 0;
				gps_checksum = 0;
				gps_parser_state = GPS_DATA;
			}
//			else
//			{
//				gps_parser_buffer_index++;
//				if (gps_parser_buffer_index > 10)
//				{
//					gps_set_baudrate();
//					gps_change_uart_baudrate();
//					gps_parser_buffer_index = 0;
//				}
//			}
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

					if (cmpn_p(gps_parser_ptr, PSTR("GSA"), 3))
						gps_parse_gsa();

					if (cmpn_p(gps_parser_ptr, PSTR("GSV"), 3))
						gps_parse_gsv();
				}
				else if (cmpn_p(gps_parser_buffer, PSTR("PMTK"), 4))
				{
					gps_parser_ptr = gps_parser_buffer + 4;

					if (cmpn_p(gps_parser_ptr, PSTR("011"), 3))
						gps_parse_hello();

					if (cmpn_p(gps_parser_ptr, PSTR("010"), 3))
						gps_parse_sys();
				}

			}
			else
			{
				DEBUG("GPS CHECKSUM IS WRONG! %02X %02X\n", gps_rx_checksum, gps_checksum);
			}
		break;


	}
}

void gps_start()
{
	GPS_UART_PWR_ON;
	gps_uart.Init(GPS_UART, 9600);
	gps_uart.SetInterruptPriority(MEDIUM);

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
	fc.gps_data.valid = false;
	fc.gps_data.fix = 0;
	fc.gps_data.fix_cnt = 0;

	for (uint8_t i = 0; i < GPS_SAT_CNT; i++)
	{
		fc.gps_data.sat_id[i] = 0;
		fc.gps_data.sat_snr[i] = 0;
	}
}

void gps_init()
{
	DEBUG("gps init\n");

	gps_uart.InitBuffers(250, 40);
}

bool gps_selftest()
{
	return gps_init_ok;
}

void gps_stop()
{
	GpioSetDirection(GPS_EN, INPUT);
	GpioSetDirection(GPS_RESET, INPUT);

	fc.gps_data.valid = false;
	fc.gps_data.fix = 0;
	fc.gps_data.fix_cnt = 0;

	gps_init_ok = false;
	gps_uart.Stop();
	GPS_UART_PWR_OFF;
}

void gps_step()
{
	while (!gps_uart.isRxBufferEmpty())
		gps_parse(&gps_uart);
}

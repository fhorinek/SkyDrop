#include "gps_l80.h"
#include "../uart.h"

#include "../../fc/fc.h"

#define GPS_UART_RX_SIZE	250
#define GPS_UART_TX_SIZE	40

uint8_t gps_uart_rx_buffer[GPS_UART_RX_SIZE];
uint8_t gps_uart_tx_buffer[GPS_UART_TX_SIZE];

Usart gps_uart(GPS_UART_RX_SIZE, gps_uart_rx_buffer, GPS_UART_TX_SIZE, gps_uart_tx_buffer);

CreateStdOut(gps_out, gps_uart.Write);

ISR(GPS_TIMER_INT)
{
	DEBUG("GPS_TIMER_INT\n");
}

#define GPS_IDLE	0
#define GPS_DATA	1
#define GPS_CRC		2
#define GPS_END		3

#define NMEA_MAX_LEN	85

volatile uint8_t gps_parser_state;
char gps_parser_buffer[NMEA_MAX_LEN];
char * gps_parser_ptr;
volatile uint8_t gps_parser_buffer_index = 0;
volatile uint8_t gps_checksum;
volatile uint8_t gps_rx_checksum;
bool gps_detail_enabled = false;

volatile bool gps_init_ok = false;

#define L80_SAT_CNT	12

uint8_t l80_sat_id[L80_SAT_CNT];
uint8_t l80_sat_snr[L80_SAT_CNT];

//132405.000,A,4809.2356,N,01704.4263,E,0.15,317.49,270115,,,A
//hhmmss.XXX,
void gps_parse_rmc()
{
//	DEBUG("\nRMC\n");

	gps_init_ok = true;

//	strcpy(gps_parser_buffer, "$GPRMC,044434.000,A,3319.1312,S,14805.03,E,24.00,37.86,090116,,,A*71");

	char * ptr = find_comma(gps_parser_buffer);
	char * old_ptr;
	uint8_t tlen;

	//UTC time
	uint8_t hour = atoi_n(ptr + 0, 2);
	uint8_t min = atoi_n(ptr + 2, 2);
	uint8_t sec = atoi_n(ptr + 4, 2);

//	DEBUG("%02d:%02d:%02d\n", hour, min, sec);

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;

	if (tlen != 10)
	{
		if (config.system.debug_gps)
			DEBUG("GPRMC timestamp len = %u\n", tlen);
		return;
	}

	//Valid (A = valid)
	fc.gps_data.valid = (*ptr) == 'A';

	if (fc.gps_data.valid)
	{
		if (fc.gps_data.fix_cnt < GPS_FIX_CNT_MAX)
			fc.gps_data.fix_cnt++;
	}
	else
	{
		fc.gps_data.fix_cnt = 0;
	}

	ptr = find_comma(ptr);

	uint32_t loc_deg;
	uint32_t loc_min;

	//Latitude
	loc_deg = atoi_n(ptr, 2);
	loc_min = atoi_n(ptr + 2, 6);

	int32_t latitude = (loc_min * 100ul) / 6;
	latitude = loc_deg * 10000000ul + latitude;

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;

	if (tlen != 9)
	{
		if (config.system.debug_gps)
			DEBUG("GPRMC latitude len = %u\n", tlen);
		return;
	}

	//Latitude sign
	if ((*ptr) == 'S')
		latitude *= -1;

	fc.gps_data.latitude = latitude;

	if (fc.gps_data.valid)
		sprintf_P((char *)fc.gps_data.cache_igc_latitude, PSTR("%02lu%05lu%c"), loc_deg, loc_min / 10, (*ptr));

	ptr = find_comma(ptr);

	//Longitude
	loc_deg = atoi_n(ptr, 3);
	loc_min = atoi_n(ptr + 3, 6);

	int32_t longitude = (loc_min * 100ul) / 6;
	longitude = loc_deg * 10000000ul + longitude;

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;

	if (tlen != 10)
	{
		if (config.system.debug_gps)
			DEBUG("GPRMC longitude len = %u\n", tlen);
		return;
	}

	//Longitude sign
	if ((*ptr) == 'W')
		longitude *= -1;

	fc.gps_data.longtitude = longitude;

	if (fc.gps_data.valid)
		sprintf_P((char *)fc.gps_data.cache_igc_longtitude, PSTR("%03lu%05lu%c"), loc_deg, loc_min / 10, (*ptr));

	ptr = find_comma(ptr);

//	DEBUG("lat+lon %07ld %08ld\n", latitude, longitude);

	fc.gps_data.groud_speed = atoi_f(ptr); //in knots

	ptr = find_comma(ptr);

	//Ground course
	fc.gps_data.heading = atoi_f(ptr);

	ptr = find_comma(ptr);

	//UTC date
	uint8_t day = atoi_n(ptr + 0, 2);
	uint8_t month = atoi_n(ptr + 2, 2);
	uint16_t year = atoi_n(ptr + 4, 2) + 2000;

	old_ptr = ptr;
	ptr = find_comma(ptr);
	tlen = ptr - old_ptr - 1;

	if (tlen != 6)
	{
		if (config.system.debug_gps)
			DEBUG("GPRMC date len = %u\n", tlen);
		return;
	}

	fc.gps_data.utc_time = datetime_to_epoch(sec, min, hour, day, month, year);

	if (config.connectivity.forward_gps && fc.gps_data.valid)
	{
		char tmp[NMEA_MAX_LEN];
		sprintf(tmp, "$%s", gps_parser_buffer);
		bt_send(tmp);

		if (config.connectivity.uart_function > UART_FORWARD_OFF)
			uart_send(tmp);
	}

	uint16_t tdeg, tmin, tsec;
	int32_t tmp;

	if (fc.gps_data.valid)
		switch (config.connectivity.gps_format_flags & GPS_FORMAT_MASK)
		{
			case(GPS_DDdddddd):
				sprintf_P((char *)fc.gps_data.cache_gui_latitude, PSTR(" %+010ld"), fc.gps_data.latitude);
				memcpy((void *)fc.gps_data.cache_gui_latitude, (void *)(fc.gps_data.cache_gui_latitude + 1), 3);
				fc.gps_data.cache_gui_latitude[3] = '.';
				if (fc.gps_data.cache_gui_latitude[1] == '0')
					memcpy((void *)(fc.gps_data.cache_gui_latitude + 1), (void *)(fc.gps_data.cache_gui_latitude + 2), 10);

				sprintf_P((char *)fc.gps_data.cache_gui_longtitude, PSTR(" %+011ld"), fc.gps_data.longtitude);
				memcpy((void *)fc.gps_data.cache_gui_longtitude, (void *)(fc.gps_data.cache_gui_longtitude + 1), 4);
				fc.gps_data.cache_gui_longtitude[4] = '.';

				if (fc.gps_data.cache_gui_longtitude[1] == '0')
				{
					if (fc.gps_data.cache_gui_longtitude[2] == '0')
						memcpy((void *)(fc.gps_data.cache_gui_longtitude + 1), (void *)(fc.gps_data.cache_gui_longtitude + 3), 10);
					else
						memcpy((void *)(fc.gps_data.cache_gui_longtitude + 1), (void *)(fc.gps_data.cache_gui_longtitude + 2), 11);
				}
			break;

			case(GPS_DDMMmmm):
				memcpy((void *)fc.gps_data.cache_gui_latitude, (void *)fc.gps_data.cache_igc_latitude, 2);
				fc.gps_data.cache_gui_latitude[2] = '*';
				memcpy((void *)(fc.gps_data.cache_gui_latitude + 3), (void *)(fc.gps_data.cache_igc_latitude + 2), 2);
				fc.gps_data.cache_gui_latitude[5] = '.';
				memcpy((void *)(fc.gps_data.cache_gui_latitude + 6), (void *)(fc.gps_data.cache_igc_latitude + 4), 4);
				fc.gps_data.cache_gui_latitude[10] = 0;

				memcpy((void *)fc.gps_data.cache_gui_longtitude, (void *)fc.gps_data.cache_igc_longtitude, 3);
				fc.gps_data.cache_gui_longtitude[3] = '*';
				memcpy((void *)(fc.gps_data.cache_gui_longtitude + 4), (void *)(fc.gps_data.cache_igc_longtitude + 3), 2);
				fc.gps_data.cache_gui_longtitude[6] = '.';
				memcpy((void *)(fc.gps_data.cache_gui_longtitude + 7), (void *)(fc.gps_data.cache_igc_longtitude + 5), 4);
				fc.gps_data.cache_gui_longtitude[11] = 0;
			break;

			case(GPS_DDMMSS):
				tdeg = abs(fc.gps_data.latitude) / 10000000ul;
				tmp = ((abs(fc.gps_data.latitude) % 10000000ul) * 60);
				tmin = tmp / 10000000ul;
				tsec = ((tmp % 10000000ul) * 60) / 10000000ul;

				sprintf_P((char *)fc.gps_data.cache_gui_latitude,
						PSTR("%02u*%02u'%02u\"%c"), tdeg, tmin, tsec, (*(fc.gps_data.cache_igc_latitude + 7)));

				tdeg = abs(fc.gps_data.longtitude) / 10000000ul;
				tmp = ((abs(fc.gps_data.longtitude) % 10000000ul) * 60);
				tmin = tmp / 10000000ul;
				tsec = ((tmp % 10000000ul) * 60) / 10000000ul;

				sprintf_P((char *)fc.gps_data.cache_gui_longtitude,
						PSTR("%03u*%02u'%02u\"%c"), tdeg, tmin, tsec, (*(fc.gps_data.cache_igc_longtitude + 8)));
			break;
		}
	fc.gps_data.new_sample = 0xFF;
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

	//skip M
	ptr = find_comma(ptr);

	//Geo id
//	float geo_id = atoi_f(ptr);
//	fc.gps_data.altitude -= geo_id;

//	DEBUG("fix %d (%d), hdop: %0.2f\n", fc.gps_data.sat_used, fc.gps_data.sat_total, fc.gps_data.hdop);
//	DEBUG("alt %0.1fm geo: %0.1fm\n", fc.gps_data.altitude, 0);

	if (config.connectivity.forward_gps && fc.gps_data.valid)
	{
		char tmp[NMEA_MAX_LEN];
		sprintf(tmp, "$%s", gps_parser_buffer);
		bt_send(tmp);
		if (config.connectivity.uart_function > UART_FORWARD_OFF)
			uart_send(tmp);
	}
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
//	DEBUG("sat_total %d\n", fc.gps_data.sat_total);
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
	//enable RMC, GGA
	fprintf_P(gps_out, PSTR("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*78\r\n"));
}

void gps_detail()
{
	gps_detail_enabled = true;
	DEBUG("set_nmea_output - detail\n");
	//enable RMC, GGA, GSA, GSV
	fprintf_P(gps_out, PSTR("$PMTK314,0,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"));
}

void gps_set_baudrate()
{
	DEBUG("set_baudrate\n");
	fprintf_P(gps_out, PSTR("$PMTK251,115200*1F\r\n"));
	gps_uart.FlushTxBuffer();
	_delay_ms(1);
}


void gps_change_uart_baudrate()
{
	gps_uart.Stop();
	gps_uart.Init(GPS_UART, 115200);
}


void gps_parse_hello()
{
	DEBUG("GPS HELLO\n");
}

void gps_parse_sys()
{
	DEBUG("GPS SYS RESP");

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
				gps_parser_buffer[gps_parser_buffer_index] = c;
				gps_parser_buffer_index++;

				gps_parser_state = GPS_CRC;
			}
			else
			{
				gps_checksum ^= c;
				gps_parser_buffer[gps_parser_buffer_index] = c;
				gps_parser_buffer_index++;
			}

			if (gps_parser_buffer_index >= NMEA_MAX_LEN)
			{
				assert(0);
				gps_parser_buffer_index = 0;
				gps_parser_state = GPS_IDLE;
			}
		break;

		case(GPS_CRC):
			gps_rx_checksum = hex_to_num(c) << 4;
			gps_parser_buffer[gps_parser_buffer_index] = c;
			gps_parser_buffer_index++;

			gps_parser_state = GPS_END;
		break;

		case(GPS_END):
			gps_rx_checksum += hex_to_num(c);
			gps_parser_buffer[gps_parser_buffer_index] = c;

			if (config.system.debug_gps)
			{
				gps_parser_buffer[gps_parser_buffer_index + 1] = '\0';
				DEBUG("GPS:\"%s\"\n", gps_parser_buffer);
			}

			gps_parser_buffer[gps_parser_buffer_index + 1] = '\n';
			gps_parser_buffer[gps_parser_buffer_index + 2] = '\0';

			gps_parser_buffer_index = 0;
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
				DEBUG("GPS:\"%s\"\n", gps_parser_buffer);
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

	gps_uart.SetupRxDMA(GPS_UART_DMA_CH, GPS_UART_DMA_TRIGGER);

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
	fc.gps_data.new_sample = 0x00;
	fc.gps_data.fix = 0;
	fc.gps_data.fix_cnt = 0;

	for (uint8_t i = 0; i < GPS_SAT_CNT; i++)
	{
		fc.gps_data.sat_id[i] = 0;
		fc.gps_data.sat_snr[i] = 0;
	}

	//reset cache
	strcpy_P((char *)fc.gps_data.cache_igc_latitude, PSTR("0000000N"));
	strcpy_P((char *)fc.gps_data.cache_igc_longtitude, PSTR("00000000E"));
	strcpy_P((char *)fc.gps_data.cache_gui_latitude, PSTR("---"));
	strcpy_P((char *)fc.gps_data.cache_gui_longtitude, PSTR("---"));
}

void gps_init()
{
	DEBUG("gps init\n");
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

	GpioSetPull(GPS_TIMER, gpio_totem);

	gps_init_ok = false;
	gps_uart.Stop();
	GPS_UART_PWR_OFF;
}

void gps_step()
{
//	gps_uart.DumpDMA();

	while (!gps_uart.isRxBufferEmpty())
		gps_parse(&gps_uart);
}

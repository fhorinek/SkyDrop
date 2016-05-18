/*
 * fake_data.h
 *
 *  Created on: 13.5.2016
 *      Author: horinek
 */

#ifndef FAKE_DATA_H_
#define FAKE_DATA_H_

//For promo screenshots

//#define FAKE_ENABLE

#define FAKE_DATA \
	fc.baro_valid = true; \
	fc.altitude1 = 657; \
	fc.altitudes[0] = 247; \
	fc.altitudes[1] = 654; \
	fc.vario = 1.6; \
	fc.digital_vario = 1.6; \
	fc.avg_vario = 1.3; \
 \
	fc.vario_history[0]= 12; \
	fc.vario_history[1]= 17; \
	fc.vario_history[2]= 24; \
	fc.vario_history[3]= 36; \
	fc.vario_history[4]= 38; \
	fc.vario_history[5]= 36; \
	fc.vario_history[6]= 43; \
	fc.vario_history[7]= 48; \
	fc.vario_history[8]= 50; \
	fc.vario_history[9]= 50; \
	fc.vario_history[10]= 50; \
	fc.vario_history[11]= 48; \
	fc.vario_history[12]= 43; \
	fc.vario_history[13]= 38; \
	fc.vario_history[14]= 36; \
	fc.vario_history[15]= 29; \
	fc.vario_history[16]= 24; \
	fc.vario_history[17]= 17; \
	fc.vario_history[18]= 17; \
	fc.vario_history[19]= 14; \
	fc.vario_history[20]= 19; \
	fc.vario_history[21]= 19; \
	fc.vario_history[22]= 24; \
	fc.vario_history[23]= 29; \
	fc.vario_history[24]= 26; \
	fc.vario_history[25]= 29; \
	fc.vario_history[26]= 29; \
	fc.vario_history[27]= 31; \
	fc.vario_history[28]= 29; \
	fc.vario_history[29]= 29; \
	fc.vario_history[30]= 36; \
	fc.vario_history[31]= 38; \
 \
	fc.gps_data.valid = true; \
	fc.gps_data.groud_speed = 19.1; \
	fc.gps_data.heading = 75; \
	strcpy((char *)fc.gps_data.cache_gui_latitude, "48.46367150"); \
	strcpy((char *)fc.gps_data.cache_gui_longtitude, "17.42317721"); \
 \
	battery_per = BATTERY_FULL; \
 \
	config.connectivity.use_bt = true; \
	extern volatile bool bt_device_connected; \
	bt_device_connected = true; \
 \
	fc.logger_state = LOGGER_ACTIVE; \
	fc.flight_state = FLIGHT_LAND; \
 \
	fc.flight_timer = 21.3 * 60 * 1000;



#endif /* FAKE_DATA_H_ */

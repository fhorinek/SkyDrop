#include "set_gps_detail.h"

#include "../gui_list.h"
#include "../gui_value.h"

#include "../../fc/conf.h"
#include "../../drivers/sensors/gps_l80.h"

void gui_set_gps_detail_init()
{
	gps_detail();
}

void gui_set_gps_detail_stop() {}

void gui_set_gps_detail_loop()
{
	disp.LoadFont(F_TEXT_S);
	uint8_t f_h = disp.GetAHeight() + 1;

	disp.GotoXY(0, 0);
	fprintf_P(lcd_out, PSTR("Lat: %s"), fc.gps_data.cache_gui_latitude);
	disp.GotoXY(0, f_h);
	fprintf_P(lcd_out, PSTR("Lon: %s"), fc.gps_data.cache_gui_longtitude);
	disp.GotoXY(0, f_h * 2);
	fprintf_P(lcd_out, PSTR("HDOP: %5.2f     VDOP: %5.2f"), fc.gps_data.hdop, fc.gps_data.vdop);
	disp.GotoXY(0, f_h * 3);
	fprintf_P(lcd_out, PSTR("PDOP: %5.2f     Alt: %6.1f"), fc.gps_data.pdop, fc.gps_data.altitude);

	char tmp[10];

	sprintf_P(tmp, PSTR("%d/%d"), fc.gps_data.sat_used, fc.gps_data.sat_total);
	gui_raligh_text(tmp, GUI_DISP_WIDTH, 0);
	switch (fc.gps_data.fix)
	{
		case(1):
				strcpy_P(tmp, PSTR("No Fix"));
		break;
		case(2):
				strcpy_P(tmp, PSTR("2D Fix"));
		break;
		case(3):
				strcpy_P(tmp, PSTR("3D Fix"));
		break;
	}
	gui_raligh_text(tmp, GUI_DISP_WIDTH, f_h);

	disp.LoadFont(F_TEXT_S);
	for (uint8_t i=0; i < GPS_SAT_CNT; i++)
	{
//		uint8_t x = i < 6 ? (GUI_DISP_WIDTH  * 1 / 4) : (GUI_DISP_WIDTH  * 3 / 4);
//		uint8_t y = (3.5 + i % 6) * f_h;
//
//		x -= GUI_DISP_WIDTH / 8;
//
//		disp.GotoXY(x, y);
//		if (fc.gps_data.sat_id[i] != 0)
//			fprintf_P(lcd_out, PSTR("%03d: %d"), fc.gps_data.sat_id[i], fc.gps_data.sat_snr[i]);

		if (fc.gps_data.sat_id[i] == 0)
			continue;

		uint8_t x = 7 * i;
		uint8_t y = GUI_DISP_HEIGHT - 2 - fc.gps_data.sat_id[i] / 3;
		disp.DrawRectangle(x, y, x + 5, GUI_DISP_HEIGHT - 1, 1, 1);
	}

}

void gui_set_gps_detail_irqh(uint8_t type, uint8_t * buff)
{
	if (*buff == BE_CLICK && type == B_MIDDLE)
		gui_switch_task(GUI_SET_GPS);

	if (*buff == BE_LONG && type == B_MIDDLE)
		gui_switch_task(GUI_SET_GPS);
}


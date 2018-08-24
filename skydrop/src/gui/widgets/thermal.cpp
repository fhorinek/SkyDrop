#include "thermal.h"

void widget_thermal_time_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("TTime"), x, y);

	uint16_t sec = fc.flight.circling_time;

	char text[10];
	if (sec < 60)
	    sprintf_P(text, PSTR("%02u"), sec);
	else
	    sprintf_P(text, PSTR("%02u:%02u"), sec / 60, sec % 60);

	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_thermal_gain_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
    uint8_t lh = widget_label_P(PSTR("TGain"), x, y);

    int16_t diff = fc.flight.circling_gain;

    if (config.altitude.alt1_flags & VARIO_UNITS_I)
    	diff *= FC_METER_TO_FEET;

    char text[10];
    sprintf_P(text, PSTR("%+d"), diff);

    widget_value_int(text, x, y + lh, w, h - lh);
}

register_widget1(w_thermal_time, "Thermal time", widget_thermal_time_draw);
register_widget1(w_thermal_gain, "Thermal gain", widget_thermal_gain_draw);

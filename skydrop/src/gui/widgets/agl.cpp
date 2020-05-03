#include "agl.h"
#include "../../fc/agl.h"

void widget_agl_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("HAGL"), x, y);

	char text[10];

	int16_t val = (int16_t)(fc.altitude1 - fc.agl.ground_level);

	if (config.altitude.alt1_flags & ALT_UNIT_I)
		val *= FC_METER_TO_FEET;

	if (fc.agl.ground_level != AGL_INVALID)
		sprintf_P(text, PSTR("%d"), val);
	else
		strcpy_P(text, PSTR("---"));

	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_agl_irqh(uint8_t type, uint8_t * buff)
{
	if (type == B_MIDDLE && *buff == BE_LONG)
	{
		if (fc.agl.ground_level != AGL_INVALID && fc.flight.state != FLIGHT_FLIGHT)
		{
			char message[40];

			fc_manual_alt0_change(fc.agl.ground_level);
			config.altitude.QNH1 = fc_alt_to_qnh(fc.agl.ground_level,
					fc.vario.pressure);
                        sprintf_P(message, PSTR("Alt1 set to ground\nlevel: %d"),
                                        (config.altitude.alt1_flags & ALT_UNIT_I) ? (int)(fc.agl.ground_level * FC_METER_TO_FEET) : fc.agl.ground_level);
			gui_showmessage(message);
		}
	}
}

void widget_gl_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	uint8_t lh = widget_label_P(PSTR("GLevel"), x, y);

	char text[10];

	int16_t val = (int16_t)(fc.agl.ground_level);

	if (config.altitude.alt1_flags & ALT_UNIT_I)
		val *= FC_METER_TO_FEET;

	if (fc.agl.ground_level != AGL_INVALID)
		sprintf_P(text, PSTR("%d"), val);
	else
		strcpy_P(text, PSTR("---"));

	widget_value_int(text, x, y + lh, w, h - lh);
}

register_widget2(w_agl_height, "Height AGL", widget_agl_draw, 0, widget_agl_irqh);
register_widget1(w_agl_ground_level, "Ground Level", widget_gl_draw);

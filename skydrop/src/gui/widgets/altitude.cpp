#include "altitude.h"
#include "../../fc/kalman.h"


void widget_alt_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t index)
{
	char label[5];

	sprintf_P(label, PSTR("Alt%d"), index);
	uint8_t lh = widget_label(label, x, y);

	float val;
	uint8_t alt_flags;

	if (index == 1)
	{
		val = fc.altitude1;
		alt_flags = config.altitude.alt1_flags;
	}
	else
	{
		val = fc.altitudes[index - 2];
		alt_flags = config.altitude.altimeter[index - 2].flags;
	}

	if (alt_flags & ALT_UNIT_I)
		val *= FC_METER_TO_FEET;

	bool valid;
	if ((alt_flags & 0b11000000) == ALT_ABS_GPS)
		valid = fc.gps_data.valid;
	else
		valid = fc.vario.valid;

	char text[10];
	if (valid)
		sprintf_P(text, PSTR("%0.0f"), val);
	else
		strcpy_P(text, PSTR("---"));
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_alt_menu_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == TASK_IRQ_BUTTON_M && (*buff == BE_HOLD || *buff == BE_RELEASED || *buff == BE_DBL_CLICK))
		return;

	uint8_t a_type;

	if (index == 1)
		a_type = ALT_ABS_QNH1;
	else
		a_type  = config.altitude.altimeter[index - 2].flags & 0b11000000;


	if (a_type != ALT_ABS_GPS && fc.vario.valid == false)
		return;

	if (a_type == ALT_ABS_GPS && fc.gps_data.valid == false)
		return;

	if (type == TASK_IRQ_BUTTON_M && *buff == BE_LONG)
	{
		switch (a_type)
		{
			case(ALT_DIFF):
				fc_zero_alt(index - 1);
			break;
		}

		return;
	}

	if (gui_enter_widget_menu())
	{
		//init sequence
		widget_menu_state = 0;
	}

	if (type == TASK_IRQ_BUTTON_M && *buff == BE_CLICK)
		gui_exit_widget_menu();

	if (type == TASK_IRQ_BUTTON_L || type == TASK_IRQ_BUTTON_R)
	{
		if (*buff == BE_CLICK || *buff == BE_DBL_CLICK)
		{
			float new_alt;

			int8_t inc;
			if (type == TASK_IRQ_BUTTON_L)
				inc = -1;
			else
				inc = +1;

			switch (a_type)
			{
			case(ALT_ABS_QNH1):
				if (index == 1)
					new_alt = fc.altitude1 + inc;
				else
					new_alt = fc.altitudes[index - 2] + inc;

				fc_manual_alt0_change(new_alt);
				config.altitude.QNH1 = fc_alt_to_qnh(new_alt, fc.vario.pressure);
				fc.vario.error_over_time = 0;
				fc.vario.time_of_last_error_update = task_get_ms_tick();
			break;

			case(ALT_ABS_QNH2):
				if (index == 1)
					new_alt = fc.altitude1 + inc;
				else
					new_alt = fc.altitudes[index - 2] + inc;

				config.altitude.QNH2 = fc_alt_to_qnh(new_alt, fc.vario.pressure);
			break;

			case(ALT_DIFF):
				config.altitude.altimeter[index - 2].delta += inc;
			break;
			}
		}

		if (*buff == BE_LONG)
		{
			if (type == TASK_IRQ_BUTTON_L)
				widget_menu_state = 1;
			if (type == TASK_IRQ_BUTTON_R)
				widget_menu_state = 2;
		}

		if (*buff == BE_RELEASED)
		{
			widget_menu_state = 0;
		}
	}




}

void widget_alt_menu_loop(uint8_t alt_index)
{
	uint8_t a_type;
	uint8_t a_flags;
	uint8_t a_index;

	if (alt_index == 1)
		a_flags = config.altitude.alt1_flags;
	else
		a_flags = config.altitude.altimeter[alt_index - 2].flags;

	a_type  = a_flags & 0b11000000;
	a_index = a_flags & 0b00001111;

	if (widget_menu_state)
	{
		gui_enter_widget_menu();

		float new_alt;
		int8_t inc;

		if (widget_menu_state == 1)
			inc = -1;
		else
			inc = +1;

		switch (a_type)
		{
			case(ALT_ABS_QNH1):
				if (alt_index == 1)
					new_alt = fc.altitude1 + inc;
				else
					new_alt = fc.altitudes[alt_index - 2] + inc;

				fc_manual_alt0_change(new_alt);

				config.altitude.QNH1 = fc_alt_to_qnh(new_alt, fc.vario.pressure);
				fc.vario.error_over_time = 0;
				fc.vario.time_of_last_error_update = task_get_ms_tick();

			break;

			case(ALT_ABS_QNH2):
				if (alt_index == 1)
					new_alt = fc.altitude1 + inc;
				else
					new_alt = fc.altitudes[alt_index - 2] + inc;

				config.altitude.QNH2 = fc_alt_to_qnh(new_alt, fc.vario.pressure);
			break;

			case(ALT_DIFF):
				config.altitude.altimeter[alt_index - 2].delta += inc;
			break;
		}
	}

	disp.LoadFont(F_VALUES_L);
	uint8_t h_v = disp.GetTextHeight();
	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();

	char title[20];
	switch (a_type)
	{
		case(ALT_ABS_QNH1):
			strcpy_P(title, PSTR("Absolute QNH1"));
		break;
		case(ALT_ABS_QNH2):
			strcpy_P(title, PSTR("Absolute QNH2"));
		break;
		case(ALT_DIFF):
			sprintf_P(title, PSTR("Relative to Alt%d"), a_index + 1);
		break;
		case(ALT_ABS_GPS):
			strcpy_P(title, PSTR("Absolute GPS"));
		break;
	}

	gui_dialog(title);

	//NAMES
	disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + 2 + h_v - h_t);
	fprintf_P(lcd_out, PSTR("ALT%d"), alt_index);

	disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + 2 + h_v + h_v - h_t);
	switch (a_type)
	{
		case(ALT_ABS_QNH1):
			fprintf_P(lcd_out, PSTR("QNH1"));
		break;
		case(ALT_ABS_QNH2):
			fprintf_P(lcd_out, PSTR("QNH2"));
		break;
		case(ALT_DIFF):
			fprintf_P(lcd_out, PSTR("delta"));
		break;
		case(ALT_ABS_GPS):
			fprintf_P(lcd_out, PSTR("GPS fix"));
		break;
	}

	//imperial unit marker
	if (a_flags & ALT_UNIT_I)
	{
		disp.GotoXY(28, GUI_DIALOG_TOP + 6 + h_v - h_t);
		disp.LoadFont(F_TEXT_S);
		fprintf_P(lcd_out, PSTR("ft"));
	}

	//VALUES
	disp.LoadFont(F_VALUES_L);
	char tmp[10];
	if (alt_index == 1)
	{
		if (a_flags & ALT_UNIT_I)
			sprintf_P(tmp, PSTR("%0.0f"), fc.altitude1 * FC_METER_TO_FEET);
		else
			sprintf_P(tmp, PSTR("%0.0f"), fc.altitude1);
	}
	else
	{
		if (a_flags & ALT_UNIT_I)
			sprintf_P(tmp, PSTR("%d"), (int16_t)(fc.altitudes[alt_index - 2] * FC_METER_TO_FEET));
		else
			sprintf_P(tmp, PSTR("%d"), fc.altitudes[alt_index - 2]);
	}
	gui_raligh_text(tmp, GUI_DIALOG_RIGHT, GUI_DIALOG_TOP + 2);


	switch (a_type)
		{
		case(ALT_ABS_QNH1):
			sprintf_P(tmp, PSTR("%0.0f"), config.altitude.QNH1 / 10);
		break;
		case(ALT_ABS_QNH2):
			sprintf_P(tmp, PSTR("%0.0f"), config.altitude.QNH2 / 10);
		break;
		case(ALT_DIFF):
			sprintf_P(tmp, PSTR("%+d"), config.altitude.altimeter[alt_index - 2].delta);
		break;
		case(ALT_ABS_GPS):
			if (fc.gps_data.valid)
				strcpy_P(tmp, PSTR("valid"));
			else
				strcpy_P(tmp, PSTR("invalid"));
		break;
	}
	gui_raligh_text(tmp, GUI_DIALOG_RIGHT, GUI_DIALOG_TOP + 2 + h_v);
}


register_widget3(w_alt1, "Altitude 1", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 1);
register_widget3(w_alt2, "Altitude 2", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 2);
register_widget3(w_alt3, "Altitude 3", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 3);
register_widget3(w_alt4, "Altitude 4", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 4);
register_widget3(w_alt5, "Altitude 5", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 5);

#include "altitude.h"


void widget_alt_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	char label[5];
	uint8_t index = flags;

	sprintf_P(label, PSTR("Alt%d"), index);
	uint8_t lh = widget_label(label, x, y);

	float val;
	if (index == 1)
		val = flight_data.altitude1;
	else
	{
		index -= 2;
		val = flight_data.altimeter[index].altitude;
	}

	char text[10];
	if (flight_data.baro_valid)
		sprintf(text, "%0.0f", val);
	else
		sprintf(text, "---");
	widget_value_int(text, x, y + lh, w, h - lh);
}

void widget_alt_menu_irqh(uint8_t type, uint8_t * buff, uint8_t flags)
{
	if (!flight_data.baro_valid)
		return;

	if (type == TASK_IRQ_BUTTON_M && (*buff == BE_HOLD || *buff == BE_RELEASED || *buff == BE_DBL_CLICK))
		return;

	uint8_t a_type;
	if (flags == 1)
		a_type = ALT_ABS_QNH1;
	else
		a_type = flight_data.altimeter[flags - 2].flags & 0xF0;

	uint8_t a_index = flight_data.altimeter[flags - 2].flags & 0x0F;

	if (type == TASK_IRQ_BUTTON_M && *buff == BE_LONG)
	{
		switch (a_type)
		{
		case(ALT_DIFF):

			if (a_index == 1)
				flight_data.altimeter[flags - 2].delta = -flight_data.altitude1;
			else
				flight_data.altimeter[flags - 2].delta = -flight_data.altimeter[a_index].altitude;
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
				if (flags == 1)
					new_alt = flight_data.altitude1 + inc;
				else
					new_alt = flight_data.altimeter[flags - 2].altitude + inc;

				flight_data.QNH1 = fc_alt_to_qnh(new_alt, flight_data.pressure);
			break;
			case(ALT_ABS_QNH2):
				if (flags == 1)
					new_alt = flight_data.altitude1 + inc;
				else
					new_alt = flight_data.altimeter[flags - 2].altitude + inc;

				flight_data.QNH2 = fc_alt_to_qnh(new_alt, flight_data.pressure);
			break;
			case(ALT_DIFF):
				flight_data.altimeter[flags - 2].delta += inc;
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

void widget_alt_menu_loop(uint8_t flags)
{
	uint8_t a_type;
	if (flags == 1)
		a_type = ALT_ABS_QNH1;
	else
		a_type = flight_data.altimeter[flags - 2].flags & 0xF0;


	if (widget_menu_state)
	{
		float new_alt;
		int8_t inc;

		if (widget_menu_state == 1)
			inc = -1;
		else
			inc = +1;

		switch (a_type)
		{
			case(ALT_ABS_QNH1):
				if (flags == 1)
					new_alt = flight_data.altitude1 + inc;
				else
					new_alt = flight_data.altimeter[flags - 2].altitude + inc;

				flight_data.QNH1 = fc_alt_to_qnh(new_alt, flight_data.pressure);
			break;
			case(ALT_ABS_QNH2):
				if (flags == 1)
					new_alt = flight_data.altitude1 + inc;
				else
					new_alt = flight_data.altimeter[flags - 2].altitude + inc;

				flight_data.QNH2 = fc_alt_to_qnh(new_alt, flight_data.pressure);
			break;
			case(ALT_DIFF):
				flight_data.altimeter[flags - 2].delta += inc;
			break;
		}
	}

	disp.LoadFont(F_VALUES_L);
	uint8_t h_v = disp.GetTextHeight();
	disp.LoadFont(F_TEXT_M);
	uint8_t h_t = disp.GetTextHeight();


	uint8_t a_index = flight_data.altimeter[flags - 2].flags & 0x0F;

	char title[20];
	switch (a_type)
	{
	case(ALT_ABS_QNH1):
		sprintf_P(title, PSTR("Absolute to QNH1"));
	break;
	case(ALT_ABS_QNH2):
		sprintf_P(title, PSTR("Absolute to QNH2"));
	break;
	case(ALT_DIFF):
		sprintf_P(title, PSTR("Relative to Alt%d"), a_index);
	break;
	}

	gui_dialog(title);

	disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + 2 + h_v - h_t);
	fprintf(lcd_out, "ALT%d", flags);

	disp.GotoXY(GUI_DIALOG_LEFT, GUI_DIALOG_TOP + 2 + h_v + h_v - h_t);
	switch (a_type)
	{
	case(ALT_ABS_QNH1):
		fprintf(lcd_out, "QNH1");
	break;
	case(ALT_ABS_QNH2):
		fprintf(lcd_out, "QNH2");
	break;
	case(ALT_DIFF):
		fprintf(lcd_out, "delta");
	break;
	}

	disp.LoadFont(F_VALUES_L);
	char tmp[10];
	if (flags == 1)
		sprintf(tmp, "%0.0f", flight_data.altitude1);
	else
		sprintf(tmp, "%d", flight_data.altimeter[flags - 2].altitude);
	gui_raligh_text(tmp, GUI_DIALOG_RIGHT, GUI_DIALOG_TOP + 2);


	switch (a_type)
		{
		case(ALT_ABS_QNH1):
			sprintf(tmp, "%0.0f", flight_data.QNH1 / 10);
		break;
		case(ALT_ABS_QNH2):
			sprintf(tmp, "%0.0f", flight_data.QNH2 / 10);
		break;
		case(ALT_DIFF):
			sprintf(tmp, "%+d", flight_data.altimeter[flags - 2].delta);
		break;
	}
	gui_raligh_text(tmp, GUI_DIALOG_RIGHT, GUI_DIALOG_TOP + 2 + h_v);
}


register_widget3(w_alt1, "Altitude 1", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 1);
register_widget3(w_alt2, "Altitude 2", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 2);
register_widget3(w_alt3, "Altitude 3", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 3);
register_widget3(w_alt4, "Altitude 4", widget_alt_draw, widget_alt_menu_loop, widget_alt_menu_irqh, 4);

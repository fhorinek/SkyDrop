#include "gui_value.h"

char gui_value_label[20];
uint8_t gui_value_type;
uint8_t gui_value_index;
char gui_value_format[16];

float gui_value_min;
float gui_value_max;
float gui_value_step;
volatile float gui_value_tmp;

void_cb * gui_value_cb;

void gui_value_conf_P(const char * label, uint8_t type, const char * format, float start, float min, float max, float step, void_cb * cb)
{
	strcpy_P(gui_value_label, label);
	gui_value_type = type;
	strcpy_P(gui_value_format, format);

	gui_value_min = min;
	gui_value_max = max;
	gui_value_step = step;

	gui_value_tmp = start;
	gui_value_cb = cb;
	gui_value_index = 0;
}

void gui_value_init() {}


void gui_value_stop() {}

void gui_value_loop()
{
	char tmp[20];
	uint8_t f_h;

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;



	gui_dialog(gui_value_label);
	disp.LoadFont(F_TEXT_L);
	f_h = disp.GetTextHeight();

	switch(gui_value_type)
	{
		case(GUI_VAL_NUMBER):
			sprintf(tmp, gui_value_format, gui_value_tmp);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2);
		break;
		case(GUI_VAL_TIME):
			time_from_epoch(time_get_actual(), &sec, &min, &hour);

			sprintf_P(tmp, PSTR("%02d : %02d . %02d"), hour, min, sec);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2);
			if (gui_value_index == 0)
				disp.Invert(18, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 32, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 1)
				disp.Invert(36, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 50, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 2)
				disp.Invert(54, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 68, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
		break;
		case(GUI_VAL_DATE):
			datetime_from_epoch(time_get_actual(), &sec, &min, &hour, &day, &wday, &month, &year);

			sprintf_P(tmp, PSTR("%02d : %02d : %04d"), hour, min, sec);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h *1.5);
			sprintf_P(tmp, PSTR("%02d / %02d / %04d"), day, month, year);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2);
			if (gui_value_index == 0)
				disp.Invert(10, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 24, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 1)
				disp.Invert(30, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 44, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 2)
				disp.Invert(50, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 76, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
		break;
	}
}

void gui_value_number_irqh(uint8_t type, uint8_t * buff)
{
	float inc = 0;

	switch (type)
	{
	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
			inc += -gui_value_step;
		if (*buff == BE_DBL_CLICK)
			inc += -gui_value_step;
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
			inc += +gui_value_step;
		if (*buff == BE_DBL_CLICK)
			inc += +gui_value_step;
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
			gui_value_cb(gui_value_tmp);
	break;
	}

	gui_value_tmp += inc;

	if (gui_value_tmp > gui_value_max)
		gui_value_tmp = gui_value_max;

	if (gui_value_tmp < gui_value_min)
		gui_value_tmp = gui_value_min;
}

void gui_value_time_irqh(uint8_t type, uint8_t * buff)
{
	int16_t inc = 0;

	switch (type)
	{
	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
			inc += -gui_value_step;
		if (*buff == BE_DBL_CLICK)
			inc += -gui_value_step;
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
			inc += +gui_value_step;
		if (*buff == BE_DBL_CLICK)
			inc += +gui_value_step;
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
		{
			if (gui_value_index == 2)
				gui_value_cb(gui_value_tmp);
			else
				gui_value_index++;
		}
	break;
	}

	switch (gui_value_index)
	{
		case(0):
			time_set_actual(time_get_actual() + (60 * 60) * inc);
		break;
		case(1):
			time_set_actual(time_get_actual() + (60) * inc);
		break;
		case(2):
			time_set_actual(time_get_actual() + inc);
		break;
	}
}

void gui_value_date_irqh(uint8_t type, uint8_t * buff)
{
	int16_t inc = 0;

	switch (type)
	{
	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
			inc += -gui_value_step;
//		if (*buff == BE_DBL_CLICK)
//			inc += -gui_value_step;
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
			inc += +gui_value_step;
//		if (*buff == BE_DBL_CLICK)
//			inc += +gui_value_step;
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
		{
			if (gui_value_index == 2)
				gui_value_cb(gui_value_tmp);
			else
				gui_value_index++;
		}
	break;
	}

	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t wday;
	uint8_t month;
	uint16_t year;

	datetime_from_epoch(time_get_actual(), &sec, &min, &hour, &day, &wday, &month, &year);

	switch (gui_value_index)
	{
		case(0):
			day += inc;
			if (day < 1)
				day = monthDays[month - 1];

			if ((((!(year % 4)) && (year % 100) ) || (!(year % 400))) && month == 2)
			{
				if (day > 29)
					day = 29;
			}
			else
			{
				if (day > monthDays[month - 1])
					day = monthDays[month - 1];
			}
		break;

		case(1):
			month += inc;
			if (month < 1)
				month = 1;
			if (month > 12)
				month = 12;
		break;

		case(2):
			year += inc;
			if (year < 2015)
				year = 2015;
			if (year > 2100)
				year = 2100;
		break;
	}

	DEBUG("inc %d\n", inc);
	print_datetime();
	time_set_actual(datetime_to_epoch(sec, min, hour, day, month, year));
	print_datetime();
}

void gui_value_irqh(uint8_t type, uint8_t * buff)
{
	switch (gui_value_type)
	{
	case(GUI_VAL_NUMBER):
		gui_value_number_irqh(type, buff);
	break;

	case(GUI_VAL_TIME):
		gui_value_time_irqh(type, buff);
	break;

	case(GUI_VAL_DATE):
		gui_value_date_irqh(type, buff);
	break;
	}
}

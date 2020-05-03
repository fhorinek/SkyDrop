#include "gui_value.h"

#include "../drivers/audio/vario.h"
#include "../drivers/audio/sequencer.h"
#include "../fc/conf.h"

char gui_value_label[20];
uint8_t gui_value_type;
uint8_t gui_value_index;
char gui_value_format[16];

float gui_value_min;
float gui_value_max;
float gui_value_step;
volatile float gui_value_tmp;
float gui_value_mul;

float_cb * gui_value_cb;

MK_SEQ(audio_feedback, ARR({600}), ARR({253}));

void gui_value_conf_P(const char * label, uint8_t type, const char * format, float start, float min, float max, float step, float_cb * cb, float mul)
{
	strcpy_P(gui_value_label, label);
	gui_value_type = type;
	strcpy_P(gui_value_format, format);

	gui_value_mul = mul;

	gui_value_min = min * mul;
	gui_value_max = max * mul;
	gui_value_step = step * mul;

	gui_value_tmp = start * mul;
	gui_value_cb = cb;
	gui_value_index = 0;
}

void gui_value_draw_bar()
{
	uint8_t pad = 3;
	uint8_t bar_width = GUI_DIALOG_RIGHT - GUI_DIALOG_LEFT - pad * 2 + 1;
	uint8_t pos = ((gui_value_tmp - gui_value_min) * bar_width) / (gui_value_max - gui_value_min);

	disp.DrawRectangle(GUI_DIALOG_LEFT + pad - 1, GUI_DIALOG_TOP + 10, GUI_DIALOG_RIGHT - pad + 1, GUI_DIALOG_TOP + 20, 1, 0);
	disp.DrawRectangle(GUI_DIALOG_LEFT + pad - 1, GUI_DIALOG_TOP + 11, GUI_DIALOG_LEFT + pad + pos - 1, GUI_DIALOG_TOP + 19, 1, 1);
}

extern volatile float audio_vario_freq;
extern volatile uint16_t audio_vario_pause;
extern volatile uint16_t audio_vario_length;
extern volatile uint16_t audio_vario_prebeep_length;
extern volatile uint16_t audio_vario_prebeep_frequency;

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

		case(GUI_VAL_NUMBER_DISABLE):
			if (gui_value_tmp > 0)
				sprintf(tmp, gui_value_format, gui_value_tmp);
			else
				strcpy_P(tmp, PSTR("disabled"));
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2);
		break;

		case(GUI_VAL_VARIO_TEST):
			sprintf(tmp, gui_value_format, gui_value_tmp);
			gui_raligh_text(tmp, GUI_DIALOG_RIGHT - 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2);
			disp.LoadFont(F_TEXT_S);
			f_h = disp.GetTextHeight();
			disp.GotoXY(GUI_DIALOG_LEFT + 1, GUI_DIALOG_TOP + 2);
			fprintf(lcd_out, "f=%0.0f", (double)audio_vario_freq);
			disp.GotoXY(GUI_DIALOG_LEFT + 1, GUI_DIALOG_TOP + 4 + f_h);
			fprintf(lcd_out, "l=%4u", audio_vario_length / 31);
			disp.GotoXY(GUI_DIALOG_LEFT + 1, GUI_DIALOG_TOP + 6 + f_h * 2);
			fprintf(lcd_out, "p=%4u", audio_vario_pause / 31);
			disp.GotoXY(GUI_DIALOG_LEFT + 1, GUI_DIALOG_TOP + 8 + f_h * 3);
			fprintf(lcd_out, "pf=%4u pl=%4u", audio_vario_prebeep_frequency, audio_vario_prebeep_length / 31);
		break;

		case(GUI_VAL_SYSTEM_TIME):
			time_from_epoch(time_get_local(), &sec, &min, &hour);

			sprintf_P(tmp, PSTR("%02d : %02d . %02d"), hour, min, sec);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2 + 1);
			if (gui_value_index == 0)
				disp.Invert(16, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 30, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 1)
				disp.Invert(34, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 48, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 2)
				disp.Invert(52, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 66, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
		break;

		case(GUI_VAL_SYSTEM_DATE):
			datetime_from_epoch(time_get_local(), &sec, &min, &hour, &day, &wday, &month, &year);

			sprintf_P(tmp, PSTR("%02d / %02d / %04d"), day, month, year);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2 + 1);
			if (gui_value_index == 0)
				disp.Invert(8, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 22, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 1)
				disp.Invert(28, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 42, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 2)
				disp.Invert(48, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 74, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
		break;

		case(GUI_VAL_CONTRAST):
			gui_value_draw_bar();
		break;

		case(GUI_VAL_BRIGTHNES):
			gui_value_draw_bar();
		break;

		case(GUI_VAL_VOLUME):
			gui_value_draw_bar();
		break;

		case(GUI_VAL_TIME):
			hour = (0xFF00 & (uint16_t)gui_value_tmp) >> 8;
			min = (0x00FF & (uint16_t)gui_value_tmp) >> 0;

			sprintf_P(tmp, PSTR("%02d : %02d"), hour, min);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2 + 1);
			if (gui_value_index == 0)
				disp.Invert(26, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 39, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
			if (gui_value_index == 1)
				disp.Invert(43, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2, 57, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 + f_h / 2 - 2);
		break;
	}

	if (button_hold(B_LEFT))
	{
		uint8_t t_param = BE_CLICK;
		gui_value_irqh(TASK_IRQ_BUTTON_L, &t_param);
	}

	if (button_hold(B_RIGHT))
	{
		uint8_t t_param = BE_CLICK;
		gui_value_irqh(TASK_IRQ_BUTTON_R, &t_param);
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
			gui_value_cb(gui_value_tmp / gui_value_mul);
	break;
	}

	gui_value_tmp += inc;

	if (gui_value_tmp > gui_value_max)
		gui_value_tmp = gui_value_max;

	if (gui_value_tmp < gui_value_min)
		gui_value_tmp = gui_value_min;
}

void gui_value_system_time_irqh(uint8_t type, uint8_t * buff)
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
			time_set_local(time_get_local() + (60 * 60) * inc);
		break;
		case(1):
			time_set_local(time_get_local() + (60) * inc);
		break;
		case(2):
			time_set_local(time_get_local() + inc);
		break;
	}
}

void gui_value_system_date_irqh(uint8_t type, uint8_t * buff)
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

	datetime_from_epoch(time_get_local(), &sec, &min, &hour, &day, &wday, &month, &year);

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

	time_set_local(datetime_to_epoch(sec, min, hour, day, month, year));
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
			if (gui_value_index == 1)
				gui_value_cb(gui_value_tmp);
			else
				gui_value_index++;
		}
	break;
	}

	uint8_t hour = (0xFF00 & (uint16_t)gui_value_tmp) >> 8;
	uint8_t min = (0x00FF & (uint16_t)gui_value_tmp) >> 0;

	switch (gui_value_index)
	{
		case(0):
			hour = (hour + inc) % 24;
		break;
		case(1):
			min = (min + inc) % 60;
		break;
	}

	gui_value_tmp = (hour << 8) | min;
}

void gui_value_irqh(uint8_t type, uint8_t * buff)
{
	float tmp;

	switch (gui_value_type)
	{
	case(GUI_VAL_NUMBER):
	case(GUI_VAL_NUMBER_DISABLE):
		gui_value_number_irqh(type, buff);
	break;

	case(GUI_VAL_SYSTEM_TIME):
		gui_value_system_time_irqh(type, buff);
	break;

	case(GUI_VAL_SYSTEM_DATE):
		gui_value_system_date_irqh(type, buff);
	break;

	case(GUI_VAL_CONTRAST):
		gui_value_number_irqh(type, buff);
		config.gui.contrast = gui_value_tmp;
		gui_change_disp_cfg();
	break;

	case(GUI_VAL_BRIGTHNES):
		gui_value_number_irqh(type, buff);
		config.gui.brightness = gui_value_tmp;
	break;

	case(GUI_VAL_VARIO_TEST):
		gui_value_number_irqh(type, buff);
		gui_value_tmp = round(gui_value_tmp * 10) / 10;

		audio_demo_val = gui_value_tmp;
	break;

	case(GUI_VAL_VOLUME):
		tmp = gui_value_tmp;
		gui_value_number_irqh(type, buff);

		//only when there is a change in volume
		if (gui_value_tmp != tmp)
		{
			uint8_t vol = gui_value_tmp;
			seq_start(&audio_feedback, vol);
		}
	break;

	case(GUI_VAL_TIME):
		gui_value_time_irqh(type, buff);
	break;
	}

}

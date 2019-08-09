#include "gui_value.h"

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

MK_SEQ(audio_feedback, ARR({600}), ARR({750}));

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

void gui_value_init() {}

void gui_value_stop() {}

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

		case(GUI_VAL_CONTRAST):
		case(GUI_VAL_CONTRAST_MIN):
		case(GUI_VAL_CONTRAST_MAX):
			gui_value_draw_bar();
		break;

		case(GUI_VAL_BRIGTHNES):
			gui_value_draw_bar();
		break;

		case(GUI_VAL_VOLUME):
			gui_value_draw_bar();
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

extern uint8_t lcd_contrast_min;
extern uint8_t lcd_contrast_max;

void gui_value_irqh(uint8_t type, uint8_t * buff)
{
	float tmp;

	switch (gui_value_type)
	{
	case(GUI_VAL_NUMBER):
	case(GUI_VAL_NUMBER_DISABLE):
		gui_value_number_irqh(type, buff);
	break;

	case(GUI_VAL_CONTRAST):
		gui_value_number_irqh(type, buff);
		config.gui.contrast = gui_value_tmp;
		gui_change_disp_cfg();
	break;

	case(GUI_VAL_CONTRAST_MIN):
		gui_value_number_irqh(type, buff);
		lcd_contrast_min = gui_value_tmp;
		gui_change_disp_cfg();
	break;

	case(GUI_VAL_CONTRAST_MAX):
		gui_value_number_irqh(type, buff);
		lcd_contrast_max = gui_value_tmp;
		gui_change_disp_cfg();
	break;

	case(GUI_VAL_BRIGTHNES):
		gui_value_number_irqh(type, buff);
		config.gui.brightness = gui_value_tmp;
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
	}

}

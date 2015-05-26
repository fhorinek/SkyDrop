#include "gui_value.h"

char gui_value_label[20];
uint8_t gui_value_type;
char gui_value_format[16];

float gui_value_min;
float gui_value_max;
float gui_value_step;
volatile float gui_value_tmp;

void_cb * gui_value_cb;

void gui_value_conf(char * label, uint8_t type, char * format, float start, float min, float max, float step, void_cb * cb)
{
	strcpy(gui_value_label, label);
	gui_value_type = type;
	strcpy(gui_value_format, format);

	gui_value_min = min;
	gui_value_max = max;
	gui_value_step = step;

	gui_value_tmp = start;
	gui_value_cb = cb;
}

void gui_value_init() {}


void gui_value_stop() {}

void gui_value_loop()
{
	char tmp[20];
	uint8_t f_h;

	gui_dialog(gui_value_label);
	disp.LoadFont(F_TEXT_L);
	f_h = disp.GetTextHeight();

	switch(gui_value_type)
	{
		case(GUI_VAL_NUMBER):
			sprintf(tmp, gui_value_format, gui_value_tmp);
			gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + (GUI_DIALOG_BOTTOM - GUI_DIALOG_TOP) / 2 - f_h / 2);
		break;
	}
}

void gui_value_irqh(uint8_t type, uint8_t * buff)
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

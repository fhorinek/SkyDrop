#include "set_layout.h"
#include "../widgets/layouts/layouts.h"
#include "../gui_list.h"

uint8_t set_layout_widget_cnt;
uint8_t set_layout_type;

void gui_set_layout_init()
{
	set_layout_type = config.gui.pages[active_page].type;
}

void gui_set_layout_loop()
{
	uint8_t widget_cnt = layout_get_number_of_widgets(set_layout_type);

	for(uint8_t i = 0; i < widget_cnt; i++)
	{
		uint8_t x, y, w, h;
		layout_get_widget_rect(set_layout_type, i, &x, &y, &w, &h);

		disp.DrawRectangle(x, y, x + w - 2, y + h - 2, 1, 0);
		disp.LoadFont(F_VALUES_M);
		char tmp[2];
		sprintf_P(tmp, PSTR("%i"), i + 1);
		uint8_t t_h = disp.GetTextHeight();
		gui_caligh_text(tmp, x + w / 2, y + h / 2 - t_h / 2);
	}


}

void gui_set_layout_irqh(uint8_t type, uint8_t * buff)
{
	switch(type)
	{
	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
		{
			if (set_layout_type > 0)
				set_layout_type -= 1;
			else
				set_layout_type = NUMBER_OF_LAYOUTS - 1;
		}
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
		{
			set_layout_type = (set_layout_type + 1) % NUMBER_OF_LAYOUTS;
		}
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
		{
			config.gui.pages[active_page].type = set_layout_type;
			//store to EE
			
			ee_update_byte(&config_ee.gui.pages[active_page].type, set_layout_type);

			gui_switch_task(GUI_LAYOUTS);
		}

	if (*buff == BE_LONG)
	{
		//escape
		gui_switch_task(GUI_LAYOUTS);
	}

	break;
	}
}

#include "pages.h"
#include "widgets/widgets.h"

uint8_t active_page = 2;
uint8_t old_page = 2;

uint8_t active_widget = WIDGET_OFF;

extern layout_t pages[NUMBER_OF_PAGES];
extern widget widget_array[];

#define PAGE_IDLE			0
#define PAGE_CHANGE			1
#define PAGE_CHANGE_INFO	2
#define PAGE_WIDGET_SELECT	3
#define PAGE_WIDGET_MENU	4

#define PAGE_SWITCH_STEPS			15
#define PAGE_WIDGET_SELECT_DURATION	10
#define PAGE_WIDGET_MENU_DURATION	15

uint8_t page_state = PAGE_IDLE;
uint8_t page_state_step;
uint32_t page_state_end;

void gui_pages_init()
{
	widgets_init();
}

void gui_pages_stop()
{

}

void gui_enter_widget_menu()
{
	page_state = PAGE_WIDGET_MENU;
	page_state_end = task_get_ms_tick() + PAGE_WIDGET_MENU_DURATION * 1000;
}

void gui_exit_widget_menu()
{
	page_state = PAGE_WIDGET_SELECT;
	page_state_end = task_get_ms_tick() + PAGE_WIDGET_SELECT_DURATION * 1000;
}

void gui_pages_loop()
{
	uint8_t start_x;

	switch (page_state)
	{
	case(PAGE_IDLE):
		widgets_draw(active_page);
	break;

	case(PAGE_WIDGET_SELECT):
		//draw widgets
		widgets_draw(active_page);

		//Highlight selected widget
		uint8_t x, y, w, h;
		layout_get_widget_rect(pages[active_page].type, active_widget, &x, &y, &w, &h);


		disp.Invert(x, y, x + w - 1, y + h - 1);
		disp.InvertPixel(x, y);
		disp.InvertPixel(x + w - 1, y);
		disp.InvertPixel(x, y + h - 1);
		disp.InvertPixel(x + w - 1, y + h - 1);

		if (page_state_end < task_get_ms_tick())
			page_state = PAGE_IDLE;
	break;

	case(PAGE_WIDGET_MENU):
		widget_array[widget_get_type(active_page, active_widget)].menu_draw();

		if (page_state_end < task_get_ms_tick())
			page_state = PAGE_IDLE;
	break;

	case(PAGE_CHANGE):
		page_state_step--;

		int8_t split;
		if (old_page < active_page)
			split = (n5110_width / PAGE_SWITCH_STEPS) * page_state_step;
		else
			split = n5110_width - ((n5110_width / PAGE_SWITCH_STEPS) * page_state_step);

		disp.SetDrawLayer(1);

		disp.ClearBuffer();
		widgets_draw((old_page > active_page) ? active_page : old_page);
		disp.CopyToLayerX(0, split - n5110_width);

		disp.ClearBuffer();
		widgets_draw((old_page > active_page) ? old_page : active_page);
		disp.CopyToLayerX(0, split);

		start_x = ( -NUMBER_OF_PAGES * (8 + 4) + 4 + n5110_width) / 2;
		disp.ClearPart(4, start_x, 5, start_x + NUMBER_OF_PAGES * (8 + 4) + (8 + 4));

		for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++)
		{
			uint8_t x = start_x + i * 12;

			disp.DrawRectangle(x, 8 * 4, x + 8 - 1, 8 * 5 - 1, 1, (i == active_page) ? 1 : 0);
			disp.CopyToLayerPart(0, 4, x, 5, x + 8);
		}

		if (page_state_step == 0)
		{
			page_state_step = PAGE_SWITCH_STEPS;
			page_state = PAGE_CHANGE_INFO;
		}

		disp.SetDrawLayer(0);
	break;

	case(PAGE_CHANGE_INFO):
		widgets_draw(active_page);

		disp.SetDrawLayer(1);
		start_x = ( -NUMBER_OF_PAGES * (8 + 4) + 4 + n5110_width) / 2;
		disp.ClearPart(4, start_x, 5, start_x + NUMBER_OF_PAGES * (8 + 4) + (8 + 4));

		for (uint8_t i = 0; i < NUMBER_OF_PAGES; i++)
		{
			uint8_t x = start_x + i * 12;

			disp.DrawRectangle(x, 8 * 4, x + 8 - 1, 8 * 5 - 1, 1, (i == active_page) ? 1 : 0);
			disp.CopyToLayerPart(0, 4, x, 5, x + 8);
		}

		page_state_step--;
		if (page_state_step == 0)
		{
			page_state = PAGE_IDLE;
		}

		disp.SetDrawLayer(0);
	break;
	}
}

void page_switch(bool right)
{
	if (right)
	{
		if (active_page < NUMBER_OF_PAGES - 1)
		{
			old_page = active_page;
			active_page++;
			page_state = PAGE_CHANGE;
			page_state_step = PAGE_SWITCH_STEPS;
		}

	}
	else
	{
		if (active_page > 0)
		{
			old_page = active_page;
			active_page--;
			page_state = PAGE_CHANGE;
			page_state_step = PAGE_SWITCH_STEPS;
		}

	}
}

bool page_widgets_have_menu()
{
	uint8_t active_widgets = layout_get_number_of_widgets(pages[active_page].type);

	for (uint8_t i = 0; i < active_widgets; i++)
	{
		if (widget_array[widget_get_type(active_page, i)].menu_handler != NULL)
			return true;
	}

	return false;
}

void page_select_next_widget()
{
	uint8_t active_widgets = layout_get_number_of_widgets(pages[active_page].type);

	do
	{
		active_widget = (active_widget + 1) % active_widgets;
	}
	while (widget_array[widget_get_type(active_page, active_widget)].menu_handler == NULL);
}

void page_select_first_widget()
{
	uint8_t active_widgets = layout_get_number_of_widgets(pages[active_page].type);

	active_widget = 0;

	while (widget_array[widget_get_type(active_page, active_widget)].menu_handler == NULL)
	{
		active_widget = (active_widget + 1) % active_widgets;
	}

}


void gui_pages_idle_irqh(uint8_t type, uint8_t * buff)
{
	switch(type)
	{
	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
		{
			page_switch(false);
		}
	break;

	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
		{
			page_switch(true);
		}
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
		{
			if (page_widgets_have_menu())
			{
				page_select_first_widget();

				page_state = PAGE_WIDGET_SELECT;
				page_state_end = task_get_ms_tick() + PAGE_WIDGET_SELECT_DURATION * 1000;
			}
		}
	break;
	}

}

void gui_pages_select_irqh(uint8_t type, uint8_t * buff)
{
	switch(type)
	{
	case(TASK_IRQ_BUTTON_L):
		widget_array[widget_get_type(active_page, active_widget)].menu_handler(type, buff);
	break;

	case(TASK_IRQ_BUTTON_R):
		widget_array[widget_get_type(active_page, active_widget)].menu_handler(type, buff);
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
		{
			page_select_next_widget();
			page_state = PAGE_WIDGET_SELECT;
			page_state_end = task_get_ms_tick() + PAGE_WIDGET_SELECT_DURATION * 1000;
		}
	break;
	}
}

void gui_pages_irqh(uint8_t type, uint8_t * buff)
{
	switch (page_state)
	{
	case(PAGE_IDLE):
		gui_pages_idle_irqh(type, buff);
	break;

	case(PAGE_WIDGET_SELECT):
		gui_pages_select_irqh(type, buff);
	break;

	case(PAGE_WIDGET_MENU):
		widget_array[widget_get_type(active_page, active_widget)].menu_handler(type, buff);
	break;
	}
}

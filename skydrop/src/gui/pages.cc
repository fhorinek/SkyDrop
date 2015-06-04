#include "pages.h"
#include "splash.h"
#include "widgets/widgets.h"

uint8_t active_page = 2;
uint8_t old_page = 2;

uint8_t active_widget;


#define PAGE_IDLE			0
#define PAGE_CHANGE			1
#define PAGE_CHANGE_INFO	2
#define PAGE_WIDGET_SELECT	3
#define PAGE_WIDGET_MENU	4
#define PAGE_MENU			5

#define PAGE_SWITCH_STEPS			7

#define PAGE_WIDGET_SELECT_DURATION	10
#define PAGE_WIDGET_MENU_DURATION	15

#define PAGE_MENU_STEPS				5
#define PAGE_MENU_HEIGHT			16
#define PAGE_MENU_POWEROFF_BLIK		3000
#define PAGE_MENU_POWEROFF			5000

#define NORMAL	1
#define REVERSE	0

uint8_t page_state;
uint8_t page_state_step; //step based animation
uint8_t page_state_dir;  //direction
uint32_t page_state_timer; //timer based timeout

uint8_t page_change_dir;

const uint8_t PROGMEM img_pwr[26]={12,16,224,248,28,12,0,63,63,0,12,28,248,224,1,7,14,12,24,24,24,24,12,14,7,1};
const uint8_t PROGMEM img_menu[22]={10,16,51,51,51,51,51,51,51,51,51,51,3,3,3,3,3,3,3,3,3,3};
const uint8_t PROGMEM img_layout[16]={14,8,252,132,132,191,161,225,33,33,225,161,191,132,132,252};

void gui_pages_init()
{
	widgets_init();
	page_state = PAGE_IDLE;
	active_widget = WIDGET_OFF;
}

void gui_pages_stop() {}

//returns true if this is a first time
bool gui_enter_widget_menu()
{
	uint8_t old_state = page_state;

	page_state = PAGE_WIDGET_MENU;
	page_state_timer = task_get_ms_tick() + PAGE_WIDGET_MENU_DURATION * 1000;

	return (old_state != page_state);
}

void gui_exit_widget_menu()
{
	page_state = PAGE_WIDGET_SELECT;
	page_state_timer = task_get_ms_tick() + PAGE_WIDGET_SELECT_DURATION * 1000;
}

void gui_pages_loop()
{
	uint8_t start_x;
	uint8_t wtype;

	switch (page_state)
	{
	case(PAGE_IDLE):
		widgets_draw(active_page);
		gui_statusbar();
	break;

	case(PAGE_WIDGET_SELECT):
		//draw widgets
		widgets_draw(active_page);
		gui_statusbar();

		//Highlight selected widget
		uint8_t x, y, w, h;
		layout_get_widget_rect(pages[active_page].type, active_widget, &x, &y, &w, &h);


		disp.Invert(x, y, x + w - 1, y + h - 1);
		disp.InvertPixel(x, y);
		disp.InvertPixel(x + w - 1, y);
		disp.InvertPixel(x, y + h - 1);
		disp.InvertPixel(x + w - 1, y + h - 1);

		if (page_state_timer < task_get_ms_tick())
			page_state = PAGE_IDLE;
	break;

	case(PAGE_WIDGET_MENU):
		wtype = widget_get_type(active_page, active_widget);
		widget_array[wtype].menu_loop(widget_array[wtype].flags);

		if (page_state_timer < task_get_ms_tick())
			page_state = PAGE_IDLE;
	break;

	case(PAGE_CHANGE):
		page_state_step--;

		int8_t split;
		if (page_change_dir)
			split = (n5110_width / PAGE_SWITCH_STEPS) * page_state_step;
		else
			split = n5110_width - ((n5110_width / PAGE_SWITCH_STEPS) * page_state_step);

		disp.SetDrawLayer(1);

		disp.ClearBuffer();
		widgets_draw((!page_change_dir) ? active_page : old_page);
		gui_statusbar();
		disp.CopyToLayerX(0, split - n5110_width);

		disp.ClearBuffer();
		widgets_draw((!page_change_dir) ? old_page : active_page);
		gui_statusbar();
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
		gui_statusbar();

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

	case(PAGE_MENU):
		widgets_draw(active_page);
		gui_statusbar();

		uint8_t top = GUI_DISP_HEIGHT - PAGE_MENU_HEIGHT + (PAGE_MENU_HEIGHT * ((float)page_state_step/PAGE_MENU_STEPS));

		if (page_state_dir == NORMAL)
		{
			if (page_state_step)
				page_state_step--;
		}
		else
		{
			if (page_state_step < PAGE_MENU_STEPS)
				page_state_step++;
			else
				page_state = PAGE_IDLE;
		}

		disp.DrawRectangle(1, top + 1, GUI_DISP_WIDTH - 2, GUI_DISP_HEIGHT - 1, 0, 1);
		disp.DrawLine(1, top, GUI_DISP_WIDTH - 2, top, 1);
		disp.DrawLine(0, GUI_DISP_HEIGHT - 1, 0, top + 1, 1);
		disp.DrawLine(GUI_DISP_WIDTH - 1, GUI_DISP_HEIGHT - 1, GUI_DISP_WIDTH - 1, top + 1, 1);

		//pwr
		if ((page_state_timer - task_get_ms_tick() > PAGE_MENU_POWEROFF_BLIK) || (task_get_ms_tick() % 200 > 100))
			disp.DrawImage(img_pwr, 36, top + 2);

		//menu
		disp.DrawImage(img_menu, 69, top + 4);

		//layout
		disp.DrawImage(img_layout, 3, top + 4);

		if (page_state_timer < task_get_ms_tick())
		{
			gui_splash_set_mode(SPLASH_OFF);
			gui_switch_task(GUI_SPLASH);
		}

	break;


	}
}

void page_switch(bool right)
{
	old_page = active_page;

	if (right)
	{
		active_page = (active_page + 1) % NUMBER_OF_PAGES;
		page_change_dir = 1;
	}
	else
	{
		if (active_page == 0)
			active_page = NUMBER_OF_PAGES - 1;
		else
			active_page = active_page - 1;
		page_change_dir = 0;
	}

	page_state = PAGE_CHANGE;
	page_state_step = PAGE_SWITCH_STEPS;
}

bool page_widgets_have_menu()
{
	uint8_t active_widgets = layout_get_number_of_widgets(pages[active_page].type);

	for (uint8_t i = 0; i < active_widgets; i++)
	{
		if (widget_array[widget_get_type(active_page, i)].menu_irqh != NULL)
			return true;
	}

	return false;
}

bool page_select_next_widget()
{
	uint8_t active_widgets = layout_get_number_of_widgets(pages[active_page].type);

	do
	{
		active_widget++;
		if (active_widget > active_widgets - 1)
			return false;
	}
	while (widget_array[widget_get_type(active_page, active_widget)].menu_irqh == NULL);

	return true;
}

void page_select_first_widget()
{
	uint8_t active_widgets = layout_get_number_of_widgets(pages[active_page].type);

	active_widget = 0;

	while (widget_array[widget_get_type(active_page, active_widget)].menu_irqh == NULL)
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
				page_state_timer = task_get_ms_tick() + PAGE_WIDGET_SELECT_DURATION * 1000;
			}
		}
	break;
	}

}

void gui_pages_select_irqh(uint8_t type, uint8_t * buff)
{
	uint8_t wtype = widget_get_type(active_page, active_widget);

	switch(type)
	{
	case(TASK_IRQ_BUTTON_L):
		widget_array[wtype].menu_irqh(type, buff, widget_array[wtype].flags);
	break;

	case(TASK_IRQ_BUTTON_R):
		widget_array[wtype].menu_irqh(type, buff, widget_array[wtype].flags);
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
		{
			if (page_select_next_widget())
			{
				page_state = PAGE_WIDGET_SELECT;
				page_state_timer = task_get_ms_tick() + PAGE_WIDGET_SELECT_DURATION * 1000;
			}
			else
				page_state = PAGE_IDLE;
		}
		else
			widget_array[wtype].menu_irqh(type, buff, widget_array[wtype].flags);
	break;
	}
}

void gui_page_menu_irqh(uint8_t type, uint8_t * buff)
{
	if (type == TASK_IRQ_BUTTON_R && *buff == BE_HOLD)
	{
		gui_switch_task(GUI_SETTINGS);
	}

	if (type == TASK_IRQ_BUTTON_L && *buff == BE_HOLD)
	{
		gui_switch_task(GUI_LAYOUTS);
	}


	if (type == TASK_IRQ_BUTTON_M && *buff == BE_RELEASED)
	{
		page_state_dir = REVERSE;
	}

}

void gui_pages_irqh(uint8_t type, uint8_t * buff)
{
	uint8_t wtype;

	switch (page_state)
	{
	case(PAGE_IDLE):
	case(PAGE_CHANGE_INFO):
		if (type == TASK_IRQ_BUTTON_M && *buff == BE_LONG)
		{
			page_state = PAGE_MENU;
			page_state_step = PAGE_MENU_STEPS;
			page_state_dir = NORMAL;
			page_state_timer = task_get_ms_tick() + PAGE_MENU_POWEROFF;
		}
		else
			gui_pages_idle_irqh(type, buff);
	break;

	case(PAGE_WIDGET_SELECT):
		gui_pages_select_irqh(type, buff);
	break;

	case(PAGE_WIDGET_MENU):
		wtype = widget_get_type(active_page, active_widget);
		widget_array[wtype].menu_irqh(type, buff, widget_array[wtype].flags);
	break;

	case(PAGE_MENU):
		gui_page_menu_irqh(type, buff);
	break;
	}
}

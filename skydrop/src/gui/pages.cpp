#include "pages.h"
#include "splash.h"
#include "widgets/widgets.h"

#include "gui_dialog.h"
#include "gui_list.h"
#include "gui_value.h"

#include "../drivers/audio/sequencer.h"

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
#define PAGE_INFO_STEPS				20

#define PAGE_WIDGET_SELECT_DURATION	10
#define PAGE_WIDGET_MENU_DURATION	15
#define PAGE_CYCLE_DURATION			10

#define PAGE_MENU_STEPS				5
#define PAGE_MENU_WAIT				3000
#define PAGE_MENU_HEIGHT			16
#define PAGE_MENU_POWEROFF_BLIK		2000
#define PAGE_MENU_POWEROFF			3000

#define NORMAL	1
#define REVERSE	0
#define WAIT	2
#define HOLD	3

uint8_t page_state;
uint8_t page_state_step; //step based animation
uint8_t page_state_dir;  //direction
uint32_t page_state_timer; //timer based timeout
uint32_t page_next_cycle_timer = 0;   // The time, when the next cycle should be done

uint8_t page_mode = PAGE_NONE;

uint8_t page_change_dir;

const uint8_t PROGMEM img_pwr[26]={12,16,224,248,28,12,0,63,63,0,12,28,248,224,1,7,14,12,24,24,24,24,12,14,7,1};
const uint8_t PROGMEM img_menu[22]={10,16,51,51,51,51,51,51,51,51,51,51,3,3,3,3,3,3,3,3,3,3};
const uint8_t PROGMEM img_layout[16]={14,8,252,132,132,191,161,225,33,33,225,161,191,132,132,252};

MK_SEQ(snd_page_0, ARR({800}), ARR({100}));
MK_SEQ(snd_page_1, ARR({800, 1000}), ARR({100, 100}));
MK_SEQ(snd_page_2, ARR({800, 1000, 1200}), ARR({100, 100, 100}));
MK_SEQ(snd_page_3, ARR({800, 1000, 1200, 1400}), ARR({100, 100, 100, 100}));
MK_SEQ(snd_page_4, ARR({800, 1000, 1200, 1400, 1600}), ARR({100, 100, 100, 100, 100, 100}));
MK_SEQ(snd_page_5, ARR({800, 1000, 1200, 1400, 1600, 1800}), ARR({100, 100, 100, 100, 100, 100, 100}));
MK_SEQ(snd_page_6, ARR({800, 1000, 1200, 1400, 1600, 1800, 2000}), ARR({100, 100, 100, 100, 100, 100, 100, 100}));
MK_SEQ(snd_page_7, ARR({800, 1000, 1200, 1400, 1600, 1800, 2000, 2200}), ARR({100, 100, 100, 100, 100, 100, 100, 100, 100}));

const sequence_t * snd_pages[] = {&snd_page_0, &snd_page_1, &snd_page_2, &snd_page_3, &snd_page_4, &snd_page_5, &snd_page_6, &snd_page_7};

void gui_page_set_mode(uint8_t new_mode)
{
    //still same mode
    if (new_mode == page_mode)
        return;

    DEBUG("mode %d new, old %d\n", new_mode, page_mode);

    page_mode = new_mode;

    uint8_t new_page = config.gui.page_mode[new_mode];
    if (new_page == PAGE_NONE)
        return;

    DEBUG("new_page %d\n", new_page);
    page_set(new_page);
}

void gui_pages_set_date_manual_cb(float ret)
{
	time_set_flags();
	gui_switch_task(GUI_PAGES);
}

void gui_pages_set_time_manual_cb(float ret)
{
	gui_value_conf_P(PSTR("Date"), GUI_VAL_DATE, PSTR(""), 0, 0, 0, 1, gui_pages_set_date_manual_cb);
	gui_switch_task(GUI_SET_VAL);
}

void gui_pages_set_time_cb(uint8_t ret)
{
	if (ret == GUI_DIALOG_SET)
	{
		gui_switch_task(GUI_PAGES);
		config.system.time_flags |= TIME_SYNC;
		time_wait_for_gps();

		if (!config.connectivity.use_gps)
		{
			config.connectivity.use_gps = true;
			gps_start();
			gui_showmessage_P(PSTR("Enabling GPS"));
		}
	}
	else
	{
		gui_value_conf_P(PSTR("Time"), GUI_VAL_TIME, PSTR(""), 0, 0, 0, 1, gui_pages_set_time_manual_cb);
		gui_switch_task(GUI_SET_VAL);
	}
}

void gui_pages_init()
{
	page_state_step = PAGE_INFO_STEPS;
	page_state = PAGE_IDLE;
	active_widget = WIDGET_OFF;

	if (time_need_set())
	{
		time_set_default();

		gui_dialog_set_P(PSTR("Clock"), PSTR("Time/Date not set.\nSet time manually\nor wait for GPS."), GUI_STYLE_TIMESET, gui_pages_set_time_cb);
		gui_switch_task(GUI_DIALOG);
	}
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

void gui_page_power_off()
{
	gui_splash_set_mode(SPLASH_OFF);
	gui_switch_task(GUI_SPLASH);
	eeprom_busy_wait();
	eeprom_update_byte(&config_ee.gui.last_page, active_page);
}

void gui_pages_loop()
{
	uint8_t start_x;
	uint8_t box_spacing;
	uint8_t wtype;


	switch (page_state)
	{
	case(PAGE_IDLE):
		#ifdef FAKE_ENABLE
			FAKE_DATA
		#endif

		if (config.gui.disp_flags & CFG_DISP_CYCLE) {
			uint32_t now = task_get_ms_tick();
			if ( now > page_next_cycle_timer ) {
				page_next_cycle_timer = now + PAGE_CYCLE_DURATION * 1000;
				page_switch(true);
			}
		}

		widgets_draw(active_page);
		gui_statusbar();
	break;

	case(PAGE_WIDGET_SELECT):
		//draw widgets
		widgets_draw(active_page);
		gui_statusbar();

		//Highlight selected widget
		uint8_t x, y, w, h;
		layout_get_widget_rect(config.gui.pages[active_page].type, active_widget, &x, &y, &w, &h);


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
			split = (GUI_DISP_WIDTH / PAGE_SWITCH_STEPS) * page_state_step;
		else
			split = GUI_DISP_WIDTH - ((GUI_DISP_WIDTH / PAGE_SWITCH_STEPS) * page_state_step);

		disp.SetDrawLayer(1);

		disp.ClearBuffer();
		widgets_draw((!page_change_dir) ? active_page : old_page);
		gui_statusbar();
		disp.CopyToLayerX(0, split - GUI_DISP_WIDTH);

		disp.ClearBuffer();
		widgets_draw((!page_change_dir) ? old_page : active_page);
		gui_statusbar();
		disp.CopyToLayerX(0, split);

		box_spacing = (config.gui.number_of_pages == 8) ? 2 : 4;
		start_x = (- config.gui.number_of_pages * (8 + box_spacing) + box_spacing + GUI_DISP_WIDTH) / 2;

		for (uint8_t i = 0; i < config.gui.number_of_pages; i++)
		{
			uint8_t x = start_x + i * (8 + box_spacing);

			if (i == active_page)
			{
				disp.DrawRectangle(x, 8 * 4, x + 8 - 1, 8 * 5 - 1, 1, 1);
			}
			else
			{
				disp.DrawRectangle(x, 8 * 4, x + 8 - 1, 8 * 5 - 1, 1, 0);
				disp.DrawRectangle(x + 1, 8 * 4 + 1, x + 8 - 2, 8 * 5 - 2, 0, 1);
			}
			disp.CopyToLayerPart(0, 4, x, 5, x + 8);
		}

		if (page_state_step == 0)
		{
			page_state_step = PAGE_INFO_STEPS;
			page_state = PAGE_CHANGE_INFO;
		}

		disp.SetDrawLayer(0);
	break;

	case(PAGE_CHANGE_INFO):
		widgets_draw(active_page);
		gui_statusbar();

		disp.SetDrawLayer(1);

		box_spacing = (config.gui.number_of_pages == 8) ? 2 : 4;
		start_x = ( - config.gui.number_of_pages * (8 + box_spacing) + box_spacing + GUI_DISP_WIDTH) / 2;

		for (uint8_t i = 0; i <  config.gui.number_of_pages; i++)
		{
			uint8_t x = start_x + i * (8 + box_spacing);

			if (i == active_page)
			{
				disp.DrawRectangle(x, 8 * 4, x + 8 - 1, 8 * 5 - 1, 1, 1);
			}
			else
			{
				disp.DrawRectangle(x, 8 * 4, x + 8 - 1, 8 * 5 - 1, 1, 0);
				disp.DrawRectangle(x + 1, 8 * 4 + 1, x + 8 - 2, 8 * 5 - 2, 0, 1);
			}

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

		switch (page_state_dir)
		{
		case (NORMAL):
			if (page_state_step > 0)
				page_state_step--;
			else
			{
				page_state_dir = HOLD;
				page_state_timer = task_get_ms_tick() + PAGE_MENU_POWEROFF;
			}
		break;

		case (HOLD):
			if (page_state_timer < task_get_ms_tick())
			{
				gui_page_power_off();
			}
		break;

		case (WAIT):
			if (task_get_ms_tick() > page_state_timer)
				page_state_dir = REVERSE;
		break;

		case (REVERSE):
			if (page_state_step < PAGE_MENU_STEPS)
				page_state_step++;
			else
				page_state = PAGE_IDLE;
		break;
		}

		disp.DrawRectangle(1, top + 1, GUI_DISP_WIDTH - 2, GUI_DISP_HEIGHT - 1, 0, 1);
		disp.DrawLine(1, top, GUI_DISP_WIDTH - 2, top, 1);
		disp.DrawLine(0, GUI_DISP_HEIGHT - 1, 0, top + 1, 1);
		disp.DrawLine(GUI_DISP_WIDTH - 1, GUI_DISP_HEIGHT - 1, GUI_DISP_WIDTH - 1, top + 1, 1);

		//pwr
		if ((page_state_timer - task_get_ms_tick() > PAGE_MENU_POWEROFF_BLIK) || GUI_BLINK_TGL(200) || page_state_dir != HOLD)
			disp.DrawImage(img_pwr, 36, top + 2);

		//menu
		disp.DrawImage(img_menu, 69, top + 4);

		//layout
		disp.DrawImage(img_layout, 3, top + 4);
	break;


	}
}

void page_set(uint8_t new_page)
{
    //page out of range
    if (config.gui.number_of_pages <= new_page)
          return;

    old_page = active_page;
    page_change_dir = old_page < new_page;
    active_page = new_page;

    if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_PAGES)
    {
        seq_start(snd_pages[active_page], config.gui.alert_volume);
    }

    if (config.gui.disp_flags & CFG_DISP_ANIM)
    {
        page_state_step = PAGE_SWITCH_STEPS;
        page_state = PAGE_CHANGE;
    }
    else
    {
        page_state_step = PAGE_INFO_STEPS;
        page_state = PAGE_CHANGE_INFO;
    }

}

void page_switch(bool right)
{
	old_page = active_page;

	if (config.gui.number_of_pages == 1)
		return;

	if (right)
	{
		active_page = (active_page + 1) % config.gui.number_of_pages;
		page_change_dir = 1;
	}
	else
	{
		if (active_page == 0)
			active_page = config.gui.number_of_pages - 1;
		else
			active_page = active_page - 1;
		page_change_dir = 0;
	}

	if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_PAGES)
	{
		seq_start(snd_pages[active_page], config.gui.alert_volume);
	}

	if (config.gui.disp_flags & CFG_DISP_ANIM)
	{
		page_state_step = PAGE_SWITCH_STEPS;
		page_state = PAGE_CHANGE;
	}
	else
	{
		page_state_step = PAGE_INFO_STEPS;
		page_state = PAGE_CHANGE_INFO;
	}


}

bool page_widgets_have_menu()
{
	uint8_t active_widgets = layout_get_number_of_widgets(config.gui.pages[active_page].type);

	for (uint8_t i = 0; i < active_widgets; i++)
	{
		if (widget_array[widget_get_type(active_page, i)].menu_irqh != NULL)
			return true;
	}

	return false;
}

bool page_select_next_widget()
{
	uint8_t active_widgets = layout_get_number_of_widgets(config.gui.pages[active_page].type);

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
	uint8_t active_widgets = layout_get_number_of_widgets(config.gui.pages[active_page].type);

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

	if (type == TASK_IRQ_BUTTON_M && *buff == BE_HOLD)
	{
		gui_page_power_off();
	}

	if (type == TASK_IRQ_BUTTON_M && *buff == BE_RELEASED)
	{
		if (page_state_step > 0)
			page_state_dir = REVERSE;
		else
		{
			page_state_dir = WAIT;
			page_state_timer = task_get_ms_tick() + PAGE_MENU_WAIT;
		}
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

#include "gui_list.h"
#include "gui.h"

gui_list_gen * gui_list_gen_f = NULL;
gui_list_act * gui_list_act_f = NULL;

uint8_t gui_list_size = 0;
uint8_t gui_list_back = 0;
int16_t gui_list_y_offset = 0;
uint8_t gui_list_index[NUMBER_OF_GUI_TASKS];
uint32_t gui_list_middle_hold;

#include "../../drivers/audio/sequencer.h"
MK_SEQ(snd_menu_exit, ARR({800}), ARR({200}));

void gui_list_draw()
{
	char tmp_text[64];
	char tmp_sub_text[32];

	uint8_t flags;

	disp.LoadFont(F_TEXT_M);
	uint8_t t_h = disp.GetTextHeight();

	int16_t y = gui_list_y_offset;
	int8_t height;
	int16_t total_height = 0;
	uint8_t sub_height;

	//emulate middle click
	if (button_in_reset(B_MIDDLE))
	{
		if (task_get_ms_tick() - gui_list_middle_hold > BUTTON_LONG)
		{
			gui_switch_task(gui_list_back);
			if (config.gui.menu_audio_flags & CFG_AUDIO_MENU_BUTTONS && gui_buttons_override == false)
				seq_start(&snd_menu_exit, config.gui.menu_volume);
		}
	}
	else
		gui_list_middle_hold = task_get_ms_tick();

	for (uint8_t i = 0; i < gui_list_size; i++)
	{
		height = 1 + t_h;

		flags = 0;
		if (i < gui_list_size - 1)
		{
			gui_list_gen_f(i, tmp_text, &flags, tmp_sub_text);
		}
		else
		{
			strcpy_P(tmp_text, PSTR("back"));
			flags = GUI_LIST_BACK;
		}

		uint8_t x_val = 5;

		switch(flags & GUI_LIST_T_MASK)
		{
			case(GUI_LIST_FOLDER):
				x_val = 5;
			break;

			case(GUI_LIST_BACK):
				x_val = 2;
			break;
		}

		if ((flags & GUI_LIST_T_MASK) == GUI_LIST_SUB_TEXT)
		{
			sub_height = disp.GetTextHeight();
			height += sub_height;
		}

		if (gui_list_index[gui_task] == i)
		{
			if (y < 0)
				gui_list_y_offset = -total_height;

			if (y > GUI_DISP_HEIGHT - height)
				gui_list_y_offset = -total_height + GUI_DISP_HEIGHT - height;
		}

		if (y > GUI_DISP_HEIGHT)
			continue;

		disp.GotoXY(x_val, y + 1);
		fprintf_P(lcd_out, PSTR("%s"), tmp_text);

		//sub text
		if ((flags & GUI_LIST_T_MASK) == GUI_LIST_SUB_TEXT)
			gui_raligh_text(tmp_sub_text, GUI_DISP_WIDTH - 3, y + t_h + 1);

		//tick
		if ((flags & GUI_LIST_T_MASK) == GUI_LIST_CHECK_OFF || (flags & GUI_LIST_T_MASK) == GUI_LIST_CHECK_ON)
		{
			disp.DrawRectangle(GUI_DISP_WIDTH - 9, y + 1, GUI_DISP_WIDTH - 3, y + 7, 1, 0);
			if ((flags & GUI_LIST_T_MASK) == GUI_LIST_CHECK_ON)
			{
				disp.DrawLine(GUI_DISP_WIDTH - 8, y + 5, GUI_DISP_WIDTH - 7, y + 6, 1);
				disp.DrawLine(GUI_DISP_WIDTH - 6, y + 5, GUI_DISP_WIDTH - 4, y + 3, 1);
			}
		}

		if (flags & GUI_LIST_DISABLED)
		{
			for (uint8_t cx = 0; cx < GUI_DISP_WIDTH - 1; cx++)
				for (uint8_t cy = y + cx % 2 + 1; cy < y + t_h; cy += 2)
					disp.PutPixel(cx, cy, 0);
		}

		if (gui_list_index[gui_task] == i)
		{
			disp.Invert(0, y, GUI_DISP_WIDTH - 1, y + height - 1);
			disp.PutPixel(0, y, 0);
			disp.PutPixel(GUI_DISP_WIDTH - 1, y, 0);
			disp.PutPixel(GUI_DISP_WIDTH - 1, y + height - 1, 0);
			disp.PutPixel(0, y + height - 1, 0);
		}

		y += height;
		total_height += height;
	}
}



void gui_list_set(gui_list_gen * f_ptr, gui_list_act * f_act, uint8_t size, uint8_t back)
{
	gui_list_gen_f = f_ptr;
	gui_list_act_f = f_act;
	gui_list_size = size + 1;
	gui_list_back = back;
	gui_list_y_offset = 0;

	gui_list_middle_hold = task_get_ms_tick();
}

void gui_list_set_index(uint8_t task, uint8_t index)
{
	gui_list_index[task] = index;
}

void gui_list_moveup()
{
	if (gui_list_index[gui_task] > 0)
		gui_list_index[gui_task]--;
	else
		gui_list_index[gui_task] = gui_list_size - 1;
}

void gui_list_movedown()
{
	if (gui_list_index[gui_task] < gui_list_size - 1)
		gui_list_index[gui_task]++;
	else
		gui_list_index[gui_task] = 0;
}

void gui_list_action()
{
	if (gui_list_index[gui_task] == gui_list_size - 1)
	{
		gui_list_set_index(gui_task, 0);
		gui_switch_task(gui_list_back);
	}
	else
		gui_list_act_f(gui_list_index[gui_task]);
}

void gui_list_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
	case(TASK_IRQ_BUTTON_R):
		if (*buff == BE_CLICK)
			gui_list_movedown();
		if (*buff == BE_DBL_CLICK)
			gui_list_movedown();
	break;

	case(TASK_IRQ_BUTTON_L):
		if (*buff == BE_CLICK)
			gui_list_moveup();
		if (*buff == BE_DBL_CLICK)
			gui_list_moveup();
	break;

	case(TASK_IRQ_BUTTON_M):
		if (*buff == BE_CLICK)
			gui_list_action();
		if (*buff == BE_LONG)
			gui_switch_task(gui_list_back);
	break;
	}
}

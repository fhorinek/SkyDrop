#include "gui_list.h"
#include "gui.h"

gui_list_gen * gui_list_gen_f = NULL;
uint8_t gui_list_size = 0;
uint8_t gui_list_index = 0;
uint8_t gui_list_top = 0;

#define ITEMS_PER_PAGE	4

void gui_list_draw()
{
	char tmp[22];
	uint8_t disabled;

	disp.LoadFont(F_TEXT_M);
	uint8_t t_h = disp.GetTextHeight();

	uint8_t i = 0;
	for (uint8_t y = 0; y < GUI_DISP_HEIGHT; y += t_h)
	{
		disp.GotoXY(2, y + 1);

		DEBUG("%d, %d, %d, %d\n", i, y, gui_list_top, gui_list_size);

		if (i + gui_list_top == gui_list_size)
			return;

		gui_list_gen_f(i + gui_list_top, tmp, &disabled);
		fprintf_P(lcd_out, PSTR("%s"), tmp);
//		if (disabled)
//		{
//			uint8_t sta = 0;
//			if (tmp[0] == ' ')
//				sta = FONT_W;
//			disp.DrawLine(sta, (2 + i) * 8 + 3, strlen(tmp) * FONT_W - 1, (2 + i) * 8 + 3, 1);
//		}

		if (gui_list_index == i + gui_list_top)
		{
			disp.Invert(0, y, GUI_DISP_WIDTH - 1, y + t_h);
			disp.PutPixel(0, y, 0);
			disp.PutPixel(GUI_DISP_WIDTH - 1, y, 0);
			disp.PutPixel(GUI_DISP_WIDTH - 1, y + t_h, 0);
			disp.PutPixel(0, y + t_h, 0);
		}
		i++;
	}
}



void gui_list_set(gui_list_gen * f_ptr, uint8_t size)
{
	gui_list_gen_f = f_ptr;
	gui_list_size = size;
	gui_list_index = 0;
	gui_list_top = 0;
}

void gui_list_moveup()
{
	if (gui_list_index > 0)
		gui_list_index--;

	if (gui_list_index < gui_list_top)
		gui_list_top = gui_list_index;
}

void gui_list_movedown()
{
	if (gui_list_index < gui_list_size - 1)
		gui_list_index++;

	if (gui_list_index > gui_list_top + ITEMS_PER_PAGE)
		gui_list_top = gui_list_index - ITEMS_PER_PAGE;
}

#include "gui.h"
#include "../drivers/buzzer.h"
#include "widgets/widgets.h"

#include "pages.h"

n5110display disp;
CreateStdOut(lcd_out, disp.Write);

volatile uint8_t gui_task = GUI_NONE;
volatile uint8_t gui_new_task = GUI_PAGES;

void (* gui_init_array[])() =
{gui_pages_init};

void (* gui_stop_array[])() =
{gui_pages_stop};

void (* gui_loop_array[])() =
{gui_pages_loop};

void (* gui_irqh_array[])(uint8_t type, uint8_t * buff) =
{gui_pages_irqh};

void gui_switch_task(uint8_t new_task)
{
	gui_new_task = new_task;
}

uint8_t lcd_brightness;
uint8_t lcd_brightness_timeout;
uint32_t lcd_brightness_end = 0;
uint8_t lcd_contrast;

void gui_trigger_backlight()
{
	lcd_bckl(lcd_brightness);
	lcd_brightness_end = task_get_ms_tick() + lcd_brightness_timeout * 1000;
}

char gui_message[32];
uint32_t gui_message_end = 0;
#define MESSAGE_DURATION	4

void gui_showmessage(const char * msg)
{
	strcpy_P(gui_message, msg);
	gui_message_end = task_get_ms_tick() + MESSAGE_DURATION * 1000;
}

void gui_draw_box()
{
	disp.DrawRectangle(GUI_BOX_LEFT - 1, GUI_BOX_TOP - 1, GUI_BOX_RIGHT + 1, GUI_BOX_BOTTOM + 1, 1, 0);
	disp.DrawRectangle(GUI_BOX_LEFT, GUI_BOX_TOP, GUI_BOX_RIGHT, GUI_BOX_BOTTOM, 0, 1);
}


void gui_raligh_text(char * text, uint8_t x, uint8_t y)
{
	disp.GotoXY(x - disp.GetTextWidth(text), y);
	fprintf_P(lcd_out, PSTR("%s"), text);
}


void gui_init()
{
	disp.Init();

	eeprom_busy_wait();
	lcd_brightness = eeprom_read_byte(&config.gui.brightness);
	lcd_brightness_timeout = eeprom_read_byte(&config.gui.brightness_timeout);
	lcd_contrast = eeprom_read_byte(&config.gui.contrast);

	disp.SetContrast(lcd_contrast);
	gui_trigger_backlight();

	for (uint8_t i = 0; i < GUI_NUMBER_OF_TASKS; i++)
		gui_init_array[i]();
}

void gui_stop()
{
	disp.Stop();
}

uint8_t asdf = 0;

void gui_loop()
{
	if (gui_new_task != gui_task)
		gui_task = gui_new_task;

	disp.ClearBuffer();

	gui_loop_array[gui_task]();

	if (gui_message_end > task_get_ms_tick())
	{
		gui_draw_box();
		disp.LoadFont(F_TEXT_L);
		uint8_t w = disp.GetTextWidth(gui_message);
		uint8_t h = disp.GetAHeight();
		disp.GotoXY(n5110_width / 2 - w / 2, n5110_height / 2 - h / 2);
		fprintf_P(lcd_out, PSTR("%s"), gui_message);
	}

	disp.Draw();

	if (lcd_brightness_end < task_get_ms_tick())
		lcd_bckl(0);
}

void gui_irqh(uint8_t type, uint8_t * buff)
{
	if (type >= TASK_IRQ_BUTTON_L && type <= TASK_IRQ_BUTTON_R)
		gui_trigger_backlight();

	switch(type)
	{

	default:
		gui_irqh_array[gui_task](type, buff);
	}
}

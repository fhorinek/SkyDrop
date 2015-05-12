#include "gui.h"
#include "../drivers/buzzer.h"
#include "widgets/widgets.h"

#include "pages.h"
#include "settings.h"

n5110display disp;
CreateStdOut(lcd_out, disp.Write);

volatile uint8_t gui_task = GUI_NONE;
volatile uint8_t gui_new_task = GUI_PAGES;

void (* gui_init_array[])() =
{gui_pages_init, gui_settings_init};

void (* gui_stop_array[])() =
{gui_pages_stop, gui_settings_stop};

void (* gui_loop_array[])() =
{gui_pages_loop, gui_settings_loop};

void (* gui_irqh_array[])(uint8_t type, uint8_t * buff) =
{gui_pages_irqh, gui_settings_irqh};

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
uint8_t fps_counter = 0;
uint8_t fps_val = 0;
uint32_t fps_timer = 0;

uint32_t gui_loop_timer = 0;

void gui_dialog(char * title)
{
	disp.LoadFont(F_TEXT_M);
	disp.GotoXY(3,2);
	fprintf_P(lcd_out, PSTR("%s"), title);

	disp.InvertPart(0, 0, n5110_width, 0);
	disp.PutPixel(0, 0, 0);
	disp.PutPixel(n5110_width - 1, 0 ,0);
	disp.Invert(0, 8, n5110_width, 11);

	disp.DrawLine(0, 12, 0, n5110_height - 2, 1);
	disp.DrawLine(n5110_width - 1, 12, n5110_width - 1, n5110_height - 2, 1);
	disp.DrawLine(1, n5110_height - 1, n5110_width - 2, n5110_height - 1, 1);
}

void gui_loop()
{
	if (gui_loop_timer > task_get_ms_tick())
		return;
	gui_loop_timer = task_get_ms_tick() + 40; //25 fps


	if (gui_new_task != gui_task)
	{
		gui_task = gui_new_task;
		gui_init_array[gui_task]();
	}

	disp.ClearBuffer();

	gui_loop_array[gui_task]();

	if (gui_message_end > task_get_ms_tick())
	{
		disp.LoadFont(F_TEXT_L);
		uint8_t w = disp.GetTextWidth(gui_message);
		uint8_t h = disp.GetAHeight();
		disp.GotoXY(n5110_width / 2 - w / 2, n5110_height / 2 - h / 2);
		fprintf_P(lcd_out, PSTR("%s"), gui_message);
	}

	// FPS counter

	fps_counter++;

	if (fps_timer < task_get_ms_tick())
	{
		fps_val = fps_counter;
		fps_counter = 0;
		fps_timer = task_get_ms_tick() + 1000;
	}

	disp.LoadFont(font_6px_normal_ttf_8);
	disp.GotoXY(1, 1);
	disp.ClearPart(0, 0, 1, 10);
	fprintf_P(lcd_out, PSTR("%d"), fps_val);

	// FPS end

	disp.Draw();

	if (buttons_read(B_LEFT) || buttons_read(B_RIGHT) || buttons_read(B_MIDDLE))
		gui_trigger_backlight();

	if (lcd_brightness_end < task_get_ms_tick())
		lcd_bckl(0);
}

void gui_irqh(uint8_t type, uint8_t * buff)
{

	switch(type)
	{

	default:
		gui_irqh_array[gui_task](type, buff);
	}
}

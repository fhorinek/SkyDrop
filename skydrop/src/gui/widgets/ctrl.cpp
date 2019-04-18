/*
 * ctrl.cpp
 *
 *  Created on: 26.10.2015
 *      Author: horinek
 */

#include "ctrl.h"

void widget_ctrl_audio_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	char text[6];
	strcpy_P(text, PSTR("beep"));
	widget_value_txt(text, x, y, w, h);

	if (config.gui.vario_mute)
	{
		disp.DrawLine(x + 2, y + 1, x + w -4, y + h - 4, 1);
		disp.DrawLine(x + 2, y + h - 4, x + w -4, y + 1, 1);
	}
}

void widget_ctrl_audio_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == TASK_IRQ_BUTTON_M && (*buff == BE_LONG))
	{
		config.gui.vario_mute = !config.gui.vario_mute;
		eeprom_update_byte(&config_ee.gui.vario_mute, config.gui.vario_mute);

		if (config.gui.vario_mute)
			gui_showmessage_P(PSTR("Vario muted"));
		else
			gui_showmessage_P(PSTR("Vario unmuted"));
	}
}

void widget_ctrl_wlift_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	char text[6];
	strcpy_P(text, PSTR("weak"));
	widget_value_txt(text, x, y, w, h);

	if (!config.audio_profile.flags & AUDIO_WEAK)
	{
		disp.DrawLine(x + 2, y + 1, x + w -4, y + h - 4, 1);
		disp.DrawLine(x + 2, y + h - 4, x + w -4, y + 1, 1);
	}
}

void widget_ctrl_wlift_irqh(uint8_t type, uint8_t * buff, uint8_t index)
{
	if (type == TASK_IRQ_BUTTON_M && (*buff == BE_LONG))
	{
		config.audio_profile.flags ^= AUDIO_WEAK;
		eeprom_busy_wait();
		eeprom_update_byte(&config_ee.audio_profile.flags, config.audio_profile.flags);

		if (config.audio_profile.flags & AUDIO_WEAK)
			gui_showmessage_P(PSTR("Weaklift enabled"));
		else
			gui_showmessage_P(PSTR("Weaklift disabled"));
	}
}

register_widget2(w_ctrl_audio, "Audio ON/OFF", widget_ctrl_audio_draw, NULL, widget_ctrl_audio_irqh);
register_widget2(w_ctrl_wlift, "Weaklift ON/OFF", widget_ctrl_wlift_draw, NULL, widget_ctrl_wlift_irqh);




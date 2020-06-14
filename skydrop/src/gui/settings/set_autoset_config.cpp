#include "set_autoset_config.h"

#include "../pages.h"
#include "../gui_list.h"
#include "../gui_value.h"
#include "../../fc/fc.h"

void gui_set_autoset_config_init()
{
    gui_list_set(gui_set_autoset_config_item, gui_set_autoset_config_action, 2, GUI_SET_AUTOSET);
}

void gui_set_autoset_config_circling_timeout_cb(float val)
{
    gui_switch_task(GUI_SET_AUTOSET_CONFIG);

    config.gui.page_circling_timeout = val;
    
    ee_update_byte(&config_ee.gui.page_circling_timeout, config.gui.page_circling_timeout);
}

void gui_set_autoset_config_acro_thold_cb(float val)
{
    gui_switch_task(GUI_SET_AUTOSET_CONFIG);

    config.gui.page_acro_thold = val * 10.0;
    
    ee_update_byte((uint8_t *)&config_ee.gui.page_acro_thold, config.gui.page_acro_thold);
}

void gui_set_autoset_config_action(uint8_t index)
{
    switch (index)
    {
        case (0):
            gui_value_conf_P(PSTR("Circling timeout"), GUI_VAL_NUMBER, PSTR("%0.0f sec"), config.gui.page_circling_timeout, 0, 250, 1, gui_set_autoset_config_circling_timeout_cb);
            gui_switch_task(GUI_SET_VAL);
        break;

        case (1):
            gui_value_conf_P(PSTR("Acro thold."), GUI_VAL_NUMBER, PSTR("%0.1f m/s"), config.gui.page_acro_thold / 10.0, -10, 0, 0.1, gui_set_autoset_config_acro_thold_cb);
            gui_switch_task(GUI_SET_VAL);
        break;

    }
}

void gui_set_autoset_config_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
    switch (index)
    {
        case (0):
            strcpy_P(text, PSTR("Circling timeout"));
            sprintf_P(sub_text, PSTR("%d s"), config.gui.page_circling_timeout);
            *flags =  GUI_LIST_SUB_TEXT;
        break;

        case (1):
            strcpy_P(text, PSTR("Acro thold."));
            sprintf_P(sub_text, PSTR("%0.1f m/s"), config.gui.page_acro_thold / 10.0);
            *flags =  GUI_LIST_SUB_TEXT;
        break;
    }
}


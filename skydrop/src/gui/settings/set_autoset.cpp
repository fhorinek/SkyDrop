#include "set_autoset.h"

#include "../pages.h"
#include "../gui_list.h"
#include "../../fc/fc.h"

void gui_set_autoset_init()
{
    gui_list_set(gui_set_autoset_item, gui_set_autoset_action, 6, GUI_LAYOUTS);
}

void gui_set_autoset_action(uint8_t index)
{
    if (index == 5)
    {
        gui_switch_task(GUI_SET_AUTOSET_CONFIG);
        return;
    }

    if (config.gui.page_mode[index] == active_page)
        config.gui.page_mode[index] = PAGE_NONE;
    else
        config.gui.page_mode[index] = active_page;

    
    ee_update_byte(&config_ee.gui.page_mode[index], config.gui.page_mode[index]);
}

void gui_set_autoset_item(uint8_t index, char * text, uint8_t * flags, char * sub_text)
{
    if (index == 5)
    {
        strcpy_P(text, PSTR("Configure"));
        return;
    }

    switch (index)
    {
        case (PAGE_MODE_PREPARE):
            strcpy_P(text, PSTR("Prepare"));
        break;

        case (PAGE_MODE_CIRCLING):
            strcpy_P(text, PSTR("Circling"));
        break;

        case (PAGE_MODE_NORMAL):
            strcpy_P(text, PSTR("Normal"));
        break;

        case (PAGE_MODE_ACRO):
            strcpy_P(text, PSTR("Acro"));
        break;

        case (PAGE_MODE_LANDED):
            strcpy_P(text, PSTR("Landed"));
        break;

    }

    if (config.gui.page_mode[index] == active_page)
        *flags =  GUI_LIST_CHECK_ON;
    else
        *flags =  GUI_LIST_CHECK_OFF;
}


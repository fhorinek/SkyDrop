#include "gui_storage.h"

#include "gui_dialog.h"
#include "../drivers/storage/storage.h"

bool gui_format_sd()
{
	gui_showmessage_P(PSTR("Formating..."));
	gui_force_loop();
	ewdt_reset();
	uint8_t ret = f_mkfs("", 0, 0);
	assert(ret == RES_OK);
	DEBUG("ret = %u\n", ret);
	if (ret == RES_OK)
	{
		gui_showmessage_P(PSTR("Done!"));
		return true;
	}
	else
	{
		char tmp[16];
		sprintf_P(tmp, PSTR("Error: %02u"), ret);
		gui_showmessage(tmp);
		return false;
	}
}

void gui_storage_format_cb2(uint8_t ret)
{
	if (ret == GUI_DIALOG_OK)
	{
		gui_format_sd();
	}
	gui_switch_task(GUI_SET_ADVANCED);
}

void gui_storage_format_cb(uint8_t ret)
{
	if (ret == GUI_DIALOG_YES)
	{
		gui_dialog_set_P(PSTR("Warning"), PSTR("This will erase\nall data from SD\ncard! Continue?"), GUI_STYLE_OKCANCEL, gui_storage_format_cb2);
		gui_switch_task(GUI_DIALOG);
	}
	else
	{
		gui_switch_task(GUI_SET_ADVANCED);
	}
}

void gui_storage_mount_error()
{
	gui_dialog_set_P(PSTR("Error"), PSTR("Unable to mount\ndata storage.\nFormat SD card?"), GUI_STYLE_YESNO, gui_storage_format_cb);
	gui_switch_task(GUI_DIALOG);
}


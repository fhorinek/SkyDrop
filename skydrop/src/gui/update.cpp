/*
 * update.cc
 *
 *  Created on: 26.8.2015
 *      Author: horinek
 */

#include "update.h"
#include "gui_dialog.h"

#include "../drivers/storage/storage.h"

FIL	* update_file;
FIL	* update_file_out;

update_header update_head;

uint32_t update_file_pos;
uint8_t update_file_crc;

volatile uint8_t update_eeprom_only = false;

#define UPDATE_CRC 		0x9B
#define UPDATE_CHUNK	DEBUG_LOG_BUFFER_SIZE

void gui_update_cb(uint8_t ret)
{
	if (ret == GUI_DIALOG_YES)
	{
		gui_switch_task(GUI_UPDATE);
	}
	else
	{
		f_unlink("SKYDROP.FW");
		task_set(TASK_ACTIVE);
	}
}

void gui_update_fail_cb(uint8_t ret)
{
	task_set(TASK_POWERDOWN);
}

void gui_update_done_cb(uint8_t ret)
{
	eeprom_busy_wait();

	gui_stop();

	task_set(TASK_POWERDOWN);
}

void gui_update_eeprom_cb(uint8_t ret)
{
	if (ret == GUI_DIALOG_OK)
	{
		update_eeprom_only = true;
		gui_switch_task(GUI_UPDATE);
		update_state = UPDATE_CHECK_EE;
	}
	else
	{
		assert(f_unlink("SKYDROP.FW") == FR_OK);
		task_set(TASK_POWERDOWN);
	}
}

void gui_update_fail(uint16_t line)
{
	update_state = UPDATE_FAIL;

	char tmp1[16], tmp2[64];

	if (line != 0)
	{
		strcpy_P(tmp1, PSTR("Error"));
		sprintf_P(tmp2, PSTR("Update failed!\n#%d"), line);
		gui_dialog_set(tmp1, tmp2, GUI_STYLE_OK, gui_update_fail_cb);

		assert(f_unlink("SKYDROP.FW") == FR_OK);
		assert(f_unlink("UPDATE.FW") == FR_OK);
	}
	else
	{
		DEBUG("Same version\n");
		strcpy_P(tmp1, PSTR("Update"));
		sprintf_P(tmp2, PSTR("Same version\nRestore default\nconfiguration?"), line);
		gui_dialog_set(tmp1, tmp2, GUI_STYLE_OKCANCEL, gui_update_eeprom_cb);
	}

	gui_switch_task(GUI_DIALOG);
}

void gui_update_init()
{
	//Temporarily disable logging on SD card
	config.system.debug_log = false;
}

void gui_update_stop() {}

void gui_update_bar()
{
	gui_dialog_P(PSTR("Updating"));

	char tmp[32];

	switch (update_state)
	{
		case(UPDATE_IDLE):
			strcpy_P(tmp, PSTR("Reading file"));
		break;
		case(UPDATE_CHECK_EE):
			strcpy_P(tmp, PSTR("Checking EE"));
		break;
		case(UPDATE_UNPACK):
			strcpy_P(tmp, PSTR("Unpacking FW"));
		break;
		case(UPDATE_CHECK_FW):
			strcpy_P(tmp, PSTR("Checking FW"));
		break;
		case(UPDATE_EE):
			strcpy_P(tmp, PSTR("Writing EE"));
		break;
	}

	gui_caligh_text(tmp, GUI_DISP_WIDTH / 2, GUI_DIALOG_TOP + 10);

	uint8_t pad = 3;
	uint8_t bar_width = GUI_DIALOG_RIGHT - GUI_DIALOG_LEFT - pad * 2;
	uint8_t pos = ((update_state + 1) * bar_width) / 5;

	disp.DrawRectangle(GUI_DIALOG_LEFT + pad - 1, GUI_DIALOG_TOP + 20, GUI_DIALOG_RIGHT - pad + 1, GUI_DIALOG_TOP + 26, 1, 0);
	disp.DrawRectangle(GUI_DIALOG_LEFT + pad, GUI_DIALOG_TOP + 21, GUI_DIALOG_LEFT + pad + pos, GUI_DIALOG_TOP + 25, 1, 1);
}

void gui_update_loop()
{
	uint16_t rd, wr;
	uint16_t to_read;

	//steal buffer from debug
	extern uint8_t debug_log_buffer[DEBUG_LOG_BUFFER_SIZE];

	uint8_t * buff = debug_log_buffer;
	uint16_t i;

//	DEBUG(" *** update loop start ***\n");

	gui_update_bar();

	DEBUG("update_state %u\n", update_state);
	DEBUG("update_file_pos %lu\n", update_file_pos);

	switch (update_state)
	{
		case(UPDATE_IDLE):
			//update task is special case
			//update files are used only during update
			//we can "steal" them from other tasks
			extern FIL log_file;
			extern FIL skybean_file_handle;

			assert(f_unlink("UPDATE.FW") == FR_OK);

			update_file = &log_file;				//stolen from IGC/KML logger
			update_file_out = &skybean_file_handle;	//stolen from Skybean protocol handler

			update_state = UPDATE_CHECK_EE;
			update_file_pos = 0;
			update_file_crc = 0;

			//able to open
			if (f_open(update_file, "SKYDROP.FW", FA_READ) != FR_OK)
			{
				gui_update_fail(__LINE__);
				break;
			}

			//sucessfull header read
			if (f_read(update_file, &update_head, sizeof(update_head), &rd) != FR_OK)
			{
				gui_update_fail(__LINE__);
				break;
			}

			//correct bytes read
			if (rd != sizeof(update_head))
			{
				gui_update_fail(__LINE__);
				break;
			}

			//check file size
			if (update_head.file_size != update_file->fsize)
			{
				gui_update_fail(__LINE__);
				break;
			}

			//check build number
			if (update_head.build_number == BUILD_NUMBER)
			{
				gui_update_fail(0);
				break;
			}
		break;

		case(UPDATE_CHECK_EE):
			if (update_file_pos + UPDATE_CHUNK <= update_head.eeprom_size)
				to_read = UPDATE_CHUNK;
			else
				to_read = update_head.eeprom_size - update_file_pos;

			if (f_read(update_file, buff, to_read, &rd) != FR_OK)
			{
				gui_update_fail(__LINE__);
				break;
			}

			if (rd != to_read)
			{
				gui_update_fail(__LINE__);
				break;
			}

			for (i = 0; i < rd; i++)
				update_file_crc = CalcCRC(update_file_crc, UPDATE_CRC, buff[i]);

			update_file_pos += rd;
			if (update_file_pos == update_head.eeprom_size)
			{
				if (update_file_crc == update_head.eeprom_crc)
				{
					if (update_eeprom_only == false)
					{
						update_state = UPDATE_UNPACK;
						update_file_pos = 0;

						if (f_open(update_file_out, "UPDATE.FW", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
						{
							gui_update_fail(__LINE__);
							break;
						}
					}
					else
					{
						update_state = UPDATE_EE;
						update_file_pos = 0;

						if (f_lseek(update_file, sizeof(update_head)) != FR_OK)
						{
							gui_update_fail(__LINE__);
							break;
						}
					}

				}
				else
				{
					gui_update_fail(__LINE__);
					break;
				}

			}
		break;

		case(UPDATE_UNPACK):
			if (update_file_pos + UPDATE_CHUNK <= update_head.flash_size)
				to_read = UPDATE_CHUNK;
			else
				to_read = update_head.flash_size - update_file_pos;

			if (f_read(update_file, buff, to_read, &rd) != FR_OK)
			{
				gui_update_fail(__LINE__);
				break;
			}

			if (rd != to_read)
			{
				gui_update_fail(__LINE__);
				break;
			}

			if (f_write(update_file_out, buff, to_read, &wr) != FR_OK)
			{
				gui_update_fail(__LINE__);
				break;
			}

			if (rd != wr)
			{
				gui_update_fail(__LINE__);
				break;
			}

			update_file_pos += to_read;

			if (update_file_pos == update_head.flash_size)
			{
				update_state = UPDATE_CHECK_FW;
				update_file_pos = 0;
				update_file_crc = 0;

				if (f_close(update_file_out) != FR_OK)
				{
					gui_update_fail(__LINE__);
					break;
				}

				if (f_open(update_file_out, "UPDATE.FW", FA_READ) != FR_OK)
				{
					gui_update_fail(__LINE__);
					break;
				}
			}

		break;

		case(UPDATE_CHECK_FW):
			if (update_file_pos + UPDATE_CHUNK <= update_head.flash_size)
				to_read = UPDATE_CHUNK;
			else
				to_read = update_head.flash_size - update_file_pos;

			if (f_read(update_file_out, buff, to_read, &rd) != FR_OK)
			{
				gui_update_fail(__LINE__);
				break;
			}

			if (rd != to_read)
			{
				gui_update_fail(__LINE__);
				break;
			}

			for (i = 0; i < rd; i++)
				update_file_crc = CalcCRC(update_file_crc, UPDATE_CRC, buff[i]);

			update_file_pos += rd;
			if (update_file_pos == update_head.flash_size)
			{
				if (update_file_crc == update_head.flash_crc)
				{
					update_state = UPDATE_EE;
					update_file_pos = 0;

					if (f_close(update_file_out) != FR_OK)
					{
						gui_update_fail(__LINE__);
						break;
					}

					if (f_lseek(update_file, sizeof(update_head)) != FR_OK)
					{
						gui_update_fail(__LINE__);
						break;
					}
				}
				else
				{
					gui_update_fail(__LINE__);
					break;
				}

			}
		break;

		case(UPDATE_EE):
			if (update_file_pos + UPDATE_CHUNK <= update_head.eeprom_size)
				to_read = UPDATE_CHUNK;
			else
				to_read = update_head.eeprom_size - update_file_pos;

			if (f_read(update_file, buff, to_read, &rd) != FR_OK)
			{
				gui_update_fail(__LINE__);
				break;
			}

			if (rd != to_read)
			{
				gui_update_fail(__LINE__);
				break;
			}


			eeprom_busy_wait();
			eeprom_update_block(buff, (uint8_t *)(APP_INFO_EE_offset + update_file_pos), to_read);

			update_file_pos += rd;
			if (update_file_pos == update_head.eeprom_size)
			{
				gui_dialog_set_P(PSTR("Success"), PSTR("Update done.\nSkyDrop will reboot"), GUI_STYLE_OK, gui_update_done_cb);
				gui_switch_task(GUI_DIALOG);

				f_rename("CFG.EE", "OLD.EE");
				f_unlink("SKYDROP.FW");

				storage_deinit();
			}

		break;

	}
//	DEBUG(" *** update_loop_end ***\n");
}

void gui_update_irqh(uint8_t type, uint8_t * buff) {}

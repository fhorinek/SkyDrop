#include "file.h"
#include "../../drivers/storage/storage.h"

FIL skybean_file_handle;

uint16_t skybean_file_readdir(char * path, uint16_t * total_length)
{
	DEBUG("skybean_file_readdir '%s'\n", path);

	DIR f_dir;
	uint16_t file_cnt = 0;
	uint16_t data_length = 0;

	if (f_opendir(&f_dir, path) == FR_OK)
	{
		FILINFO f_info;

		while(1)
		{
			uint8_t res = f_readdir(&f_dir, &f_info);
			if (res == FR_OK)
			{
				if (f_info.fname[0] != '\0')
				{
					if (f_info.fname[0] == 0xFF)
						continue;

					file_cnt++;
					data_length += 4 + 1 + strlen(f_info.fname);
				}
				else
					break;
			}
			else
				break;
		}
	}

	*total_length = data_length;
	return file_cnt;
}

void skybean_file_list()
{
	uint8_t len = skybean_stream.Read();
	char path[128];

	for (uint8_t i = 0; i < len; i++)
		path[i] = skybean_stream.Read();

	path[len] = 0;

	uint16_t total_length;
	uint16_t file_count = skybean_file_readdir(path, &total_length);

	total_length += 2;

	skybean_startpacket(total_length, SKYBEAN_FILE, 0x81);
	skybean_stream.Write(2, (uint8_t *)&file_count);

	DIR f_dir;

	if (f_opendir(&f_dir, path) == FR_OK)
	{
		FILINFO f_info;

		while(1)
		{
			uint8_t res = f_readdir(&f_dir, &f_info);
			if (res == FR_OK)
			{
				if (f_info.fname[0] != '\0')
				{
					if (f_info.fname[0] == 0xFF)
						continue;

					if (f_info.fattrib & AM_DIR)
					{
						skybean_stream.Write(0xFF);
						skybean_stream.Write(0xFF);
						skybean_stream.Write(0xFF);
						skybean_stream.Write(0xFF);
					}
					else
						skybean_stream.Write(4, (uint8_t *)&f_info.fsize);

					uint8_t len = strlen(f_info.fname);
					skybean_stream.Write(len);
					skybean_stream.Write(len, (uint8_t *)f_info.fname);
				}
				else
					break;
			}
			else
				break;
		}
	}

}

uint8_t skybean_file_write()
{
	char path[128];
	uint8_t len;

	len = skybean_stream.Read();
	for (uint8_t i = 0; i < len; i++)
		path[i] = skybean_stream.Read();

	path[len] = 0;

	if (f_open(&skybean_file_handle, path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
		return SB_FAIL;

	return SB_SUCCESS;
}

uint8_t skybean_file_delete()
{
	char path[128];
	uint8_t len;

	len = skybean_stream.Read();
	for (uint8_t i = 0; i < len; i++)
		path[i] = skybean_stream.Read();

	path[len] = 0;

	if (f_unlink(path) != FR_OK)
		return SB_FAIL;

	return SB_SUCCESS;
}

void skybean_file_read()
{
	char path[128];
	uint8_t len;

	len = skybean_stream.Read();
	for (uint8_t i = 0; i < len; i++)
		path[i] = skybean_stream.Read();

	path[len] = 0;

	if (f_open(&skybean_file_handle, path, FA_READ) != FR_OK)
	{
		skybean_startpacket(1, SKYBEAN_FILE, 0x03);
		skybean_stream.Write(SB_ERROR);
	}

	skybean_startpacket(5, SKYBEAN_FILE, 0x03);
	skybean_stream.Write(SB_SUCCESS);
	skybean_stream.Write(4, (uint8_t *)&skybean_file_handle.fsize);
}

uint8_t skybean_file_close()
{
	if (f_close(&skybean_file_handle) != FR_OK)
		return SB_FAIL;

	return SB_SUCCESS;
}

uint8_t skybean_file_push()
{
	byte4 pos;
	byte2 len;

	pos.uint8[0] = skybean_stream.Read();
	pos.uint8[1] = skybean_stream.Read();
	pos.uint8[2] = skybean_stream.Read();
	pos.uint8[3] = skybean_stream.Read();

	len.uint8[0] = skybean_stream.Read();
	len.uint8[1] = skybean_stream.Read();

	if (f_lseek(&skybean_file_handle, pos.uint32) != FR_OK)
		return SB_FAIL;

	char buffer[512];
	uint16_t bw;

	for (uint16_t i = 0; i < len.uint16; i++)
		buffer[i] = skybean_stream.Read();

	if (f_write(&skybean_file_handle, buffer, len.uint16, &bw) != FR_OK)
		return SB_FAIL;

	if (bw != len.uint16)
		return SB_FAIL;

	return SB_SUCCESS;
}

void skybean_file_pull()
{
	byte4 pos;
	byte2 len;

	pos.uint8[0] = skybean_stream.Read();
	pos.uint8[1] = skybean_stream.Read();
	pos.uint8[2] = skybean_stream.Read();
	pos.uint8[3] = skybean_stream.Read();

	len.uint8[0] = skybean_stream.Read();
	len.uint8[1] = skybean_stream.Read();

	DEBUG("skybean_file_pull pos %lu\n", pos.uint32);
	DEBUG("skybean_file_pull len %u\n", len.uint16);


	if (f_lseek(&skybean_file_handle, pos.uint32) != FR_OK)
		{
			skybean_startpacket(1, SKYBEAN_FILE, 0x07);
			skybean_stream.Write(SB_FAIL);
		}

	char buffer[512];
	uint16_t br;

	if (f_read(&skybean_file_handle, buffer, len.uint16, &br) != FR_OK)
	{
		skybean_startpacket(1, SKYBEAN_FILE, 0x07);
		skybean_stream.Write(SB_FAIL);
	}

	DEBUG("skybean_file_pull br %u\n", br);

//	for (uint16_t i = 0; i < br; i++)
//		DEBUG(" %02X", buffer[i]);
//
//	DEBUG("\n-----\n");

	skybean_startpacket(3 + br, SKYBEAN_FILE, 0x07);
	skybean_stream.Write(SB_SUCCESS);
	skybean_stream.Write(2, (uint8_t *)&br);
	skybean_stream.Write(br, (uint8_t *)buffer);
}

uint8_t skybean_file_mkdir()
{
	char path[128];
	uint8_t len;

	len = skybean_stream.Read();
	for (uint8_t i = 0; i < len; i++)
		path[i] = skybean_stream.Read();

	path[len] = 0;

	if (f_mkdir(path) == FR_OK)
		return SB_SUCCESS;
	else
		return SB_FAIL;
}

void skybean_file(uint8_t op)
{
	switch (op)
	{
		case(0x00):
			skybean_startpacket(1, SKYBEAN_FILE, op);
			skybean_stream.Write(storage_ready());
		break;

		case(0x01):
			skybean_file_list();
		break;

		case(0x02):
			skybean_startpacket(1, SKYBEAN_FILE, op);
			skybean_stream.Write(skybean_file_write());
		break;

		case(0x03):
			skybean_file_read();
		break;

		case(0x04):
			skybean_startpacket(1, SKYBEAN_FILE, op);
			skybean_stream.Write(skybean_file_close());
		break;

		case(0x05):
			skybean_startpacket(1, SKYBEAN_FILE, op);
			skybean_stream.Write(skybean_file_delete());
		break;

		case(0x06):
			skybean_startpacket(1, SKYBEAN_FILE, op);
			skybean_stream.Write(skybean_file_push());
		break;

		case(0x07):
			skybean_file_pull();
		break;

		case(0x08):
			skybean_startpacket(1, SKYBEAN_FILE, op);
			skybean_stream.Write(skybean_file_mkdir());
		break;
	}
}

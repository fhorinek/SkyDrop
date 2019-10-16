#include "storage.h"

FATFS FatFs;		/* FatFs work area needed for each volume */

extern Usart sd_spi_usart;

uint32_t storage_space = 0;
uint32_t storage_free_space = 0;

bool sd_avalible = false;
bool sd_error = false;

//handle for dir operations
DIR storage_dir_h;

#define SD_CARD_DETECT	(GpioRead(SD_IN) == LOW)

bool storage_init()
{
	uint8_t res;

	GpioSetPull(SD_IN, gpio_pull_up);
	GpioSetDirection(SD_IN, INPUT);

	DEBUG("SD_IN %d\n", GpioRead(SD_IN));

//	if (!SD_CARD_DETECT)
//		return false;

	//power spi & sdcard
	SD_EN_ON;
	SD_SPI_PWR_ON;

	DEBUG("Mounting SD card ... ");

	for (uint8_t i = 0; i < 3; i++)
	{

		res = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive */
		DEBUG("%d ", i + 1);
		if (res == RES_OK)
			break;

		for (uint8_t j = 0; j < i +1; j++)
			_delay_ms(10);
	}

	if (res != RES_OK)
	{
		DEBUG("Error %02X\n", res);

		sd_spi_usart.Stop();

		GpioSetDirection(SD_SS, INPUT);
		GpioSetPull(SD_IN, gpio_totem);

		//power spi & sdcard
		SD_EN_OFF;
		SD_SPI_PWR_OFF;

		sd_error = true;
		sd_avalible = false;

		task_irqh(TASK_IRQ_MOUNT_ERROR, 0);

		return false;
	}

	DEBUG("OK\n");

	uint32_t size;

	FATFS * FatFs1;

	res = f_getfree("", &size, &FatFs1);

	uint32_t sector_count;

	res = disk_ioctl(0, GET_SECTOR_COUNT, &sector_count);

	uint16_t sector_size;

	res = disk_ioctl(0, GET_SECTOR_SIZE, &sector_size);

	storage_space = sector_count / 2;
	storage_free_space = size * 4 * 1024;

	DEBUG("Disk info\n");
	DEBUG(" sector size  %12u B\n", sector_size);
	DEBUG(" sector count %12lu\n", sector_count);
	DEBUG(" total space  %12lu kB\n", storage_space);
	DEBUG(" free space   %12lu\n", storage_free_space);

	sd_avalible = true;
	sd_error = false;

	return true;
}

void storage_deinit()
{
	DEBUG("storage_deinit\n");

	if (!sd_avalible)
		return;

	assert(f_mount(NULL, "", 1) == FR_OK); //unmount

	sd_spi_usart.Stop();

	sd_avalible = false;

	GpioSetPull(SD_IN, gpio_totem);
	GpioSetDirection(SD_SS, INPUT);

	//power spi & sdcard
	SD_EN_OFF;
	SD_SPI_PWR_OFF;

	//let it cool down :)
	_delay_ms(100);
}

void storage_step()
{
//	if (SD_CARD_DETECT)
//	{
//		if (!sd_avalible && !sd_error)
//			storage_init();
//	}
//	else
//	{
//		if (sd_avalible)
//			storage_deinit();
//		sd_error = false;
//	}
}

bool storage_card_in()
{
	return sd_avalible;
}

bool storage_ready()
{
	return sd_avalible && !sd_error;
}

uint8_t storage_get_files()
{
	FILINFO f_info;
	uint8_t cnt = 0;

	storage_dir_rewind();

	while(f_readdir(&storage_dir_h, &f_info) == FR_OK)
	{
		if (f_info.fname[0] == 0)
			break;

		if(f_info.fname[0] == 0xFF) //bugs in FatFs?
			continue;

		cnt++;
	}

	storage_dir_rewind();

	return cnt;
}

bool storage_dir_open(char * path)
{
	DEBUG("Open dir %s\n", path);

	return f_opendir(&storage_dir_h, path) == FR_OK;
}

void storage_dir_close()
{
	assert(f_closedir(&storage_dir_h) == FR_OK);
}

void storage_dir_rewind()
{
	//rewind directory
	f_readdir(&storage_dir_h, NULL);
}

bool storage_dir_list(char * fname, uint8_t * flags)
{
	FILINFO f_info;
	*flags = 0;

	while(f_readdir(&storage_dir_h, &f_info) == FR_OK)
	{
		if (f_info.fname[0] == 0)
			break;

		if(f_info.fname[0] == 0xFF) //bugs in FatFs?
			continue;

		strcpy(fname, f_info.fname);

		if (f_info.fattrib & AM_DIR)
			*flags |= STORAGE_IS_DIR;

		return true;
	}

	storage_dir_rewind();
	return false;
}



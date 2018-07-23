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

uint8_t storage_dir_get_count()
{
	int16_t count;
	char name[13];
	uint8_t flags;

#define MAX_DIRENTRIES 1000

	storage_dir_rewind();
	count = storage_dir_list_n(name, &flags, MAX_DIRENTRIES);
	storage_dir_rewind();

	return MAX_DIRENTRIES - count;
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

/**
 * Read the "no" directory entry. If count is "1" this is
 * the next entry. For "2" it is the second entry ....
 *
 * @param fname a pointer to memory where the name of the
 *              directory entry is returned to the caller.
 * @param flags a pointer to the flags returned to the caller.
 *              Entries describing a sub directory the flags
 *              contain STORAGE_IS_DIR.
 * @param no    the number of the directory entry to read.
 *
 * @return the number of remaining entries, after hitting
 *         the end of the directory. If the entry could be
 *         read successfully, this is "0". Otherwise something
 *         between 0 and the number given in. E.g. if the
 *         directory holds 500 entries and you try to read
 *         number 600. It will return 100.
 */
int16_t storage_dir_list_n(char * fname, uint8_t * flags, int16_t no)
{
	bool ret;

	while (no > 0) {
		ret = storage_dir_list(fname, flags);
		if ( ret == false ) {
			// we reached end of directory before count is 0.
			break;
		}
		no--;
	}

	return no;
}

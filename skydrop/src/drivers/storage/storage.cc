#include "storage.h"

FATFS FatFs;		/* FatFs work area needed for each volume */
//FIL Fil;			/* File object needed for each open file */

extern Usart sd_spi_usart;

uint32_t storage_space = 0;
uint32_t storage_free_space = 0;

bool sd_avalible = false;

#define SD_CARD_DETECT	(GpioRead(SD_IN) == LOW)

bool storage_init()
{
	uint8_t res;

	GpioSetPull(SD_IN, gpio_pull_up);
	GpioSetDirection(SD_IN, INPUT);

	DEBUG("SD_IN %d\n", GpioRead(SD_IN));

	if (!SD_CARD_DETECT)
		return false;

	//power spi & sdcard
	SD_EN_ON;
	SD_SPI_PWR_ON;

	DEBUG("Mounting SD card ... ");

	for (uint8_t i = 0; i < 5; i++)
	{
		//power spi & sdcard
		SD_EN_ON;
		SD_SPI_PWR_ON;

		res = f_mount(&FatFs, "", 1);		/* Give a work area to the default drive */
		DEBUG("%d ", i + 1);
		if (res == RES_OK)
			break;


		sd_spi_usart.Stop();

		//power spi & sdcard
		SD_EN_OFF;
		SD_SPI_PWR_OFF;

		for (uint8_t j = 0; j < i +1; j++)
			_delay_ms(10);
	}

	if (res != RES_OK)
	{
		DEBUG("Error %02X\n", res);

		sd_spi_usart.Stop();

		//power spi & sdcard
		SD_EN_OFF;
		SD_SPI_PWR_OFF;

		sd_avalible = false;

		return false;
	}

	DEBUG("OK\n");

	uint32_t size;

	FATFS * FatFs1;

	res = f_getfree("", &size, &FatFs1);

//	DEBUG1("f_getfree res = %d, size = %lu MiB", res, size / 256);

	uint32_t sector_count;

	res = disk_ioctl(0, GET_SECTOR_COUNT, &sector_count);

//	DEBUG1("GET_SECTOR_COUNT res = %d, size = %lu", res, sector_count);

	uint16_t sector_size;

	res = disk_ioctl(0, GET_SECTOR_SIZE, &sector_size);

//	DEBUG1("GET_SECTOR_SIZE res = %d, size = %u", res, sector_size);

	storage_space = sector_count * sector_size;
	storage_free_space = size * 4 * 1024;

	DEBUG("Disk info\n");
	DEBUG(" sector size  %12u\n", sector_size);
	DEBUG(" sector count %12lu\n", sector_count);
	DEBUG(" total space  %12lu\n", storage_space);
	DEBUG(" free space   %12lu\n", storage_free_space);

	sd_avalible = true;
	return true;
}

void storage_deinit()
{
	if (!sd_avalible)
		return;

	uint8_t res;
	res = f_mount(NULL, "", 1); //unmount

	sd_spi_usart.Stop();

	sd_avalible = false;

	//power spi & sdcard
	SD_EN_OFF;
	SD_SPI_PWR_OFF;
}

void storage_step()
{
	if (SD_CARD_DETECT)
	{
		if (!sd_avalible)
			storage_init();
	}
	else
	{
		if (sd_avalible)
			storage_deinit();
	}
}

bool storage_selftest()
{
	return sd_avalible;
}

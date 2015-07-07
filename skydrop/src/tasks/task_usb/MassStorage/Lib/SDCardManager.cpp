/*
             LUFA Library
     Copyright (C) Dean Camera, 2009.
              
  dean [at] fourwalledcubicle [dot] com
      www.fourwalledcubicle.com
*/

/*
  Copyright 2009  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, and distribute this software
  and its documentation for any purpose and without fee is hereby
  granted, provided that the above copyright notice appear in all
  copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Functions to manage the physical dataflash media, including reading and writing of
 *  blocks of data. These functions are called by the SCSI layer when data must be stored
 *  or retrieved to/from the physical storage media. If a different media is used (such
 *  as a SD card or EEPROM), functions similar to these will need to be generated.
 */

#define  INCLUDE_FROM_SDCARDMANAGER_C
#include "SDCardManager.h"
#include "../../drivers/storage/FatFs/diskio.h"


static uint32_t CachedTotalBlocks = 0;

typedef uint64_t offset_t;

extern bool IsMassStoreReset;

bool SDCardManager_Init(void)
{
	uint8_t res = disk_initialize(0);

	return (res == RES_OK);
}

uint32_t SDCardManager_GetNbBlocks(void)
{
	uint32_t sector_count;
	uint8_t res;

	if (CachedTotalBlocks != 0)
		return CachedTotalBlocks;
		
	res = disk_ioctl(0, GET_SECTOR_COUNT, &sector_count);

	if(res != RES_OK)
	{
		DEBUG("Error reading SD card info %d\n", res);
		return 0;
	}

	CachedTotalBlocks = sector_count;
	//DEBUG(PSTR("SD blocks: %li\r\n"), TotalBlocks);
	
	return CachedTotalBlocks;
}

/** Writes blocks (OS blocks, not Dataflash pages) to the storage medium, the board dataflash IC(s), from
 *  the pre-selected data OUT endpoint. This routine reads in OS sized blocks from the endpoint and writes
 *  them to the dataflash in Dataflash page sized blocks.
 *
 *  \param[in] BlockAddress  Data block starting address for the write sequence
 *  \param[in] TotalBlocks   Number of blocks of data to write
 */
uintptr_t SDCardManager_WriteBlockHandler(uint8_t* buffer, offset_t offset, void* p)
{
	/* Check if the endpoint is currently empty */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the current endpoint bank */
		Endpoint_ClearOUT();
		
		/* Wait until the host has sent another packet */
		if (Endpoint_WaitUntilReady())
		  return 0;
	}
	
	/* Write one 16-byte chunk of data to the dataflash */
	buffer[0] = Endpoint_Read_8();
	buffer[1] = Endpoint_Read_8();
	buffer[2] = Endpoint_Read_8();
	buffer[3] = Endpoint_Read_8();
	buffer[4] = Endpoint_Read_8();
	buffer[5] = Endpoint_Read_8();
	buffer[6] = Endpoint_Read_8();
	buffer[7] = Endpoint_Read_8();
	buffer[8] = Endpoint_Read_8();
	buffer[9] = Endpoint_Read_8();
	buffer[10] = Endpoint_Read_8();
	buffer[11] = Endpoint_Read_8();
	buffer[12] = Endpoint_Read_8();
	buffer[13] = Endpoint_Read_8();
	buffer[14] = Endpoint_Read_8();
	buffer[15] = Endpoint_Read_8();
	
	return 16;
}

void SDCardManager_WriteBlocks(uint32_t BlockAddress, uint16_t TotalBlocks)
{
//	DEBUG("W %li %i\n", BlockAddress, TotalBlocks);

	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		uint8_t tmp_buffer[VIRTUAL_MEMORY_BLOCK_SIZE];

		for (uint8_t i = 0; i < VIRTUAL_MEMORY_BLOCK_SIZE / 16; i++)
		{
			SDCardManager_WriteBlockHandler(tmp_buffer + (i * 16), 0, 0);
		}

		disk_write(0, tmp_buffer, BlockAddress, 1);
		
		/* Check if the current command is being aborted by the host */
		if (IsMassStoreReset)
		  return;
			
		/* Decrement the blocks remaining counter and reset the sub block counter */
		BlockAddress++;
		TotalBlocks--;
	}

	/* If the endpoint is empty, clear it ready for the next packet from the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearOUT();
}

/** Reads blocks (OS blocks, not Dataflash pages) from the storage medium, the board dataflash IC(s), into
 *  the pre-selected data IN endpoint. This routine reads in Dataflash page sized blocks from the Dataflash
 *  and writes them in OS sized blocks to the endpoint.
 *
 *  \param[in] BlockAddress  Data block starting address for the read sequence
 *  \param[in] TotalBlocks   Number of blocks of data to read
 */

uint8_t SDCardManager_ReadBlockHandler(uint8_t* buffer, offset_t offset, void* p)
{
	/* Check if the endpoint is currently full */
	if (!(Endpoint_IsReadWriteAllowed()))
	{
		/* Clear the endpoint bank to send its contents to the host */
		Endpoint_ClearIN();
		
		/* Wait until the endpoint is ready for more data */
		if (Endpoint_WaitUntilReady())
		  return 0;
	}
		
	Endpoint_Write_8(buffer[0]);
	Endpoint_Write_8(buffer[1]);
	Endpoint_Write_8(buffer[2]);
	Endpoint_Write_8(buffer[3]);
	Endpoint_Write_8(buffer[4]);
	Endpoint_Write_8(buffer[5]);
	Endpoint_Write_8(buffer[6]);
	Endpoint_Write_8(buffer[7]);
	Endpoint_Write_8(buffer[8]);
	Endpoint_Write_8(buffer[9]);
	Endpoint_Write_8(buffer[10]);
	Endpoint_Write_8(buffer[11]);
	Endpoint_Write_8(buffer[12]);
	Endpoint_Write_8(buffer[13]);
	Endpoint_Write_8(buffer[14]);
	Endpoint_Write_8(buffer[15]);
	
	/* Check if the current command is being aborted by the host */
	if (IsMassStoreReset)
	  return 0;
	
	return 1;
}

void SDCardManager_ReadBlocks(uint32_t BlockAddress, uint16_t TotalBlocks)
{
//	DEBUG("R %li %i\n", BlockAddress, TotalBlocks);
	
	/* Wait until endpoint is ready before continuing */
	if (Endpoint_WaitUntilReady())
	  return;
	
	while (TotalBlocks)
	{
		/* Read a data block from the SD card */
		uint8_t tmp_buffer[VIRTUAL_MEMORY_BLOCK_SIZE];

		disk_read(0, tmp_buffer, BlockAddress, 1);

		for (uint8_t i = 0; i < VIRTUAL_MEMORY_BLOCK_SIZE / 16; i++)
		{
			SDCardManager_ReadBlockHandler(tmp_buffer + (i * 16), 0, 0);
		}

		
		/* Decrement the blocks remaining counter */
		BlockAddress++;
		TotalBlocks--;
	}
	
	/* If the endpoint is full, send its contents to the host */
	if (!(Endpoint_IsReadWriteAllowed()))
	  Endpoint_ClearIN();
}

/** Performs a simple test on the attached Dataflash IC(s) to ensure that they are working.
 *
 *  \return Boolean true if all media chips are working, false otherwise
 */
bool SDCardManager_CheckDataflashOperation(void)
{	
	return true;
}

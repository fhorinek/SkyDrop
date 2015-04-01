/** \file
 *
 *  Header file for SDCardManager.c.
 */
 
#ifndef _SD_MANAGER_H
#define _SD_MANAGER_H

	/* Includes: */
		#include <avr/io.h>


		#include "../../drivers/uart.h"

		#include <LUFA/Common/Common.h>
		#include <LUFA/Drivers/USB/USB.h>

	/* Defines: */
		/** Total number of Logical Units (drives) in the device. The total device capacity is shared equally between
		 *  each drive - this can be set to any positive non-zero amount.
		 */
		#define TOTAL_LUNS                 1

		/** Blocks in each LUN, calculated from the total capacity divided by the total number of Logical Units in the device. */
		#define LUN_MEDIA_BLOCKS           (SDCardManager_GetNbBlocks() / TOTAL_LUNS)




		/** Block size of the device. This is kept at 512 to remain compatible with the OS despite the underlying
		 *  storage media (Dataflash) using a different native block size. Do not change this value.
		 */
		#define VIRTUAL_MEMORY_BLOCK_SIZE           512

	/* Function Prototypes: */
		bool SDCardManager_Init(void);
		uint32_t SDCardManager_GetNbBlocks(void);
		void SDCardManager_WriteBlocks(const uint32_t BlockAddress, uint16_t TotalBlocks);
		void SDCardManager_ReadBlocks(uint32_t BlockAddress, uint16_t TotalBlocks);
		void SDCardManager_WriteBlocks_RAM(const uint32_t BlockAddress, uint16_t TotalBlocks,
		                                      uint8_t* BufferPtr) ATTR_NON_NULL_PTR_ARG(3);
		void SDCardManagerManager_ReadBlocks_RAM(const uint32_t BlockAddress, uint16_t TotalBlocks,
		                                     uint8_t* BufferPtr) ATTR_NON_NULL_PTR_ARG(3);
		void SDCardManager_ResetDataflashProtections(void);
		bool SDCardManager_CheckDataflashOperation(void);
		
#endif

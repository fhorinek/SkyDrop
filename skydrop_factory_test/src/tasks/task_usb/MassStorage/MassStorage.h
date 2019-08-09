/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
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
 *  Header file for MassStorage.c.
 */

#ifndef _MASS_STORAGE_H_
#define _MASS_STORAGE_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>

		#include "Descriptors.h"

		#include "Lib/SCSI.h"
		#include "Lib/SDCardManager.h"
		#include "Config/AppConfig.h"

		#include "../../../drivers/uart.h"
		#include "../../../drivers/led.h"

		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Platform/Platform.h>


	/* Global Variables: */
		extern MS_CommandBlockWrapper_t  CommandBlock;
		extern MS_CommandStatusWrapper_t CommandStatus;
		extern volatile bool             IsMassStoreReset;

	/* Function Prototypes: */
		void SetupHardware(void);
		void MassStorage_Task(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);

		#if defined(INCLUDE_FROM_MASSSTORAGE_C)
			static bool ReadInCommandBlock(void);
			static void ReturnCommandStatus(void);
		#endif

	void MassStorage_Loop(void);

#define USB_IDLE	0
#define USB_ENUM	1
#define USB_NOT_RDY	2
#define USB_READY	3

extern uint8_t usb_int_state;


#endif


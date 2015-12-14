#include "common.h"

/*! \brief This macro will protect the following code from interrupts. */
#define AVR_ENTER_CRITICAL_REGION( ) uint8_t volatile saved_sreg = SREG; \
                                     cli();

/*! \brief This macro must always be used in conjunction with AVR_ENTER_CRITICAL_REGION
 *        so the interrupts are enabled again.
 */
#define AVR_LEAVE_CRITICAL_REGION( ) SREG = saved_sreg;


void CCPIOWrite(volatile uint8_t * address, uint8_t value)
{
#ifdef __ICCAVR__

	// Store global interrupt setting in scratch register and disable interrupts.
        asm("in  R1, 0x3F \n"
	    "cli"
	    );

	// Move destination address pointer to Z pointer registers.
	asm("movw r30, r16");
#ifdef RAMPZ
	asm("ldi  R16, 0 \n"
            "out  0x3B, R16"
	    );

#endif
	asm("ldi  r16,  0xD8 \n"
	    "out  0x34, r16  \n"
#if (__MEMORY_MODEL__ == 1)
	    "st     Z,  r17  \n");
#elif (__MEMORY_MODEL__ == 2)
	    "st     Z,  r18  \n");
#else /* (__MEMORY_MODEL__ == 3) || (__MEMORY_MODEL__ == 5) */
	    "st     Z,  r19  \n");
#endif /* __MEMORY_MODEL__ */

	// Restore global interrupt setting from scratch register.
        asm("out  0x3F, R1");

#elif defined __GNUC__
	AVR_ENTER_CRITICAL_REGION( );
	volatile uint8_t * tmpAddr = address;
#ifdef RAMPZ
	RAMPZ = 0;
#endif
	asm volatile(
		"movw r30,  %0"	      "\n\t"
		"ldi  r16,  %2"	      "\n\t"
		"out   %3, r16"	      "\n\t"
		"st     Z,  %1"       "\n\t"
		:
		: "r" (tmpAddr), "r" (value), "M" (CCP_IOREG_gc), "i" (&CCP)
		: "r16", "r30", "r31"
		);

	AVR_LEAVE_CRITICAL_REGION( );
#endif
}

void EnableInterrupts()
{
	//enable all levels
	PMIC.CTRL |= PMIC_LOLVLEX_bm | PMIC_MEDLVLEX_bm | PMIC_HILVLEX_bm;
	//round robin enabled for low level
	PMIC.CTRL |= PMIC_RREN_bm;
	sei();
}

void DisableInterrupts()
{
	PMIC.CTRL &= ~(PMIC_LOLVLEX_bm | PMIC_MEDLVLEX_bm | PMIC_HILVLEX_bm);
	cli();
}

uint8_t CalcCRC(uint8_t crc, uint8_t key, uint8_t data)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		if ((data & 0x01) ^ (crc & 0x01))
		{
			crc = crc >> 1;
			crc = crc ^ key;
		}
		else
			crc = crc >> 1;
		data = data >> 1;
	}

	return crc;
}

uint8_t SP_ReadCalibrationByte(uint8_t index)
{
	uint8_t result;

	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
 	result = pgm_read_byte(index);

	/* Clean up NVM Command register. */
 	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	return result;
}

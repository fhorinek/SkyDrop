#include "common.h"

void CCPIOWrite( volatile uint8_t * address, uint8_t value )
{
   volatile uint8_t * tmpAddr = address;
#ifdef RAMPZ
   RAMPZ = 0;
#endif
   asm volatile(
      "movw r30,  %0"   "\n\t"
      "ldi  r16,  %2"   "\n\t"
      "out   %3, r16"   "\n\t"
      "st     Z,  %1"
      :
      : "r" (tmpAddr), "r" (value), "M" (CCP_IOREG_gc), "m" (CCP)
      : "r16", "r30", "r31"
      );
}

void EnableInterrupts()
{
	PMIC.CTRL |= PMIC_LOLVLEX_bm | PMIC_MEDLVLEX_bm | PMIC_HILVLEX_bm;
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

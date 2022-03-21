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


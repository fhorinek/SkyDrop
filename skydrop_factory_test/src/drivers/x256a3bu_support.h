/*
 * x256a3bu_support.h
 *
 *  Created on: 3. 6. 2022
 *      Author: horinek
 */

#ifndef DRIVERS_X256A3BU_SUPPORT_H_
#define DRIVERS_X256A3BU_SUPPORT_H_

/* RTC32 - 32-bit Real-Time Counter */
/* RTC32.CTRL  bit masks and bit positions */
#define RTC32_ENABLE_bm  0x01  /* RTC enable bit mask. */
#define RTC32_ENABLE_bp  0  /* RTC enable bit position. */

/* RTC32.SYNCCTRL  bit masks and bit positions */
#define RTC32_SYNCCNT_bm  0x10  /* Synchronization Busy Flag bit mask. */
#define RTC32_SYNCCNT_bp  4  /* Synchronization Busy Flag bit position. */

#define RTC32_SYNCBUSY_bm  0x01  /* Synchronization Busy Flag bit mask. */
#define RTC32_SYNCBUSY_bp  0  /* Synchronization Busy Flag bit position. */

/* RTC32.INTCTRL  bit masks and bit positions */
#define RTC32_COMPINTLVL_gm  0x0C  /* Compare Match Interrupt Level group mask. */
#define RTC32_COMPINTLVL_gp  2  /* Compare Match Interrupt Level group position. */
#define RTC32_COMPINTLVL0_bm  (1<<2)  /* Compare Match Interrupt Level bit 0 mask. */
#define RTC32_COMPINTLVL0_bp  2  /* Compare Match Interrupt Level bit 0 position. */
#define RTC32_COMPINTLVL1_bm  (1<<3)  /* Compare Match Interrupt Level bit 1 mask. */
#define RTC32_COMPINTLVL1_bp  3  /* Compare Match Interrupt Level bit 1 position. */

#define RTC32_OVFINTLVL_gm  0x03  /* Overflow Interrupt Level group mask. */
#define RTC32_OVFINTLVL_gp  0  /* Overflow Interrupt Level group position. */
#define RTC32_OVFINTLVL0_bm  (1<<0)  /* Overflow Interrupt Level bit 0 mask. */
#define RTC32_OVFINTLVL0_bp  0  /* Overflow Interrupt Level bit 0 position. */
#define RTC32_OVFINTLVL1_bm  (1<<1)  /* Overflow Interrupt Level bit 1 mask. */
#define RTC32_OVFINTLVL1_bp  1  /* Overflow Interrupt Level bit 1 position. */

/* RTC32.INTFLAGS  bit masks and bit positions */
#define RTC32_COMPIF_bm  0x02  /* Compare Match Interrupt Flag bit mask. */
#define RTC32_COMPIF_bp  1  /* Compare Match Interrupt Flag bit position. */

#define RTC32_OVFIF_bm  0x01  /* Overflow Interrupt Flag bit mask. */
#define RTC32_OVFIF_bp  0  /* Overflow Interrupt Flag bit position. */

/* VBAT - Battery Backup Module */
/* VBAT.CTRL  bit masks and bit positions */
#define VBAT_HIGHESR_bm  0x20  /* 32-kHz Crystal Oscillator High Power Mode bit mask. */
#define VBAT_HIGHESR_bp  5  /* 32-kHz Crystal Oscillator High Power Mode bit position. */

#define VBAT_XOSCSEL_bm  0x10  /* 32-kHz Crystal Oscillator Output Selection bit mask. */
#define VBAT_XOSCSEL_bp  4  /* 32-kHz Crystal Oscillator Output Selection bit position. */

#define VBAT_XOSCEN_bm  0x08  /* Crystal Oscillator Enable bit mask. */
#define VBAT_XOSCEN_bp  3  /* Crystal Oscillator Enable bit position. */

#define VBAT_XOSCFDEN_bm  0x04  /* Crystal Oscillator Failure Detection Monitor Enable bit mask. */
#define VBAT_XOSCFDEN_bp  2  /* Crystal Oscillator Failure Detection Monitor Enable bit position. */

#define VBAT_ACCEN_bm  0x02  /* Access Enable bit mask. */
#define VBAT_ACCEN_bp  1  /* Access Enable bit position. */

#define VBAT_RESET_bm  0x01  /* Reset bit mask. */
#define VBAT_RESET_bp  0  /* Reset bit position. */

/* VBAT.STATUS  bit masks and bit positions */
#define VBAT_BBPWR_bm  0x80  /* Battery backup Power bit mask. */
#define VBAT_BBPWR_bp  7  /* Battery backup Power bit position. */

#define VBAT_XOSCRDY_bm  0x08  /* Crystal Oscillator Ready bit mask. */
#define VBAT_XOSCRDY_bp  3  /* Crystal Oscillator Ready bit position. */

#define VBAT_XOSCFAIL_bm  0x04  /* Crystal Oscillator Failure bit mask. */
#define VBAT_XOSCFAIL_bp  2  /* Crystal Oscillator Failure bit position. */

#define VBAT_BBBORF_bm  0x02  /* Battery Backup Brown-Out Reset Flag bit mask. */
#define VBAT_BBBORF_bp  1  /* Battery Backup Brown-Out Reset Flag bit position. */

#define VBAT_BBPORF_bm  0x01  /* Battery Backup Power-On Reset Flag bit mask. */
#define VBAT_BBPORF_bp  0  /* Battery Backup Power-On Reset Flag bit position. */

typedef struct VBAT_struct
{
    register8_t CTRL;  /* Control Register */
    register8_t STATUS;  /* Status Register */
    register8_t BACKUP0;  /* Backup Register 0 */
    register8_t BACKUP1;  /* Backup Register 1 */
} VBAT_t;

/* 32-bit Real-Time Counter */
typedef struct RTC32_struct
{
    register8_t CTRL;  /* Control Register */
    register8_t SYNCCTRL;  /* Synchronization Control/Status Register */
    register8_t INTCTRL;  /* Interrupt Control Register */
    register8_t INTFLAGS;  /* Interrupt Flags */
    _DWORDREGISTER(CNT);  /* Count Register */
    _DWORDREGISTER(PER);  /* Period Register */
    _DWORDREGISTER(COMP);  /* Compare Register */
} RTC32_t;

#define VBAT    (*(VBAT_t *) 0x00F0)  /* Battery Backup Module */
#define RTC32    (*(RTC32_t *) 0x0420)  /* 32-bit Real-Time Counter */


#endif /* DRIVERS_X256A3BU_SUPPORT_H_ */

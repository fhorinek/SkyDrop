/*
 * pgmhack.h
 *
 *  Created on: 17. 5. 2017
 *      Author: horinek
 */

#ifndef XLIB_PGMHACK_H_
#define XLIB_PGMHACK_H_

#include <avr/pgmspace.h>

//Deduplicating PSTR based on
//http://michael-buschbeck.github.io/arduino/2013/10/22/string-merging-pstr-percent-codes/

#undef PSTR


#define PSTR_STORE_STR(str, label) \
 asm volatile \
	( \
	  ".pushsection .progmem.data, \"SM\", @progbits, 1" "\n\t" \
	  "_p" #label ":"  									 "\n\t" \
	  "  .string " #str                                  "\n\t" \
	  "  .type _p" #label ", @object"                    "\n\t" \
	  ".popsection"                                      "\n\t" \
	);


#define PSTR_GET_ADDR(label) \
    asm volatile \
    ( \
      "ldi %A0, lo8(_p" #label ")"                   "\n\t" \
      "ldi %B0, hi8(_p" #label ")"                   "\n\t" \
      : "=d" (ptr) \
    );

#define PSTR_JOIN(str, label) \
	PSTR_STORE_STR(str, label) \
	PSTR_GET_ADDR(label)

#define PSTR(str) \
  (__extension__({ \
    PGM_P ptr;  \
    PSTR_JOIN(str, __COUNTER__) \
    &ptr[0]; \
  }))

#endif /* XLIB_PGMHACK_H_ */

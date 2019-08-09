/*
 * stdio.h
 *
 *  Created on: 4.10.2010
 *      Author: horinek
 */

#include <stdio.h>

#undef FDEV_SETUP_STREAM
#define FDEV_SETUP_STREAM(p, g, f) { 0, 0, f, 0, 0, p, g, 0 } //!< avr-gcc workaround for FDEV_SETUP_STREAM macro for c++

/**
 * Define In FILE and using getc as input method/function
 *
 * \param file FILE name
 * \param getc void method/function with single uint8_t parameter
 *
 * \note this macro have to be used outside of method/function
 */
#define CreateStdIn(file, getc) \
	int xlib_stdin_ ## file ## _call(FILE * stream) \
	{ \
		return getc();\
	} \
	\
	static FILE file ## _wrap = FDEV_SETUP_STREAM(NULL, xlib_stdin_ ## file ## _call, _FDEV_SETUP_READ); \
	FILE * file = &file ## _wrap;

/**
 * Define Out FILE and using putc as output method/function
 *
 * \param file FILE name
 * \param putc method/function with no parameter returning uint8_t byte
 *
 * \note this macro have to be used outside of method/function
 */
#define CreateStdOut(file, putc) \
	int xlib_stdout_ ## file ## _call(char data, FILE * stream) \
	{ \
		putc(data); \
		return 0;\
	} \
	\
	static FILE file ## _wrap = FDEV_SETUP_STREAM(xlib_stdout_ ## file ## _call, NULL, _FDEV_SETUP_WRITE); \
	FILE * file = &file ## _wrap;

/**
 * Set file as standard output
 *
 * \param file FILE used for output
 */
#define SetStdOut(file) stdout = file;

/**
 * Set file as standard input
 *
 * \param file FILE used for input
 */
#define SetStdIn(file) stdin = file;

/**
 * Set standard input/output
 *
 * \param in FILE used for input
 * \param out FILE used for output
 */
#define SetStdIO(in, out) \
		SetStdIn(in); \
		SetStdOut(out);

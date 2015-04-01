#ifndef OOFIX_H_
#define OOFIX_H_

extern "C" {
	#include <stdlib.h>
}

/**
 * avr-gcc workaround for new operator used in c++
 */
void * operator new(size_t size);

/**
 * avr-gcc workaround for delete operator used in c++
 */
void operator delete(void * ptr);

/**
 * avr-gcc workaround for new[] operator used in c++
 */
void * operator new[](size_t size);

/**
 * avr-gcc workaround for delete[] operator used in c++
 */
void operator delete[](void * ptr);

#endif /* OOFIX_H_ */

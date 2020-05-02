/*
 * debug_on.h
 *
 *  Created on: 6. 8. 2018
 *      Author: horinek
 */

#ifndef DISABLE_DEBUG
#ifndef DEBUG_ON_H_
#define DEBUG_ON_H_


#define DEBUG(format, ...) debug(PSTR(format), ##__VA_ARGS__)
#define DEBUG_TRACE()       DEBUG("Executing line %3d in %s\n", __LINE__, __FILE__)

// Set this to "1" to get additional debug information as DEBUG_1 will be compiled in
#define DEBUGLEVEL 0

#if DEBUGLEVEL > 0
#define DEBUG_1(args ...) DEBUG(args)
#else
#define DEBUG_1(args ...) do { } while (0)
#endif

//assert
//#define assert

#define assert(cond) \
    do{ \
    if (!(cond)) \
        {\
            PGM_P __file__ = PSTR(__FILE__); \
            DEBUG("Assert fail %S@%d!\n", __file__, __LINE__); \
        } \
    } while(0);



#endif /* DEBUG_ON_H_ */
#endif

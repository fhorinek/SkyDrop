/*
 * debug_off.h
 *
 *  Created on: 6. 8. 2018
 *      Author: horinek
 */

#ifndef DEBUG_OFF_H_
#define DEBUG_OFF_H_

#undef DEBUG
#define DEBUG(format, ...)

#undef DEBUG_1
#define DEBUG_1(args ...) do { } while (0)

#undef assert
#define assert(cond) \
    do{ \
    if (!(cond)); \
    } while(0);


#endif /* DEBUG_OFF_H_ */

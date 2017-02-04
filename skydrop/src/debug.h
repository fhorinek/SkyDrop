#ifndef DEBUG_COMMON_H_
#define DEBUG_COMMON_H_

#include "common.h"

#define DEBUG_FILE	"debug.log"

#define DEBUG_LOG_BUFFER_CHUNK	512
#define DEBUG_LOG_BUFFER_SIZE	2048

void debug_log(char * msg);
void debug(const char *format, ...);
#define DEBUG(format, ...) debug(PSTR(format), ##__VA_ARGS__)

//assert
#define assert(cond) \
	do{ \
	if (!(cond)) \
		DEBUG("Assertion failed %S@%d!\n", PSTR(__FILE__), __LINE__); \
	} while(0); \

extern uint32_t debug_last_pc;
extern volatile uint16_t debug_min_stack_pointer;
extern volatile uint16_t debug_max_heap_pointer;

void debug_uart_send(char * msg);
void debug_log(char * msg);
void debug_timer_init();
void debug_last_dump();
void debug_step();
void debug_end();
bool debug_disabled();

void ewdt_init();
void ewdt_reset();

#endif

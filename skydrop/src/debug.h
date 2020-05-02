#ifndef DEBUG_COMMON_H_
#define DEBUG_COMMON_H_

#include "common.h"

#define DEBUG_FILE	"debug.log"

#define DEBUG_LOG_BUFFER_CHUNK	256
#define DEBUG_LOG_BUFFER_SIZE	512

//#define NEED_LOT_OF_SPACE_FOR_DEBUG
#define DISABLE_DEBUG

void debug_log(char * msg);


//debug is globaly disabled use #include "debug_on.h" to enable globaly here
//or include it in other files to enable it localy
#include "debug_off.h"

extern uint32_t debug_last_pc;
extern volatile uint16_t debug_min_stack_pointer;
extern volatile uint16_t debug_max_heap_pointer;

#ifdef DISABLE_DEBUG

#define debug_step()
#define debug_end()
#define debug(...)
#define debug_last_dump()

#else

void debug_step();
void debug_end();
void debug(const char *format, ...);
void debug_last_dump();

#endif

void debug_uart_send(char * msg);
void debug_log(char * msg);
void debug_timer_init();


void debug_flush();
bool debug_disabled();

void ewdt_init();
void ewdt_reset();


#endif

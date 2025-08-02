#ifndef HOTRELOAD_HEADER
#define HOTRELOAD_HEADER
struct hotreload_t; 
typedef struct hotreload_t hotreload_t; 
typedef void (*hotreload_main_func)(void);
typedef void (*hotreload_set_state_func)(void*);
typedef void (*hotreload_get_state_func)(void*);
#endif

#ifndef GAME_HEADER
#define GAME_HEADER

// Shared state
typedef struct {
    int was_init;
    Vector2 player_pos;
    Vector2 player_vel;
} game_state_t;

typedef void (*game_loop_func)(); 
typedef game_state_t (*game_get_state_func)(); 
typedef void (*game_set_state_func)(game_state_t); 


#endif

#include "raylib.h"
#include "hotreload.c"
#include "game.h"

#include <string.h>



game_loop_func game_loop;
game_get_state_func game_get_state;
game_set_state_func game_set_state;
game_state_t game_state;


void hotreload_before()
{
    game_state = game_get_state();
}

void hotreload_after(void *main_func, void *get_state_func, void *set_state_func)
{
    game_loop = main_func;
    game_get_state = get_state_func;
    game_set_state = set_state_func;
    game_set_state(game_state);  
}



int main(void)
{
    hotreload_t hotreload_state = {
        .before_func = hotreload_before,
        .after_func = hotreload_after,
        .main_symbol = "game_loop",
        .get_state_symbol = "game_get_state",
        .set_state_symbol = "game_set_state",
    };

    const int screenWidth  = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hotreload example");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        //game_loop = (game_loop_func)hotreload_update(&hotreload_state);
        hotreload_update(&hotreload_state);
        game_loop();
    }
    CloseWindow();
    return 0;
}

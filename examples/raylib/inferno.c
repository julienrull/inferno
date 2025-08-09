#include <stdio.h>
#include <stdlib.h>
#include "../../src/inferno.h"
#include "./include/raylib.h"

typedef struct {
    int was_init;
    Vector2 player_pos;
    Vector2 player_vel;
} game_state_t;


static game_state_t game_state_inter = {0};

void game_init()
{
    game_state_inter.player_vel.y = -20;
    game_state_inter.player_pos.x = 100;
}

void game_update(float dt)
{
    if(game_state_inter.player_pos.y <= 0 || game_state_inter.player_pos.y >= GetScreenHeight()) game_state_inter.player_vel.y *= -1;
    game_state_inter.player_pos.y += game_state_inter.player_vel.y;
}

void game_draw()
{
    ClearBackground(RAYWHITE);
    DrawCircleV(game_state_inter.player_pos, 50, BLUE);
}

void game_loop()
{
    if(!game_state_inter.was_init)
    {
        game_init();
        game_state_inter.was_init = 1;
    }
    game_update(0);
    BeginDrawing();
    game_draw();
    EndDrawing();
}

// Hotreload interface

void inferno_main()
{
    game_loop();
}

void inferno_get_state(void *out_raw)
{
    *((game_state_t*)out_raw) = game_state_inter;
}

void inferno_set_state(void *in_raw)
{
    game_state_inter = *((game_state_t*)in_raw); 
}

__inferno_export inferno_interface_t inferno_interface = {
    .main = inferno_main,
    .get_state = inferno_get_state,
    .set_state = inferno_set_state,
};

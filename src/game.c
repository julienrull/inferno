#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "game.h"


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
    DrawCircleV(game_state_inter.player_pos, 50, RED);
}

// Hotreload interface

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

game_state_t game_get_state()
{
    return game_state_inter;
}

void game_set_state(game_state_t game_state)
{
    game_state_inter = game_state; 
}

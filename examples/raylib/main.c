#include <stdio.h>
#include <stdlib.h>
#include "../../src/inferno.h"
#include "raylib.h"


int main(void)
{


    inferno_t inferno = {0};
    const int screenWidth  = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hotreload example");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        inferno_update(&inferno);
    }
    CloseWindow();
    inferno_destroy(&inferno);
    return 0;
}

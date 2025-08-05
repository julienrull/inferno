#include <stdio.h>
#include <stdlib.h>
#include "../../src/inferno.h"
#include "raylib.h"


int main(void)
{


    inferno_t inferno = {0};

#if defined(_WIN32) || defined(_WIN64)
    inferno.config.libs[0] = ".\\lib\\raylib.lib";
    inferno.config.libs[1] = NULL;
#elif defined(__APPLE__) && defined(__MACH__)
    inferno.config.cc = "clang";
    inferno.config.libs[0] = "./lib/libraylib.dylib";
    inferno.config.libs[1] = NULL;

#endif
    inferno_init(&inferno);
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

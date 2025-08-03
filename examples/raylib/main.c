#include "../../src/hotreload.h"
#include "raylib.h"
int main(void)
{
    hotreload_t hotreload_state = {0};

    hotreload_init(&hotreload_state);
    const int screenWidth  = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Hotreload example");
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        hotreload_update(&hotreload_state);
    }
    CloseWindow();
    return 0;
}

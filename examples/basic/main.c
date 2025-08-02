#include <stdio.h>
#include <stdlib.h>
#define HOTRELOAD_IMPL
#include "../../src/hotreload.h"

int main(void)
{
    hotreload_t hotreload = {0};
    hotreload_init(&hotreload);

    int running = 1; 
    while(running)
    {
        hotreload_update(&hotreload);
    }

    hotreload_detroy(&hotreload);
    return 0;
}

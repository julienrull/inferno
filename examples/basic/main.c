#include <stdio.h>
#include <stdlib.h>
#include "../../src/hotreload.c"

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

#include <stdio.h>
#include <stdlib.h>
#define INFERNO_IMPL
#include "../../src/inferno.h"

int main(void)
{
    inferno_t inferno = {0};
    inferno_init(&inferno);

    int running = 1; 
    while(running)
    {
        inferno_update(&inferno);
    }
    inferno_destroy(&inferno);
    return 0;
}

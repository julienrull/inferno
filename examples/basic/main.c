#define INFERNO_IMPL
#include "../../src/inferno.h"

int main(void)
{
    inferno_t inferno = {0};

    int running = 1; 
    while(running)
    {
        inferno_update(&inferno);
    }
    inferno_destroy(&inferno);
    return 0;
}

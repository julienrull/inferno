#include <stdio.h>
#include <stdlib.h>


#if defined(_WIN32) || defined(_WIN64)
#include "inferno_config_windows.h"
#elif defined(__APPLE__) || defined(__MACH__)
#include "inferno_config_macos.h"
#elif defined(__linux__)
#include "inferno_config_linux.h"
#endif

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

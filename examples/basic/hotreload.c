#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <windows.h>

typedef struct basic_state
{
    int age;
} basic_state;

static basic_state state = {0};


__declspec(dllexport) void hotreload_main()
{
    printf("Hola ! I'm %d\n", state.age);
    Sleep(1000);
    state.age += 1;
}

__declspec(dllexport) void hotreload_get_state(void *in_raw)
{
    *((basic_state*)in_raw) = state;
}

__declspec(dllexport) void hotreload_set_state(void *out_raw)
{
    state = *((basic_state*)out_raw);
}

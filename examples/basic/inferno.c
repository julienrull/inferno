#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include "../../src/inferno.h"

typedef struct basic_state
{
    int age;
} basic_state;

static basic_state state = {0};


__inferno_export void inferno_main()
{
    printf("Heo ! I'm %d\n", state.age);
    state.age += 1;
}

__inferno_export void inferno_get_state(void *in_raw)
{
    *((basic_state*)in_raw) = state;
}

__inferno_export void inferno_set_state(void *out_raw)
{
    state = *((basic_state*)out_raw);
}

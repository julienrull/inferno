#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include "../../src/inferno_interface.h"

typedef struct basic_state
{
    int age;
} basic_state;

static basic_state state = {0};


static void inferno_main()
{
    printf("Hela ! I'm %d\n", state.age);
    state.age += 1;
}

static void inferno_get_state(void *in_raw)
{
    *((basic_state*)in_raw) = state;
}

static void inferno_set_state(void *out_raw)
{
    state = *((basic_state*)out_raw);
}


__inferno_export inferno_interface_t inferno_interface = {
    .main = inferno_main,
    .get_state = inferno_get_state,
    .set_state = inferno_set_state,
};

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>



typedef struct basic_state
{
    int age;
} basic_state;

static basic_state state = {0};


void hotreload_main()
{
    printf("hello ! I'm %d\n", state.age);
    sleep(1);
    state.age += 1;
}

void hotreload_get_state(void *in_raw)
{
    *((basic_state*)in_raw) = state;
}

void hotreload_set_state(void *out_raw)
{
    state = *((basic_state*)out_raw);
}

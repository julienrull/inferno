#ifndef HOTRELOAD_CONFIG_HEADER
#define HOTRELOAD_CONFIG_HEADER


static char *const hotreload_compiler_output = "./bin/game.so";
static char *const hotreload_compiler_output_tmp = "./bin/game.tmp.so";

static char *const hotreload_compiler = "gcc";
static char *const hotreload_compiler_flags[] = {
    "-std=c99",
    "-L./lib",
    "-I./include",
    "-shared",
};
static char *const hotreload_compiler_sources[] = {
    "./src/game.c",
};
static char *const reload_cmd[] = { 
    hotreload_compiler,
    hotreload_compiler,

    hotreload_compiler_flags[0],
    hotreload_compiler_flags[1],
    hotreload_compiler_flags[2],
    hotreload_compiler_flags[3],

    hotreload_compiler_sources[0],

    "-o", hotreload_compiler_output, (char*)NULL 
};


#endif


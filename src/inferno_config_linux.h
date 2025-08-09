#ifndef INFERNO_CONFIG_H
#define INFERNO_CONFIG_H

 
/* hot reloaded code state storage */ 
#define INFERNO_STORAGE_SIZE 1024

/* watched source files for changes */ 
static const char *watched[] = { "inferno.c" };

/* shared lib name */ 
static const char output[] = "./inferno.so";

static const char *shared_build_cmd[] = {
    "gcc", 
    "-g", 
    "-fPIC",
    "-shared", 
    "-std=c99", 
    "-Wall", 
    "-Werror",
    "inferno.c",
    "-o", output,
    (char*)NULL
};
#endif

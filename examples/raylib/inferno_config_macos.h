#ifndef INFERNO_CONFIG_H
#define INFERNO_CONFIG_H

 
/* hot reloaded code state storage */ 
#define INFERNO_STORAGE_SIZE 1024

/* source files watched for changes */ 
static const char *const watched[] = { "inferno.c" };


/* shared lib name */ 
static const char output[] = "./bin/inferno.dylib";

static const char *shared_build_cmd[] = {
    "clang", 
    "-g", 
    "-shared", 
    "-std=c99", 
    "-Wall", 
    "-Werror",
    watched[0],
    "-o", output,
    "./lib/libraylib.5.5.0.dylib", 
    (char*)NULL
};
#endif

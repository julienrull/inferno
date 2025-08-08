#ifndef INFERNO_CONFIG_H
#define INFERNO_CONFIG_H

 
/* hot reloaded code state storage */ 
#define INFERNO_STORAGE_SIZE 1024

/* source files watched for changes */ 
static const char *const watched[] = { "inferno.c" };

/* shared lib name */ 
static const char output[] = "./bin/inferno.dll";

static const char *shared_build_cmd[] = {
    "cl", 
    "/LD", 
    watched[0],
    "/link", "/OUT:", output,
    ".\\lib\\raylib.lib"
    (char*)NULL
};
#endif

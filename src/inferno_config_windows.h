#ifndef INFERNO_CONFIG_H
#define INFERNO_CONFIG_H

 
/* hot reloaded code state storage */ 
#define INFERNO_STORAGE_SIZE 1024

/* watched source files for changes */ 
static const char *watched[] = { "inferno.c" };

/* shared lib name */ 
#define INFERNO_OUTPUT ".\\inferno.dll"
static const char output[] = INFERNO_OUTPUT;

static const char *shared_build_cmd[] = {
    "cl", 
    "/LD", 
    "inferno.c",
    "/link", "/OUT:" INFERNO_OUTPUT,
    (char*)NULL
};
#endif

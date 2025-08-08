#ifndef INFERNO_CONFIG_H
#define INFERNO_CONFIG_H



 
/* hot reloaded code state storage */ 
#define INFERNO_STORAGE_SIZE 1024
static char storage[INFERNO_STORAGE_SIZE] = {0};


/* source files watched for changes */ 
static const char *const watched[] = { "inferno.c" };


/* Linux */ 

/* shared lib name */ 
static const char output[] = "inferno.so";

static const char *shared_build_cmd[] = {
    "gcc", 
    "-g", 
    "-shared", 
    "-std=c99", 
    "-Wall", 
    "-Werror",
    watched[0],
    "-o", output,
    (char*)NULL
};




/* MacOS 
static const char *shared_build_cmd[] = {
    "clang", 
    "-g", 
    "-shared", 
    "-std=c99", 
    "-Wall", 
    "-Werror",
    watched[0],
    "-o", "inferno.dylib",
    (char*)NULL
};
 */

/* Windows 
static const char *shared_build_cmd[] = {
    "cl", 
    "/LD", 
    watched[0],
    "/link", "/OUT:inferno.dll",
    (char*)NULL
};
 */
#endif

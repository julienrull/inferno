#ifndef HOTRELOAD_H
#define HOTRELOAD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

struct hotreload_t; 
typedef struct hotreload_t hotreload_t; 
enum hotreload_action_t;
typedef enum hotreload_action_t hotreload_action_t;

typedef void (*hotreload_main_func)(void);
typedef void (*hotreload_set_state_func)(void*);
typedef void (*hotreload_get_state_func)(void*);

void hotreload_init(hotreload_t *hotreload_state);
int hotreload_watch_sources(hotreload_t *hotreload_state);
int hotreload_copy(const char *dst, const char *src); 
hotreload_action_t hotreload_get_action(hotreload_t *hotreload);
void hotreload_update(hotreload_t *hotreload);
void hotreload_detroy(hotreload_t *hotreload);
void hotreload_compile(hotreload_t *hotreload_state);
void hotreload_reload(hotreload_t *hotreload);

#if defined(__linux__) || defined(__unix__) || defined(__posix__)

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h> 
#ifdef HOTRELOAD_IMPL
#define HOTRELOAD_LINUX_IMPL
#endif //HOTRELOAD_IMPL
       
#elif defined(_WIN32) || defined(_WIN64)

#ifdef HOTRELOAD_IMPL
#define HOTRELOAD_WIN32_IMPL
#endif // HOTRELOAD_IMPL
       
#elif defined(__APPLE__) && defined(__MACH__)

#ifdef HOTRELOAD_IMPL
#define HOTRELOAD_DARWIN_IMPL
#endif // HOTRELOAD_IMPL
       
#endif // defined(__linux__) || defined(__unix__) || defined(__posix__)


#ifdef HOTRELOAD_IMPL
struct hotreload_t 
{
    const char *exe_path;
    char **sources;
    int source_count;
    char *const  *cmd;
    const char *main_symbol;
    const char *get_state_symbol;
    const char *set_state_symbol;
    void *handle;
    char *error;
    int last_mtime;
    int srcs_last_mtime;
    int has_started;
};

typedef enum hotreload_action_t
{
    HOTRELOAD_ACTION_NONE       = 0,
    HOTRELOAD_ACTION_COMPILE    = 1,
    HOTRELOAD_ACTION_RELOAD     = 2
} hotreload_action_t;


#define HOTRELOAD_COMPILER "gcc"
#define HOTRELOAD_SHARED_LIB_NAME "hotreload.so"
#define HOTRELOAD_PATH_SIZE 1024
#define HOTRELOAD_STATE_SIZE 1024

static char shared_lib_path[HOTRELOAD_PATH_SIZE] = {0};
static char shared_lib_tmp_path[HOTRELOAD_PATH_SIZE] = {0};
static char hotreload_app_state[HOTRELOAD_STATE_SIZE] = {0};

static char *hotreload_default_sources[] = { 
    "hotreload.c",
};

static char *const hotreload_default_linux_cmd[] = { 
    HOTRELOAD_COMPILER,
    HOTRELOAD_COMPILER,
    "-std=c99",
    "-L./lib",
    "-I./include",
    "-shared",
    "hotreload.c",
    "-o", 
    HOTRELOAD_SHARED_LIB_NAME, 
    (char*)NULL 
};



static hotreload_main_func hotreload_main_extern;
static hotreload_get_state_func hotreload_get_state_extern;
static hotreload_set_state_func hotreload_set_state_extern;

static hotreload_t default_config = (hotreload_t){
    .exe_path           = ".",
    .sources            = hotreload_default_sources,
    .source_count       = 1,
    .cmd                = hotreload_default_linux_cmd,
    .main_symbol        = "hotreload_main",
    .get_state_symbol   = "hotreload_get_state",
    .set_state_symbol   = "hotreload_set_state",
    .handle             = NULL,
    .error              = NULL,
    .last_mtime         = 0,
    .srcs_last_mtime    = 0,
    .has_started        = 0,
}; 


void hotreload_init(hotreload_t *hotreload_state)
{
    if(!hotreload_state)
    {
        fprintf(stderr, "HOTRELOAD INIT ERROR: invalid input.\n"); 
        exit(1);
    }
    *hotreload_state = default_config;

}

int hotreload_watch_sources(hotreload_t *hotreload_state)
{
    int i = 0;
    int are_changes = -1;
    while(i < hotreload_state->source_count && are_changes != 0)
    {
        struct stat attr;
        if (stat(hotreload_state->sources[i], &attr) == 0) {
            if (attr.st_mtime != hotreload_state->srcs_last_mtime) {
                hotreload_state->srcs_last_mtime = attr.st_mtime;
                are_changes = 0;
            }
        }
        i += 1;
    }
    return are_changes;
}

int hotreload_copy(const char *dst, const char *src) 
{
    FILE *in = fopen(src, "rb");
    if (!in) return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        fwrite(buffer, 1, bytes, out);
    }
    fclose(in);
    fclose(out);
    return 0;
}
hotreload_action_t hotreload_get_action(hotreload_t *hotreload)
{
    struct stat attr;
    memset(shared_lib_path, 0, HOTRELOAD_PATH_SIZE); 
    memset(shared_lib_tmp_path, 0, HOTRELOAD_PATH_SIZE); 

    strcat(shared_lib_path, hotreload->exe_path);
    strcat(shared_lib_path, "/");
    strcat(shared_lib_path, HOTRELOAD_SHARED_LIB_NAME);

    strcat(shared_lib_tmp_path, hotreload->exe_path);
    strcat(shared_lib_tmp_path, "/tmp_");
    strcat(shared_lib_tmp_path, HOTRELOAD_SHARED_LIB_NAME);

    if (stat(shared_lib_path, &attr) != 0) {
        return HOTRELOAD_ACTION_COMPILE;
    }
    if(hotreload_watch_sources(hotreload) == 0)
    {
        return HOTRELOAD_ACTION_COMPILE;
    }
    if (attr.st_mtime != hotreload->last_mtime) {
        hotreload->last_mtime = attr.st_mtime;
        return HOTRELOAD_ACTION_RELOAD;
    }
    return HOTRELOAD_ACTION_NONE;
}
void hotreload_update(hotreload_t *hotreload)
{
    hotreload_action_t action = hotreload_get_action(hotreload);
    if(hotreload_get_state_extern)
    {
        hotreload_get_state_extern(&hotreload_app_state);
    }
    switch(action)
    {
        case HOTRELOAD_ACTION_COMPILE:{
            printf("HOTRELOAD_ACTION_COMPILE\n");
            hotreload_compile(hotreload);
        }break;
        case HOTRELOAD_ACTION_RELOAD:{
            printf("HOTRELOAD_ACTION_RELOAD\n");
            hotreload_reload(hotreload);
            if(hotreload_set_state_extern)
            {
                hotreload_set_state_extern(&hotreload_app_state);
            }
        }break;
        case HOTRELOAD_ACTION_NONE: {
            printf("HOTRELOAD_ACTION_NONE\n");
            hotreload_main_extern();
        }break;
    }
}

void hotreload_detroy(hotreload_t *hotreload)
{
    if(hotreload) dlclose(hotreload->handle);
}
#endif

#ifdef HOTRELOAD_LINUX_IMPL
void hotreload_compile(hotreload_t *hotreload_state)
{
    pid_t pid = fork();
    if(pid == -1)
    {
        fprintf(stderr, "REOLAD COMMAND ERROR: fork failed.\n"); 
        exit(1);
    }

    // Child process
    if(pid == 0){
        // Exec stuff
        if(execvp(hotreload_state->cmd[0], &hotreload_state->cmd[1]) == -1) 
        {
            fprintf(stderr, "REOLAD COMMAND ERROR: execl failed.\n"); 
            exit(1); 
            // TODO(julienrull): may quit the child process but not the parent. 
            // How to notice the parent ?
        }
    }else{
        // parent stuff
        // wait of child with pid to finish
        wait(NULL);
    }
}
void hotreload_reload(hotreload_t *hotreload)
{
    // UNLOAD CURRENT 
    if(hotreload->handle) dlclose(hotreload->handle);
    // COPY & RENAME TMP
    if(hotreload_copy(shared_lib_tmp_path, shared_lib_path) == -1)
    {
        fprintf(stderr, "RELOAD ERROR: Fail to copy new shared.");
        exit(1);
    }
    // LOAD DYNLIB
    hotreload->handle = dlopen(shared_lib_tmp_path, RTLD_LAZY);
    if (!hotreload->handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        exit(1);
    }
    // CLEAR ERROR
    dlerror();    
    // FETCH API
    hotreload_main_extern = dlsym(hotreload->handle, hotreload->main_symbol);
    if ((hotreload->error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", hotreload->error);
        exit(1);
    }
    hotreload_get_state_extern = dlsym(hotreload->handle, hotreload->get_state_symbol);
    if ((hotreload->error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", hotreload->error);
        exit(1);
    }
    hotreload_set_state_extern = dlsym(hotreload->handle, hotreload->set_state_symbol);
    if ((hotreload->error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", hotreload->error);
        exit(1);
    }
    hotreload->has_started = 1;
}
#endif


#ifdef HOTRELOAD_WIN32_IMPL

#endif

#endif














#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h> 
#include <stdint.h>
#include <string.h>
#include "hotreload.h"


#define HOTRELOAD_COMPILER "gcc"
#define HOTRELOAD_SHARED_LIB_NAME "hotreload.so"

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

//: default
//


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


static char hotreload_app_state[1024] = {0};

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

void trigger_reload(hotreload_t *hotreload_state)
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

void trigger_on_save(hotreload_t *hotreload_state)
{
    int i = 0;
    int reload = 0;
    while(i < hotreload_state->source_count && !reload)
    {
        struct stat attr;
        if (stat(hotreload_state->sources[i], &attr) == 0) {
            if (attr.st_mtime != hotreload_state->srcs_last_mtime) {
                printf("Srcs changed!\n");
                hotreload_state->srcs_last_mtime = attr.st_mtime;
                reload = 1;
                trigger_reload(hotreload_state);
            }
        }
        i += 1;
    }


}


int reload_copy(const char *dst, const char *src) 
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


void reload(hotreload_t *hotreload)
{
    int changed = 0;
    struct stat attr;
    char shared_lib_path[1024] = {0};
    char shared_lib_tmp_path[1024] = {0};
    
    strcat(shared_lib_path, hotreload->exe_path);
    strcat(shared_lib_path, "/");
    strcat(shared_lib_path, HOTRELOAD_SHARED_LIB_NAME);

    strcat(shared_lib_tmp_path, hotreload->exe_path);
    strcat(shared_lib_tmp_path, "/tmp_");
    strcat(shared_lib_tmp_path, HOTRELOAD_SHARED_LIB_NAME);

    if (stat(shared_lib_path, &attr) == 0) {
        if (attr.st_mtime != hotreload->last_mtime) {
            printf("File changed!\n");
            hotreload->last_mtime = attr.st_mtime;
            changed = 1;
        }
    }
    else
    {
        trigger_reload(hotreload);
        changed = 1;
    }
    if(hotreload_get_state_extern)
    {
        hotreload_get_state_extern(&hotreload_app_state);
    }
    if(changed)
    {
        // UNLOAD CURRENT 
        if(hotreload->handle) dlclose(hotreload->handle);
        // COPY & RENAME TMP
        if(reload_copy(shared_lib_tmp_path, shared_lib_path) == -1)
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
    if(hotreload_set_state_extern)
    {
        hotreload_set_state_extern(&hotreload_app_state);
    }
    hotreload_main_extern();
}

void hotreload_update(hotreload_t *hotreload)
{
        trigger_on_save(hotreload);
        reload(hotreload);
}

void hotreload_detroy(hotreload_t *hotreload)
{
    if(hotreload) dlclose(hotreload->handle);
}

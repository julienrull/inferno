#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h> 
#include <stdint.h>
#include "hotreload_config.h"
#include "hotreload.h"


struct hotreload_t 
{
    void *handle;
    char *error;
    int last_mtime;
    int srcs_last_mtime;
    int has_started;
    const char *main_symbol;
    const char *get_state_symbol;
    const char *set_state_symbol;
    void (*before_func)();
    void (*after_func)(void *main_func, void *get_state_func, void *set_state_func);
};


void trigger_reload()
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
        if(execvp(reload_cmd[0], &reload_cmd[1]) == -1) 
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

void trigger_on_save(hotreload_t *hotreload)
{
    int i = 0;
    int reload = 0;
    int count = (int)(sizeof(hotreload_compiler_sources) / sizeof(hotreload_compiler_sources[0]));
    while(i < count && !reload)
    {
        struct stat attr;
        if (stat(hotreload_compiler_sources[i], &attr) == 0) {
            if (attr.st_mtime != hotreload->srcs_last_mtime) {
                reload = 1;
                printf("Srcs changed!\n");
                hotreload->srcs_last_mtime = attr.st_mtime;
                trigger_reload();
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
    if (stat(hotreload_compiler_output, &attr) == 0) {
        if (attr.st_mtime != hotreload->last_mtime) {
            printf("File changed!\n");
            hotreload->last_mtime = attr.st_mtime;
            changed = 1;
        }
    }
    else
    {
        trigger_reload();
        changed = 1;
    }
    if(changed)
    {
        if(hotreload->has_started)
        {
            hotreload->before_func();
        }

        hotreload = hotreload;
        // UNLOAD CURRENT 
        if(hotreload->handle) dlclose(hotreload->handle);
        // COPY & RENAME TMP
        if(reload_copy(hotreload_compiler_output_tmp, hotreload_compiler_output) == -1)
        {
            fprintf(stderr, "RELOAD ERROR: Fail to copy new shared.");
            exit(1);
        }
        // LOAD DYNLIB
        hotreload->handle = dlopen(hotreload_compiler_output_tmp, RTLD_LAZY);
        if (!hotreload->handle) {
            fprintf(stderr, "Error: %s\n", dlerror());
            exit(1);
        }
        // CLEAR ERROR
        dlerror();    
        // FETCH API
        void *main_func = dlsym(hotreload->handle, hotreload->main_symbol);
        if ((hotreload->error = dlerror()) != NULL)  {
            fprintf(stderr, "Error: %s\n", hotreload->error);
            exit(1);
        }
        void *get_state_func = dlsym(hotreload->handle, hotreload->get_state_symbol);
        if ((hotreload->error = dlerror()) != NULL)  {
            fprintf(stderr, "Error: %s\n", hotreload->error);
            exit(1);
        }
        void *set_state_func = dlsym(hotreload->handle, hotreload->set_state_symbol);
        if ((hotreload->error = dlerror()) != NULL)  {
            fprintf(stderr, "Error: %s\n", hotreload->error);
            exit(1);
        }
        hotreload->after_func(main_func, get_state_func, set_state_func);
        hotreload->has_started = 1;
    }
}

void hotreload_update(hotreload_t *hotreload)
{
        trigger_on_save(hotreload);
        reload(hotreload);
}

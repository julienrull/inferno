#ifndef INFERNO_H
#define INFERNO_H


#define INFERNO_CMD_MAX_SIZE 1024
#define INFERNO_MAX_SOURCES 1024
#define INFERNO_TMP_OUTPUT_SIZE 128
typedef struct inferno_t 
{
    void *handle;
    char *error;
    int last_mtime;
    int has_started;
    int srcs_last_mtime[INFERNO_MAX_SOURCES];
} inferno_t;

typedef enum inferno_action_t
{
    INFERNO_ACTION_NONE       = 0,
    INFERNO_ACTION_COMPILE    = 1,
    INFERNO_ACTION_RELOAD     = 2
} inferno_action_t;




int inferno_watch_sources(inferno_t *inferno);
int inferno_copy(const char *dst, const char *src); 
inferno_action_t inferno_get_action(inferno_t *inferno);
void inferno_swap();
void inferno_update(inferno_t *inferno);
void inferno_destroy(inferno_t *inferno);
void inferno_compile(inferno_t *inferno);
void inferno_reload(inferno_t *inferno);



#if defined(__linux__) || defined(__unix__) || defined(__posix__) ||defined(__APPLE__) || defined(__MACH__)


#ifdef INFERNO_IMPL
#define INFERNO_LINUX_IMPL
#endif //INFERNO_IMPL
       
#elif defined(_WIN32) || defined(_WIN64)


#ifdef INFERNO_IMPL
#define INFERNO_WIN32_IMPL
#endif // INFERNO_IMPL
       //
#endif // defined


#ifdef INFERNO_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "inferno_config.h"
#include "inferno_interface.h"

static inferno_interface_t *inferno_interface;
static char tmp_output[INFERNO_TMP_OUTPUT_SIZE] = {0}; 


int inferno_watch_sources(inferno_t *inferno)
{
    int are_changes = -1;
    size_t count = sizeof(watched) / sizeof(watched[0]);
    int i = 0;
    while( i < count && are_changes != 0)
    {
        struct stat attr;
        if (stat(watched[i], &attr) == 0) {
            if (attr.st_mtime != inferno->srcs_last_mtime[i]) {
                inferno->srcs_last_mtime[i] = attr.st_mtime;
                are_changes = 0;
            }
        }
        i += 1;
    }
    return are_changes;
}

int inferno_copy(const char *dst, const char *src) 
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


void inferno_swap()
{

    memset(tmp_output, 0, INFERNO_TMP_OUTPUT_SIZE);
    strcat(tmp_output, "./tmp_");
    strcat(tmp_output, output);
    if(inferno_copy(tmp_output, output) == -1)
    {
        fprintf(stderr, "RELOAD ERROR: Fail to copy new shared.");
        exit(1);
    }
}

inferno_action_t inferno_get_action(inferno_t *inferno)
{
    struct stat attr;
    int res_state = stat(output, &attr);
    if ( res_state != 0 || inferno_watch_sources(inferno) == 0) 
    {
        return INFERNO_ACTION_COMPILE;
    }

    if ((res_state == 0 && !inferno_interface) || (res_state == 0 && attr.st_mtime != inferno->last_mtime)) 
    {
        inferno->last_mtime = attr.st_mtime;
        return INFERNO_ACTION_RELOAD;
    }
    return INFERNO_ACTION_NONE;
}
void inferno_update(inferno_t *inferno)
{
    inferno_action_t action = inferno_get_action(inferno);
    if(inferno_interface && inferno_interface->get_state)
    {
        inferno_interface->get_state(&storage);
    }
    switch(action)
    {
        case INFERNO_ACTION_COMPILE:{
            printf("INFERNO_ACTION_COMPILE\n");
            inferno_compile(inferno);
        }break;
        case INFERNO_ACTION_RELOAD:{
            printf("INFERNO_ACTION_RELOAD\n");
            inferno_reload(inferno);
            if(inferno_interface && inferno_interface->set_state)
            {
                inferno_interface->set_state(&storage);
            }
        }break;
        case INFERNO_ACTION_NONE: {
            printf("INFERNO_ACTION_NONE\n");
            if(inferno_interface && inferno_interface->main)
            {
                inferno_interface->main();
            }
        }break;
    }
}

#endif

#if defined(INFERNO_LINUX_IMPL) || defined(INFERNO_DARWIN_IMPL)


#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h> 

void inferno_compile(inferno_t *inferno)
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
        if(execvp(shared_build_cmd[0], (char *const *)shared_build_cmd) == -1) 
        {
            fprintf(stderr, "REOLAD COMMAND ERROR: execv failed.\n"); 
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
void inferno_reload(inferno_t *inferno)
{
    // UNLOAD CURRENT 
    if(inferno->handle) dlclose(inferno->handle);
    // COPY & RENAME TMP
    
    inferno_swap();
    // LOAD DYNLIB
    printf("%s\n", tmp_output);
    inferno->handle = dlopen(tmp_output, RTLD_LAZY);
    if (!inferno->handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        exit(1);
    }
    // CLEAR ERROR
    dlerror();    
    // FETCH API
    
    inferno_interface = (inferno_interface_t*)dlsym(inferno->handle, "inferno_interface");
    if ((inferno->error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", inferno->error);
        exit(1);
    }
    inferno->has_started = 1;
}
void inferno_destroy(inferno_t *inferno)
{
    if(inferno) dlclose(inferno->handle);
}
#endif


#ifdef INFERNO_WIN32_IMPL
#include <windows.h>



void inferno_compile(inferno_t *inferno)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char cmd[1024] = {0}; 
    int i = 0;
    while(shared_build_cmd[i] != NULL)
    {
        strcat(cmd, shared_build_cmd[i]);
        if(shared_build_cmd[i + 1] != NULL)
        {
            strcat(cmd, " ");
        }
        i += 1;
    }

    if (!CreateProcess(
            NULL,       
            cmd,    
            NULL,       
            NULL,       
            FALSE,      
            0,          
            NULL,       
            NULL,       
            &si,        
            &pi)) 
    {
        fprintf(stderr, "Could not create child process.\n");
        exit(1);
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
void inferno_reload(inferno_t *inferno)
{
    // UNLOAD CURRENT 
    if(inferno->handle) FreeLibrary((HMODULE)inferno->handle);
    // COPY & RENAME TMP
    inferno_swap();
    // LOAD DYNLIB
    inferno->handle = LoadLibraryA(inferno_shared_lib_tmp_path);
    if (!inferno->handle) {
        fprintf(stderr, "Failed to load DLL. Error code: %lu\n", GetLastError());
        exit(1);
    }
    // FETCH API
    inferno_interface = (inferno_interface_t*)GetProcAddress((HMODULE)inferno->handle, "inferno_interface");
    if (!inferno_interface)  {
        fprintf(stderr, "Failed to load main_symbol. Error code: %lu\n", GetLastError());
        exit(1);
    }
    inferno->has_started = 1;
}

void inferno_destroy(inferno_t *inferno)
{
    if(inferno) FreeLibrary((HMODULE)inferno->handle);
}
#endif

#endif














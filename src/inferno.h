#ifndef INFERNO_H
#define INFERNO_H

#define INFERNO_CMD_MAX_ARGS 128
#define INFERNO_CMD_STR_MAX_SIZE 1024

#define INFERNO_CMD_MAX_FLAGS 1024
#define INFERNO_CMD_MAX_SRCS 1024
#define INFERNO_CMD_MAX_LIBS 1024

#define INFERNO_PATH_SIZE 1024
#define INFERNO_STATE_SIZE 1024

typedef struct inferno_config_t {
    char *cc; 
    char *exe_path; 
    char *exe; 
    char *flags[INFERNO_CMD_MAX_FLAGS]; 
    char *srcs[INFERNO_CMD_MAX_SRCS]; 
    char *libs[INFERNO_CMD_MAX_LIBS]; 
    char *main_symbol;
    char *get_state_symbol;
    char *set_state_symbol;
} inferno_config_t; 


typedef struct inferno_state_t {
    void *handle;
    char *error;
    int last_mtime;
    int srcs_last_mtime;
    int has_started;
    char *cmd[INFERNO_CMD_MAX_ARGS]; 
    char cmd_str[INFERNO_CMD_STR_MAX_SIZE]; 
} inferno_state_t; 

typedef struct inferno_t 
{
    inferno_config_t config;
    inferno_state_t state;
} inferno_t;

typedef enum inferno_action_t
{
    INFERNO_ACTION_NONE       = 0,
    INFERNO_ACTION_COMPILE    = 1,
    INFERNO_ACTION_RELOAD     = 2
} inferno_action_t;


typedef void (*inferno_main_func)(void);
typedef void (*inferno_set_state_func)(void*);
typedef void (*inferno_get_state_func)(void*);



void inferno_init(inferno_t *inferno);
int inferno_watch_sources(inferno_t *inferno);
int inferno_copy(const char *dst, const char *src); 
inferno_action_t inferno_get_action(inferno_t *inferno);
void inferno_update(inferno_t *inferno);
void inferno_destroy(inferno_t *inferno);
void inferno_compile(inferno_t *inferno);
void inferno_reload(inferno_t *inferno);


#if defined(__linux__) || defined(__unix__) || defined(__posix__)
#define __inferno_export
#ifdef INFERNO_IMPL
#define INFERNO_LINUX_IMPL
#endif //INFERNO_IMPL
       
#elif defined(_WIN32) || defined(_WIN64)

#define __inferno_export __declspec(dllexport)

#ifdef INFERNO_IMPL
#define INFERNO_WIN32_IMPL
#endif // INFERNO_IMPL
       
#elif defined(__APPLE__) && defined(__MACH__)

#ifdef INFERNO_IMPL
#define INFERNO_DARWIN_IMPL
#endif // INFERNO_IMPL
       
#endif // defined(__linux__) || defined(__unix__) || defined(__posix__)


#ifdef INFERNO_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

static char inferno_shared_lib_path[INFERNO_PATH_SIZE] = {0};
static char inferno_shared_lib_tmp_path[INFERNO_PATH_SIZE] = {0};
static char inferno_app_state[INFERNO_STATE_SIZE] = {0};

static inferno_main_func inferno_main_extern;
static inferno_get_state_func inferno_get_state_extern;
static inferno_set_state_func inferno_set_state_extern;


int inferno_watch_sources(inferno_t *inferno)
{
    int i = 0;
    int are_changes = -1;
    while(inferno->config.srcs[i] != NULL && are_changes != 0)
    {
        struct stat attr;
        if (stat(inferno->config.srcs[i], &attr) == 0) {
            if (attr.st_mtime != inferno->state.srcs_last_mtime) {
                inferno->state.srcs_last_mtime = attr.st_mtime;
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
inferno_action_t inferno_get_action(inferno_t *inferno)
{
    struct stat attr;
    memset(inferno_shared_lib_path, 0, INFERNO_PATH_SIZE); 
    memset(inferno_shared_lib_tmp_path, 0, INFERNO_PATH_SIZE); 

    strcat(inferno_shared_lib_path, inferno->config.exe_path);
    strcat(inferno_shared_lib_path, "/");
    strcat(inferno_shared_lib_path, inferno->config.exe);

    strcat(inferno_shared_lib_tmp_path, inferno->config.exe_path);
    strcat(inferno_shared_lib_tmp_path, "/tmp_");
    strcat(inferno_shared_lib_tmp_path, inferno->config.exe);

    if (stat(inferno_shared_lib_path, &attr) != 0) {
        return INFERNO_ACTION_COMPILE;
    }
    if(inferno_watch_sources(inferno) == 0)
    {
        return INFERNO_ACTION_COMPILE;
    }
    if (attr.st_mtime != inferno->state.last_mtime) {
        inferno->state.last_mtime = attr.st_mtime;
        return INFERNO_ACTION_RELOAD;
    }
    return INFERNO_ACTION_NONE;
}
void inferno_update(inferno_t *inferno)
{
    inferno_action_t action = inferno_get_action(inferno);
    if(inferno_get_state_extern)
    {
        inferno_get_state_extern(&inferno_app_state);
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
            if(inferno_set_state_extern)
            {
                inferno_set_state_extern(&inferno_app_state);
            }
        }break;
        case INFERNO_ACTION_NONE: {
            printf("INFERNO_ACTION_NONE\n");
            inferno_main_extern();
        }break;
    }
}

void inferno_init(inferno_t *inferno)
{
    if(!inferno) 
    {
        fprintf(stderr, "ERROR in inferno_init: wrong input.\n"); 
        exit(1); 
    }
    inferno_t tmp_inferno = {
        .config = {
            .cc = inferno->config.cc ? inferno->config.cc : "gcc",
            .flags = { (char*)NULL },
            .srcs = { "inferno.c", (char*)NULL }, 
            .libs = { (char*)NULL },
            .exe_path = inferno->config.exe_path ? inferno->config.exe_path : ".", 
            .exe = inferno->config.exe ? inferno->config.exe : "inferno.so", 
            .main_symbol = inferno->config.main_symbol ? inferno->config.main_symbol: "inferno_main", 
            .get_state_symbol = inferno->config.get_state_symbol ? inferno->config.get_state_symbol : "inferno_main", 
            .set_state_symbol = inferno->config.set_state_symbol ? inferno->config.set_state_symbol : "inferno_main", 
        }
    };

    tmp_inferno.state.cmd[0] = tmp_inferno.config.cc;
    tmp_inferno.state.cmd[1] = tmp_inferno.config.cc;
    tmp_inferno.state.cmd[2] = "-std=c99";
    tmp_inferno.state.cmd[3] = "-shared";
    strcat(tmp_inferno.state.cmd_str, "cl");
    strcat(tmp_inferno.state.cmd_str, " /nologo /LD");

    int i = 4;
    if (inferno->config.flags[0] != NULL)
    {
        int j = 0;
        while (inferno->config.flags[j] != NULL)
        {
            tmp_inferno.state.cmd[i] = inferno->config.flags[j];
            strcat(tmp_inferno.state.cmd_str, " ");
            strcat(tmp_inferno.state.cmd_str, inferno->config.flags[j]);
            i += 1;
            j += 1;
        }
    }
    tmp_inferno.state.cmd[i] = "inferno.c";
    if (inferno->config.srcs[0] != NULL)
    {
        int j = 0;
        while (inferno->config.srcs[j] != NULL)
        {
            tmp_inferno.state.cmd[i] = inferno->config.srcs[j];
            tmp_inferno.config.srcs[j] = inferno->config.srcs[j];
            strcat(tmp_inferno.state.cmd_str, " ");
            strcat(tmp_inferno.state.cmd_str, inferno->config.srcs[j]);
            i += 1;
            j += 1;
        }
        tmp_inferno.config.srcs[j] = (char*)NULL;
    }else{
            strcat(tmp_inferno.state.cmd_str, " inferno.c");
            i+=1;
    }

    tmp_inferno.state.cmd[i] = "-o";
    i += 1;
    tmp_inferno.state.cmd[i] = tmp_inferno.config.exe;
    i += 1;
    strcat(tmp_inferno.state.cmd_str, " /link /OUT:");
    strcat(tmp_inferno.state.cmd_str, "inferno.dll");

    tmp_inferno.state.cmd[i] = (char*)NULL;
    if (inferno->config.libs[0] != NULL)
    {
        int j = 0;
        while (inferno->config.libs[j] != NULL)
        {
            tmp_inferno.state.cmd[i] = inferno->config.libs[j];
            strcat(tmp_inferno.state.cmd_str, " ");
            strcat(tmp_inferno.state.cmd_str, inferno->config.libs[j]);
            i += 1;
            j += 1;
        }
    }
    tmp_inferno.state.cmd[i] = (char*)NULL;
    *inferno = tmp_inferno;
    //i = 0;
    //while(tmp_inferno.state.cmd[i] != NULL)
    //{
    //    printf("%s\n", tmp_inferno.state.cmd[i]);
    //    i += 1;
    //}
    //exit(0);
}

#endif

#ifdef INFERNO_LINUX_IMPL


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
        if(execvp(inferno->state.cmd[0], &inferno->state.cmd[1]) == -1) 
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
    if(inferno->state.handle) dlclose(inferno->state.handle);
    // COPY & RENAME TMP
    if(inferno_copy(inferno_shared_lib_tmp_path, inferno_shared_lib_path) == -1)
    {
        fprintf(stderr, "RELOAD ERROR: Fail to copy new shared.");
        exit(1);
    }
    // LOAD DYNLIB
    inferno->state.handle = dlopen(inferno_shared_lib_tmp_path, RTLD_LAZY);
    if (!inferno->state.handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        exit(1);
    }
    // CLEAR ERROR
    dlerror();    
    // FETCH API
    inferno_main_extern = dlsym(inferno->state.handle, inferno->config.main_symbol);
    if ((inferno->state.error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", inferno->state.error);
        exit(1);
    }
    inferno_get_state_extern = dlsym(inferno->state.handle, inferno->config.get_state_symbol);
    if ((inferno->state.error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", inferno->state.error);
        exit(1);
    }
    inferno_set_state_extern = dlsym(inferno->state.handle, inferno->config.set_state_symbol);
    if ((inferno->state.error = dlerror()) != NULL)  {
        fprintf(stderr, "Error: %s\n", inferno->state.error);
        exit(1);
    }
    inferno->state.has_started = 1;
}
void inferno_destroy(inferno_t *inferno)
{
    if(inferno) dlclose(inferno->state.handle);
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

    if (!CreateProcess(
            NULL,       
            inferno->state.cmd_str,    
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
    if(inferno->state.handle) FreeLibrary((HMODULE)inferno->state.handle);
    // COPY & RENAME TMP
    if(inferno_copy(inferno_shared_lib_tmp_path, inferno_shared_lib_path) == -1)
    {
        fprintf(stderr, "RELOAD ERROR: Fail to copy new shared.");
        exit(1);
    }
    // LOAD DYNLIB
    inferno->state.handle = LoadLibraryA(inferno_shared_lib_tmp_path);
    if (!inferno->state.handle) {
        fprintf(stderr, "Failed to load DLL. Error code: %lu\n", GetLastError());
        exit(1);
    }
    // FETCH API
    inferno_main_extern = (inferno_main_func)GetProcAddress((HMODULE)inferno->state.handle, inferno->config.main_symbol);
    if (!inferno_main_extern)  {
        fprintf(stderr, "Failed to load main_symbol. Error code: %lu\n", GetLastError());
        exit(1);
    }
    inferno_get_state_extern = (inferno_get_state_func)GetProcAddress((HMODULE)inferno->state.handle, inferno->config.get_state_symbol);
    if (!inferno_get_state_extern)  {
        fprintf(stderr, "Failed to load get_state_symbol. Error code: %lu\n", GetLastError());
        exit(1);
    }
    inferno_set_state_extern = (inferno_set_state_func)GetProcAddress((HMODULE)inferno->state.handle, inferno->config.set_state_symbol);
    if (!inferno_set_state_extern)  {
        fprintf(stderr, "Failed to load set_state_symbol. Error code: %lu\n", GetLastError());
        exit(1);
    }
    inferno->state.has_started = 1;
}

void inferno_destroy(inferno_t *inferno)
{
    if(inferno) FreeLibrary((HMODULE)inferno->state.handle);
}
#endif

#endif














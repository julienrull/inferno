#ifndef INFERNO_INTERFACE_H
#define INFERNO_INTERFACE_H

#ifdef _WIN32
  #define __inferno_export __declspec(dllexport)
#else
  #define __inferno_export __attribute__((visibility("default")))
#endif

typedef struct inferno_interface_t{
    void (*main)();
    void (*get_state)(void*);
    void (*set_state)(void*);
}inferno_interface_t;

#endif

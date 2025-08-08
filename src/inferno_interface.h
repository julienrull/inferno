#ifndef INFERNO_INTERFACE_H
#define INFERNO_INTERFACE_H

#ifndef __declspec
#define dllexport
#define __declspec(DLL_EXPORT)
#endif

#define __inferno_export __declspec(dllexport)

typedef struct inferno_interface_t{
    void (*main)();
    void (*get_state)(void*);
    void (*set_state)(void*);
}inferno_interface_t;

#endif

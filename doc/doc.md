# Documentation

## Introduction

### What is Inferno?

Inferno is a lightweight library designed to simplify hot-reload integration in your project.
While it requires a minimal setup, the configuration is straightforward and generic.

The library consists of a single header file: `inferno.h`. It follows the **Single Header Library** format, a C library style popularized by Sean Barrett through his [stb](https://github.com/nothings/stb) libraries. Although Inferno does not yet fully adhere to all [Sean Barrett's guidelines](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt), it strives to follow them as closely as possible.

### Why a Single Header Library?

The Single Header Library approach enables faster and simpler project integration by combining both definitions and implementation into a single file, eliminating the need for separate compilation and linking steps. One header file to rule them all!

## Basic Usage

Hot-reload is particularly useful for applications running in a loop.
Inferno packages the code that needs to be reloaded into a shared library, dynamically loads it, and executes it. By default, Inferno expects the file `inferno.c` to contain the hot-reloaded code. It monitors this file for changes on each iteration and reloads it if modifications are detected.

Your `inferno.c` file must at least implement the function:

```c
void inferno_main();
```

This function is executed by Inferno to run your program’s core logic.

To preserve the application state between reloads, you also need to implement:

```c
void inferno_get_state(void*);
void inferno_set_state(void*);
```

### Example: `inferno.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include "inferno.h"

typedef struct app_state_t {
    int counter;
} app_state_t;

app_state_t app_state = {0}; 

__inferno_export void inferno_main() {
    printf("This is the logic of my app: counter -> %d!\n", app_state.counter);
    app_state.counter += 1;
}

__inferno_export void inferno_get_state(void *out) {
    *((app_state_t*)out) = app_state;
}

__inferno_export void inferno_set_state(void *in) {
    app_state = *((app_state_t*)in);
}
```

### Example: `main.c`

```c
#include <stdio.h>
#include <stdlib.h>

#define INFERNO_IMPLEMENTATION
#include "inferno.h"

int main(void) {
    inferno_t inferno = {0};
    inferno_init(&inferno);
    while (1) {
        inferno_update(&inferno);
    }
    return 0;
}
```

The `inferno_update` function monitors `inferno.c` for changes. When changes are detected, it unloads the current shared library, recompiles the code, and loads the updated versions of:

* `void inferno_main();`
* `void inferno_get_state(void*);`
* `void inferno_set_state(void*);`

## Configuration

Each platform uses a default compiler:

* **Linux** → `gcc`
* **Windows** → `cl`
* **MacOS** → `clang`

Inferno is configurable to adapt to your specific needs. Below are some configuration examples:

```c
inferno_t inferno = {0};

// Change the default shared library compiler
inferno.config.cc = "clang";

// Add extra compiler flags
inferno.config.flags[0] = "-Wall";
inferno.config.flags[1] = "-Werror";
inferno.config.flags[2] = (char*)NULL;

// The first source file in the list acts as the shared library entry point, overriding the default inferno.c.
inferno.config.srcs[0] = "./src/hotreload_code_entry_point.c";
inferno.config.srcs[1] = "./src/file2.c";
inferno.config.srcs[2] = "./src/sub_folder/file3.c";
inferno.config.srcs[3] = (char*)NULL;

// Specify additional source files to include during shared library compilation,
// particularly useful for projects using a unity-build approach.

inferno.config.compilation_include_all_srcs = 0; // enable 
inferno.config.compilation_include_all_srcs = -1; // disable

// Add external libraries for linking
inferno.config.libs[0] = "./lib/libraylib.dylib";
inferno.config.libs[1] = (char*)NULL;

// Override symbol names
inferno.config.inferno_main = "my_main";
inferno.config.inferno_get_state = "my_get_state";
inferno.config.inferno_set_state = "my_set_state";

// Change the location of the hot-reloadable code
inferno.config.location = "./src/my_hotreload_code.c";

inferno_init(&inferno);
```

## Platform-Specific Considerations

If you are using Inferno without external libraries, no additional setup is required.
However, when working with external libraries like raylib, you must ensure that the shared library can resolve external symbols.

* **Windows:** To use `raylib.dll` symbols in the Inferno shared library, you need to link against `raylib.lib`.
* **MacOS:** Unlike Linux, you must explicitly add `raylib.dylib` to the shared library compiler command to resolve symbols.

### Workarounds

#### Windows

```c
inferno_t inferno = {0};
inferno.config.libs[0] = ".\\lib\\raylib.lib";
inferno.config.libs[1] = (char*)NULL;
inferno_init(&inferno);
```

#### MacOS

```c
inferno_t inferno = {0};
inferno.config.libs[0] = "./lib/libraylib.dylib";
inferno.config.libs[1] = (char*)NULL;
inferno_init(&inferno);
```

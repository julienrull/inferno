# Inferno

## Introduction

**Inferno** is a lightweight C library for hot-reloading.
It’s a single-header library (`inferno.h` + `inferno_config.h`) inspired by Sean Barrett’s [stb](https://github.com/nothings/stb) style.

### Why single-header?

One file contains both the interface and implementation.
Drop it into your project, `#define INFERNO_IMPL` in one source file, and you’re done.
No separate build steps.

---

## Basic usage

Hot-reload is useful for programs running in a loop.
Inferno compiles your code into a shared library, loads it, and runs it.
It watches `inferno.c` for changes and reloads on the fly.

### Interface

Your `inferno.c` must provide:

```c
typedef struct inferno_interface_t {
    void (*main)();          /* Run your program’s logic. */
    void (*get_state)(void*);/* Save app state between reloads. */
    void (*set_state)(void*);/* Restore state after reload.    */
} inferno_interface_t;
```

---

### Example: `inferno.c`

```c
#include <stdio.h>
#include "inferno.h"

typedef struct {
    int counter;
} app_state_t;

static app_state_t app_state = {0};

void inferno_main() {
    printf("Counter: %d\n", app_state.counter++);
}

void inferno_get_state(void *out) { *(app_state_t*)out = app_state; }
void inferno_set_state(void *in)  { app_state = *(app_state_t*)in;  }

__inferno_export inferno_interface_t inferno_interface = {
    .main = inferno_main,
    .get_state = inferno_get_state,
    .set_state = inferno_set_state
};
```

---

### Example: `main.c`

```c
#define INFERNO_IMPL
#include "inferno.h"

int main(void) {
    inferno_t inferno = {0};
    while (1) inferno_update(&inferno);
}
```

`inferno_update()` checks for changes, recompiles, unloads the old library, and loads the new one.

---

## Configuration

Default config (`inferno_config.h`):

```c
#ifndef INFERNO_CONFIG_H
#define INFERNO_CONFIG_H

#define INFERNO_STORAGE_SIZE 1024 /* Bytes for saved state. */

static const char *watched[] = { "inferno.c", NULL };
static const char output[] = "./inferno.so";

static const char *shared_build_cmd[] = {
    "gcc", "-g", "-fPIC", "-shared", "-std=c99", "-Wall", "-Werror",
    "inferno.c", "-o", output, NULL
};

#endif
```

Override by creating your own `inferno_config.h`:

```c
#include "my_inferno_config.h"
#define INFERNO_IMPL
#include "inferno.h"
```

---

## Examples

See [examples](https://github.com/julienrull/inferno/tree/master/examples) for

* External shared libraries
* Cross-platform builds

#include <stdlib.h>
#if defined(__linux__)
    #define PLATFORM_LINUX
#elif defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__) || defined(__MACH__)
    #define PLATFORM_MACOS
#endif

#if defined(PLATFORM_LINUX)
    #include "inferno_config_linux.h"
#elif defined(PLATFORM_WINDOWS)
    #include "inferno_config_windows.h"
#elif defined(PLATFORM_MACOS)
    #include "inferno_config_macos.h"
#endif

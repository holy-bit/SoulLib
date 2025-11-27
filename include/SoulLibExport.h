#pragma once

#if defined(SOULLIB_STATIC)
    #define SOULLIB_API
#elif defined(_WIN32) || defined(__CYGWIN__)
    #ifdef SOULLIB_EXPORTS
        #define SOULLIB_API __declspec(dllexport)
    #else
        #define SOULLIB_API __declspec(dllimport)
    #endif
#else
    #define SOULLIB_API __attribute__((visibility("default")))
#endif

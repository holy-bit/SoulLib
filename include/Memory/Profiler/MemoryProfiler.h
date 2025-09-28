#pragma once
#include <cstddef>
#include <type_traits>

namespace Memory::Profiler {
    class MemoryProfiler {
    public:
        // Measure size of object (default: sizeof(T))
        template<typename T>
        static size_t measureObject(const T& obj) {
            return sizeof(T);
        }

        // Measure size of container: sizeof(container) + elements
        template<template<typename...> class C, typename T>
        static size_t measureContainer(const C<T>& container) {
            return sizeof(container) + container.size() * sizeof(T);
        }

        // Measure raw allocated size via MemoryManager
        static size_t measureRaw(void* ptr);
    };
}
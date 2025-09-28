#pragma once
#include <cstddef>
#include "Memory/Core/MemoryStatistics.h"
#include "Memory/Core/MemoryTag.h"

namespace Memory::Core {
    class IMemoryManager {
    public:
        virtual ~IMemoryManager() = default;
        virtual void registerAllocation(void* ptr, size_t size, MemoryTag tag) = 0;
        virtual void unregisterAllocation(void* ptr) = 0;
        virtual void reportLeaks() = 0;
        virtual size_t getTotalAllocated() = 0;
        virtual size_t getAllocationCount() = 0;
        virtual size_t getAllocationSize(void* ptr) = 0;
        virtual size_t getAllocatedByTag(MemoryTag tag) = 0;
        virtual MemoryStatistics snapshot() = 0;
    };
}
#pragma once

#include <atomic>
#include "Memory/Core/IMemoryManager.h"

namespace Memory::Core {

class MemoryRegistry {
public:
    static IMemoryManager& Get();
    static void Set(IMemoryManager& manager);
    static void Reset();

private:
    static std::atomic<IMemoryManager*>& storage();
    static IMemoryManager& defaultManager();
};

} // namespace Memory::Core

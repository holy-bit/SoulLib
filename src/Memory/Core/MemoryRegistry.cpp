#include "Memory/Core/MemoryRegistry.h"
#include "Memory/Core/MemoryManager.h"

namespace Memory::Core {
namespace {
    std::atomic<IMemoryManager*> g_registry{nullptr};
}

std::atomic<IMemoryManager*>& MemoryRegistry::storage() {
    return g_registry;
}

IMemoryManager& MemoryRegistry::defaultManager() {
    static MemoryManager manager;
    return manager;
}

IMemoryManager& MemoryRegistry::Get() {
    IMemoryManager* current = storage().load(std::memory_order_acquire);
    if (current) {
        return *current;
    }

    IMemoryManager& fallback = defaultManager();
    IMemoryManager* expected = nullptr;
    if (storage().compare_exchange_strong(expected, &fallback, std::memory_order_acq_rel)) {
        return fallback;
    }
    return *expected;
}

void MemoryRegistry::Set(IMemoryManager& manager) {
    storage().store(&manager, std::memory_order_release);
}

void MemoryRegistry::Reset() {
    storage().store(nullptr, std::memory_order_release);
}

} // namespace Memory::Core

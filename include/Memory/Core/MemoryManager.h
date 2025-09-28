#pragma once
#include "IMemoryManager.h"
#include "Memory/Core/MemoryStatistics.h"
#include <unordered_map>
#include <mutex>

namespace Memory::Core {
    struct AllocationInfo {
        size_t size;
        MemoryTag tag;
    };

    class MemoryManager : public IMemoryManager {
    public:
        MemoryManager() = default;
        ~MemoryManager() override = default;
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;
        MemoryManager(MemoryManager&&) = delete;
        MemoryManager& operator=(MemoryManager&&) = delete;

        void setDebugMode(bool enabled);
        bool getDebugMode() const noexcept { return debugMode_; }
        void clear();

        // IMemoryManager overrides
        void registerAllocation(void* ptr, size_t size, MemoryTag tag) override;
        void unregisterAllocation(void* ptr) override;
        void reportLeaks() override;
        size_t getTotalAllocated() override;
        size_t getAllocationCount() override;
        size_t getAllocationSize(void* ptr) override;
        size_t getAllocatedByTag(MemoryTag tag) override;
    MemoryStatistics snapshot() override;

    private:
        bool debugMode_{false};
        std::unordered_map<void*, AllocationInfo> allocations_;
        std::mutex mutex_;
    };
}
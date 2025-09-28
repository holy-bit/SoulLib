module;

#include <cstddef>

#include "Memory/Core/MemoryStatistics.h"

export module Soul.Memory.Statistics;

export namespace Memory::Modules {

using Memory::Core::MemoryStatistics;

inline MemoryStatistics MakeEmptyStatistics() {
    return {};
}

inline MemoryStatistics MakeStatistics(std::size_t bytes, std::size_t allocations) {
    MemoryStatistics stats;
    stats.totalBytes = bytes;
    stats.allocationCount = allocations;
    return stats;
}

} // namespace Memory::Modules

#pragma once

#include <cstddef>
#include <vector>

#include "Memory/Core/MemoryTag.h"

namespace Memory::Core {

struct MemoryTagStats {
    MemoryTag tag;
    std::size_t bytes{0};
    std::size_t allocations{0};
};

struct MemoryStatistics {
    std::size_t totalBytes{0};
    std::size_t allocationCount{0};
    std::vector<MemoryTagStats> tags;
};

} // namespace Memory::Core

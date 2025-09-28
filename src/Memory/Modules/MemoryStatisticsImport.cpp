#include <cstddef>
#include "Memory/Core/MemoryStatistics.h"

import Soul.Memory.Statistics;

namespace Memory::Modules::Detail {

Memory::Core::MemoryStatistics CreateModuleStatistic(std::size_t bytes, std::size_t allocations) {
    return Memory::Modules::MakeStatistics(bytes, allocations);
}

} // namespace Memory::Modules::Detail

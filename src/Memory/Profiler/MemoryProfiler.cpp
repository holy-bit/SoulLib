#include "Memory/Profiler/MemoryProfiler.h"
#include "Memory/Core/MemoryRegistry.h"

namespace Memory::Profiler {

size_t MemoryProfiler::measureRaw(void* ptr) {
    return Memory::Core::MemoryRegistry::Get().getAllocationSize(ptr);
}

} // namespace Memory::Profiler

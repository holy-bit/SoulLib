#include <benchmark/benchmark.h>
#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/MemoryRegistry.h"

int main(int argc, char** argv) {
    Memory::Core::MemoryManager manager;
    Memory::Core::MemoryRegistry::Set(manager);

    benchmark::Initialize(&argc, argv);
    const int result = benchmark::RunSpecifiedBenchmarks();

    Memory::Core::MemoryRegistry::Reset();
    return result;
}
